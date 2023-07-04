/* Copyright (c) 2011-2017, 2019-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2018, Linaro Limited
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/spinlock.h>
#include <linux/idr.h>
#include <linux/slab.h>
#include <linux/of_device.h>
#include <ipc/gpr-lite.h>
#include <linux/rpmsg.h>
#include <linux/of.h>

#include <soc/snd_event.h>
#include <dsp/audio_notifier.h>

#define APM_EVENT_MODULE_TO_CLIENT	0x03001000
#define WAKELOCK_TIMEOUT 500

struct gpr {
	struct rpmsg_endpoint *ch;
	struct device *dev;
	spinlock_t gpr_lock;
	bool is_initial_boot;

	spinlock_t svcs_lock;
	struct idr svcs_idr;
	int dest_domain_id;
	struct work_struct notifier_reg_work;
	struct wakeup_source *wsource;
};

static struct gpr_q6 q6;
static struct gpr *gpr_priv;

enum gpr_subsys_state gpr_get_q6_state(void)
{
	return atomic_read(&q6.q6_state);
}
EXPORT_SYMBOL(gpr_get_q6_state);

enum gpr_subsys_state gpr_get_modem_state(void)
{
	return atomic_read(&q6.modem_state);
}
EXPORT_SYMBOL(gpr_get_modem_state);


void gpr_subsys_notif_register(char *client_name, int domain,
			   struct notifier_block *nb)
{
	int ret;

	ret = audio_notifier_register(client_name, domain, nb);
	if (ret < 0)
		dev_err(gpr_priv->dev, "%s: Audio notifier register failed for domain %d ret = %d\n",
			__func__, domain, ret);
}

void gpr_subsys_notif_deregister(char *client_name)
{
	int ret;

	ret = audio_notifier_deregister(client_name);
	if (ret < 0)
		dev_err(gpr_priv->dev, "%s: Audio notifier de-register failed for client %s\n",
			__func__, client_name);
}

/**
 * gpr_send_pkt() - Send a gpr message from gpr device
 *
 * @adev: Pointer to previously registered gpr device.
 * @pkt: Pointer to gpr packet to send
 *
 * Return: Will be an negative and/or packet size on success.
 */
int gpr_send_pkt(struct gpr_device *adev, struct gpr_pkt *pkt)
{
	struct gpr *gpr;
	struct gpr_hdr *hdr;
	unsigned long flags;
	uint32_t pkt_size;
	int ret;

	if(!adev)
	{
		pr_err("%s: enter pointer adev[%pK] \n", __func__, adev);
		return -EINVAL;
	}

	if(!(adev->dev.parent))
	{
		pr_err("%s: enter pointer adev->dev.parent[%pK] \n",
			__func__, adev->dev.parent);
		return -EINVAL;
	}

	gpr = dev_get_drvdata(adev->dev.parent);

	if (!gpr) {
		pr_err_ratelimited("%s: Failed to get gpr dev pointer : gpr[%pK] \n",
			__func__, gpr);
		return -EINVAL;
	}

	if ((adev->domain_id == GPR_DOMAIN_ADSP) &&
	    (gpr_get_q6_state() != GPR_SUBSYS_LOADED)) {
		dev_err_ratelimited(gpr->dev,"%s: domain_id[%d], Still Dsp is not Up\n",
			__func__, adev->domain_id);
		return -ENETRESET;
		} else if ((adev->domain_id == GPR_DOMAIN_MODEM) &&
		   (gpr_get_modem_state() == GPR_SUBSYS_DOWN)) {
		dev_err_ratelimited(gpr->dev, "%s: domain_id[%d], Still Modem is not Up\n",
			__func__, adev->domain_id );
		return -ENETRESET;
	}

	spin_lock_irqsave(&adev->lock, flags);

	hdr = &pkt->hdr;
	hdr->dst_domain_id = adev->domain_id;
	pkt_size = GPR_PKT_GET_PACKET_BYTE_SIZE(hdr->header);

	dev_dbg(gpr->dev, "SVC_ID %d %s packet size %d\n",
		adev->svc_id, __func__, pkt_size);
	ret = rpmsg_trysend(gpr->ch, pkt, pkt_size);
	spin_unlock_irqrestore(&adev->lock, flags);

	return ret ? ret : pkt_size;
}
EXPORT_SYMBOL_GPL(gpr_send_pkt);

 /**
  * apr_set_modem_state - Update modem load status.
  *
  * @state: State to update modem load status
  *
 */
void gpr_set_modem_state(enum gpr_subsys_state state)
{
	atomic_set(&q6.modem_state, state);
}
EXPORT_SYMBOL(gpr_set_modem_state);


static void gpr_modem_down(unsigned long opcode)
{
	gpr_set_modem_state(GPR_SUBSYS_DOWN);
	//dispatch_event(opcode, APR_DEST_MODEM);
}

static void gpr_modem_up(void)
{
	//if (apr_cmpxchg_modem_state(APR_SUBSYS_DOWN, APR_SUBSYS_UP) ==
	//						APR_SUBSYS_DOWN)
	//	wake_up(&modem_wait);
	//is_modem_up = 1;
}


int gpr_set_q6_state(enum gpr_subsys_state state)
{
	dev_dbg(gpr_priv->dev,"%s: setting adsp state %d\n", __func__, state);
	if (state < GPR_SUBSYS_DOWN || state > GPR_SUBSYS_LOADED)
		return -EINVAL;
	atomic_set(&q6.q6_state, state);
	return 0;
}
EXPORT_SYMBOL(gpr_set_q6_state);

static void gpr_ssr_disable(struct device *dev, void *data)
{
	gpr_set_q6_state(GPR_SUBSYS_DOWN);
}

static const struct snd_event_ops gpr_ssr_ops = {
	.disable = gpr_ssr_disable,
};

static void gpr_adsp_down(unsigned long opcode)
{
	dev_info(gpr_priv->dev,"%s: Q6 is Down\n", __func__);
	gpr_set_q6_state(GPR_SUBSYS_DOWN);
	snd_event_notify(gpr_priv->dev, SND_EVENT_DOWN);
}

static void gpr_adsp_up(void)
{
	dev_info(gpr_priv->dev,"%s: Q6 is Up\n", __func__);
	gpr_set_q6_state(GPR_SUBSYS_LOADED);
	snd_event_notify(gpr_priv->dev, SND_EVENT_UP);
}

static int gpr_notifier_service_cb(struct notifier_block *this,
			   unsigned long opcode, void *data)
{
	struct audio_notifier_cb_data *cb_data = data;

	if (cb_data == NULL) {
		dev_err(gpr_priv->dev,"%s: Callback data is NULL!\n", __func__);
		goto done;
	}

	dev_dbg(gpr_priv->dev,"%s: Service opcode 0x%lx, domain %d\n",
		__func__, opcode, cb_data->domain);

	switch (opcode) {
	case AUDIO_NOTIFIER_SERVICE_DOWN:
		/*
		 * Use flag to ignore down notifications during
		 * initial boot. There is no benefit from error
		 * recovery notifications during initial boot
		 * up since everything is expected to be down.
		 */
		spin_lock(&gpr_priv->gpr_lock);
		if (gpr_priv->is_initial_boot) {
			spin_unlock(&gpr_priv->gpr_lock);
			break;
		}
		spin_unlock(&gpr_priv->gpr_lock);
		if (cb_data->domain == AUDIO_NOTIFIER_MODEM_DOMAIN)
			gpr_modem_down(opcode);
		else
			gpr_adsp_down(opcode);
		break;
	case AUDIO_NOTIFIER_SERVICE_UP:
		if (cb_data->domain == AUDIO_NOTIFIER_MODEM_DOMAIN)
			gpr_modem_up();
		else
			gpr_adsp_up();
		spin_lock(&gpr_priv->gpr_lock);
		gpr_priv->is_initial_boot = false;
		spin_unlock(&gpr_priv->gpr_lock);
		break;
	default:
		break;
	}
done:
	return NOTIFY_OK;
}

static struct notifier_block adsp_service_nb = {
	.notifier_call  = gpr_notifier_service_cb,
	.priority = 0,
};

static struct notifier_block modem_service_nb = {
	.notifier_call  = gpr_notifier_service_cb,
	.priority = 0,
};


static void gpr_dev_release(struct device *dev)
{
	struct gpr_device *adev = to_gpr_device(dev);

	kfree(adev);
}

static int gpr_callback(struct rpmsg_device *rpdev, void *buf,
				  int len, void *priv, u32 addr)
{
	struct gpr *gpr = dev_get_drvdata(&rpdev->dev);
	uint16_t hdr_size, pkt_size, svc_id;
	//uint16_t ver;
	struct gpr_device *svc = NULL;
	struct gpr_driver *adrv = NULL;
	struct gpr_hdr *hdr;
	unsigned long flags;
	//uint32_t opcode_type;

	if (len <= GPR_HDR_SIZE) {
		dev_err(gpr->dev, "GPR: Improper gpr pkt received:%p %d\n",
			buf, len);
		return -EINVAL;
	}

	hdr = buf;

	hdr_size = GPR_PKT_GET_HEADER_BYTE_SIZE(hdr->header);
	if (hdr_size < GPR_HDR_SIZE) {
		dev_err(gpr->dev, "GPR: Wrong hdr size:%d\n", hdr_size);
		return -EINVAL;
	}

	pkt_size = GPR_PKT_GET_PACKET_BYTE_SIZE(hdr->header);
	dev_dbg(gpr->dev,"Header %x", hdr->header);

	if (pkt_size < GPR_HDR_SIZE || pkt_size != len) {
		dev_err(gpr->dev, "GPR: Wrong packet size\n");
		return -EINVAL;
	}

	dev_dbg(gpr->dev, "%s: dst_port %x hdr_size %d pkt_size %d\n",
	__func__ , hdr->dst_port, hdr_size, pkt_size);

	if (hdr->opcode == APM_EVENT_MODULE_TO_CLIENT) {
		dev_info(gpr->dev, "%s: Acquire wakelock in case of module event with timeout %d",
			__func__, WAKELOCK_TIMEOUT);
		pm_wakeup_ws_event(gpr_priv->wsource, WAKELOCK_TIMEOUT, true);
	}
	svc_id = hdr->dst_port;
	spin_lock_irqsave(&gpr->svcs_lock, flags);
	svc = idr_find(&gpr->svcs_idr, svc_id);
	if (svc && svc->dev.driver) {
		adrv = to_gpr_driver(svc->dev.driver);
	} else {
		/*Does not match any SVC ID hence would be routed to audio passthrough*/
		svc = idr_find(&gpr->svcs_idr, GPR_SVC_MAX);
		if (svc && svc->dev.driver)
			adrv = to_gpr_driver(svc->dev.driver);
	}
	spin_unlock_irqrestore(&gpr->svcs_lock, flags);

	if (!adrv) {
		dev_err(gpr->dev, "GPR: service is not registered\n");
		if (hdr->opcode == APM_EVENT_MODULE_TO_CLIENT)
			__pm_relax(gpr_priv->wsource);
		return -EINVAL;
	}

	/*
	 * NOTE: hdr_size is not same as GPR_HDR_SIZE as remote can include
	 * optional headers in to gpr_hdr which should be ignored
	 */

	adrv->callback(svc, buf);

	return 0;
}

static int gpr_device_match(struct device *dev, struct device_driver *drv)
{
	struct gpr_device *adev = to_gpr_device(dev);
	struct gpr_driver *adrv = to_gpr_driver(drv);
	const struct gpr_device_id *id = adrv->id_table;

	/* Attempt an OF style match first */
	if (of_driver_match_device(dev, drv))
		return 1;

	if (!id)
		return 0;

	while (id->domain_id != 0 || id->svc_id != 0) {
		if (id->domain_id == adev->domain_id &&
		    id->svc_id == adev->svc_id)
			return 1;
		id++;
	}

	return 0;
}

static int gpr_device_probe(struct device *dev)
{
	struct gpr_device *adev = to_gpr_device(dev);
	struct gpr_driver *adrv = to_gpr_driver(dev->driver);

	return adrv->probe(adev);
}

static int gpr_device_remove(struct device *dev)
{
	struct gpr_device *adev = to_gpr_device(dev);
	struct gpr_driver *adrv;
	struct gpr *gpr = dev_get_drvdata(adev->dev.parent);

	if (dev->driver) {
		adrv = to_gpr_driver(dev->driver);
		if (adrv->remove)
			adrv->remove(adev);
		spin_lock(&gpr->svcs_lock);
		idr_remove(&gpr->svcs_idr, adev->svc_id);
		spin_unlock(&gpr->svcs_lock);
	}

	return 0;
}

static int gpr_uevent(struct device *dev, struct kobj_uevent_env *env)
{
	struct gpr_device *adev = to_gpr_device(dev);
	int ret;

	ret = of_device_uevent_modalias(dev, env);
	if (ret != -ENODEV)
		return ret;

	return add_uevent_var(env, "MODALIAS=gpr:%s", adev->name);
}

struct bus_type gprbus = {
	.name		= "gprbus",
	.match		= gpr_device_match,
	.probe		= gpr_device_probe,
	.uevent		= gpr_uevent,
	.remove		= gpr_device_remove,
};
EXPORT_SYMBOL_GPL(gprbus);

static int gpr_add_device(struct device *dev, struct device_node *np,
			  const struct gpr_device_id *id)
{
	struct gpr *gpr = dev_get_drvdata(dev);
	struct gpr_device *adev = NULL;
	int ret;

	adev = kzalloc(sizeof(*adev), GFP_KERNEL);
	if (!adev)
		return -ENOMEM;

	spin_lock_init(&adev->lock);

	adev->svc_id = id->svc_id;
	adev->domain_id = id->domain_id;
	adev->version = id->svc_version;
	if (np)
		strscpy(adev->name, np->name, GPR_NAME_SIZE);
	else
		strscpy(adev->name, id->name, GPR_NAME_SIZE);

	dev_set_name(&adev->dev, "gprsvc:%s:%x:%x", adev->name,
		     id->domain_id, id->svc_id);

	adev->dev.bus = &gprbus;
	adev->dev.parent = dev;
	adev->dev.of_node = np;
	adev->dev.release = gpr_dev_release;
	adev->dev.driver = NULL;

	spin_lock(&gpr->svcs_lock);
	idr_alloc(&gpr->svcs_idr, adev, id->svc_id,
		  id->svc_id + 1, GFP_ATOMIC);
	spin_unlock(&gpr->svcs_lock);

	dev_info(dev, "Adding GPR dev: %s\n", dev_name(&adev->dev));

	ret = device_register(&adev->dev);
	if (ret) {
		dev_err(dev, "device_register failed: %d\n", ret);
		put_device(&adev->dev);
	}

	return ret;
}

static void of_register_gpr_devices(struct device *dev)
{
	struct gpr *gpr = dev_get_drvdata(dev);
	struct device_node *node;

	for_each_child_of_node(dev->of_node, node) {
		struct gpr_device_id id = { {0} };

		if (of_property_read_u32(node, "reg", &id.svc_id))
			continue;

		id.domain_id = gpr->dest_domain_id;

		if (gpr_add_device(dev, node, &id))
			dev_err(dev, "Failed to add gpr %d svc\n", id.svc_id);
	}
}

static void gpr_notifier_register(struct work_struct *work)
{
	if (GPR_DOMAIN_ADSP == gpr_priv->dest_domain_id) {
		gpr_subsys_notif_register("gpr_adsp",
				       AUDIO_NOTIFIER_ADSP_DOMAIN,
				       &adsp_service_nb);
	} else if (GPR_DOMAIN_MODEM == gpr_priv->dest_domain_id) {
		gpr_subsys_notif_register("gpr_modem",
				       AUDIO_NOTIFIER_MODEM_DOMAIN,
				       &modem_service_nb);
	}

	dev_info(gpr_priv->dev, "%s: registered via subsys_notif_register for domain id(%d)",
		__func__, gpr_priv->dest_domain_id  );
	return;
}

static int gpr_probe(struct rpmsg_device *rpdev)
{
	struct device *dev = &rpdev->dev;
	int ret;

	if (!audio_notifier_probe_status()) {
		pr_err("%s: Audio notify probe not completed, defer audio gpr probe\n",
			__func__);
		return -EPROBE_DEFER;
	}

	gpr_priv = devm_kzalloc(dev, sizeof(*gpr_priv), GFP_KERNEL);
	if (!gpr_priv)
		return -ENOMEM;

	spin_lock_init(&gpr_priv->gpr_lock);
	mutex_init(&q6.lock);

	spin_lock(&gpr_priv->gpr_lock);
	gpr_priv->is_initial_boot = true;
	spin_unlock(&gpr_priv->gpr_lock);

	dev_set_drvdata(dev, gpr_priv);
	gpr_priv->ch = rpdev->ept;
	gpr_priv->dev = dev;
	spin_lock_init(&gpr_priv->svcs_lock);
	idr_init(&gpr_priv->svcs_idr);

	ret = snd_event_client_register(&rpdev->dev, &gpr_ssr_ops, NULL);
	if (ret) {
		dev_err(dev,"%s: Registration with SND event fwk failed ret = %d\n",
			__func__, ret);
		ret = 0;
	}

	ret = of_property_read_u32(dev->of_node, "reg", &gpr_priv->dest_domain_id);
	if (ret) {
		dev_err(dev, "GPR Domain ID not specified in DT\n");
		return ret;
	}

	of_register_gpr_devices(dev);

	INIT_WORK(&gpr_priv->notifier_reg_work, gpr_notifier_register);

	if (GPR_DOMAIN_ADSP == gpr_priv->dest_domain_id ||
		GPR_DOMAIN_MODEM == gpr_priv->dest_domain_id) {
		schedule_work(&gpr_priv->notifier_reg_work);
	} else {
		dev_err(dev, "%s: invalid dest_domain_id %s\n", __func__,
		  gpr_priv->dest_domain_id);
		return -EINVAL;
	}

	gpr_priv->wsource = wakeup_source_register(gpr_priv->dev, "audio-gpr");
	dev_info(dev, "%s: gpr-lite probe success\n",
		__func__);

	return 0;
}

static int gpr_remove_device(struct device *dev, void *null)
{
	struct gpr_device *adev = to_gpr_device(dev);
	device_unregister(&adev->dev);
	return 0;
}

static void gpr_remove(struct rpmsg_device *rpdev)
{
	struct device *dev = &rpdev->dev;

	wakeup_source_unregister(gpr_priv->wsource);
	snd_event_client_deregister(&rpdev->dev);
	dev_info(dev, "%s: deregistering via subsys_notif_register for domain_id(%d)",
		__func__, gpr_priv->dest_domain_id );
	if (GPR_DOMAIN_ADSP == gpr_priv->dest_domain_id)
	{
		gpr_subsys_notif_deregister("gpr_adsp");
	}
	else if(GPR_DOMAIN_MODEM == gpr_priv->dest_domain_id)
	{
		gpr_subsys_notif_deregister("gpr_modem");
	}
	device_for_each_child(&rpdev->dev, NULL, gpr_remove_device);
}

/*
 * __gpr_driver_register() - Client driver registration with gprbus
 *
 * @drv:Client driver to be associated with client-device.
 * @owner: owning module/driver
 *
 * This API will register the client driver with the gprbus
 * It is called from the driver's module-init function.
 */
int __gpr_driver_register(struct gpr_driver *drv, struct module *owner)
{
	drv->driver.bus = &gprbus;
	drv->driver.owner = owner;

	return driver_register(&drv->driver);
}
EXPORT_SYMBOL_GPL(__gpr_driver_register);

/*
 * gpr_driver_unregister() - Undo effect of gpr_driver_register
 *
 * @drv: Client driver to be unregistered
 */
void gpr_driver_unregister(struct gpr_driver *drv)
{
	driver_unregister(&drv->driver);
}
EXPORT_SYMBOL_GPL(gpr_driver_unregister);

static const struct of_device_id gpr_of_match[] = {
	{ .compatible = "qcom,gpr"},
	{}
};
MODULE_DEVICE_TABLE(of, gpr_of_match);

static struct rpmsg_driver gpr_driver = {
	.probe = gpr_probe,
	.remove = gpr_remove,
	.callback = gpr_callback,
	.drv = {
		.name = "qcom,gpr",
		.of_match_table = gpr_of_match,
	},
};

static int __init gpr_init(void)
{
	int ret;

	ret = bus_register(&gprbus);
	if (!ret)
		ret = register_rpmsg_driver(&gpr_driver);
	else
		bus_unregister(&gprbus);

	return ret;
}

static void __exit gpr_exit(void)
{
	bus_unregister(&gprbus);
	unregister_rpmsg_driver(&gpr_driver);
}

subsys_initcall(gpr_init);
module_exit(gpr_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("QTI GPR Bus");
