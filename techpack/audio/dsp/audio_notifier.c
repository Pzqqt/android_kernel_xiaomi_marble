// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2016-2017, 2020-2021 The Linux Foundation. All rights reserved.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <linux/remoteproc.h>
#include <linux/remoteproc/qcom_rproc.h>
#include <dsp/audio_notifier.h>
#include "audio_ssr.h"
#include "audio_pdr.h"



/* Audio states internal to notifier. Client */
/* used states defined in audio_notifier.h */
/* for AUDIO_NOTIFIER_SERVICE_DOWN & UP */
#define NO_SERVICE -2
#define UNINIT_SERVICE -1

static struct platform_device *adsp_private;

struct adsp_notify_private {
	struct rproc *rproc_h;
	bool notifier_probe_complete;
};

/*
 * Used for each client registered with audio notifier
 */
struct client_data {
	struct list_head        list;
	/* Notifier block given by client */
	struct notifier_block   *nb;
	char                    client_name[20];
	int                     service;
	int                     domain;
};

/*
 * Used for each service and domain combination
 * Tracks information specific to the underlying
 * service.
 */
struct service_info {
	const char                      name[20];
	int                             domain_id;
	int                             state;
	void                            *handle;
	/* Hook registered to service */
	union {
		void (*cb)(int, char *, void *);
		struct notifier_block *nb;
	} hook;
	/* Used to determine when to register and deregister service */
	int                             num_of_clients;
	/* List of all clients registered to the service and domain */
	struct srcu_notifier_head       client_nb_list;
};

static int audio_notifier_ssr_adsp_cb(struct notifier_block *this,
				     unsigned long opcode, void *data);
static int audio_notifier_ssr_modem_cb(struct notifier_block *this,
				     unsigned long opcode, void *data);
static void audio_notifier_pdr_adsp_cb(int status, char *service_name, void *priv);

static struct notifier_block notifier_ssr_adsp_nb = {
	.notifier_call  = audio_notifier_ssr_adsp_cb,
	.priority = 0,
};

static struct notifier_block notifier_ssr_modem_nb = {
	.notifier_call  = audio_notifier_ssr_modem_cb,
	.priority = 0,
};

static struct service_info service_data[AUDIO_NOTIFIER_MAX_SERVICES]
				       [AUDIO_NOTIFIER_MAX_DOMAINS] = {

	{{
		.name = "SSR_ADSP",
		.domain_id = AUDIO_SSR_DOMAIN_ADSP,
		.state = AUDIO_NOTIFIER_SERVICE_DOWN,
		.hook.nb = &notifier_ssr_adsp_nb
	},
	{
		.name = "SSR_MODEM",
		.domain_id = AUDIO_SSR_DOMAIN_MODEM,
		.state = AUDIO_NOTIFIER_SERVICE_DOWN,
		.hook.nb = &notifier_ssr_modem_nb
	} },

	{{
		.name = "PDR_ADSP",
		.domain_id = AUDIO_PDR_DOMAIN_ADSP,
		.state = UNINIT_SERVICE,
		.hook.cb = &audio_notifier_pdr_adsp_cb
	},
	{	/* PDR MODEM service not enabled */
		.name = "INVALID",
		.state = NO_SERVICE,
		.hook.nb = NULL
	} }
};

/* Master list of all audio notifier clients */
LIST_HEAD(client_list);
struct mutex       notifier_mutex;

static int audio_notifier_get_default_service(int domain)
{
	int service = NO_SERVICE;

	/* initial service to connect per domain */
	switch (domain) {
	case AUDIO_NOTIFIER_ADSP_DOMAIN:
		service = AUDIO_NOTIFIER_PDR_SERVICE;
		break;
	case AUDIO_NOTIFIER_MODEM_DOMAIN:
		service = AUDIO_NOTIFIER_SSR_SERVICE;
		break;
	}

	return service;
}

#ifdef CONFIG_MSM_QDSP6_PDR
static void audio_notifier_init_service(int service)
{
	int i;

	for (i = 0; i < AUDIO_NOTIFIER_MAX_DOMAINS; i++) {
		if (service_data[service][i].state == UNINIT_SERVICE)
			service_data[service][i].state =
				AUDIO_NOTIFIER_SERVICE_DOWN;
	}
}
#else
static void audio_notifier_init_service(int service)
{
	int i;

	for (i = 0; i < AUDIO_NOTIFIER_MAX_DOMAINS; i++)
		service_data[service][i].state = NO_SERVICE;
}
#endif

static bool audio_notifier_is_service_enabled(int service)
{
	int i;

	for (i = 0; i < AUDIO_NOTIFIER_MAX_DOMAINS; i++)
		if (service_data[service][i].state != NO_SERVICE)
			return true;
	return false;
}

static int audio_notifier_reg_service(int service, int domain)
{
	void *handle;
	int ret = 0;
	int curr_state = AUDIO_NOTIFIER_SERVICE_DOWN;
	struct platform_device *pdev = adsp_private;
	struct adsp_notify_private *priv = NULL;
	struct rproc *rproc;

	priv = platform_get_drvdata(pdev);
	if (!priv) {
		dev_err(&pdev->dev," %s: Private data get failed\n", __func__);
		return ret;;
	}

	rproc = priv->rproc_h;

	switch (service) {
	case AUDIO_NOTIFIER_SSR_SERVICE:
		handle = audio_ssr_register(rproc->name,
			service_data[service][domain].hook.nb);
		break;
	case AUDIO_NOTIFIER_PDR_SERVICE:
		handle = audio_pdr_service_register(
			service_data[service][domain].domain_id,
			service_data[service][domain].hook.cb);

		curr_state = AUDIO_NOTIFIER_SERVICE_DOWN;
		break;
	default:
		pr_err("%s: Invalid service %d\n",
			__func__, service);
		ret = -EINVAL;
		goto done;
	}
	if (IS_ERR_OR_NULL(handle)) {
		pr_err("%s: handle is incorrect for service %s\n",
			__func__, service_data[service][domain].name);
		ret = -EINVAL;
		goto done;
	}
	service_data[service][domain].state = curr_state;
	service_data[service][domain].handle = handle;

	pr_info("%s: service %s is in use\n",
		__func__, service_data[service][domain].name);
	pr_debug("%s: service %s has current state %d, handle 0x%pK\n",
		__func__, service_data[service][domain].name,
		service_data[service][domain].state,
		service_data[service][domain].handle);
done:
	return ret;
}

static int audio_notifier_dereg_service(int service, int domain)
{
	int ret;

	switch (service) {
	case AUDIO_NOTIFIER_SSR_SERVICE:
		ret = audio_ssr_deregister(
			service_data[service][domain].handle,
			service_data[service][domain].hook.nb);
		break;
	case AUDIO_NOTIFIER_PDR_SERVICE:
		ret = audio_pdr_service_deregister(
			service_data[service][domain].domain_id);
		break;
	default:
		pr_err("%s: Invalid service %d\n",
			__func__, service);
		ret = -EINVAL;
		goto done;
	}
	if (ret < 0) {
		pr_err("%s: deregister failed for service %s, ret %d\n",
			__func__, service_data[service][domain].name, ret);
		goto done;
	}

	pr_debug("%s: service %s with handle 0x%pK deregistered\n",
		__func__, service_data[service][domain].name,
		service_data[service][domain].handle);

	service_data[service][domain].state = AUDIO_NOTIFIER_SERVICE_DOWN;
	service_data[service][domain].handle = NULL;
done:
	return ret;
}

static int audio_notifier_reg_client_service(struct client_data *client_data,
					    int service)
{
	int ret = 0;
	int domain = client_data->domain;
	struct audio_notifier_cb_data data;

	switch (service) {
	case AUDIO_NOTIFIER_SSR_SERVICE:
	case AUDIO_NOTIFIER_PDR_SERVICE:
		if (service_data[service][domain].num_of_clients == 0)
			ret = audio_notifier_reg_service(service, domain);
		break;
	default:
		pr_err("%s: Invalid service for client %s, service %d, domain %d\n",
			__func__, client_data->client_name, service, domain);
		ret = -EINVAL;
		goto done;
	}

	if (ret < 0) {
		pr_err("%s: service registration failed on service %s for client %s\n",
			__func__, service_data[service][domain].name,
			client_data->client_name);
		goto done;
	}

	client_data->service = service;
	srcu_notifier_chain_register(
		&service_data[service][domain].client_nb_list,
		client_data->nb);
	service_data[service][domain].num_of_clients++;

	pr_debug("%s: registered client %s on service %s, current state 0x%x\n",
		__func__, client_data->client_name,
		service_data[service][domain].name,
		service_data[service][domain].state);

	/*
	 * PDR registration returns current state
	 * Force callback of client with current state for PDR
	 */
	if (client_data->service == AUDIO_NOTIFIER_PDR_SERVICE) {
		data.service = service;
		data.domain = domain;
		(void)client_data->nb->notifier_call(client_data->nb,
			service_data[service][domain].state, &data);
	}
done:
	return ret;
}

static int audio_notifier_reg_client(struct client_data *client_data)
{
	int ret = 0;
	int service;
	int domain = client_data->domain;

	service = audio_notifier_get_default_service(domain);
	if (service < 0) {
		pr_err("%s: service %d is incorrect\n", __func__, service);
		ret = -EINVAL;
		goto done;
	}

	/* Search through services to find a valid one to register client on. */
	for (; service >= 0; service--) {
		/* If a service is not initialized, wait for it to come up. */
		if (service_data[service][domain].state == UNINIT_SERVICE) {
			pr_err_ratelimited("%s: failed in client registration to PDR\n",
				 __func__);
			ret = -EINVAL;
			goto done;
		}
		/* Skip unsupported service and domain combinations. */
		if (service_data[service][domain].state < 0)
			continue;
		/* Only register clients who have not acquired a service. */
		if (client_data->service != NO_SERVICE)
			continue;

		/*
		 * Only register clients, who have not acquired a service, on
		 * the best available service for their domain. Uninitialized
		 * services will try to register all of their clients after
		 * they initialize correctly or will disable their service and
		 * register clients on the next best avaialable service.
		 */
		pr_debug("%s: register client %s on service %s",
				__func__, client_data->client_name,
				service_data[service][domain].name);

		ret = audio_notifier_reg_client_service(client_data, service);
		if (ret < 0)
			pr_err("%s: client %s failed to register on service %s",
				__func__, client_data->client_name,
				service_data[service][domain].name);
	}

done:
	return ret;
}

static int audio_notifier_dereg_client(struct client_data *client_data)
{
	int ret = 0;
	int service = client_data->service;
	int domain = client_data->domain;

	switch (client_data->service) {
	case AUDIO_NOTIFIER_SSR_SERVICE:
	case AUDIO_NOTIFIER_PDR_SERVICE:
		if (service_data[service][domain].num_of_clients == 1)
			ret = audio_notifier_dereg_service(service, domain);
		break;
	case NO_SERVICE:
		goto done;
	default:
		pr_err("%s: Invalid service for client %s, service %d\n",
			__func__, client_data->client_name,
			client_data->service);
		ret = -EINVAL;
		goto done;
	}

	if (ret < 0) {
		pr_err("%s: deregister failed for client %s on service %s, ret %d\n",
			__func__, client_data->client_name,
			service_data[service][domain].name, ret);
		goto done;
	}

	ret = srcu_notifier_chain_unregister(&service_data[service][domain].
					     client_nb_list, client_data->nb);
	if (ret < 0) {
		pr_err("%s: srcu_notifier_chain_unregister failed, ret %d\n",
			__func__, ret);
		goto done;
	}

	pr_debug("%s: deregistered client %s on service %s\n",
		__func__, client_data->client_name,
		service_data[service][domain].name);

	client_data->service = NO_SERVICE;
	if (service_data[service][domain].num_of_clients > 0)
		service_data[service][domain].num_of_clients--;
done:
	return ret;
}

static void audio_notifier_reg_all_clients(void)
{
	struct list_head *ptr, *next;
	struct client_data *client_data;
	int ret;

	list_for_each_safe(ptr, next, &client_list) {
		client_data = list_entry(ptr, struct client_data, list);

		ret = audio_notifier_reg_client(client_data);
		if (ret < 0)
			pr_err("%s: audio_notifier_reg_client failed for client %s, ret %d\n",
				__func__, client_data->client_name,
				ret);
	}
}

static int audio_notifier_convert_opcode(unsigned long opcode,
					unsigned long *notifier_opcode)
{
	int ret = 0;

	switch (opcode) {
	case QCOM_SSR_BEFORE_SHUTDOWN:
	case SERVREG_SERVICE_STATE_DOWN:
		*notifier_opcode = AUDIO_NOTIFIER_SERVICE_DOWN;
		break;
	case QCOM_SSR_AFTER_POWERUP:
	case SERVREG_SERVICE_STATE_UP:
		*notifier_opcode = AUDIO_NOTIFIER_SERVICE_UP;
		break;
	default:
		pr_debug("%s: Unused opcode 0x%lx\n", __func__, opcode);
		ret = -EINVAL;
	}

	return ret;
}

static int audio_notifier_service_cb(unsigned long opcode,
				    int service, int domain)
{
	int ret = 0;
	unsigned long notifier_opcode;
	struct audio_notifier_cb_data data;

	if (audio_notifier_convert_opcode(opcode, &notifier_opcode) < 0)
		goto done;

	data.service = service;
	data.domain = domain;

	pr_info("%s: service %s, opcode 0x%lx\n",
		__func__, service_data[service][domain].name, notifier_opcode);

	mutex_lock(&notifier_mutex);

	service_data[service][domain].state = notifier_opcode;
	ret = srcu_notifier_call_chain(&service_data[service][domain].
		client_nb_list, notifier_opcode, &data);
	if (ret < 0)
		pr_err("%s: srcu_notifier_call_chain returned %d, service %s, opcode 0x%lx\n",
			__func__, ret, service_data[service][domain].name,
			notifier_opcode);

	mutex_unlock(&notifier_mutex);
done:
	return NOTIFY_OK;
}

static void audio_notifier_pdr_adsp_cb(int status, char *service_name, void *priv)
{
	audio_notifier_service_cb(status, AUDIO_NOTIFIER_PDR_SERVICE, AUDIO_NOTIFIER_ADSP_DOMAIN);
}

static int audio_notifier_ssr_adsp_cb(struct notifier_block *this,
				     unsigned long opcode, void *data)
{
	return audio_notifier_service_cb(opcode,
					AUDIO_NOTIFIER_SSR_SERVICE,
					AUDIO_NOTIFIER_ADSP_DOMAIN);
}

static int audio_notifier_ssr_modem_cb(struct notifier_block *this,
				      unsigned long opcode, void *data)
{
	return audio_notifier_service_cb(opcode,
					AUDIO_NOTIFIER_SSR_SERVICE,
					AUDIO_NOTIFIER_MODEM_DOMAIN);
}

int audio_notifier_deregister(char *client_name)
{
	int ret = 0;
	int ret2;
	struct list_head *ptr, *next;
	struct client_data *client_data = NULL;

	if (client_name == NULL) {
		pr_err("%s: client_name is NULL\n", __func__);
		ret = -EINVAL;
		goto done;
	}
	mutex_lock(&notifier_mutex);
	list_for_each_safe(ptr, next, &client_list) {
		client_data = list_entry(ptr, struct client_data, list);
		if (!strcmp(client_name, client_data->client_name)) {
			ret2 = audio_notifier_dereg_client(client_data);
			if (ret2 < 0) {
				pr_err("%s: audio_notifier_dereg_client failed, ret %d\n, service %d, domain %d",
					__func__, ret2, client_data->service,
					client_data->domain);
				ret = ret2;
				continue;
			}
			list_del(&client_data->list);
			kfree(client_data);
		}
	}
	mutex_unlock(&notifier_mutex);
done:
	return ret;
}
EXPORT_SYMBOL(audio_notifier_deregister);

int audio_notifier_register(char *client_name, int domain,
			    struct notifier_block *nb)
{
	int ret;
	struct client_data *client_data;

	if (client_name == NULL) {
		pr_err("%s: client_name is NULL\n", __func__);
		ret = -EINVAL;
		goto done;
	} else if (nb == NULL) {
		pr_err("%s: Notifier block is NULL\n", __func__);
		ret = -EINVAL;
		goto done;
	}

	client_data = kmalloc(sizeof(*client_data), GFP_KERNEL);
	if (client_data == NULL) {
		ret = -ENOMEM;
		goto done;
	}
	INIT_LIST_HEAD(&client_data->list);
	client_data->nb = nb;
	strlcpy(client_data->client_name, client_name,
		sizeof(client_data->client_name));
	client_data->service = NO_SERVICE;
	client_data->domain = domain;

	mutex_lock(&notifier_mutex);
	ret = audio_notifier_reg_client(client_data);
	if (ret < 0) {
		mutex_unlock(&notifier_mutex);
		pr_err("%s: audio_notifier_reg_client for client %s failed ret = %d\n",
			__func__, client_data->client_name,
			ret);
		kfree(client_data);
		goto done;
	}
	list_add_tail(&client_data->list, &client_list);
	mutex_unlock(&notifier_mutex);
done:
	return ret;
}
EXPORT_SYMBOL(audio_notifier_register);

static int audio_notifier_subsys_init(void)
{
	int i, j;

	mutex_init(&notifier_mutex);
	for (i = 0; i < AUDIO_NOTIFIER_MAX_SERVICES; i++) {
		for (j = 0; j < AUDIO_NOTIFIER_MAX_DOMAINS; j++) {
			if (service_data[i][j].state <= NO_SERVICE)
				continue;

			srcu_init_notifier_head(
				&service_data[i][j].client_nb_list);
		}
	}

	return 0;
}

static int audio_notifier_late_init(void)
{
	/*
	 * If pdr registration failed, register clients on next service
	 * Do in late init to ensure that SSR subsystem is initialized
	 */
	mutex_lock(&notifier_mutex);
	if (!audio_notifier_is_service_enabled(AUDIO_NOTIFIER_PDR_SERVICE))
		audio_notifier_reg_all_clients();

	mutex_unlock(&notifier_mutex);
	return 0;
}

bool audio_notifier_probe_status(void)
{
	struct adsp_notify_private *priv = NULL;
	struct platform_device *pdev = NULL;

	if (!adsp_private)
		goto exit;

	pdev = adsp_private;
	priv = platform_get_drvdata(pdev);
	if (!priv) {
		dev_err(&pdev->dev," %s: Private data get failed\n", __func__);
		goto exit;
	}
	if (priv->notifier_probe_complete) {
		dev_dbg(&pdev->dev, "%s: audio notify probe successfully completed\n",
			__func__);
		return true;
	}
exit:
	return false;
}
EXPORT_SYMBOL(audio_notifier_probe_status);

static int audio_notify_probe(struct platform_device *pdev)
{
	int ret = -EINVAL;
	struct adsp_notify_private *priv = NULL;
	struct property *prop;
	int size;
	phandle rproc_phandle;

	adsp_private = NULL;
	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		ret = -ENOMEM;
		return ret;
	}
	priv->notifier_probe_complete = false;
	platform_set_drvdata(pdev, priv);
	prop = of_find_property(pdev->dev.of_node, "qcom,rproc-handle", &size);
	if (!prop) {
		dev_err(&pdev->dev, "Missing remotproc handle\n");
		return ret;
	}
	rproc_phandle = be32_to_cpup(prop->value);

	priv->rproc_h = rproc_get_by_phandle(rproc_phandle);
	if (!priv->rproc_h) {
		dev_info_ratelimited(&pdev->dev, "remotproc handle NULL\n");
		ret = -EPROBE_DEFER;
		return ret;
	}

	adsp_private = pdev;

	audio_notifier_subsys_init();

	audio_notifier_init_service(AUDIO_NOTIFIER_PDR_SERVICE);
	/* Do not return error since PDR enablement is not critical */
	audio_notifier_late_init();

	priv->notifier_probe_complete = true;

	return 0;
}

static int audio_notify_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id adsp_notify_dt_match[] = {
	{ .compatible = "qcom,adsp-notify" },
	{ }
};
MODULE_DEVICE_TABLE(of, adsp_notify_dt_match);

static struct platform_driver adsp_notify_driver = {
	.driver = {
		.name = "adsp-notify",
		.owner = THIS_MODULE,
		.of_match_table = adsp_notify_dt_match,
		.suppress_bind_attrs = true,
	},
	.probe = audio_notify_probe,
	.remove = audio_notify_remove,
};

static int __init audio_notifier_init(void)
{
	return platform_driver_register(&adsp_notify_driver);
}
module_init(audio_notifier_init);

static void __exit audio_notifier_exit(void)
{
	platform_driver_unregister(&adsp_notify_driver);
}
module_exit(audio_notifier_exit);

MODULE_SOFTDEP("pre: qcom_q6v5_pas");
MODULE_DESCRIPTION("Audio notifier driver");
MODULE_LICENSE("GPL v2");
