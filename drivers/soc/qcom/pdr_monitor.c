// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/rbtree.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/soc/qcom/smem.h>
#include <linux/soc/qcom/pdr.h>
#include <linux/firmware.h>
#include <linux/mm.h>

#define NAME_LEN 64
#define LINE_LEN 128
#define UUID_LEN 36

struct pdr_crash_work {
	struct work_struct work;
	struct pdr_monitor_service *svc;
};

struct pdr_monitor_service {
	struct list_head node;
	struct pdr_service *service;
	struct pdr_monitor *pdr_mon;
	char service_name[NAME_LEN];
	char service_path[NAME_LEN];
	unsigned int smem_id;
	unsigned int smem_host_id;
	struct pdr_crash_work crash_work;
};

struct symbol_entry {
	struct rb_node node;
	u32 addr;
	const char *name;
};

struct pdr_monitor {
	struct device *dev;
	struct pdr_handle *pdr;
	struct list_head services;
	struct workqueue_struct *crash_wq;
	struct mutex services_lock;
};

static void free_symbol_tree(struct rb_root *root)
{
	struct rb_node *node;
	struct symbol_entry *entry;

	while ((node = rb_first(root))) {
		entry = rb_entry(node, struct symbol_entry, node);
		rb_erase(node, root);
		kfree(entry->name);
		kfree(entry);
	}
}

static char *read_symbol_file(struct device *dev, const char *path, size_t *size_out)
{
	char *buf;
	int ret;
	const struct firmware *symtab = NULL;

	ret = request_firmware(&symtab, path, dev);
	if (ret < 0) {
		dev_err(dev, "request_firmware failed: %s (%d)\n", path, ret);
		return ERR_PTR(ret);
	}

	if (!symtab->data || symtab->size == 0) {
		dev_err(dev, "Firmware is empty or invalid\n");
		release_firmware(symtab);
		return ERR_PTR(-EINVAL);
	}

	buf = kvzalloc(symtab->size + 1, GFP_KERNEL);
	if (!buf) {
		release_firmware(symtab);
		return ERR_PTR(-ENOMEM);
	}

	memcpy(buf, symtab->data, symtab->size);
	*size_out = symtab->size;
	release_firmware(symtab);
	return buf;
}

static int parse_symbols(struct device *dev, struct rb_root *root,
			 const char *buf, size_t size)
{
	const char *cur = buf;
	const char *end = buf + size;
	const char *line_start;
	char line[LINE_LEN];
	char name[NAME_LEN];
	uint32_t addr;
	int len;
	struct symbol_entry *entry, *this;
	struct rb_node **new;
	struct rb_node *parent;

	while (cur < end) {
		line_start = cur;

		while (cur < end && *cur != '\n')
			cur++;

		len = min((int)(cur - line_start), (int)(sizeof(line) - 1));
		memcpy(line, line_start, len);
		line[len] = '\0';

		if (cur < end)
			cur++;

		if (sscanf(line, "%x %63s", &addr, name) == 2) {
			entry = kzalloc(sizeof(*entry), GFP_KERNEL);
			if (!entry)
				return -ENOMEM;

			entry->addr = addr;
			entry->name = kstrdup(name, GFP_KERNEL);
			if (!entry->name) {
				kfree(entry);
				return -ENOMEM;
			}

			new = &root->rb_node;
			parent = NULL;
			while (*new) {
				this = rb_entry(*new, struct symbol_entry, node);
				parent = *new;
				if (addr < this->addr)
					new = &(*new)->rb_left;
				else
					new = &(*new)->rb_right;
			}
			rb_link_node(&entry->node, parent, new);
			rb_insert_color(&entry->node, root);
		}
	}
	return 0;
}

static const char *match_function(struct rb_root *root, u32 addr)
{
	struct rb_node *node = root->rb_node;
	const char *closest = "none";

	while (node) {
		struct symbol_entry *entry = rb_entry(node, struct symbol_entry, node);

		if (entry->addr == addr) {
			return entry->name;
		} else if (entry->addr < addr) {
			closest = entry->name;
			node = node->rb_right;
		} else {
			node = node->rb_left;
		}
	}
	return closest;
}

static void symbol_loader_work(struct work_struct *work)
{
	struct pdr_crash_work *crash_work = container_of(work, struct pdr_crash_work, work);
	struct pdr_monitor_service *svc = crash_work->svc;
	struct pdr_monitor *pdr_mon = svc->pdr_mon;
	struct rb_root symbol_tree = RB_ROOT;
	size_t len;
	char *msg, *cur, *end, *token, *buf, *callstack_entry, *addr_start;
	const char *func;
	char uuid[UUID_LEN + 1];
	char path[LINE_LEN];
	size_t size;
	uint32_t addr;
	int ret;

	dev_err(pdr_mon->dev, "PDR CRASH: %s\n", svc->service_path);

	msg = qcom_smem_get(svc->smem_host_id, svc->smem_id, &len);
	if (IS_ERR(msg)) {
		dev_err(pdr_mon->dev, "Failed to get crash data from SMEM: %ld\n",
			PTR_ERR(msg));
		return;
	}

	if (len < (UUID_LEN + 1)) {
		dev_err(pdr_mon->dev, "Not enough data for UUID: %zu\n", len);
		return;
	}

	memcpy(uuid, msg + (len - (UUID_LEN + 1)), UUID_LEN);
	uuid[UUID_LEN] = '\0';
	dev_info(pdr_mon->dev, "UUID: %s\n", uuid);

	snprintf(path, sizeof(path), "%s_symtab.txt", uuid);
	buf = read_symbol_file(pdr_mon->dev, path, &size);
	if (IS_ERR(buf)) {
		dev_err(pdr_mon->dev, "Failed to load symbol file: %s\n", path);
		return;
	}

	ret = parse_symbols(pdr_mon->dev, &symbol_tree, buf, size);
	kvfree(buf);
	if (ret) {
		dev_err(pdr_mon->dev, "Failed to parse symbols: %d\n", ret);
		goto cleanup_tree;
	}

	cur = msg;
	end = msg + len;
	token = strsep(&cur, "|");
	dev_err(pdr_mon->dev, "Stack Trace:\n");

	while (cur && cur < end) {
		callstack_entry = strsep(&cur, "|");
		if (!callstack_entry)
			break;
		addr_start = strpbrk(callstack_entry, ")");
		if (!addr_start)
			break;
		token = addr_start + 1;
		if (token[0] == '\0')
			continue;

		if (kstrtou32(token, 16, &addr) == 0) {
			func = match_function(&symbol_tree, addr);
			dev_err(pdr_mon->dev, "%s (0x%08x)\n", func, addr);
		}
	}

cleanup_tree:
	free_symbol_tree(&symbol_tree);
}

static void pdr_monitor_callback(int state, char *service_path, void *priv)
{
	struct pdr_monitor *pdr_mon = priv;
	struct pdr_monitor_service *svc;

	dev_info(pdr_mon->dev, "PDR notification: %s state=%d\n", service_path, state);

	if (state != SERVREG_SERVICE_STATE_DOWN)
		return;

	mutex_lock(&pdr_mon->services_lock);
	list_for_each_entry(svc, &pdr_mon->services, node) {
		if (strcmp(svc->service_path, service_path) == 0) {
			if (!queue_work(pdr_mon->crash_wq, &svc->crash_work.work))
				dev_warn(pdr_mon->dev,
					"Crash for %s while work already queued\n",
					service_path);
			break;
		}
	}
	mutex_unlock(&pdr_mon->services_lock);
}

static int pdr_monitor_parse_dt(struct pdr_monitor *pdr_mon)
{
	struct device_node *node = pdr_mon->dev->of_node;
	struct device_node *child;
	struct pdr_monitor_service *svc;
	const char *name_ptr;
	const char *path_ptr;
	int ret;

	for_each_available_child_of_node(node, child) {
		svc = devm_kzalloc(pdr_mon->dev, sizeof(*svc), GFP_KERNEL);
		if (!svc) {
			of_node_put(child);
			return -ENOMEM;
		}

		ret = of_property_read_string(child, "service-name", &name_ptr);
		if (ret) {
			dev_err(pdr_mon->dev, "Missing service-name in DT\n");
			of_node_put(child);
			return ret;
		}
		strscpy(svc->service_name, name_ptr, sizeof(svc->service_name));

		ret = of_property_read_string(child, "service-path", &path_ptr);
		if (ret) {
			dev_err(pdr_mon->dev, "Missing service-path in DT\n");
			of_node_put(child);
			return ret;
		}
		strscpy(svc->service_path, path_ptr, sizeof(svc->service_path));

		of_property_read_u32(child, "qcom,smem-id", &svc->smem_id);
		of_property_read_u32(child, "qcom,smem-host", &svc->smem_host_id);

		svc->pdr_mon = pdr_mon;

		INIT_WORK(&svc->crash_work.work, symbol_loader_work);
		svc->crash_work.svc = svc;

		list_add_tail(&svc->node, &pdr_mon->services);

		dev_info(pdr_mon->dev, "Registered PD: %s (smem_id=%u, smem_host=%u)\n",
			 svc->service_path, svc->smem_id, svc->smem_host_id);
	}

	return 0;
}

static int pdr_monitor_probe(struct platform_device *pdev)
{
	struct pdr_monitor *pdr_mon;
	struct pdr_monitor_service *svc;
	int ret;

	pdr_mon = devm_kzalloc(&pdev->dev, sizeof(*pdr_mon), GFP_KERNEL);
	if (!pdr_mon)
		return -ENOMEM;

	pdr_mon->dev = &pdev->dev;
	INIT_LIST_HEAD(&pdr_mon->services);
	mutex_init(&pdr_mon->services_lock);

	platform_set_drvdata(pdev, pdr_mon);

	pdr_mon->crash_wq = alloc_ordered_workqueue("pdr_crash_wq", WQ_FREEZABLE);
	if (!pdr_mon->crash_wq) {
		dev_err(&pdev->dev, "Failed to create workqueue\n");
		return -ENOMEM;
	}

	ret = pdr_monitor_parse_dt(pdr_mon);
	if (ret) {
		dev_err(&pdev->dev, "Failed to parse device tree: %d\n", ret);
		goto err_destroy_wq;
	}

	pdr_mon->pdr = pdr_handle_alloc(pdr_monitor_callback, pdr_mon);
	if (IS_ERR(pdr_mon->pdr)) {
		dev_err(&pdev->dev, "Failed to allocate PDR handle: %ld\n",
			PTR_ERR(pdr_mon->pdr));
		ret = PTR_ERR(pdr_mon->pdr);
		goto err_destroy_wq;
	}

	list_for_each_entry(svc, &pdr_mon->services, node) {
		svc->service = pdr_add_lookup(pdr_mon->pdr, svc->service_name,
					      svc->service_path);
		if (IS_ERR(svc->service) && PTR_ERR(svc->service) != -EALREADY) {
			dev_err(&pdev->dev, "Failed to add lookup for %s: %ld\n",
				svc->service_path, PTR_ERR(svc->service));
		}
	}

	dev_info(&pdev->dev, "PDR monitor initialized\n");
	return 0;

err_destroy_wq:
	destroy_workqueue(pdr_mon->crash_wq);
	return ret;
}

static int pdr_monitor_remove(struct platform_device *pdev)
{
	struct pdr_monitor *pdr_mon = platform_get_drvdata(pdev);
	struct pdr_monitor_service *svc;

	/* Release PDR handle first to stop new callbacks */
	if (pdr_mon->pdr)
		pdr_handle_release(pdr_mon->pdr);

	/* Cancel any pending or in-flight work */
	list_for_each_entry(svc, &pdr_mon->services, node)
		cancel_work_sync(&svc->crash_work.work);

	/* Destroy workqueue */
	destroy_workqueue(pdr_mon->crash_wq);

	/* Cleanup mutex */
	mutex_destroy(&pdr_mon->services_lock);

	return 0;
}

static const struct of_device_id pdr_monitor_of_match[] = {
	{ .compatible = "qcom,pdr-monitor"},
	{}
};
MODULE_DEVICE_TABLE(of, pdr_monitor_of_match);

static struct platform_driver pdr_monitor_driver = {
	.probe = pdr_monitor_probe,
	.remove = pdr_monitor_remove,
	.driver = {
		.name = "pdr_monitor",
		.of_match_table = pdr_monitor_of_match,
	},
};

module_platform_driver(pdr_monitor_driver);

MODULE_DESCRIPTION("Qualcomm Technologies, Inc. PDR Monitor Driver");
MODULE_LICENSE("GPL");