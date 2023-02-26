/* Copyright (c) 2019-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include <linux/ipc_logging.h>
#include <linux/refcount.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/idr.h>
#include <linux/of.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/termios.h>
#include <ipc/gpr-lite.h>
#include <dsp/spf-core.h>
#include <dsp/msm_audio_ion.h>

/* Define IPC Logging Macros */
#define AUDIO_PKT_IPC_LOG_PAGE_CNT 2
static void *audio_pkt_ilctxt;

static int audio_pkt_debug_mask;
module_param_named(debug_mask, audio_pkt_debug_mask, int, 0664);

#define APM_CMD_SHARED_MEM_MAP_REGIONS		0x0100100C
#define APM_MEMORY_MAP_BIT_MASK_IS_OFFSET_MODE	0x00000004UL
enum {
	AUDIO_PKT_INFO = 1U << 0,
};

#define AUDIO_PKT_INFO(x, ...)						\
do {									\
	if (audio_pkt_debug_mask & AUDIO_PKT_INFO) {			\
		ipc_log_string(audio_pkt_ilctxt,			\
			"[%s]: "x, __func__, ##__VA_ARGS__);		\
	}								\
} while (0)

#define AUDIO_PKT_ERR(x, ...)						      \
do {									      \
	pr_err_ratelimited("[%s]: "x, __func__, ##__VA_ARGS__);		      \
	ipc_log_string(audio_pkt_ilctxt, "[%s]: "x, __func__, ##__VA_ARGS__); \
} while (0)


#define MODULE_NAME "audio-pkt"
#define MINOR_NUMBER_COUNT 1
#define AUDPKT_DRIVER_NAME "aud_pasthru_adsp"
#define CHANNEL_NAME "adsp_apps"
#define MAX_PACKET_SIZE 4096

enum audio_pkt_state {
	AUDIO_PKT_INIT,
	AUDIO_PKT_PROBED,
	AUDIO_PKT_REMOVED,
	AUDIO_PKT_DEINIT,
};

/**
 * struct audio_pkt_device - driver context, relates to platform dev
 * @dev:	audio pkt device
 * @cdev:	cdev for the audio pkt device
 * @lock:	synchronization of @dev
 * @queue_lock:	synchronization of @queue operations
 * @queue:	incoming message queue
 * @readq:	wait object for incoming queue
 * @dev_name:	/dev/@dev_name for audio_pkt device
 * @ch_name:	audio channel to match to
 * @audio_pkt_major: Major number of audio pkt driver
 * @audio_pkt_class: audio pkt class pointer
 */
struct audio_pkt_device {
	struct device *dev;
	struct cdev cdev;

	struct mutex lock;
	spinlock_t queue_lock;
	struct sk_buff_head queue;
	wait_queue_head_t readq;

	char dev_name[20];
	char ch_name[20];

	dev_t audio_pkt_major;
	struct class *audio_pkt_class;
};

struct audio_pkt_priv {
	struct gpr_device *adev;
	struct device *dev;
	struct audio_pkt_device *ap_dev;

	struct mutex lock;
	enum audio_pkt_state status;
};

static struct audio_pkt_priv *ap_priv;


struct audio_pkt_apm_cmd_shared_mem_map_regions_t {
	uint16_t mem_pool_id;
	uint16_t num_regions;
	uint32_t property_flag;

};

struct audio_pkt_apm_shared_map_region_payload_t {
	uint32_t shm_addr_lsw;
	uint32_t shm_addr_msw;
	uint32_t mem_size_bytes;
};

struct audio_pkt_apm_mem_map {
	struct audio_pkt_apm_cmd_shared_mem_map_regions_t mmap_header;
	struct audio_pkt_apm_shared_map_region_payload_t mmap_payload;
};

struct audio_gpr_pkt {
	struct gpr_hdr audpkt_hdr;
	struct audio_pkt_apm_mem_map audpkt_mem_map;
};

typedef void (*audio_pkt_clnt_cb_fn)(void *buf, int len, void *priv);

struct audio_pkt_clnt_ch {
	int client_id;
	audio_pkt_clnt_cb_fn func;
};

/**
 * audio_pkt_open() - open() syscall for the audio_pkt device
 * inode:	Pointer to the inode structure.
 * file:	Pointer to the file structure.
 *
 * This function is used to open the audio pkt device when
 * userspace client do a open() system call. All input arguments are
 * validated by the virtual file system before calling this function.
 */
int audio_pkt_open(struct inode *inode, struct file *file)
{
	struct audio_pkt_device *audpkt_dev = ap_priv->ap_dev;
	AUDIO_PKT_INFO("%s: for %s \n", __func__,audpkt_dev->ch_name);
	file->private_data = ap_priv;
	return 0;
}

/**
 * audio_pkt_release() - release operation on audio_pkt device
 * inode:	Pointer to the inode structure.
 * file:	Pointer to the file structure.
 *
 * This function is used to release the audio pkt device when
 * userspace client do a close() system call. All input arguments are
 * validated by the virtual file system before calling this function.
 */
int audio_pkt_release(struct inode *inode, struct file *file)
{
	struct audio_pkt_priv *ap_priv = file->private_data;
	struct audio_pkt_device *audpkt_dev = ap_priv->ap_dev;

	struct sk_buff *skb;
	unsigned long flags;

	if ((!audpkt_dev)) {
		AUDIO_PKT_ERR("invalid device handle\n");
		return -EINVAL;
	}

	AUDIO_PKT_INFO("%s: for %s \n", __func__,audpkt_dev->ch_name);
	spin_lock_irqsave(&audpkt_dev->queue_lock, flags);

	/* Discard all SKBs */
	while (!skb_queue_empty(&audpkt_dev->queue)) {
		skb = skb_dequeue(&audpkt_dev->queue);
		kfree_skb(skb);
	}
	wake_up_interruptible(&audpkt_dev->readq);
	spin_unlock_irqrestore(&audpkt_dev->queue_lock, flags);

	file->private_data = NULL;
	spf_core_apm_close_all();
	msm_audio_ion_crash_handler();

	return 0;
}


static int audio_pkt_internal_release(struct platform_device *adev)
{
	struct audio_pkt_priv *ap_priv = platform_get_drvdata(adev);
	struct audio_pkt_device *audpkt_dev = ap_priv->ap_dev;
	struct sk_buff *skb;
	unsigned long flags;

	if ((!audpkt_dev)) {
		AUDIO_PKT_ERR("invalid device handle\n");
		return -EINVAL;
	}

	AUDIO_PKT_INFO("%s: for %s\n", __func__,audpkt_dev->ch_name);
	spin_lock_irqsave(&audpkt_dev->queue_lock, flags);
	/* Discard all SKBs */
	while (!skb_queue_empty(&audpkt_dev->queue)) {
		skb = skb_dequeue(&audpkt_dev->queue);
		kfree_skb(skb);
	}
	spin_unlock_irqrestore(&audpkt_dev->queue_lock, flags);

	wake_up_interruptible(&audpkt_dev->readq);

	return 0;
}

/**
 * audio_pkt_read() - read() syscall for the audio_pkt device
 * file:	Pointer to the file structure.
 * buf:		Pointer to the userspace buffer.
 * count:	Number bytes to read from the file.
 * ppos:	Pointer to the position into the file.
 *
 * This function is used to Read the data from audio pkt device when
 * userspace client do a read() system call. All input arguments are
 * validated by the virtual file system before calling this function.
 */
ssize_t audio_pkt_read(struct file *file, char __user *buf,
		       size_t count, loff_t *ppos)
{
	struct audio_pkt_priv *ap_priv = file->private_data;
	struct audio_pkt_device *audpkt_dev = ap_priv->ap_dev;

	unsigned long flags;
	struct sk_buff *skb;
	int use;
	uint32_t *temp;

	if (!audpkt_dev) {
		AUDIO_PKT_ERR("invalid device handle\n");
		return -EINVAL;
	}

	mutex_lock(&ap_priv->lock);
	if (AUDIO_PKT_PROBED != ap_priv->status)
	{
		mutex_unlock(&ap_priv->lock);
		AUDIO_PKT_ERR("dev is in reset\n");
		return -ENETRESET;
	}
	mutex_unlock(&ap_priv->lock);

	spin_lock_irqsave(&audpkt_dev->queue_lock, flags);
	/* Wait for data in the queue */
	if (skb_queue_empty(&audpkt_dev->queue)) {
		spin_unlock_irqrestore(&audpkt_dev->queue_lock, flags);

		if (file->f_flags & O_NONBLOCK)
			return -EAGAIN;

		/* Wait until we get data or the endpoint goes away */
		if (wait_event_interruptible(audpkt_dev->readq,
					!skb_queue_empty(&audpkt_dev->queue)))
			return -ERESTARTSYS;

		spin_lock_irqsave(&audpkt_dev->queue_lock, flags);
	}

	skb = skb_dequeue(&audpkt_dev->queue);
	spin_unlock_irqrestore(&audpkt_dev->queue_lock, flags);
	if (!skb)
		return -EFAULT;

	use = min_t(size_t, count, skb->len);
	if (copy_to_user(buf, skb->data, use))
		use = -EFAULT;
	temp = (uint32_t *) skb->data;
	kfree_skb(skb);

	return use;
}

/**
 * audpkt_update_physical_addr - Update physical address
 * audpkt_hdr:	Pointer to the file structure.
 */
int audpkt_chk_and_update_physical_addr(struct audio_gpr_pkt *gpr_pkt)
{
	int ret = 0;
        size_t pa_len = 0;
	dma_addr_t paddr;

	if (gpr_pkt->audpkt_mem_map.mmap_header.property_flag &
				APM_MEMORY_MAP_BIT_MASK_IS_OFFSET_MODE) {
		ret = msm_audio_get_phy_addr(
			(int) gpr_pkt->audpkt_mem_map.mmap_payload.shm_addr_lsw,
			&paddr, &pa_len);
		if (ret < 0) {
			AUDIO_PKT_ERR("%s Get phy. address failed, ret %d\n",
					__func__, ret);
			return ret;
		}
		AUDIO_PKT_INFO("%s physical address %pK", __func__,
				(void *) paddr);
		gpr_pkt->audpkt_mem_map.mmap_payload.shm_addr_lsw = (uint32_t) paddr;
		gpr_pkt->audpkt_mem_map.mmap_payload.shm_addr_msw = (uint64_t) paddr >> 32;
	}
	return ret;
}

/**
 * audio_pkt_write() - write() syscall for the audio_pkt device
 * file:	Pointer to the file structure.
 * buf:		Pointer to the userspace buffer.
 * count:	Number bytes to read from the file.
 * ppos:	Pointer to the position into the file.
 *
 * This function is used to write the data to audio pkt device when
 * userspace client do a write() system call. All input arguments are
 * validated by the virtual file system before calling this function.
 */
ssize_t audio_pkt_write(struct file *file, const char __user *buf,
			size_t count, loff_t *ppos)
{
	struct audio_pkt_priv *ap_priv = file->private_data;
	struct audio_pkt_device *audpkt_dev = ap_priv->ap_dev;
	struct gpr_hdr *audpkt_hdr = NULL;
	void *kbuf;
	int ret;

	if (!audpkt_dev)  {
		AUDIO_PKT_ERR("invalid device handle\n");
		return -EINVAL;
	}

	mutex_lock(&ap_priv->lock);
	if (AUDIO_PKT_PROBED != ap_priv->status)
	{
		mutex_unlock(&ap_priv->lock);
		AUDIO_PKT_ERR("dev is in reset\n");
		return -ENETRESET;
	}
	mutex_unlock(&ap_priv->lock);
	if (count < sizeof(struct gpr_hdr)) {
		AUDIO_PKT_ERR("Invalid count %zu\n",count);
		return  -EINVAL;
	}

	kbuf = memdup_user(buf, count);
	if (IS_ERR(kbuf))
		return PTR_ERR(kbuf);

	audpkt_hdr = (struct gpr_hdr *) kbuf;

	/* validate packet size */
	if ((count > MAX_PACKET_SIZE) || (count < GPR_PKT_GET_PACKET_BYTE_SIZE(audpkt_hdr->header)))
	{
		ret = -EINVAL;
		goto free_kbuf;
	}

	if (audpkt_hdr->opcode == APM_CMD_SHARED_MEM_MAP_REGIONS) {
		if (count < sizeof(struct audio_gpr_pkt )) {
			AUDIO_PKT_ERR("Invalid count %zu\n",count);
			ret = -EINVAL;
			goto free_kbuf;
		}
		ret = audpkt_chk_and_update_physical_addr((struct audio_gpr_pkt *) audpkt_hdr);
		if (ret < 0) {
			AUDIO_PKT_ERR("Update Physical Address Failed -%d\n", ret);
			goto free_kbuf;
		}
	}

	if (mutex_lock_interruptible(&audpkt_dev->lock)) {
		ret = -ERESTARTSYS;
		goto free_kbuf;
	}
	if (count < sizeof(struct gpr_pkt )) {
		AUDIO_PKT_ERR("Invalid count %zu\n",count);
		ret = -EINVAL;
		mutex_unlock(&audpkt_dev->lock);
		goto free_kbuf;
	}
	ret = gpr_send_pkt(ap_priv->adev,(struct gpr_pkt *) kbuf);
	if (ret < 0) {
		AUDIO_PKT_ERR("APR Send Packet Failed ret -%d\n", ret);
		if (ret == -ECONNRESET)
			ret = -ENETRESET;
	}
	mutex_unlock(&audpkt_dev->lock);

free_kbuf:
	kfree(kbuf);
	return ret < 0 ? ret : count;
}

/**
 * audio_pkt_poll() - poll() syscall for the audio_pkt device
 * file:	Pointer to the file structure.
 * wait:	pointer to Poll table.
 *
 * This function is used to poll on the audio pkt device when
 * userspace client do a poll() system call. All input arguments are
 * validated by the virtual file system before calling this function.
 */
static unsigned int audio_pkt_poll(struct file *file, poll_table *wait)
{
	struct audio_pkt_priv *ap_priv = file->private_data;
	struct audio_pkt_device *audpkt_dev = ap_priv->ap_dev;
	unsigned int mask = 0;
	unsigned long flags;
	if (!audpkt_dev) {
		AUDIO_PKT_ERR("invalid device handle\n");
		return POLLERR;
	}

	poll_wait(file, &audpkt_dev->readq, wait);
	mutex_lock(&audpkt_dev->lock);

	spin_lock_irqsave(&audpkt_dev->queue_lock, flags);
	if (!skb_queue_empty(&audpkt_dev->queue))
		mask |= POLLIN | POLLRDNORM;

	spin_unlock_irqrestore(&audpkt_dev->queue_lock, flags);
	mutex_unlock(&audpkt_dev->lock);

	return mask;
}

static const struct file_operations audio_pkt_fops = {
	.owner = THIS_MODULE,
	.open = audio_pkt_open,
	.release = audio_pkt_release,
	.read = audio_pkt_read,
	.write = audio_pkt_write,
	.poll = audio_pkt_poll,
};

/**
 * audio_pkt_srvc_callback() - Callback from gpr driver
 * adev:	pointer to the gpr device of this audio packet device
 * data:	APR response data packet
 *
 * return:	0 for success, Standard Linux errors
 */
static int audio_pkt_srvc_callback(struct gpr_device *adev,
				void *data)
{
	struct audio_pkt_priv *ap_priv =  dev_get_drvdata(&adev->dev);
	struct audio_pkt_device *audpkt_dev = ap_priv->ap_dev;

	unsigned long flags;
	struct sk_buff *skb;
	struct gpr_hdr *hdr = (struct gpr_hdr *)data;
	uint16_t hdr_size, pkt_size;
	hdr_size = GPR_PKT_GET_HEADER_BYTE_SIZE(hdr->header);
	pkt_size = GPR_PKT_GET_PACKET_BYTE_SIZE(hdr->header);

    AUDIO_PKT_INFO("%s: header %d packet %d \n",
		__func__,hdr_size, pkt_size);

	skb = alloc_skb(pkt_size, GFP_ATOMIC);
	if (!skb)
		return -ENOMEM;

	skb_put_data(skb, data, pkt_size);

	spin_lock_irqsave(&audpkt_dev->queue_lock, flags);
	skb_queue_tail(&audpkt_dev->queue, skb);
	spin_unlock_irqrestore(&audpkt_dev->queue_lock, flags);

	/* wake up any blocking processes, waiting for new data */
	wake_up_interruptible(&audpkt_dev->readq);
	return 0;
}

/**
 * audio_pkt_probe() - Probe a AUDIO packet device
 *
 * adev:	Pointer to gpr device.
 *
 * return:	0 on success, standard Linux error codes on error.
 *
 * This function is called when the underlying device tree driver registers
 * a gpr device, mapped to a Audio packet device.
 */
static int audio_pkt_probe(struct gpr_device *adev)
{
	if(ap_priv)
	{
		mutex_lock(&ap_priv->lock);
		ap_priv->adev = adev;
		ap_priv->status = AUDIO_PKT_PROBED;
		mutex_unlock(&ap_priv->lock);

		dev_set_drvdata(&adev->dev, ap_priv);

		dev_dbg(&adev->dev, "%s: Driver[%s] Probed\n",
		 __func__, adev->name);
	}
	else
	{
		dev_err(&adev->dev, "%s: Driver[%s] Probe Failed\n",
		 __func__, adev->name);
		return -EINVAL;
	}

	return 0;

}

/**
 * audio_pkt_remove() - Remove a AUDIO packet device
 *
 * adev:	Pointer to gpr device.
 *
 * return:	0 on success, standard Linux error codes on error.
 *
 * This function is called when the underlying device tree driver
 * removeds a gpr device, mapped to a Audio packet device.
 */
static int audio_pkt_remove(struct gpr_device *adev)
{
	if(ap_priv)
	{
		mutex_lock(&ap_priv->lock);
		ap_priv->adev = NULL;
		ap_priv->status = AUDIO_PKT_REMOVED;
		mutex_unlock(&ap_priv->lock);
		dev_dbg(&adev->dev, "%s: Driver[%s] Removing\n",
		 __func__, adev->name);
		dev_set_drvdata(&adev->dev, NULL);
	}
	else
	{
		dev_err(&adev->dev, "%s: Driver[%s] Remove Failed\n",
		 __func__, adev->name);
		return -EINVAL;
	}
	return 0;
}

static const struct of_device_id audio_pkt_match_table[] = {
	{ .compatible = "qcom,audio-pkt" },
	{}
};
MODULE_DEVICE_TABLE(of, audio_pkt_match_table);

static struct gpr_driver audio_pkt_driver = {
	.probe = audio_pkt_probe,
	.remove = audio_pkt_remove,
	.callback = audio_pkt_srvc_callback,
	.driver = {
		.name = MODULE_NAME,
		.of_match_table = of_match_ptr(audio_pkt_match_table),
	 },
};

static int audio_pkt_plaform_driver_register_gpr(struct platform_device *pdev,
				struct audio_pkt_device *audpkt_dev)
{
	int ret = 0;

	ap_priv = devm_kzalloc(&pdev->dev,
			     sizeof(*ap_priv), GFP_KERNEL);
	if (!ap_priv)
		return -ENOMEM;

	ret = gpr_driver_register(&audio_pkt_driver);
	if (ret < 0) {
		dev_err(&pdev->dev, "%s: registering to gpr driver failed, err = %d\n",
			__func__, ret);
		goto err;
	}

	mutex_init(&ap_priv->lock);
	ap_priv->status = AUDIO_PKT_INIT;
	ap_priv->ap_dev = audpkt_dev;
	ap_priv->dev = audpkt_dev->dev;
err:
	return ret;
}

/**
 * audio_pkt_platform_driver_probe() - Probe a AUDIO packet device
 *
 * adev:	Pointer to platform device.
 *
 * return:	0 on success, standard Linux error codes on error.
 *
 * This function is called when the underlying device tree driver registers
 * a platform device, mapped to a Audio packet device.
 */
static int audio_pkt_platform_driver_probe(struct platform_device *pdev)
{
	int ret;
	struct audio_pkt_device *audpkt_dev;

	audpkt_dev = devm_kzalloc(&pdev->dev, sizeof(*audpkt_dev), GFP_KERNEL);
	if (!audpkt_dev)
		return -ENOMEM;

	ret = alloc_chrdev_region(&audpkt_dev->audio_pkt_major, 0,
				  MINOR_NUMBER_COUNT,AUDPKT_DRIVER_NAME);
	if (ret < 0) {
		AUDIO_PKT_ERR("alloc_chrdev_region failed ret:%d\n", ret);
		goto err_chrdev;
	}

	audpkt_dev->audio_pkt_class = class_create(THIS_MODULE,
						   AUDPKT_DRIVER_NAME);
	if (IS_ERR(audpkt_dev->audio_pkt_class)) {
		ret = PTR_ERR(audpkt_dev->audio_pkt_class);
		AUDIO_PKT_ERR("class_create failed ret:%ld\n",
			      PTR_ERR(audpkt_dev->audio_pkt_class));
		goto err_class;
	}

	audpkt_dev->dev = device_create(audpkt_dev->audio_pkt_class, NULL,
					audpkt_dev->audio_pkt_major, NULL,
					AUDPKT_DRIVER_NAME);
	if (IS_ERR(audpkt_dev->dev)) {
		ret = PTR_ERR(audpkt_dev->dev);
		AUDIO_PKT_ERR("device_create failed ret:%ld\n",
			      PTR_ERR(audpkt_dev->dev));
		goto err_device;
	}
	strlcpy(audpkt_dev->dev_name, AUDPKT_DRIVER_NAME, 20);
	strlcpy(audpkt_dev->ch_name, CHANNEL_NAME, 20);
	dev_set_name(audpkt_dev->dev, audpkt_dev->dev_name);

	mutex_init(&audpkt_dev->lock);

	spin_lock_init(&audpkt_dev->queue_lock);
	skb_queue_head_init(&audpkt_dev->queue);
	init_waitqueue_head(&audpkt_dev->readq);

	cdev_init(&audpkt_dev->cdev, &audio_pkt_fops);
	audpkt_dev->cdev.owner = THIS_MODULE;

	ret = cdev_add(&audpkt_dev->cdev, audpkt_dev->audio_pkt_major,
		       MINOR_NUMBER_COUNT);
	if (ret) {
		AUDIO_PKT_ERR("cdev_add failed for %s ret:%d\n",
			      audpkt_dev->dev_name, ret);
		goto free_dev;
	}

	ret = audio_pkt_plaform_driver_register_gpr(pdev, audpkt_dev);
	if (ret < 0) {
		dev_err(&pdev->dev, "%s: Failed to register with gpr, err = %d\n",
			__func__, ret);
		goto free_dev;
	}

	platform_set_drvdata(pdev, ap_priv);
	AUDIO_PKT_INFO("Audio Packet Port Driver Initialized\n");

	goto done;
	//return of_platform_populate(dev->of_node, NULL, NULL, dev);

free_dev:
	device_destroy(audpkt_dev->audio_pkt_class,audpkt_dev->audio_pkt_major);
err_device:
	class_destroy(audpkt_dev->audio_pkt_class);
err_class:
	unregister_chrdev_region(MAJOR(audpkt_dev->audio_pkt_major),
				 MINOR_NUMBER_COUNT);
err_chrdev:
done:
	return ret;

}

/**
 * audio_pkt_platform_driver_remove() - Remove a AUDIO packet platform device
 *
 * adev:	Pointer to platform device.
 *
 * return:	0 on success, standard Linux error codes on error.
 *
 * This function is called when the underlying device tree driver
 * removes a platform device, mapped to a Audio packet device.
 */
static int audio_pkt_platform_driver_remove(struct platform_device *adev)
{
	struct audio_pkt_priv *ap_priv = platform_get_drvdata(adev);
	struct audio_pkt_device *audpkt_dev = ap_priv->ap_dev;

	gpr_driver_unregister(&audio_pkt_driver);

	audio_pkt_internal_release(adev);

	if (audpkt_dev) {
		cdev_del(&audpkt_dev->cdev);
		device_destroy(audpkt_dev->audio_pkt_class,audpkt_dev->audio_pkt_major);
		class_destroy(audpkt_dev->audio_pkt_class);
		unregister_chrdev_region(MAJOR(audpkt_dev->audio_pkt_major),
				 MINOR_NUMBER_COUNT);
	}

	//of_platform_depopulate(&adev->dev);
	AUDIO_PKT_INFO("Audio Packet Port Driver Removed\n");

	return 0;

}

static const struct of_device_id audio_pkt_platform_match_table[] = {
	{ .compatible = "qcom,audio-pkt-core-platform"},
	{}
};
MODULE_DEVICE_TABLE(of, audio_pkt_platform_match_table);


static struct platform_driver audio_pkt_core_platform_driver = {
	.probe          = audio_pkt_platform_driver_probe,
	.remove         = audio_pkt_platform_driver_remove,
	.driver         = {
		.name = MODULE_NAME,
		.of_match_table = of_match_ptr(audio_pkt_platform_match_table),
	},
};


static int __init audio_pkt_init(void)
{
	return platform_driver_register(&audio_pkt_core_platform_driver);
}

static void __exit audio_pkt_exit(void)
{
	platform_driver_unregister(&audio_pkt_core_platform_driver);
}
module_init(audio_pkt_init);
module_exit(audio_pkt_exit);

MODULE_DESCRIPTION("MSM Audio Packet Driver");
MODULE_LICENSE("GPL v2");
