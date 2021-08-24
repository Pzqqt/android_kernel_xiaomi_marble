// SPDX-License-Identifier: GPL-2.0-only
/*
* Copyright (c) 2017-2021, The Linux Foundation. All rights reserved.
*/

#include <linux/types.h>	/* u32 */
#include <linux/kernel.h>	/* pr_debug() */
#include <linux/slab.h>		/* kzalloc() */
#include <linux/mutex.h>	/* mutex */
#include <linux/list.h>		/* list_head */
#include <linux/delay.h>	/* msleep */
#include <linux/memory.h>	/* memset */
#include <linux/device.h>	/* device */
#include <linux/cdev.h>		/* cdev_alloc() */
#include <linux/fs.h>		/* alloc_chrdev_region() */
#include <linux/module.h>	/* module_init() */
#include <linux/dma-mapping.h>	/* dma_alloc_coherent() */
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/ipa.h>
#include <linux/sched.h>
#include <linux/skbuff.h>	/* sk_buff */
#include <linux/kfifo.h>  /* Kernel FIFO Implementation */
#include <linux/delay.h> /* msleep() */
#include <linux/string.h>
#include <linux/printk.h>
#include <linux/msm_gsi.h>
#include "gsi.h"
#include "ipa_i.h"
#include "ipa_rm_ut.h"
#include "ipa_test_module.h"

#ifdef INIT_COMPLETION
#define reinit_completion(x) INIT_COMPLETION(*(x))
#endif /* INIT_COMPLETION */


/** Module name string */
#define IPA_TEST_DRV_NAME "ipa_test"

//#define IPA_SUMMING_THRESHOLD 0x10
#define IPA_EVENT_THRESHOLD 0x10
#define IPA_NUM_PIPES 0x24

#define TEST_SIGNATURE 0xfacecafe
#define DFAB_ARB1_HCLK_CTL		(MSM_CLK_CTL_BASE + 0x2564)

#define DESC_FIFO_SZ 0x100
#define DATA_FIFO_SZ 0x2000

#define GSI_CHANNEL_RING_LEN 4096
#define GSI_EVT_RING_LEN 4096

#define TX_NUM_BUFFS 16
#define TX_SZ 32768
#define TX_BUFF_SIZE ((TX_SZ)/(TX_NUM_BUFFS))

#define RX_NUM_BUFFS 16
#define RX_SZ 32768
/* Lowest power of 2 that is bigger than what is used in Ulso test */
#define MAX_ULSO_SEGMENT_SZ 16384
#define RX_SZ_ULSO ((MAX_ULSO_SEGMENT_SZ) * (RX_NUM_BUFFS))
#define RX_BUFF_SIZE ((rx_size)/(RX_NUM_BUFFS))

#define IPA_TEST_DMUX_HEADER_LENGTH           8
#define IPA_TEST_META_DATA_IS_VALID           1
#define IPA_TEST_DMUX_HEADER_META_DATA_OFFSET 4

#define IPA_TEST_ADDITIONAL_HDR_LEN 4

#define IPA_TEST_META_DATA_OFFSET_NONE        0

#define IPA_TEST_HDI_802_HEADER_LENGTH             22
#define IPA_TEST_HDI_802_LENGTH_FIELD_OFFSET       11
#define IPA_TEST_HDI_802_LENGTH_FIELD_OFFSET_VALID  1
#define IPA_TEST_HDI_802_ADD_CONST_LENGTH           0

#define IPA_TEST_HDI_RMNET_HEADER_LENGTH              6
#define IPA_TEST_HDI_RMNET_LENGTH_FIELD_OFFSET        0
#define IPA_TEST_HDI_RMNET_LENGTH_FIELD_OFFSET_VALID  0
#define IPA_TEST_HDI_RMNET_ADD_CONST_LENGTH           0

/* Settings of Exception Handling */
#define RX_DESCRIPTOR_SIZE 2048
#define EXCEPTION_DRV_NAME "ipa_exception_pipe"
#define EXCEPTION_KFIFO_SIZE (8)
#define EXCEPTION_KFIFO_SLEEP_MS (EXCEPTION_KFIFO_SLEEP_MS)
#define EXCEPTION_KFIFO_DEBUG_VERBOSE 1
#define SAVE_HEADER 1

#define IPATEST_DBG(fmt, args...) \
	do { \
		pr_debug(IPA_TEST_DRV_NAME " %s:%d " fmt, __func__, __LINE__, ## args);\
	} while (0)

#define IPATEST_ERR(fmt, args...) \
	do { \
		pr_err(IPA_TEST_DRV_NAME " %s:%d " fmt, __func__, __LINE__, ## args);\
	} while (0)

#define IPATEST_DUMP(fmt, args...) \
	do { \
		pr_debug(fmt, ## args);\
	} while (0)

int ipa_sys_setup(struct ipa_sys_connect_params *sys_in,
		  unsigned long *ipa_gsi_hdl,
		  u32 *ipa_pipe_num, u32 *clnt_hdl, bool en_status);
int ipa_sys_teardown(u32 clnt_hdl);
int ipa_sys_update_gsi_hdls(u32 clnt_hdl, unsigned long gsi_ch_hdl,
	unsigned long gsi_ev_hdl);
struct device *ipa_get_pdev(void);
enum fops_type {
	IPA_TEST_REG_CHANNEL,
	IPA_TEST_DATA_PATH_TEST_CHANNEL,
	IPA_TEST_ULSO_DATA_PATH_TEST_CHANNEL,
	MAX_FOPS
};

struct notify_cb_data_st {
	struct kfifo_rec_ptr_2 exception_kfifo;
};

struct exception_hdl_data {
	struct class *class;
	struct device *dev;
	struct cdev *p_cdev;
	dev_t dev_num;
	struct notify_cb_data_st notify_cb_data;
};

/*struct exception_hdl_data *p_exception_hdl_data = NULL;*/
struct exception_hdl_data *p_exception_hdl_data;

struct ipa_dma_chan {
	u32 dest_pipe_index;
	u32 src_pipe_index;
};

/**
 *  This struct specifies memory buffer properties.
 *
 * @base - Buffer virtual address.
 * @phys_base - Buffer physical address.
 * @size - Specifies buffer size (or maximum size).
 *
 */
//struct ipa_mem_buffer {
//	void *base;
//	phys_addr_t phys_base;
//	u32 size;
//};

struct test_endpoint_sys {
	struct completion xfer_done; /*A completion object for end transfer*/
	struct gsi_chan_props gsi_channel_props;
	struct gsi_evt_ring_props gsi_evt_ring_props;
	bool gsi_valid;
	unsigned long gsi_chan_hdl;
	unsigned long gsi_evt_ring_hdl;
};

#define MAX_CHANNEL_NAME (20)

/* A channel device is the representation of the flow of data from APPS
	to IPA and vice versa. */
struct channel_dev {
	/*OS structures for representation of a channel.*/
	struct class *class;
	dev_t dev_num;
	struct device *dev;
	struct cdev cdev;

	/*The representation of the connection from APPS to GSI/IPA*/
	struct test_endpoint_sys ep;
	/*The representation of the connection from GSI to IPA*/
	struct ipa_dma_chan dma_ep;
	/*The data FIFO for the APPS to IPA*/
	struct ipa_mem_buffer mem;
	/*Index of the next available buffer to use under mem.buff*/
	int mem_buff_index;
	/*A pointer to the test context - should be deleted - TODO*/
	struct test_context *test;

	int index;/*to_ipa_<index>/from_ipa_<index>*/
	char name[MAX_CHANNEL_NAME];
	int rx_pool_inited;/*check... - should be moved to contex?*/
	int ipa_client_hdl;/*returned from ipa_connect*/
};

#define MAX_CHANNEL_DEVS (10)
static struct channel_dev *to_ipa_devs[MAX_CHANNEL_DEVS/2];
/*TODO - legacy*/
static struct channel_dev *from_ipa_devs[MAX_CHANNEL_DEVS/2];

/*This structure holds all the data required for the test module.*/
struct test_context {

	/*OS structures for representation of the test module.*/
	dev_t dev_num;
	struct device *dev;
	struct cdev *cdev;

	/*All channels that are used to read data from
	 * the IPA(Receive channel)*/
	struct channel_dev *rx_channels[MAX_CHANNEL_DEVS/2];

	/*All channels that are used to write data
	 * to the IPA(Transmit channel)*/
	struct channel_dev *tx_channels[MAX_CHANNEL_DEVS/2];
	int num_rx_channels;
	int num_tx_channels;

	/*current test case(-EINVAL is for not-configured)TODO*/
	s32 configuration_idx;
	s32 current_configuration_idx;

	u32 signature;/*Legacy*/
};

/**
 * struct ipa_tx_suspend_private_data - private data for IPA_TX_SUSPEND_IRQ use
 * @clnt_hdl: client handle assigned by IPA
 */
struct ipa_tx_suspend_private_data {
	u32 clnt_hdl;
	u32 gsi_chan_hdl;
};

static struct test_context *ipa_test;

static size_t rx_size;


/**
 * Allocate memory from system memory.
 *
 * @param mem
 */
static void test_alloc_mem(struct ipa_mem_buffer *mem)
{
	dma_addr_t dma_addr;
	struct device *pdev;

	pdev = ipa3_get_pdev();
	/* need to check return value in formal code */
	if (pdev != NULL) {
		mem->base = dma_alloc_coherent(pdev, mem->size, &dma_addr, GFP_KERNEL);
		mem->phys_base = dma_addr;
	}
}

/**
 * Free memory from system memory.
 *
 * @param mem
 */
static void test_free_mem(struct ipa_mem_buffer *mem)
{
	dma_addr_t dma_addr = mem->phys_base;
	struct device *pdev;

	pdev = ipa3_get_pdev();
	if (dma_addr && pdev != NULL)
		dma_free_coherent(pdev, mem->size, mem->base, dma_addr);

	mem->phys_base = 0;
	mem->base = NULL;
}

void print_buff(void *data, size_t size)
{
	u8 bytes_in_line = 16;
	int i, j, num_lines;
	char str[256], tmp[4];

	num_lines = size / bytes_in_line;
	if (size % bytes_in_line > 0)
		num_lines++;

	IPATEST_DBG("Printing buffer at address 0x%px, size = %zu:\n"
		, data, size);
	for (i = 0; i < num_lines; i++) {
		str[0] = '\0';
		for (j = 0; (j < bytes_in_line) &&
			((i * bytes_in_line + j) < size); j++) {
			snprintf(tmp, sizeof(tmp), "%02x ",
					((unsigned char *)data)
					[i * bytes_in_line + j]);
			strlcat(str, tmp, sizeof(str));
		}
		IPATEST_DUMP(": %s\n", str);
	}
}

static int channel_open(struct inode *inode, struct file *filp)
{
	struct channel_dev *channel_dev;

	/* Get the channel device data */
	channel_dev = container_of(inode->i_cdev, struct channel_dev, cdev);

	IPATEST_DBG("channel_dev address = 0x%px\n", channel_dev);

	filp->private_data = channel_dev;

	return 0;
}

int insert_descriptors_into_rx_endpoints(u32 count)
{
	struct channel_dev *rx_channel = NULL;
	int i, j, res = 0;

	/* Insert a descriptor into the receiving end(s) */
	for (i = 0; i < ipa_test->num_rx_channels; i++) {
		rx_channel = ipa_test->rx_channels[i];
		if (!rx_channel->rx_pool_inited) {
			res = 0;
			for (j = 0; j < RX_NUM_BUFFS; j++) {
				struct gsi_xfer_elem gsi_xfer;

				memset(&gsi_xfer, 0, sizeof(gsi_xfer));
				gsi_xfer.addr = rx_channel->mem.phys_base + j * count;
				gsi_xfer.flags |= GSI_XFER_FLAG_EOT;
				gsi_xfer.len = count;
				gsi_xfer.type = GSI_XFER_ELEM_DATA;
				gsi_xfer.xfer_user_data = (void*)(rx_channel->mem.phys_base + j * count);

				IPATEST_DBG("submitting credit to gsi\n");
				res |= gsi_queue_xfer(rx_channel->ep.gsi_chan_hdl, 1, &gsi_xfer, true);
				if (res) {
					IPATEST_ERR("gsi_queue_xfer failed %d\n", res);
					return -EFAULT;
				}
			}

			if (res == 0)
				rx_channel->rx_pool_inited = 1;
		}
	}

	return res;
}

static ssize_t channel_write_gsi(struct file *filp, const char __user *buf,
	size_t count, loff_t *f_pos)
{
	struct channel_dev *channel_dev = filp->private_data;
	int res = 0;
	void *data_address = channel_dev->mem.base
		+ channel_dev->mem_buff_index * TX_BUFF_SIZE;
	u32 data_phys_addr = channel_dev->mem.phys_base
		+ channel_dev->mem_buff_index * TX_BUFF_SIZE;
	struct gsi_xfer_elem gsi_xfer;

	if (count > (RX_BUFF_SIZE))
		IPATEST_ERR("-----PROBLEM----- count=%zu RX_BUFF_SIZE=%d\n",
		count, RX_BUFF_SIZE);

	/* Copy the data from the user and transmit */
	res = copy_from_user(data_address, buf, count);
	if (res) {
		IPATEST_ERR("copy_from_user() failure.\n");
		return -EINVAL;
	}

	/* Print the data */
	print_buff(data_address, count);

	IPATEST_DBG("-----Start Transfer-----\n");

	/* Transmit */
	memset(&gsi_xfer, 0, sizeof(gsi_xfer));
	gsi_xfer.addr = data_phys_addr;
	gsi_xfer.flags |= GSI_XFER_FLAG_EOT;
	gsi_xfer.len = count;
	gsi_xfer.type = GSI_XFER_ELEM_DATA;

	IPATEST_DBG("sending a packet to gsi\n");
	res = gsi_queue_xfer(channel_dev->ep.gsi_chan_hdl, 1, &gsi_xfer, true);
	if (res != GSI_STATUS_SUCCESS) {
		IPATEST_ERR("GSI xfer failed %d\n", res);
		return res;
	}

	channel_dev->mem_buff_index = (channel_dev->mem_buff_index + 1) %
					TX_NUM_BUFFS;
	return count;
}

static ssize_t channel_read_gsi(struct file *filp, char __user *buf,
	size_t count, loff_t *f_pos)
{
	struct channel_dev *channel_dev = filp->private_data;
	int res = 0;
	int i;
	phys_addr_t offset = 0;
	struct gsi_chan_xfer_notify xfer_notify;
	int max_retry = 10;
	struct gsi_xfer_elem gsi_xfer;

	IPATEST_DBG("size to read = %zu\n", count);
	for (i = 0; i < max_retry; i++) {
		res = gsi_poll_channel(channel_dev->ep.gsi_chan_hdl,
			&xfer_notify);

		if (res != GSI_STATUS_SUCCESS && res != GSI_STATUS_POLL_EMPTY) {
			IPATEST_ERR("gsi_poll_channel failed %d\n", res);
			return res;
		}
		if (res == GSI_STATUS_SUCCESS)
			break;

		IPATEST_DBG("channel empty %d/%d\n", i + 1, max_retry);
		msleep(5);
	}

	if (i == max_retry) {
		IPATEST_ERR("transfer not completed.\n");
		return 0;
	}

	IPATEST_DBG("received %d bytes from 0x%px.\n",
		xfer_notify.bytes_xfered, xfer_notify.xfer_user_data);

	/* Copy the received data to the user buffer */
	offset = (phys_addr_t)xfer_notify.xfer_user_data - channel_dev->mem.phys_base;
	res = copy_to_user(buf,
		channel_dev->mem.base + offset,
		xfer_notify.bytes_xfered);
	if (res < 0) {
		IPATEST_ERR("copy_to_user() failed.\n");
		return 0;
	}

	/* Re-insert the descriptor back to pipe */
	memset(&gsi_xfer, 0, sizeof(gsi_xfer));
	gsi_xfer.addr = (phys_addr_t)xfer_notify.xfer_user_data;
	gsi_xfer.flags |= GSI_XFER_FLAG_EOT;
	gsi_xfer.len = RX_BUFF_SIZE;
	gsi_xfer.type = GSI_XFER_ELEM_DATA;
	gsi_xfer.xfer_user_data = xfer_notify.xfer_user_data;

	IPATEST_DBG("submitting credit to gsi\n");
	res = gsi_queue_xfer(channel_dev->ep.gsi_chan_hdl, 1, &gsi_xfer, true);
	if (res) {
		IPATEST_ERR("gsi_queue_xfer failed %d\n", res);
		return 0;
	}

	IPATEST_DBG("Returning %d.\n", xfer_notify.bytes_xfered);
	return xfer_notify.bytes_xfered;
}

static ssize_t channel_write(struct file *filp, const char __user *buf,
	size_t count, loff_t *f_pos)
{
	return channel_write_gsi(filp, buf, count, f_pos);
}

static ssize_t channel_read(struct file *filp, char __user *buf,
	size_t count, loff_t *f_pos)
{
	return channel_read_gsi(filp, buf, count, f_pos);
}

static const struct file_operations channel_dev_fops = {
	.owner = THIS_MODULE,
	.open = channel_open,
	.write = channel_write,
	.read = channel_read,
};

static ssize_t set_skb_for_user(struct file *filp, char __user *buf,
		       size_t size, loff_t *p_pos);

static ssize_t get_skb_from_user(struct file *filp, const char __user *buf,
		       size_t size, loff_t *f_pos);

static ssize_t get_ulso_skb_from_user(struct file *filp, const char __user *buf,
		       size_t size, loff_t *f_pos);

static const struct file_operations data_path_fops = {
	.owner = THIS_MODULE,
	.open = channel_open,
	.read = set_skb_for_user,
	.write = get_skb_from_user,
};

static const struct file_operations ulso_data_path_fops = {
	.owner = THIS_MODULE,
	.open = channel_open,
	.read = set_skb_for_user,
	.write = get_ulso_skb_from_user,
};

/*
 * This will create the char device named
 * "<dev_name>_<index>" and allocate data
 * FIFO(size mem_size).
 * In this case, we will differentiate
 * channel_dev_fops, which are used for
 * regular data transmission
 * in all the tests,and data_path_fops
 * which will be used
 * in DataPath tests for handling
 * the SKB we will transfer
 */
int create_channel_device_by_type(
			const int index,
			const char *dev_name,
			struct channel_dev **channel_dev_ptr,
			size_t mem_size,
			enum fops_type type)
{
	int ret;
	char name[MAX_CHANNEL_NAME];
	struct channel_dev *channel_dev = NULL;

	scnprintf(name, sizeof(name), "%s_%d", dev_name, index);

	IPATEST_DBG(":Creating channel %d device, name = %s.\n",
		index, name);

	/* Allocate memory for the device */
	*channel_dev_ptr = kzalloc(sizeof(struct channel_dev), GFP_KERNEL);
	if (NULL == *channel_dev_ptr) {
		IPATEST_ERR("kzalloc err for channel dev\n");
		ret = -ENOMEM;
		goto create_channel_device_failure;
	}

	channel_dev = *channel_dev_ptr;

	strlcpy(channel_dev->name, name, MAX_CHANNEL_NAME);

	/* Allocate memory data buffer for the pipe */
	IPATEST_DBG(":-----Allocate memory data buffer-----\n");
	channel_dev->mem.size = mem_size;
	test_alloc_mem(&channel_dev->mem);
	if (NULL == channel_dev->mem.base) {
		IPATEST_ERR("data fifo alloc fail\n");
		ret = -ENOMEM;
		goto create_channel_device_failure;
	}
	IPATEST_DBG(": data fifo: mem phys=0x%pa.virt=0x%px.\n",
		&channel_dev->mem.phys_base, channel_dev->mem.base);
	memset(channel_dev->mem.base, 0xbb, channel_dev->mem.size);

	channel_dev->mem_buff_index = 0;

	/* Add a pointer from the channel device to the test context info */
	channel_dev->test = ipa_test;

	channel_dev->class = class_create(THIS_MODULE, channel_dev->name);
	if (IS_ERR(channel_dev->class)) {
		IPATEST_ERR(":class_create() err.\n");
		ret = -ENOMEM;
		goto create_class_failure;
	}

	ret = alloc_chrdev_region(&channel_dev->dev_num, 0, 1, channel_dev->name);
	if (ret) {
		IPATEST_ERR("alloc_chrdev_region err.\n");
		ret = -ENOMEM;
		goto alloc_chrdev_failure;
	}

	channel_dev->dev = device_create(channel_dev->class, NULL,
		channel_dev->dev_num, channel_dev, channel_dev->name);
	if (IS_ERR(channel_dev->dev)) {
		IPATEST_ERR("device_create err.\n");
		ret = -ENODEV;
		goto device_create_failure;
	}
	switch (type) {
	case IPA_TEST_REG_CHANNEL:
		cdev_init(&channel_dev->cdev, &channel_dev_fops);
		break;
	case IPA_TEST_DATA_PATH_TEST_CHANNEL:
		cdev_init(&channel_dev->cdev, &data_path_fops);
		break;
	case IPA_TEST_ULSO_DATA_PATH_TEST_CHANNEL:
		cdev_init(&channel_dev->cdev, &ulso_data_path_fops);
		break;
	default:
		IPATEST_ERR("Wrong fops type");
		ret = -EINVAL;
		goto invalid_type_err;
	}
	channel_dev->cdev.owner = THIS_MODULE;

	ret = cdev_add(&channel_dev->cdev, channel_dev->dev_num, 1);
	if (ret) {
		IPATEST_ERR("cdev_add err=%d\n", -ret);
		ret = -ENODEV;
		goto cdev_add_failure;
	}

	if (!ret)
		IPATEST_DBG("Channel device:%d, name:%s created, address:0x%px.\n",
			index, channel_dev->name, channel_dev);

	return 0;

cdev_add_failure:
	memset(&channel_dev->cdev, 0, sizeof(channel_dev->cdev));
invalid_type_err:
	device_destroy(channel_dev->class, channel_dev->dev_num);
device_create_failure:
	unregister_chrdev_region(channel_dev->dev_num, 1);
alloc_chrdev_failure:
	class_destroy(channel_dev->class);
create_class_failure:
	test_free_mem(&channel_dev->mem);
create_channel_device_failure:
	kfree(channel_dev);
	IPATEST_ERR("Channel device %d, name %s creation FAILED.\n",
		index, channel_dev->name);

	return ret;
}

int create_channel_device(const int index,
			  const char *dev_name,
			  struct channel_dev **channel_dev_ptr,
			  size_t mem_size) {
	return create_channel_device_by_type(
			index,
			dev_name,
			channel_dev_ptr,
			mem_size,
			IPA_TEST_REG_CHANNEL);
}

/*
 * DataPath test definitions:
 */

#define MAX_TEST_SKB 15
#define TIME_OUT_TIME 2000 /* 2 seconds */

struct datapath_ctx {
	struct mutex lock;
	struct kfifo_rec_ptr_2 fifo;
	struct completion write_done_completion;
	struct completion ipa_receive_completion;
};

struct datapath_ctx *p_data_path_ctx;

bool init_write_done_completion;


/*
 * Inits the kfifo needed for the
 * DataPath tests
 */
int datapath_ds_init(void)
{
	int res = 0;
	rx_size = RX_SZ;

	p_data_path_ctx = kzalloc(sizeof(struct datapath_ctx), GFP_KERNEL);
	if (!p_data_path_ctx) {
		IPATEST_ERR("kzalloc returned error (%d)\n", res);
		return res;
	}
	IPATEST_DBG("called.\n");

	res = kfifo_alloc(&p_data_path_ctx->fifo
			, (sizeof(struct sk_buff *)*MAX_TEST_SKB)
			, GFP_KERNEL);
	if (0 != res) {
		IPATEST_ERR("kfifo_alloc returned error (%d)\n", res);
		kfree(p_data_path_ctx);
		return res;
	}

	mutex_init(&p_data_path_ctx->lock);
	init_completion(&p_data_path_ctx->ipa_receive_completion);

	IPATEST_DBG("completed.(%d)\n", res);

	return res;
}

static struct sk_buff *datapath_create_skb(const char *buf, size_t size)
{
	struct sk_buff *skb;
	unsigned char *data;
	int err = 0;

	IPATEST_DBG("allocating SKB, len=%zu", size);
	skb = alloc_skb(size, GFP_KERNEL);
	if (unlikely(!skb))
		return NULL;
	IPATEST_DBG("skb allocated, skb->len=%d", skb->len);
	data = skb_put(skb, size);
	if (unlikely(!data)) {
		kfree_skb(skb);
		return NULL;
	}
	IPATEST_DBG("skb put finish, skb->len=%d", skb->len);
	skb->csum = csum_and_copy_from_user(
			buf, data,
			size, 0, &err);
	if (err) {
		kfree_skb(skb);
		return NULL;
	}
	IPATEST_DBG("The following packet was created:\n");
	print_buff(skb->data, size);
	IPATEST_DBG("Exiting\n");

	return skb;
}

static struct sk_buff *ulso_create_skb(const char *buf, size_t size)
{
	struct sk_buff *skb;

	IPATEST_DBG("Entering\n");

	skb = datapath_create_skb(buf, size);
	if (unlikely(!skb))
		return NULL;

	/* Mark the skb as gso skb */
	skb_increase_gso_size(skb_shinfo(skb), 1);

	IPATEST_DBG("Exiting\n");

	return skb;
}

static int datapath_read_data(void *element, int size)
{
	int res;

	IPATEST_DBG("Entering\n");

	mutex_lock(&p_data_path_ctx->lock);
	reinit_completion(&p_data_path_ctx->ipa_receive_completion);
	IPATEST_DBG("Init completion\n");
	IPATEST_DBG("Checking if kfifo is empty\n");
	if (kfifo_is_empty(&p_data_path_ctx->fifo)) {
		IPATEST_DBG("kfifo is empty\n");
		mutex_unlock(&p_data_path_ctx->lock);
		IPATEST_DBG("wait_for_ipa_receive_completion\n");
		res = wait_for_completion_timeout(
			&p_data_path_ctx->ipa_receive_completion,
			msecs_to_jiffies(TIME_OUT_TIME));
		IPATEST_DBG("came back from wait_for_completion_timeout\n");
		if (!res) {
			IPATEST_ERR("Error in wait_for_ipa_receive_completion\n");
			return -EINVAL;
		}
		IPATEST_DBG("locking lock\n");
		mutex_lock(&p_data_path_ctx->lock);
	}
	res = kfifo_out(&p_data_path_ctx->fifo, element, size);
	if (res != size) {
		IPATEST_ERR("Error in taking out an element\n");
		IPATEST_ERR("unlocking lock\n");
		mutex_unlock(&p_data_path_ctx->lock);
		return -EINVAL;
	}
	IPATEST_DBG("took %d bytes out\n", res);
	IPATEST_DBG("unlocking lock\n");
	IPATEST_DBG("Exiting\n");
	mutex_unlock(&p_data_path_ctx->lock);
	return res;
}

static int datapath_write_fifo(
		void *element,
		int size) {

	int res;

	IPATEST_DBG("Entering\n");
	mutex_lock(&p_data_path_ctx->lock);
	IPATEST_DBG("Mutex locked\n");
	IPATEST_DBG("putting %px into fifo\n", element);
	res = kfifo_in(&p_data_path_ctx->fifo, &element, size);
	IPATEST_DBG("finished kfifo in\n");
	if (res != size) {
		IPATEST_ERR("Error in saving element\n");
		return -EINVAL;
	}
	IPATEST_DBG("Mutex unlocked\n");

	complete(&p_data_path_ctx->ipa_receive_completion);
	IPATEST_DBG("Completed ipa_receive_completion\n");
	mutex_unlock(&p_data_path_ctx->lock);
	return 0;
}

/*
 * Receives from the user space the buff,
 * create an SKB, and send it through
 * ipa_tx_dp that was received in the system
 */
static ssize_t get_skb_from_user(struct file *filp, const char __user *buf,
		       size_t size, loff_t *f_pos) {

	int res = 0;
	struct sk_buff *skb;

	IPATEST_DBG("Entering\n");
	/* Copy the data from the user and transmit */
	IPATEST_DBG("-----Copy the data from the user-----\n");

	IPATEST_DBG("Creating SKB\n");

	skb = datapath_create_skb(buf, size);
	if (!skb)
		return -EINVAL;
	init_completion(&p_data_path_ctx->write_done_completion);
	IPATEST_DBG(
		"Starting transfer through ipa_tx_dp\n");
	res = ipa_tx_dp(IPA_CLIENT_TEST_CONS, skb,
			       NULL);
	IPATEST_DBG("ipa_tx_dp res = %d.\n", res);
	res = wait_for_completion_timeout(
			&p_data_path_ctx->write_done_completion,
			msecs_to_jiffies(TIME_OUT_TIME));
	IPATEST_DBG("timeout result = %d", res);
	if (!res)
		return -EINVAL;
	IPATEST_DBG("-----Exiting-----\n");

	return size;
}

/*
 * Receives from the user space the buff,
 * create an SKB, and send it through
 * ipa_tx_dp that was received in the system
 */
static ssize_t get_ulso_skb_from_user(struct file *filp,
const char __user *buf, size_t size, loff_t *f_pos) {
	int res = 0;
	struct sk_buff *skb;

	IPATEST_DBG("Entering\n");
	/* Copy the data from the user and transmit */
	IPATEST_DBG("-----Copy the data from the user-----\n");
	IPATEST_DBG("Creating SKB\n");
	skb = ulso_create_skb(buf, size);
	if (!skb)
		return -EINVAL;

	if (!init_write_done_completion) {
		init_completion(&p_data_path_ctx->write_done_completion);
		init_write_done_completion = true;
	} else {
		reinit_completion(&p_data_path_ctx->write_done_completion);
	}

	IPATEST_DBG("Starting transfer through ipa_tx_dp\n");
	res = ipa_tx_dp(IPA_CLIENT_TEST_CONS, skb, NULL);
	IPATEST_DBG("ipa_tx_dp res = %d.\n", res);
	res = wait_for_completion_timeout(
		&p_data_path_ctx->write_done_completion,
		msecs_to_jiffies(TIME_OUT_TIME));
	IPATEST_DBG("timeout result = %d", res);
	if (!res)
		return -EINVAL;
	IPATEST_DBG("-----Exiting-----\n");

	return size;
}

/*
 * Sends the user space the next SKB
 * that was received in the system
 */

static ssize_t set_skb_for_user(struct file *filp, char __user *buf,
		       size_t size, loff_t *p_pos)
{
	int res;
	struct sk_buff *p_skb;

	IPATEST_DBG("Entering\n");
	/* Copy the result to the user buffer */

	IPATEST_DBG("datapath_read_data\n");
	if (datapath_read_data(
			(void *)&p_skb,
			sizeof(struct sk_buff *)) < 0) {
		IPATEST_ERR("error in datapath_read_data\n");
		return -EINVAL;
	}
	print_buff(p_skb->data, size);
	IPATEST_DBG("Copying data back to user\n");
	res = copy_to_user(buf, p_skb->data, size);
	kfree_skb(p_skb);
	/* Return the number of bytes copied to the user */

	return res;
}

static void datapath_ds_clean(void)
{
	kfifo_reset(&p_data_path_ctx->fifo);
}

/*
 * Destroy the kfifo needed for the
 * DataPath tests
 */

static void datapath_exit(void)
{
	kfifo_free(&p_data_path_ctx->fifo);
	/* freeing kfifo */
	kfree(p_data_path_ctx);
	p_data_path_ctx = NULL;
}

/*
 * CB func. for the IPA_WRITE_DONE
 * event. Used in IpaTxDpTest
 */

static void notify_ipa_write_done(
		void *priv,
		enum ipa_dp_evt_type evt,
		unsigned long data) {

	IPATEST_DBG("Entering function\n");

	if (evt == IPA_WRITE_DONE) {
		IPATEST_DBG("evt IPA_WRITE_DONE\n");
		IPATEST_DBG("Printing received buff from IPA\n");
		print_buff(
			((struct sk_buff *)data)->data,
			((struct sk_buff *)data)->len);

		kfree_skb((struct sk_buff *)data);
		complete(&p_data_path_ctx->write_done_completion);

	} else {
		IPATEST_DBG("Error, wrong event %d\n", evt);
	}
}

/*
 * CB func. for the IPA_RECEIVE
 * event. Used in IPAToAppsTest
 */

static void notify_ipa_received(
		void *priv,
		enum ipa_dp_evt_type evt,
		unsigned long data){

	struct sk_buff *p_skb = (struct sk_buff *)data;

	IPATEST_DBG("Entering function\n");

	if (evt == IPA_RECEIVE) {
		IPATEST_DBG("evt IPA_RECEIVE\n");
		IPATEST_DBG("Printing received buff from IPA\n");
		print_buff(p_skb->data, p_skb->len);
		datapath_write_fifo(p_skb, sizeof(struct sk_buff *));
	} else {
		IPATEST_ERR("Error: wrong event %d", evt);
	}
}

static void ipa_test_gsi_evt_ring_err_cb(struct gsi_evt_err_notify *notify)
{
	if (notify) {
		switch (notify->evt_id) {
		case GSI_EVT_OUT_OF_BUFFERS_ERR:
			IPATEST_ERR("Received GSI_EVT_OUT_OF_BUFFERS_ERR\n");
			break;
		case GSI_EVT_OUT_OF_RESOURCES_ERR:
			IPATEST_ERR("Received GSI_EVT_OUT_OF_RESOURCES_ERR\n");
			break;
		case GSI_EVT_UNSUPPORTED_INTER_EE_OP_ERR:
			IPATEST_ERR("Received GSI_EVT_UNSUPPORTED_INTER_EE_OP_ERR\n");
			break;
		case GSI_EVT_EVT_RING_EMPTY_ERR:
			IPATEST_ERR("Received GSI_EVT_EVT_RING_EMPTY_ERR\n");
			break;
		default:
			IPATEST_ERR("Unexpected err evt: %d\n", notify->evt_id);
		}
	}
	return;
}

static void ipa_test_gsi_chan_err_cb(struct gsi_chan_err_notify *notify)
{
	if (notify) {
		switch (notify->evt_id) {
		case GSI_CHAN_INVALID_TRE_ERR:
			IPATEST_ERR("Received GSI_CHAN_INVALID_TRE_ERR\n");
			break;
		case GSI_CHAN_NON_ALLOCATED_EVT_ACCESS_ERR:
			IPATEST_ERR("Received GSI_CHAN_NON_ALLOCATED_EVT_ACCESS_ERR\n");
			break;
		case GSI_CHAN_OUT_OF_BUFFERS_ERR:
			IPATEST_ERR("Received GSI_CHAN_OUT_OF_BUFFERS_ERR\n");
			break;
		case GSI_CHAN_OUT_OF_RESOURCES_ERR:
			IPATEST_ERR("Received GSI_CHAN_OUT_OF_RESOURCES_ERR\n");
			break;
		case GSI_CHAN_UNSUPPORTED_INTER_EE_OP_ERR:
			IPATEST_ERR("Received GSI_CHAN_UNSUPPORTED_INTER_EE_OP_ERR\n");
			break;
		case GSI_CHAN_HWO_1_ERR:
			IPATEST_ERR("Received GSI_CHAN_HWO_1_ERR\n");
			break;
		default:
			IPATEST_ERR("Unexpected err ch: %d\n", notify->evt_id);
		}
	}
	return;
}

static void ipa_test_gsi_irq_notify_cb(struct gsi_chan_xfer_notify *notify)
{
	IPATEST_DBG("ipa_test_gsi_irq_notify_cb: channel 0x%px xfer 0x%px\n", notify->chan_user_data, notify->xfer_user_data);
	IPATEST_DBG("ipa_test_gsi_irq_notify_cb: event %d notified\n", notify->evt_id);
	IPATEST_DBG("ipa_test_gsi_irq_notify_cb: bytes_xfered %d\n", notify->bytes_xfered);
}

int connect_ipa_to_apps(struct test_endpoint_sys *rx_ep,
			   enum ipa_client_type client,
			   u32 pipe_index,
			   unsigned long ipa_gsi_hdl)
{
	int res = 0;

	dma_addr_t dma_addr;
	const struct ipa_gsi_ep_config *gsi_ep_config;
	struct device *pdev;

	pdev = ipa3_get_pdev();
	if (!pdev) {
		IPATEST_ERR("IPA module not initialized\n");
		return -EINVAL;
	}

	memset(&rx_ep->gsi_evt_ring_props, 0, sizeof(rx_ep->gsi_evt_ring_props));
	rx_ep->gsi_evt_ring_props.intf = GSI_EVT_CHTYPE_GPI_EV;
	rx_ep->gsi_evt_ring_props.intr = GSI_INTR_IRQ;
	rx_ep->gsi_evt_ring_props.re_size =
		GSI_EVT_RING_RE_SIZE_16B;

	rx_ep->gsi_evt_ring_props.ring_len = GSI_EVT_RING_LEN;
	rx_ep->gsi_evt_ring_props.ring_base_vaddr =
		dma_alloc_coherent(pdev, GSI_EVT_RING_LEN,
		&dma_addr, 0);
	rx_ep->gsi_evt_ring_props.ring_base_addr = dma_addr;

	rx_ep->gsi_evt_ring_props.int_modt = 3200; //0.1s under 32KHz clock
	rx_ep->gsi_evt_ring_props.int_modc = 1;
	rx_ep->gsi_evt_ring_props.rp_update_addr = 0;
	rx_ep->gsi_evt_ring_props.exclusive = true;
	rx_ep->gsi_evt_ring_props.err_cb = ipa_test_gsi_evt_ring_err_cb;
	rx_ep->gsi_evt_ring_props.user_data = NULL;

	res = gsi_alloc_evt_ring(&rx_ep->gsi_evt_ring_props, ipa_gsi_hdl,
		&rx_ep->gsi_evt_ring_hdl);
	if (res != GSI_STATUS_SUCCESS) {
		IPATEST_ERR("gsi_alloc_evt_ring failed %d\n", res);
		return -EFAULT;
	}

	memset(&rx_ep->gsi_channel_props, 0,
		sizeof(rx_ep->gsi_channel_props));
	rx_ep->gsi_channel_props.prot = GSI_CHAN_PROT_GPI;
	rx_ep->gsi_channel_props.dir = GSI_CHAN_DIR_FROM_GSI;
	gsi_ep_config = ipa_get_gsi_ep_info(client);
	if (!gsi_ep_config) {
		IPATEST_ERR("invalid gsi_ep_config\n");
		return -EFAULT;
	}
	rx_ep->gsi_channel_props.ch_id =
		gsi_ep_config->ipa_gsi_chan_num;
	rx_ep->gsi_channel_props.evt_ring_hdl = rx_ep->gsi_evt_ring_hdl;
	rx_ep->gsi_channel_props.re_size = GSI_CHAN_RE_SIZE_16B;

	rx_ep->gsi_channel_props.ring_len = GSI_CHANNEL_RING_LEN;
	rx_ep->gsi_channel_props.ring_base_vaddr =
		dma_alloc_coherent(pdev, GSI_CHANNEL_RING_LEN,
		&dma_addr, 0);
	if (!rx_ep->gsi_channel_props.ring_base_vaddr) {
		IPATEST_ERR("connect_ipa_to_apps: falied to alloc GSI ring\n");
		return -EFAULT;
	}

	rx_ep->gsi_channel_props.ring_base_addr = dma_addr;
	rx_ep->gsi_channel_props.use_db_eng = GSI_CHAN_DIRECT_MODE;
	rx_ep->gsi_channel_props.max_prefetch = GSI_ONE_PREFETCH_SEG;
	rx_ep->gsi_channel_props.low_weight = 1;
	rx_ep->gsi_channel_props.chan_user_data = rx_ep;
	if (ipa_get_hw_type() >= IPA_HW_v4_9)
		rx_ep->gsi_channel_props.db_in_bytes = 1;

	rx_ep->gsi_channel_props.err_cb = ipa_test_gsi_chan_err_cb;
	rx_ep->gsi_channel_props.xfer_cb = ipa_test_gsi_irq_notify_cb;
	res = gsi_alloc_channel(&rx_ep->gsi_channel_props, ipa_gsi_hdl,
		&rx_ep->gsi_chan_hdl);
	if (res != GSI_STATUS_SUCCESS) {
		IPATEST_ERR("gsi_alloc_channel failed %d\n", res);
		return -EFAULT;
	}
	rx_ep->gsi_valid = true;

	res = ipa3_sys_update_gsi_hdls(pipe_index, rx_ep->gsi_chan_hdl, rx_ep->gsi_evt_ring_hdl);
	if (res) {
		IPATEST_ERR("ipa_sys_update_gsi_hdls failed %d\n", res);
		return -EFAULT;
	}

	res = gsi_start_channel(rx_ep->gsi_chan_hdl);
	if (res != GSI_STATUS_SUCCESS) {
		IPATEST_ERR("gsi_start_channel failed %d\n", res);
		return -EFAULT;
	}

	IPATEST_DBG("setting channel to polling mode\n");
	res = gsi_config_channel_mode(rx_ep->gsi_chan_hdl, GSI_CHAN_MODE_POLL);
	if (res != GSI_STATUS_SUCCESS) {
		IPATEST_ERR("gsi_config_channel_mode failed %d\n", res);
		return -EFAULT;
	}

	return 0;
}

int connect_apps_to_ipa(struct test_endpoint_sys *tx_ep,
			   enum ipa_client_type client,
			   u32 pipe_index,
			   struct ipa_mem_buffer *desc_fifo,
			   unsigned long ipa_gsi_hdl)
{
	int res = 0;
	dma_addr_t dma_addr;
	const struct ipa_gsi_ep_config *gsi_ep_config;
	struct device *pdev;

	pdev = ipa3_get_pdev();
	if (!pdev) {
		IPATEST_ERR("IPA module not initialized\n");
		return -EINVAL;
	}

	memset(&tx_ep->gsi_evt_ring_props, 0, sizeof(tx_ep->gsi_evt_ring_props));
	tx_ep->gsi_evt_ring_props.intf = GSI_EVT_CHTYPE_GPI_EV;
	tx_ep->gsi_evt_ring_props.intr = GSI_INTR_IRQ;
	tx_ep->gsi_evt_ring_props.re_size =
		GSI_EVT_RING_RE_SIZE_16B;

	tx_ep->gsi_evt_ring_props.ring_len = GSI_EVT_RING_LEN;
	tx_ep->gsi_evt_ring_props.ring_base_vaddr =
		dma_alloc_coherent(pdev, GSI_EVT_RING_LEN,
		&dma_addr, 0);
	tx_ep->gsi_evt_ring_props.ring_base_addr = dma_addr;

	tx_ep->gsi_evt_ring_props.int_modt = 3200; //0.1s under 32KHz clock
	tx_ep->gsi_evt_ring_props.int_modc = 1;
	tx_ep->gsi_evt_ring_props.rp_update_addr = 0;
	tx_ep->gsi_evt_ring_props.exclusive = true;
	tx_ep->gsi_evt_ring_props.err_cb = ipa_test_gsi_evt_ring_err_cb;
	tx_ep->gsi_evt_ring_props.user_data = NULL;

	res = gsi_alloc_evt_ring(&tx_ep->gsi_evt_ring_props, ipa_gsi_hdl,
		&tx_ep->gsi_evt_ring_hdl);
	if (res != GSI_STATUS_SUCCESS) {
		IPATEST_ERR("gsi_alloc_evt_ring failed %d\n", res);
		return -EFAULT;
	}

	memset(&tx_ep->gsi_channel_props, 0, sizeof(tx_ep->gsi_channel_props));
	tx_ep->gsi_channel_props.prot = GSI_CHAN_PROT_GPI;
	tx_ep->gsi_channel_props.dir = GSI_CHAN_DIR_TO_GSI;
	gsi_ep_config = ipa_get_gsi_ep_info(client);
	if (!gsi_ep_config) {
		IPATEST_ERR("invalid gsi_ep_config\n");
		return -EFAULT;
	}
	tx_ep->gsi_channel_props.ch_id =
		gsi_ep_config->ipa_gsi_chan_num;
	tx_ep->gsi_channel_props.evt_ring_hdl = tx_ep->gsi_evt_ring_hdl;
	tx_ep->gsi_channel_props.re_size = GSI_CHAN_RE_SIZE_16B;

	tx_ep->gsi_channel_props.ring_len = GSI_CHANNEL_RING_LEN;
	tx_ep->gsi_channel_props.ring_base_vaddr =
		dma_alloc_coherent(pdev, GSI_CHANNEL_RING_LEN,
		&dma_addr, 0);
	if (!tx_ep->gsi_channel_props.ring_base_vaddr) {
		IPATEST_ERR("connect_apps_to_ipa: falied to alloc GSI ring\n");
		return -EFAULT;
	}

	tx_ep->gsi_channel_props.ring_base_addr = dma_addr;
	tx_ep->gsi_channel_props.use_db_eng = GSI_CHAN_DIRECT_MODE;
	tx_ep->gsi_channel_props.max_prefetch = GSI_ONE_PREFETCH_SEG;
	tx_ep->gsi_channel_props.low_weight = 1;
	tx_ep->gsi_channel_props.chan_user_data = tx_ep;
	if (ipa_get_hw_type() >= IPA_HW_v4_9)
		tx_ep->gsi_channel_props.db_in_bytes = 1; 

	tx_ep->gsi_channel_props.err_cb = ipa_test_gsi_chan_err_cb;
	tx_ep->gsi_channel_props.xfer_cb = ipa_test_gsi_irq_notify_cb;
	res = gsi_alloc_channel(&tx_ep->gsi_channel_props, ipa_gsi_hdl,
		&tx_ep->gsi_chan_hdl);
	if (res != GSI_STATUS_SUCCESS) {
		IPATEST_ERR("gsi_alloc_channel failed %d\n", res);
		return -EFAULT;
	}
	tx_ep->gsi_valid = true;

	res = ipa3_sys_update_gsi_hdls(pipe_index, tx_ep->gsi_chan_hdl, tx_ep->gsi_evt_ring_hdl);
	if (res) {
		IPATEST_ERR("ipa_sys_update_gsi_hdls failed %d\n", res);
		return -EFAULT;
	}

	res = gsi_start_channel(tx_ep->gsi_chan_hdl);
	if (res != GSI_STATUS_SUCCESS) {
		IPATEST_ERR("gsi_start_channel failed %d\n", res);
		return -EFAULT;
	}

	return 0;
}

int configure_ipa_endpoint(struct ipa_ep_cfg *ipa_ep_cfg,
			   enum ipa_mode_type mode)
{
	const char *DEFAULT_ROUTING_TABLE_NAME = "LAN";
	const enum ipa_client_type DEFAULT_CLIENT = IPA_CLIENT_TEST4_CONS;
	struct ipa_ioc_add_rt_rule *rt_rule;
	struct ipa_rt_rule_add *rt_rule_entry;
	struct ipa_ioc_get_rt_tbl rt_lookup;

	memset(ipa_ep_cfg, 0, sizeof(*ipa_ep_cfg));
	/* Configure mode */
	ipa_ep_cfg->mode.mode = mode;
	/* Add default routing rule */
	rt_rule = kzalloc(sizeof(struct ipa_ioc_add_rt_rule) +
			1 * sizeof(struct ipa_rt_rule_add), GFP_KERNEL);
	if (!rt_rule) {
		IPATEST_ERR("Allocation failure.\n");
		return -EINVAL;
	}
	rt_rule->commit = 1;
	rt_rule->num_rules = 1;
	rt_rule->ip = IPA_IP_v4;
	strlcpy(rt_rule->rt_tbl_name,
			DEFAULT_ROUTING_TABLE_NAME,
			IPA_RESOURCE_NAME_MAX);
	rt_rule_entry = &rt_rule->rules[0];
	rt_rule_entry->at_rear = 1;
	rt_rule_entry->rule.dst = DEFAULT_CLIENT;
	/* Issuing a routing rule without
	 * any equations will cause a default rule
	   which catches every packet and sends it
	   to the default endpoint. */
	if (ipa_add_rt_rule(rt_rule)) {
		IPATEST_ERR("ipa_add_rt_rule() failure.\n");
	}
	IPATEST_DBG("rt rule hdl1=%x.\n", rt_rule_entry->rt_rule_hdl);
	/*	At this point, there is a routing table in memory,
		with one defalut rule.
		user space test application will enter more valid
		rules the default rule
		must be last on the list.
		Get a handle to the routing table which holds
		the added rule. This will
		also increment an internal reference count. */
	memset(&rt_lookup, 0, sizeof(struct ipa_ioc_get_rt_tbl));
	rt_lookup.ip = IPA_IP_v4;
	strlcpy(rt_lookup.name,
			DEFAULT_ROUTING_TABLE_NAME,
			IPA_RESOURCE_NAME_MAX);
	if (ipa3_get_rt_tbl(&rt_lookup)) {
		IPATEST_ERR("ipa_get_rt_tbl() failure.\n");
	}
	ipa_ep_cfg->route.rt_tbl_hdl = rt_lookup.hdl;
	/* Now release the routing table hdl. This code assumes
	   that routing table
	   will continue to exist when the endpoint
	   connection is requested. */
	if (ipa_put_rt_tbl(rt_lookup.hdl)) {
		IPATEST_ERR("ipa_put_rt_tbl() failure.\n");
	}
	return 0;
}

/*
 * Configures the system as follows:
 * This configuration is for one input pipe
 * and one output pipe where both are USB1
 * /dev/to_ipa_0 -> MEM -> IPA
 * -> MEM -> /dev/from_ipa_0
 * Those client will be configured in DMA mode thus
 * no Header removal/insertion will be
 * made on their data.
*/
int configure_system_1(void)
{
	int res = 0;
	struct ipa_ep_cfg ipa_ep_cfg;

	struct ipa_sys_connect_params sys_in;
	unsigned long ipa_gsi_hdl;
	u32 ipa_pipe_num;

	memset(&ipa_ep_cfg, 0, sizeof(ipa_ep_cfg));
	IPATEST_DBG("Configure_system_1 was called\n");

	/* Connect IPA --> APPS MEM */
	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST_CONS;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl, &ipa_pipe_num,
			&from_ipa_devs[0]->ipa_client_hdl, false))
		goto fail;

	res = connect_ipa_to_apps(&from_ipa_devs[0]->ep,
				  IPA_CLIENT_TEST_CONS,
				  ipa_pipe_num,
				  ipa_gsi_hdl);
	if (res)
		goto fail;
	/* Prepare EP configuration details */
	memset(&ipa_ep_cfg, 0, sizeof(ipa_ep_cfg));
	ipa_ep_cfg.mode.mode = IPA_DMA;
	ipa_ep_cfg.mode.dst = IPA_CLIENT_TEST_CONS;

	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST_PROD;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl, &ipa_pipe_num,
			&to_ipa_devs[0]->ipa_client_hdl, false))
		goto fail;

	/* Connect APPS MEM --> Tx IPA */
	res = connect_apps_to_ipa(&to_ipa_devs[0]->ep,
				  IPA_CLIENT_TEST_PROD,
				  ipa_pipe_num,
				  &to_ipa_devs[0]->mem,
				  ipa_gsi_hdl);
	if (res)
		goto fail;
fail:
	/* cleanup and tear down goes here*/
	return res;
}

/*
 Configures the system with one input to IPA and 2 outputs.
 /dev/to_ipa_0 -> MEM -> GSI -> IPA |-> GSI
 -> MEM -> /dev/from_ipa_0
|-> GSI -> MEM -> /dev/from_ipa_1
*/
int configure_system_2(void)
{
	int res = 0;
	struct ipa_ep_cfg ipa_ep_cfg;

	struct ipa_sys_connect_params sys_in;
	unsigned long ipa_gsi_hdl;
	u32 ipa_pipe_num;

	memset(&ipa_ep_cfg, 0, sizeof(ipa_ep_cfg));


	/* Connect first Rx IPA --> APPS MEM */
	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST2_CONS;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl, &ipa_pipe_num,
			&from_ipa_devs[0]->ipa_client_hdl, false))
		goto fail;

	res = connect_ipa_to_apps(&from_ipa_devs[0]->ep,
				  IPA_CLIENT_TEST2_CONS,
				  ipa_pipe_num,
				  ipa_gsi_hdl);
	if (res)
		goto fail;

	/* Connect second Rx IPA --> APPS MEM */
	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST3_CONS;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl, &ipa_pipe_num,
			&from_ipa_devs[1]->ipa_client_hdl, false))
		goto fail;

	res = connect_ipa_to_apps(&from_ipa_devs[1]->ep,
				  IPA_CLIENT_TEST3_CONS,
				  ipa_pipe_num,
				  ipa_gsi_hdl);
	if (res)
		goto fail;

	/* Connect third (Default) Rx IPA --> APPS MEM */
	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST4_CONS;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl, &ipa_pipe_num,
			&from_ipa_devs[2]->ipa_client_hdl, false))
		goto fail;

	res = connect_ipa_to_apps(&from_ipa_devs[2]->ep,
			IPA_CLIENT_TEST4_CONS,
			ipa_pipe_num,
			ipa_gsi_hdl);
	if (res)
		goto fail;


	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST_PROD;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl, &ipa_pipe_num,
			&to_ipa_devs[0]->ipa_client_hdl, false))
		goto fail;

	/* Connect APPS MEM --> Tx IPA */
	res = connect_apps_to_ipa(&to_ipa_devs[0]->ep,
				  IPA_CLIENT_TEST_PROD,
				  ipa_pipe_num,
				  &to_ipa_devs[0]->mem,
				  ipa_gsi_hdl);


	if (res)
		goto fail;

	/* Connect Tx GSI -> IPA */
	/* Prepare an endpoint configuration structure */
	res = configure_ipa_endpoint(&ipa_ep_cfg, IPA_BASIC);
	if (res)
		goto fail;

	/* configure header removal on Tx */
	ipa_ep_cfg.hdr.hdr_len = ETH_HLEN;

	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST2_PROD;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl, &ipa_pipe_num,
			&to_ipa_devs[1]->ipa_client_hdl, false))
		goto fail;

	/* Connect APPS MEM --> Tx IPA */
	res = connect_apps_to_ipa(&to_ipa_devs[1]->ep,
				  IPA_CLIENT_TEST2_PROD,
				  ipa_pipe_num,
				  &to_ipa_devs[1]->mem,
				  ipa_gsi_hdl);


	if (res)
		goto fail;

fail:
	/* cleanup and tear down goes here */
	return res;
}

/*Configuration used for Header Insertion Tests*/
int configure_system_5(void)
{
	int res = 0;
	struct ipa_ep_cfg ipa_ep_cfg;

	struct ipa_sys_connect_params sys_in;
	unsigned long ipa_gsi_hdl;
	u32 ipa_pipe_num;

	memset(&ipa_ep_cfg, 0, sizeof(ipa_ep_cfg));

	/* configure header Insertion on Tx */

/*	ipa_ep_cfg.hdr.hdr_len  = IPA_TEST_DMUX_HEADER_LENGTH;
 !< Header length in bytes to be added/removed.
	Assuming heaser len is constant per endpoint.
	Valid for both Input and Output Pipes
	Length of Header to add / to remove
	ipa_ep_cfg.hdr.hdr_additional_const_len = 0;
	!< Defines the constant length that should be added
	to the payload length in order for IPA to update
	correctly the length field within the header
	(valid only in case Hdr_Ofst_Pkt_Size_Valid=1)
	Valid for Output Pipes (IPA Producer)
	ipa_ep_cfg.hdr.hdr_ofst_pkt_size_valid = 0;
	!< 0: Hdr_Ofst_Pkt_Size  value is invalid, i.e.,
	no length field within the inserted header
	1: Hdr_Ofst_Pkt_Size  value is valid, i.e.,
	a packet length field resides within the header
	Valid for Output Pipes (IPA Producer)
	ipa_ep_cfg.hdr.hdr_ofst_pkt_size = 0;
	!< Offset within header in which packet size
	reside. Upon Header Insertion, IPA will update this
	field within the header with the packet length .
	Assumption is that header length field size is
	constant and is 2Bytes Valid for Output Pipes (IPA Producer)
	ipa_ep_cfg.hdr.hdr_a5_mux = 0;
	0: Do not insert APPS Mux Header
	1: Insert APPS Mux Header
	!< Determines whether APPS Mux header should be
	added to the packet. This bit is valid only when
	Hdr_En=01(Header Insertion) SW should set this bit
	for IPA-to-APPS pipes.
	0: Do not insert APPS Mux Header
	1: Insert APPS Mux Header
	Valid for Output Pipes (IPA Producer) */



	/* Connect IPA -> first Rx GSI */
	memset(&ipa_ep_cfg, 0, sizeof(ipa_ep_cfg));
	ipa_ep_cfg.hdr.hdr_len =
			IPA_TEST_HDI_RMNET_HEADER_LENGTH;
	/* Length of Header to add / to remove */
	ipa_ep_cfg.hdr.hdr_additional_const_len =
			IPA_TEST_HDI_RMNET_ADD_CONST_LENGTH;
	/* constant length that should be added to the payload
	 * length or IPA to update correctly the length
	 *  field within the header */
	ipa_ep_cfg.hdr.hdr_ofst_pkt_size_valid
		= IPA_TEST_HDI_RMNET_LENGTH_FIELD_OFFSET_VALID;
	/*0: Hdr_Ofst_Pkt_Size  value is invalid, i.e.,
	  no length field within the inserted header
	  1: Hdr_Ofst_Pkt_Size  value is valid, i.e.,
	  a packet length field resides within the header*/
	ipa_ep_cfg.hdr.hdr_ofst_pkt_size
		= IPA_TEST_HDI_RMNET_LENGTH_FIELD_OFFSET;
	/*	Offset within header in which packet size reside.
		Upon Header Insertion, IPA will update this field
		within the header with the packet length .
		Assumption is that header length field size
		is constant and is 2Bytes */

	/* Connect first Rx IPA --> APPS MEM */
	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST2_CONS;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl, &ipa_pipe_num,
			&from_ipa_devs[0]->ipa_client_hdl, false))
		goto fail;

	res = connect_ipa_to_apps(&from_ipa_devs[0]->ep,
				  IPA_CLIENT_TEST2_CONS,
				  ipa_pipe_num,
				  ipa_gsi_hdl);
	if (res)
		goto fail;

	memset(&ipa_ep_cfg, 0, sizeof(ipa_ep_cfg));
	ipa_ep_cfg.hdr.hdr_len = IPA_TEST_HDI_802_HEADER_LENGTH;
	/*Length of Header to add / to remove*/
	ipa_ep_cfg.hdr.hdr_additional_const_len
		= IPA_TEST_HDI_802_ADD_CONST_LENGTH;
	/* constant length that should be added
	 * to the payload length
	   or IPA to update correctly the
	   length field within the header */
	ipa_ep_cfg.hdr.hdr_ofst_pkt_size_valid
		= IPA_TEST_HDI_802_LENGTH_FIELD_OFFSET_VALID;
	/*0: Hdr_Ofst_Pkt_Size  value is invalid, i.e.,
	 no length field within the inserted header
	 1: Hdr_Ofst_Pkt_Size  value is valid, i.e.
	  a packet length field resides within the header*/
	ipa_ep_cfg.hdr.hdr_ofst_pkt_size
		= IPA_TEST_HDI_802_LENGTH_FIELD_OFFSET;
	/* Offset within header in which packet size reside.
	   Upon Header Insertion, IPA will update this field
	   within the header with the packet length.
	   Assumption is that header length field size is constant
	   and is 2Bytes */
		/* Connect second Rx IPA --> APPS MEM */
	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST3_CONS;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl,
				&ipa_pipe_num,
				&from_ipa_devs[1]->ipa_client_hdl,
				false))
		goto fail;

	res = connect_ipa_to_apps(&from_ipa_devs[1]->ep,
				  IPA_CLIENT_TEST3_CONS,
				  ipa_pipe_num,
				  ipa_gsi_hdl);
	if (res)
		goto fail;

	/* Connect IPA -> third (default) Rx GSI */
	memset(&ipa_ep_cfg, 0, sizeof(ipa_ep_cfg));
	ipa_ep_cfg.hdr.hdr_len
		= IPA_TEST_HDI_802_HEADER_LENGTH;
	/* Length of Header to add / to remove */
	ipa_ep_cfg.hdr.hdr_additional_const_len
		= IPA_TEST_HDI_802_ADD_CONST_LENGTH+1;
	/*  constant length that should be
	 *  added to the payload length
		or IPA to update correctly the length
		field within the header */
	ipa_ep_cfg.hdr.hdr_ofst_pkt_size_valid
		= IPA_TEST_HDI_802_LENGTH_FIELD_OFFSET_VALID;
	/* 0: Hdr_Ofst_Pkt_Size  value is invalid, i.e.,
	   no length field within the inserted header
	   1: Hdr_Ofst_Pkt_Size  value is valid, i.e.,
	   a packet length field resides within the header */
	ipa_ep_cfg.hdr.hdr_ofst_pkt_size
		= IPA_TEST_HDI_802_LENGTH_FIELD_OFFSET;
	/* Offset within header in which packet size reside.
	 * Upon Header Insertion, IPA will update this field
	 * within the header with the packet length .
	 * Assumption is that header length field size is constant
	 * and is 2Bytes */
	/* Connect third (Default) Rx IPA --> APPS MEM */
	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST4_CONS;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl,
			&ipa_pipe_num, &from_ipa_devs[2]->ipa_client_hdl, false))
		goto fail;

	res = connect_ipa_to_apps(&from_ipa_devs[2]->ep,
				  IPA_CLIENT_TEST4_CONS,
				  ipa_pipe_num,
				  ipa_gsi_hdl);

	if (res)
		goto fail;

	/* Connect Tx GSI -> IPA */
	/* Prepare an endpoint configuration structure */
	res = configure_ipa_endpoint(&ipa_ep_cfg, IPA_BASIC);
	if (res)
		goto fail;

	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST_PROD;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl, &ipa_pipe_num,
			&to_ipa_devs[0]->ipa_client_hdl, false))
		goto fail;

	/* Connect APPS MEM --> Tx IPA */
	res = connect_apps_to_ipa(&to_ipa_devs[0]->ep,
				  IPA_CLIENT_TEST_PROD,
				  ipa_pipe_num,
				  &to_ipa_devs[0]->mem,
				  ipa_gsi_hdl);

	if (res)
		goto fail;

fail:
	/* cleanup and tear down goes here */
	return res;
}

/*Configuration Used for USB Integration (on R3PC) */
int configure_system_6(void)
{
	int res = 0;
	struct ipa_ep_cfg ipa_ep_cfg;
	struct ipa_sys_connect_params sys_in;
	unsigned long ipa_gsi_hdl;
	u32 ipa_pipe_num;

	memset(&ipa_ep_cfg, 0, sizeof(ipa_ep_cfg));
	/* Connect first Rx IPA --> APPS MEM */
	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST2_CONS;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl, &ipa_pipe_num,
			&from_ipa_devs[0]->ipa_client_hdl, false))
		goto fail;
	res = connect_ipa_to_apps(&from_ipa_devs[0]->ep,
				  IPA_CLIENT_TEST2_CONS,
				  ipa_pipe_num,
				  ipa_gsi_hdl);
	if (res)
		goto fail;
	memset(&ipa_ep_cfg, 0, sizeof(ipa_ep_cfg));
	/* Prepare an endpoint configuration structure */
	ipa_ep_cfg.mode.mode = IPA_BASIC;
	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST2_PROD;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl, &ipa_pipe_num,
			&to_ipa_devs[0]->ipa_client_hdl, false))
		goto fail;
	/* Connect APPS MEM --> Tx IPA */
	res = connect_apps_to_ipa(&to_ipa_devs[0]->ep,
				  IPA_CLIENT_TEST2_PROD,
				  ipa_pipe_num,
				  &to_ipa_devs[0]->mem,
				  ipa_gsi_hdl);
	if (res)
		goto fail;
fail:
	/* cleanup and tear down goes here */
	return res;
}


/*
 Configures the system as follows:
 This configuration is for 4 input pipes and 3 output pipes:
 /dev/to_ipa_0 -> MEM -> GSI ->
  * IPA -> GSI -> MEM -> /dev/from_ipa_0
 /dev/to_ipa_1 -> MEM -> GSI ->
  * IPA -> GSI -> MEM -> /dev/from_ipa_1
 /dev/to_ipa_2 -> MEM -> GSI ->
  * IPA -> GSI -> MEM -> /dev/from_ipa_0
 /dev/to_ipa_3 -> MEM -> GSI ->
  * IPA -> GSI -> MEM -> /dev/from_ipa_2
 to_ipa_1, to_ipa_2, from_ipa_0 &
 from_ipa_2 transfer TLP aggregated packets
 to_ipa_0, to_ipa_3 & from_ipa_1 transfer raw packets
*/
int configure_system_8(void)
{
	int res = 0;
	struct ipa_ep_cfg ipa_ep_cfg;

	struct ipa_sys_connect_params sys_in;
	unsigned long ipa_gsi_hdl;
	u32 ipa_pipe_num;

	memset(&ipa_ep_cfg, 0, sizeof(ipa_ep_cfg));
	IPATEST_DBG("Configure_system_8 was called\n");

	/* Setup GSI pipes */
	to_ipa_devs[0]->dma_ep.src_pipe_index = 4;
	to_ipa_devs[0]->dma_ep.dest_pipe_index = 5;
	to_ipa_devs[1]->dma_ep.src_pipe_index = 6;
	to_ipa_devs[1]->dma_ep.dest_pipe_index = 7;
	to_ipa_devs[2]->dma_ep.src_pipe_index = 8;
	to_ipa_devs[2]->dma_ep.dest_pipe_index = 9;
	to_ipa_devs[3]->dma_ep.src_pipe_index = 10;
	to_ipa_devs[3]->dma_ep.dest_pipe_index = 11;

	from_ipa_devs[0]->dma_ep.src_pipe_index = 14;
	from_ipa_devs[0]->dma_ep.dest_pipe_index = 15;
	from_ipa_devs[1]->dma_ep.src_pipe_index = 16;
	from_ipa_devs[1]->dma_ep.dest_pipe_index = 17;
	from_ipa_devs[2]->dma_ep.src_pipe_index = 18;
	from_ipa_devs[2]->dma_ep.dest_pipe_index = 19;
	/* configure aggregation on Tx */
	ipa_ep_cfg.aggr.aggr_en = IPA_ENABLE_AGGR;
	ipa_ep_cfg.aggr.aggr = IPA_TLP;
	ipa_ep_cfg.aggr.aggr_byte_limit = 1;
	ipa_ep_cfg.aggr.aggr_time_limit = 0;

	/* Connect IPA --> APPS MEM */
	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST_CONS;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl, &ipa_pipe_num,
			&from_ipa_devs[0]->ipa_client_hdl, false))
		goto fail;

	res = connect_ipa_to_apps(&from_ipa_devs[0]->ep,
				  IPA_CLIENT_TEST_CONS,
				  ipa_pipe_num,
				  ipa_gsi_hdl);
	if (res)
		goto fail;

	/* Connect IPA --> APPS MEM */
	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST3_CONS;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl, &ipa_pipe_num,
			&from_ipa_devs[1]->ipa_client_hdl, false))
		goto fail;

	res = connect_ipa_to_apps(&from_ipa_devs[1]->ep,
				  IPA_CLIENT_TEST3_CONS,
				  ipa_pipe_num,
				  ipa_gsi_hdl);
	if (res)
		goto fail;

	/* configure aggregation on Tx */
	ipa_ep_cfg.aggr.aggr_en = IPA_ENABLE_AGGR;
	ipa_ep_cfg.aggr.aggr = IPA_TLP;
	ipa_ep_cfg.aggr.aggr_byte_limit = 1;
	ipa_ep_cfg.aggr.aggr_time_limit = 30;
	if (ipa_get_hw_type() >= IPA_HW_v4_2)
		ipa_ep_cfg.aggr.aggr_time_limit *= 1000;

	/* Connect IPA --> APPS MEM */
	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST2_CONS;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl, &ipa_pipe_num,
			&from_ipa_devs[2]->ipa_client_hdl, false))
		goto fail;

	res = connect_ipa_to_apps(&from_ipa_devs[2]->ep,
			IPA_CLIENT_TEST2_CONS,
			ipa_pipe_num,
			ipa_gsi_hdl);
	if (res)
		goto fail;

	/* Prepare EP configuration details */
	memset(&ipa_ep_cfg, 0, sizeof(ipa_ep_cfg));
	ipa_ep_cfg.mode.mode = IPA_DMA;
	ipa_ep_cfg.mode.dst = IPA_CLIENT_TEST_CONS;

	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST3_PROD;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl, &ipa_pipe_num,
			&to_ipa_devs[0]->ipa_client_hdl, false))
		goto fail;

	/* Connect APPS MEM --> Tx IPA */
	res = connect_apps_to_ipa(&to_ipa_devs[0]->ep,
				  IPA_CLIENT_TEST3_PROD,
				  ipa_pipe_num,
				  &to_ipa_devs[0]->mem,
				  ipa_gsi_hdl);
	if (res)
		goto fail;

	/* configure deaggregation on Rx */
	memset(&ipa_ep_cfg, 0, sizeof(ipa_ep_cfg));
	ipa_ep_cfg.mode.mode = IPA_DMA;
	ipa_ep_cfg.mode.dst = IPA_CLIENT_TEST3_CONS;
	ipa_ep_cfg.aggr.aggr_en = IPA_ENABLE_DEAGGR;
	ipa_ep_cfg.aggr.aggr = IPA_TLP;

	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST_PROD;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl, &ipa_pipe_num,
			&to_ipa_devs[1]->ipa_client_hdl, false))
		goto fail;

	/* Connect APPS MEM --> Tx IPA */
	res = connect_apps_to_ipa(&to_ipa_devs[1]->ep,
				  IPA_CLIENT_TEST_PROD,
				  ipa_pipe_num,
				  &to_ipa_devs[1]->mem,
				  ipa_gsi_hdl);
	if (res)
		goto fail;

	/* configure deaggregation on Rx */
	memset(&ipa_ep_cfg, 0, sizeof(ipa_ep_cfg));
	ipa_ep_cfg.mode.mode = IPA_DMA;
	ipa_ep_cfg.mode.dst = IPA_CLIENT_TEST_CONS;
	ipa_ep_cfg.aggr.aggr_en = IPA_ENABLE_DEAGGR;
	ipa_ep_cfg.aggr.aggr = IPA_TLP;

	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST2_PROD;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl, &ipa_pipe_num,
			&to_ipa_devs[2]->ipa_client_hdl, false))
		goto fail;

	/* Connect APPS MEM --> Tx IPA */
	res = connect_apps_to_ipa(&to_ipa_devs[2]->ep,
				  IPA_CLIENT_TEST2_PROD,
				  ipa_pipe_num,
				  &to_ipa_devs[2]->mem,
				  ipa_gsi_hdl);
	if (res)
		goto fail;

	/* Prepare EP configuration details */
	memset(&ipa_ep_cfg, 0, sizeof(ipa_ep_cfg));
	ipa_ep_cfg.mode.mode = IPA_DMA;
	ipa_ep_cfg.mode.dst = IPA_CLIENT_TEST2_CONS;

	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST4_PROD;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl, &ipa_pipe_num,
			&to_ipa_devs[3]->ipa_client_hdl, false))
		goto fail;

	/* Connect APPS MEM --> Tx IPA */
	res = connect_apps_to_ipa(&to_ipa_devs[3]->ep,
			IPA_CLIENT_TEST4_PROD,
			ipa_pipe_num,
			&to_ipa_devs[3]->mem,
			ipa_gsi_hdl);
	if (res)
		goto fail;

fail:
	/* cleanup and tear down goes here */
	return res;
}


/*
 Configures the system as follows:
 This configuration is for 4 input pipes and 3 output pipes:
 /dev/to_ipa_0 -> MEM -> GSI
  * -> IPA -> GSI -> MEM -> /dev/from_ipa_0
 /dev/to_ipa_1 -> MEM -> GSI
  * -> IPA -> GSI -> MEM -> /dev/from_ipa_1
 /dev/to_ipa_2 -> MEM -> GSI
  * -> IPA -> GSI -> MEM -> /dev/from_ipa_0
 /dev/to_ipa_3 -> MEM -> GSI
  * -> IPA -> GSI -> MEM -> /dev/from_ipa_2
 to_ipa_1, to_ipa_2, from_ipa_0 &
 from_ipa_2 transfer MBIM aggregated packets
 to_ipa_0, to_ipa_3 & from_ipa_1 transfer raw packets
*/
int configure_system_9(void)
{
	int res = 0;
	struct ipa_ep_cfg ipa_ep_cfg;
	enum ipa_aggr_mode mode;
	struct ipa_sys_connect_params sys_in;
	unsigned long ipa_gsi_hdl;
	u32 ipa_pipe_num;

	mode = IPA_MBIM_AGGR;
	res = ipa_set_aggr_mode(mode);
	if (res)
		goto fail;
	res = ipa_set_single_ndp_per_mbim(true);
	if (res)
		goto fail;

	memset(&ipa_ep_cfg, 0, sizeof(ipa_ep_cfg));
	IPATEST_DBG("Configure_system_9 was called\n");

	/* Setup GSI pipes */
	to_ipa_devs[0]->dma_ep.src_pipe_index = 4;
	to_ipa_devs[0]->dma_ep.dest_pipe_index = 5;
	to_ipa_devs[1]->dma_ep.src_pipe_index = 6;
	to_ipa_devs[1]->dma_ep.dest_pipe_index = 7;
	to_ipa_devs[2]->dma_ep.src_pipe_index = 8;
	to_ipa_devs[2]->dma_ep.dest_pipe_index = 9;
	to_ipa_devs[3]->dma_ep.src_pipe_index = 10;
	to_ipa_devs[3]->dma_ep.dest_pipe_index = 11;

	from_ipa_devs[0]->dma_ep.src_pipe_index = 14;
	from_ipa_devs[0]->dma_ep.dest_pipe_index = 15;
	from_ipa_devs[1]->dma_ep.src_pipe_index = 16;
	from_ipa_devs[1]->dma_ep.dest_pipe_index = 17;
	from_ipa_devs[2]->dma_ep.src_pipe_index = 18;
	from_ipa_devs[2]->dma_ep.dest_pipe_index = 19;

	/* configure aggregation on Tx */
	ipa_ep_cfg.aggr.aggr_en = IPA_ENABLE_AGGR;
	ipa_ep_cfg.aggr.aggr = IPA_MBIM_16;
	ipa_ep_cfg.aggr.aggr_byte_limit = 1;
	ipa_ep_cfg.aggr.aggr_time_limit = 0;
	ipa_ep_cfg.hdr.hdr_len = 1;

	/* Connect IPA --> APPS MEM */
	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST_CONS;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl, &ipa_pipe_num,
			&from_ipa_devs[0]->ipa_client_hdl, false))
		goto fail;

	res = connect_ipa_to_apps(&from_ipa_devs[0]->ep,
				  IPA_CLIENT_TEST_CONS,
				  ipa_pipe_num,
				  ipa_gsi_hdl);
	if (res)
		goto fail;

	/* Connect IPA --> APPS MEM */
	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST3_CONS;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl, &ipa_pipe_num,
			&from_ipa_devs[1]->ipa_client_hdl, false))
		goto fail;

	res = connect_ipa_to_apps(&from_ipa_devs[1]->ep,
				  IPA_CLIENT_TEST3_CONS,
				  ipa_pipe_num,
				  ipa_gsi_hdl);
	if (res)
		goto fail;

	/* configure aggregation on Tx */
	ipa_ep_cfg.aggr.aggr_en = IPA_ENABLE_AGGR;
	ipa_ep_cfg.aggr.aggr = IPA_MBIM_16;
	ipa_ep_cfg.aggr.aggr_byte_limit = 1;
	ipa_ep_cfg.aggr.aggr_time_limit = 30;
	if (ipa_get_hw_type() >= IPA_HW_v4_2)
		ipa_ep_cfg.aggr.aggr_time_limit *= 1000;
	ipa_ep_cfg.hdr.hdr_len = 1;

	/* Connect IPA --> APPS MEM */
	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST2_CONS;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl, &ipa_pipe_num,
			&from_ipa_devs[2]->ipa_client_hdl, false))
		goto fail;

	res = connect_ipa_to_apps(&from_ipa_devs[2]->ep,
				  IPA_CLIENT_TEST2_CONS,
				  ipa_pipe_num,
				  ipa_gsi_hdl);
	if (res)
		goto fail;

	/* Prepare EP configuration details */
	memset(&ipa_ep_cfg, 0, sizeof(ipa_ep_cfg));
	ipa_ep_cfg.mode.mode = IPA_DMA;
	ipa_ep_cfg.mode.dst = IPA_CLIENT_TEST_CONS;

	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST3_PROD;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl, &ipa_pipe_num,
			&to_ipa_devs[0]->ipa_client_hdl, false))
		goto fail;

	/* Connect APPS MEM --> Tx IPA */
	res = connect_apps_to_ipa(&to_ipa_devs[0]->ep,
				  IPA_CLIENT_TEST3_PROD,
				  ipa_pipe_num,
				  &to_ipa_devs[0]->mem,
				  ipa_gsi_hdl);
	if (res)
		goto fail;

	/* configure deaggregation on Rx */
	memset(&ipa_ep_cfg, 0, sizeof(ipa_ep_cfg));
	ipa_ep_cfg.mode.mode = IPA_DMA;
	ipa_ep_cfg.mode.dst = IPA_CLIENT_TEST3_CONS;
	ipa_ep_cfg.aggr.aggr_en = IPA_ENABLE_DEAGGR;
	ipa_ep_cfg.aggr.aggr = IPA_MBIM_16;

	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST_PROD;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl, &ipa_pipe_num,
			&to_ipa_devs[1]->ipa_client_hdl, false))
		goto fail;

	/* Connect APPS MEM --> Tx IPA */
	res = connect_apps_to_ipa(&to_ipa_devs[1]->ep,
				  IPA_CLIENT_TEST_PROD,
				  ipa_pipe_num,
				  &to_ipa_devs[1]->mem,
				  ipa_gsi_hdl);
	if (res)
		goto fail;

	/* configure deaggregation on Rx */
	memset(&ipa_ep_cfg, 0, sizeof(ipa_ep_cfg));
	ipa_ep_cfg.mode.mode = IPA_DMA;
	ipa_ep_cfg.mode.dst = IPA_CLIENT_TEST_CONS;
	ipa_ep_cfg.aggr.aggr_en = IPA_ENABLE_DEAGGR;
	ipa_ep_cfg.aggr.aggr = IPA_MBIM_16;

	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST2_PROD;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl, &ipa_pipe_num,
			&to_ipa_devs[2]->ipa_client_hdl, false))
		goto fail;

	/* Connect APPS MEM --> Tx IPA */
	res = connect_apps_to_ipa(&to_ipa_devs[2]->ep,
				  IPA_CLIENT_TEST2_PROD,
				  ipa_pipe_num,
				  &to_ipa_devs[2]->mem,
				  ipa_gsi_hdl);
	if (res)
		goto fail;

	/* Prepare EP configuration details */
	memset(&ipa_ep_cfg, 0, sizeof(ipa_ep_cfg));
	ipa_ep_cfg.mode.mode = IPA_DMA;
	ipa_ep_cfg.mode.dst = IPA_CLIENT_TEST2_CONS;

	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST4_PROD;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl, &ipa_pipe_num,
			&to_ipa_devs[3]->ipa_client_hdl, false))
		goto fail;

	/* Connect APPS MEM --> Tx IPA */
	res = connect_apps_to_ipa(&to_ipa_devs[3]->ep,
				  IPA_CLIENT_TEST4_PROD,
				  ipa_pipe_num,
				  &to_ipa_devs[3]->mem,
				  ipa_gsi_hdl);
	if (res)
		goto fail;

fail:
	/* cleanup and tear down goes here */
	return res;
}

/*
 Configures the system as follows:
 This configuration is for 1 input pipe and 1 output pipe:
 /dev/to_ipa_0 -> MEM -> GSI -> IPA -> GSI -> MEM -> /dev/from_ipa_0
 /dev/to_ipa_1 -> MEM -> GSI -> IPA -> GSI -> MEM -> /dev/from_ipa_1
 from_ipa_0, from_ipa_1 transfer IPA_MBIM_AGGR aggregated packets
 to_ipa_0, to_ipa_1 transfer raw packets
*/
int configure_system_10(void)
{
	int res = 0;
	struct ipa_ep_cfg ipa_ep_cfg;
	enum ipa_aggr_mode mode;
	struct ipa_sys_connect_params sys_in;
	unsigned long ipa_gsi_hdl;
	u32 ipa_pipe_num;

	mode = IPA_MBIM_AGGR;
	res = ipa_set_aggr_mode(mode);
	if (res)
		goto fail;
	res = ipa_set_single_ndp_per_mbim(false);
	if (res)
		goto fail;

	memset(&ipa_ep_cfg, 0, sizeof(ipa_ep_cfg));
	IPATEST_DBG("Configure_system_10 was called\n");

	/* Setup GSI pipes */
	to_ipa_devs[0]->dma_ep.src_pipe_index = 4;
	to_ipa_devs[0]->dma_ep.dest_pipe_index = 5;

	from_ipa_devs[0]->dma_ep.src_pipe_index = 6;
	from_ipa_devs[0]->dma_ep.dest_pipe_index = 7;

	/* configure aggregation on Tx */
	ipa_ep_cfg.aggr.aggr_en = IPA_ENABLE_AGGR;
	ipa_ep_cfg.aggr.aggr = IPA_MBIM_16;
	ipa_ep_cfg.aggr.aggr_byte_limit = 0;
	ipa_ep_cfg.aggr.aggr_time_limit = 0;
	ipa_ep_cfg.hdr.hdr_len = 1;

	/* Connect IPA --> APPS MEM */
	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST_CONS;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl, &ipa_pipe_num,
			&from_ipa_devs[0]->ipa_client_hdl, false))
		goto fail;

	res = connect_ipa_to_apps(&from_ipa_devs[0]->ep,
				  IPA_CLIENT_TEST_CONS,
				  ipa_pipe_num,
				  ipa_gsi_hdl);
	if (res)
		goto fail;

	/* Prepare EP configuration details */
	memset(&ipa_ep_cfg, 0, sizeof(ipa_ep_cfg));
	ipa_ep_cfg.mode.mode = IPA_DMA;
	ipa_ep_cfg.mode.dst = IPA_CLIENT_TEST_CONS;

	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST_PROD;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl,
			&ipa_pipe_num, &to_ipa_devs[0]->ipa_client_hdl, false))
		goto fail;

	/* Connect APPS MEM --> Tx IPA */
	res = connect_apps_to_ipa(&to_ipa_devs[0]->ep,
				  IPA_CLIENT_TEST_PROD,
				  ipa_pipe_num,
				  &to_ipa_devs[0]->mem,
				  ipa_gsi_hdl);
	if (res)
		goto fail;

fail:
	/* cleanup and tear down goes here */
	return res;
}

int configure_system_12(void)
{
	int res = 0;
	struct ipa_ep_cfg ipa_ep_cfg;
	enum ipa_aggr_mode mode;
	char qcncm_sig[3];

	struct ipa_sys_connect_params sys_in;
	unsigned long ipa_gsi_hdl;
	u32 ipa_pipe_num;
	mode = IPA_QCNCM_AGGR;
	res = ipa_set_aggr_mode(mode);
	if (res)
		goto fail;
	res = ipa_set_single_ndp_per_mbim(false);
	if (res)
		goto fail;
	qcncm_sig[0] = 0x51;
	qcncm_sig[1] = 0x4e;
	qcncm_sig[2] = 0x44;
	res = ipa_set_qcncm_ndp_sig(qcncm_sig);
	if (res)
		goto fail;

	memset(&ipa_ep_cfg, 0, sizeof(ipa_ep_cfg));


	/* Connect IPA -> first Rx GSI */
	memset(&ipa_ep_cfg, 0, sizeof(ipa_ep_cfg));
	ipa_ep_cfg.aggr.aggr_en = IPA_ENABLE_AGGR;
	ipa_ep_cfg.aggr.aggr = IPA_MBIM_16;
	ipa_ep_cfg.aggr.aggr_byte_limit = 1;
	ipa_ep_cfg.aggr.aggr_time_limit = 0;
	ipa_ep_cfg.hdr.hdr_len = 1;

	/* Connect first Rx IPA --> APPS MEM */
	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST2_CONS;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl,
			&ipa_pipe_num, &from_ipa_devs[0]->ipa_client_hdl, false))
		goto fail;

	res = connect_ipa_to_apps(&from_ipa_devs[0]->ep,
				  IPA_CLIENT_TEST2_CONS,
				  ipa_pipe_num,
				  ipa_gsi_hdl);
	if (res)
		goto fail;

	memset(&ipa_ep_cfg, 0, sizeof(ipa_ep_cfg));

		/* Connect second Rx IPA --> APPS MEM */
	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST3_CONS;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl,
			&ipa_pipe_num, &from_ipa_devs[1]->ipa_client_hdl, false))
		goto fail;

	res = connect_ipa_to_apps(&from_ipa_devs[1]->ep,
				  IPA_CLIENT_TEST3_CONS,
				  ipa_pipe_num,
				  ipa_gsi_hdl);
	if (res)
		goto fail;

	/* Connect IPA -> first Rx GSI */
	memset(&ipa_ep_cfg, 0, sizeof(ipa_ep_cfg));
	ipa_ep_cfg.aggr.aggr_en = IPA_ENABLE_AGGR;
	ipa_ep_cfg.aggr.aggr = IPA_MBIM_16;
	ipa_ep_cfg.aggr.aggr_byte_limit = 0;
	ipa_ep_cfg.aggr.aggr_time_limit = 30;
	if (ipa_get_hw_type() >= IPA_HW_v4_2)
		ipa_ep_cfg.aggr.aggr_time_limit *= 1000;
	ipa_ep_cfg.hdr.hdr_len = 1;

	/* Connect first Rx IPA --> APPS MEM */
	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST_CONS;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl,
			&ipa_pipe_num, &from_ipa_devs[2]->ipa_client_hdl, false))
		goto fail;

	res = connect_ipa_to_apps(&from_ipa_devs[2]->ep,
				  IPA_CLIENT_TEST_CONS,
				  ipa_pipe_num,
				  ipa_gsi_hdl);
	if (res)
		goto fail;

	/* Connect IPA -> first Rx GSI */
	memset(&ipa_ep_cfg, 0, sizeof(ipa_ep_cfg));
	ipa_ep_cfg.aggr.aggr_en = IPA_ENABLE_AGGR;
	ipa_ep_cfg.aggr.aggr = IPA_MBIM_16;
	ipa_ep_cfg.aggr.aggr_byte_limit = 0;
	ipa_ep_cfg.aggr.aggr_time_limit = 0;
	ipa_ep_cfg.hdr.hdr_len = 1;

	/* Connect first Rx IPA --> APPS MEM */
	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST4_CONS;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl,
			&ipa_pipe_num, &from_ipa_devs[3]->ipa_client_hdl, false))
		goto fail;

	res = connect_ipa_to_apps(&from_ipa_devs[3]->ep,
				  IPA_CLIENT_TEST4_CONS,
				  ipa_pipe_num,
				  ipa_gsi_hdl);
	if (res)
		goto fail;

	/* Connect Tx GSI -> IPA */
	/* Prepare an endpoint configuration structure */
	res = configure_ipa_endpoint(&ipa_ep_cfg, IPA_BASIC);
	if (res)
		goto fail;

	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST_PROD;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl,
			&ipa_pipe_num, &to_ipa_devs[0]->ipa_client_hdl, false))
		goto fail;

	/* Connect APPS MEM --> Tx IPA */
	res = connect_apps_to_ipa(&to_ipa_devs[0]->ep,
				  IPA_CLIENT_TEST_PROD,
				  ipa_pipe_num,
				  &to_ipa_devs[0]->mem,
				  ipa_gsi_hdl);

	if (res)
		goto fail;

	/* Connect Tx GSI -> IPA */
	/* Prepare an endpoint configuration structure */
	res = configure_ipa_endpoint(&ipa_ep_cfg, IPA_BASIC);
	if (res)
		goto fail;
	ipa_ep_cfg.aggr.aggr_en = IPA_ENABLE_DEAGGR;
	ipa_ep_cfg.aggr.aggr = IPA_MBIM_16;

	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST2_PROD;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl,
			&ipa_pipe_num, &to_ipa_devs[1]->ipa_client_hdl, false))
		goto fail;

	/* Connect APPS MEM --> Tx IPA */
	res = connect_apps_to_ipa(&to_ipa_devs[1]->ep,
				  IPA_CLIENT_TEST2_PROD,
				  ipa_pipe_num,
				  &to_ipa_devs[1]->mem,
				  ipa_gsi_hdl);

	if (res)
		goto fail;

fail:
	/* cleanup and tear down goes here */
	return res;
}

void suspend_handler(enum ipa_irq_type interrupt,
				void *private_data,
				void *interrupt_data)
{
	u32 *suspend_data;
	u32 clnt_hdl;
	u32 gsi_chan_hdl;
	struct ipa_ep_cfg_ctrl ipa_to_usb_ep_cfg_ctrl;
	int i, res;

	suspend_data =
		((struct ipa_tx_suspend_irq_data *)interrupt_data)->endpoints;
	clnt_hdl =
		((struct ipa_tx_suspend_private_data *)private_data)->clnt_hdl;
	gsi_chan_hdl =
		((struct ipa_tx_suspend_private_data *)private_data)->gsi_chan_hdl;

	IPATEST_DBG("in suspend handler: interrupt=%d, private_data=%d, interrupt_data=",
			 interrupt, clnt_hdl, suspend_data[0], suspend_data[1]);
	for (i = 0; i < IPA_EP_ARR_SIZE; i++)
		IPATEST_DBG("%d", suspend_data[i]);

	IPATEST_DBG("\nEnabling back data path for IPA_CLIENT_USB_CONS\n");
	memset(&ipa_to_usb_ep_cfg_ctrl, 0 , sizeof(struct ipa_ep_cfg_ctrl));
	ipa_to_usb_ep_cfg_ctrl.ipa_ep_suspend = false;

	if(ipa_get_hw_type() >= IPA_HW_v4_0)
		res = gsi_start_channel(gsi_chan_hdl);
	else
		res = ipa_cfg_ep_ctrl(clnt_hdl, &ipa_to_usb_ep_cfg_ctrl);

	if (res)
		IPATEST_ERR("failed enabling back data path for IPA_CLIENT_USB_CONS\n");
}
/*
 * Configures the system as follows:
 * This configuration is for one input pipe and one output pipe
 * where both are USB1
 * /dev/to_ipa_0 -> MEM -> GSI -> IPA -> GSI-> MEM -> /dev/from_ipa_0
 * Those clients will be configured in DMA mode thus no Header removal/insertion
 * will be made on their data.
 * Then disable USB_CONS EP for creating the suspend interrupt and register
 * a handler for it.
*/
int configure_system_19(void)
{
	struct ipa_ep_cfg_ctrl ipa_to_usb_ep_cfg_ctrl;
	int res;

	res = configure_system_1();
	if (res) {
		IPATEST_ERR("configure system (19) failed\n");
		goto fail;
	}

	memset(&ipa_to_usb_ep_cfg_ctrl, 0 , sizeof(struct ipa_ep_cfg_ctrl));
	ipa_to_usb_ep_cfg_ctrl.ipa_ep_suspend = true;

	if(ipa_get_hw_type() >= IPA_HW_v4_0)
		res = ipa_stop_gsi_channel(from_ipa_devs[0]->ipa_client_hdl);
	else
		res = ipa_cfg_ep_ctrl(from_ipa_devs[0]->ipa_client_hdl, &ipa_to_usb_ep_cfg_ctrl);

	if (res) {
		IPATEST_ERR("end-point ctrl register configuration failed\n");
		goto fail;
	}
	IPATEST_DBG("end-point ctrl register configured successfully (ipa_ep_suspend = true)\n");

	return 0;

fail:

	return res;
}

int configure_system_18(void)
{
	int res = 0;
	struct ipa_ep_cfg ipa_ep_cfg;
	struct ipa_sys_connect_params sys_in;
	unsigned long ipa_gsi_hdl;
	u32 ipa_pipe_num;

	datapath_ds_clean();

	/* Connect IPA -> Rx GSI */
	memset(&ipa_ep_cfg, 0, sizeof(ipa_ep_cfg));

	/* Connect Rx IPA --> APPS MEM */
	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST_CONS;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	sys_in.notify = notify_ipa_write_done;
	if (ipa3_sys_setup(&sys_in,
				&ipa_gsi_hdl,
				&ipa_pipe_num,
				&from_ipa_devs[0]->ipa_client_hdl,
				false))
		goto fail;

	res = connect_ipa_to_apps(&from_ipa_devs[0]->ep,
				  IPA_CLIENT_TEST_CONS,
				  ipa_pipe_num,
				  ipa_gsi_hdl);
	if (res)
		goto fail;

	memset(&ipa_ep_cfg, 0, sizeof(ipa_ep_cfg));

	/* Connect Tx GSI -> IPA */
	/* Prepare an endpoint configuration structure */
	res = configure_ipa_endpoint(&ipa_ep_cfg, IPA_BASIC);
	if (res)
		goto fail;

	memset(&ipa_ep_cfg, 0, sizeof(ipa_ep_cfg));

	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST_PROD;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	sys_in.notify = notify_ipa_received;
	if (ipa3_sys_setup(&sys_in,
			&ipa_gsi_hdl,
			&ipa_pipe_num,
			&to_ipa_devs[0]->ipa_client_hdl,
			false))
		goto fail;

	/* Connect APPS MEM --> Tx IPA */
	res = connect_apps_to_ipa(&to_ipa_devs[0]->ep,
				  IPA_CLIENT_TEST_PROD,
				  ipa_pipe_num,
				  &to_ipa_devs[0]->mem,
				  ipa_gsi_hdl);

	if (res)
		goto fail;

fail:
	/* cleanup and tear down goes here */
	return res;
}


/**
 * Read File.
 *
 * @note This function is used by User Mode Application
 * in order to read data from the device node /dev/ipa_exception_pipe.
 * This implementation assumes Single Reader and Single Writer.
 *
 */
ssize_t exception_kfifo_read(struct file *filp, char __user *buf,
		       size_t count, loff_t *p_pos)
{
	int ret = 0;
	size_t data_len = 0;
	unsigned int copied;

	if (kfifo_is_empty(
			&(p_exception_hdl_data->
					notify_cb_data.exception_kfifo))) {
		/* Optimization*/

		IPATEST_DBG("No Data in exception pipe, Sleeping...\n");
		msleep(200);
		IPATEST_DBG("Sleeping Done...\n");
		if (kfifo_is_empty(&(p_exception_hdl_data->
				notify_cb_data.exception_kfifo))) {
			IPATEST_DBG("No Data in exception pipe.Returning\n");
			return 0;
		}
	}
	data_len = kfifo_peek_len
			(&(p_exception_hdl_data
					->notify_cb_data.exception_kfifo));
	if (data_len > count) {
		IPATEST_ERR("buffer(%zu) too small (%zu) required\n",
			data_len, count);
		return -ENOSPC;
	}
	ret = kfifo_to_user(&
			(p_exception_hdl_data->
					notify_cb_data.exception_kfifo)
			, buf, data_len, &copied);
#if (EXCEPTION_KFIFO_DEBUG_VERBOSE)
	{
		int i = 0;

		IPATEST_DBG("Exception packet's length=%zu, Packet's content:\n"
					, data_len);
		if (data_len - 3 > 0) {
			for (i = 0; i < data_len-4; i += 4) {
				IPATEST_DUMP("%02x %02x %02x %02x\n",
						(buf)[i], (buf)[i+1],
						(buf)[i+2], (buf)[i+3]);
			}
		}
	}
#endif
	return ret ? ret : copied;
}

/**
 * Write File.
 *
 * @note This function is used by User
 * in order to write data to the device node /dev/ipa_exception_pipe.
 * This implementation assumes Single Reader and Single Writer.
 *
 */
ssize_t exception_kfifo_write(struct file *file, const char __user *buf,
	size_t count, loff_t *ppos)
{
	int ret;
	unsigned int copied;

	ret = kfifo_from_user
			(&(p_exception_hdl_data->notify_cb_data.exception_kfifo)
					, buf, count, &copied);
	if (ret) {
		IPATEST_ERR("(%d/%zu) Bytes were written to kfifo.\n",
			copied, count);
	}
	return ret ? ret : copied;
}

static const struct file_operations exception_fops = {
	.owner = THIS_MODULE,
	.read = exception_kfifo_read,
	.write = exception_kfifo_write,
};

void notify_upon_exception(void *priv,
		enum ipa_dp_evt_type evt, unsigned long data)
{
	int i = 0;
	size_t data_len;
	int res = 0;
	char *p_data = NULL;
	struct sk_buff *p_sk_buff = (struct sk_buff *)data;
	struct notify_cb_data_st *p_notify_cb_data
		= (struct notify_cb_data_st *)priv;

	IPATEST_DBG("was called, evt=%s(%d)",
			(evt == IPA_RECEIVE) ?
				"IPA_RECEIVE" :
				"IPA_WRITE_DONE", evt);
	if (IPA_RECEIVE != evt) {
		IPATEST_ERR("unexpected value of evt == %d\n", evt);
		return;
	}

#if (SAVE_HEADER)
	data_len = p_sk_buff->len + 8; /* store len */
	p_data = (p_sk_buff->data) - 8; /* store pointer to the data */
#else
	data_len = p_sk_buff->len; /* store len */
	p_data = p_sk_buff->data; /* store pointer to the data */
#endif

#if (EXCEPTION_KFIFO_DEBUG_VERBOSE)
		IPATEST_DBG("Exception packet length = %zu,Packet content:\n",
				data_len);
		for (i = 0; i < data_len - 4; i += 4) {
			IPATEST_DUMP("%02x %02x %02x %02x",
					(p_data)[i], (p_data)[i+1],
					(p_data)[i+2], (p_data)[i+3]);
		}
#endif
	res = kfifo_in(
			&p_notify_cb_data->exception_kfifo,
			p_data , data_len);
	if (res != data_len) {
		IPATEST_ERR("kfifo_in copied %d Bytes instead of %zu\n",
				res, data_len);
		return;
	}
}
/*
 * This function Inits the KFIFO and the Device Node of the exceptions
 */
int exception_hdl_init(void)
{
	int res = 0;
	struct notify_cb_data_st *p_notify_cb_data;

	IPATEST_DBG("called.\n");

	if (NULL != p_exception_hdl_data) {
		IPATEST_ERR("p_exception_hdl_data is initialized?=(0x%px)\n",
			p_exception_hdl_data);
		return -EINVAL;
	}
	p_exception_hdl_data =
			kzalloc(sizeof(struct exception_hdl_data), GFP_KERNEL);
	if (NULL == p_exception_hdl_data) {
		IPATEST_ERR("kzalloc return NULL, can't alloc %zu Bytes\n",
			sizeof(struct exception_hdl_data));
		return -ENOMEM;
	}
	IPATEST_DBG("Continue...\n");
	p_notify_cb_data = &(p_exception_hdl_data->notify_cb_data);

	res = kfifo_alloc(&(p_notify_cb_data->exception_kfifo)
			, EXCEPTION_KFIFO_SIZE*(sizeof(char)*RX_DESCRIPTOR_SIZE)
			, GFP_KERNEL);
	if (0 != res) {
		IPATEST_ERR("kfifo_alloc returned error (%d)\n",
			res);
		return res;
	}
	res = alloc_chrdev_region(&p_exception_hdl_data->dev_num
			, 0, 1, EXCEPTION_DRV_NAME);
	if (0 != res) {
		IPATEST_ERR("alloc_chrdev_region failed (%d)\n", res);
		return res;
	}
	p_exception_hdl_data->class =
			class_create(THIS_MODULE, EXCEPTION_DRV_NAME);
	p_exception_hdl_data->dev =
			device_create(p_exception_hdl_data->class
					, NULL, p_exception_hdl_data->dev_num,
				      ipa_test, EXCEPTION_DRV_NAME);
	if (IS_ERR(p_exception_hdl_data->dev)) {
		IPATEST_ERR("device_create returned error\n");
		return -ENODEV;
	}
	p_exception_hdl_data->p_cdev = cdev_alloc();
	if (NULL == p_exception_hdl_data->p_cdev) {
		IPATEST_ERR("cdev_alloc() returned NULL (0x%px)\n"
				, p_exception_hdl_data->p_cdev);
		return -EINVAL;
	}
	cdev_init(p_exception_hdl_data->p_cdev, &exception_fops);
	p_exception_hdl_data->p_cdev->owner = THIS_MODULE;
	res = cdev_add(p_exception_hdl_data->p_cdev
			, p_exception_hdl_data->dev_num, 1);
	if (0 != res) {
		IPATEST_ERR("cdev_add failed (%d)\n", res);
		return res;
	}

	IPATEST_DBG("completed.(%d)\n", res);
	return res;
}
/*
 * Clear the Exception Device and KFIFO
 */
void exception_hdl_exit(void)
{

	unregister_chrdev_region
			(p_exception_hdl_data->dev_num, 1);
	kfifo_free(&(p_exception_hdl_data
				->notify_cb_data.exception_kfifo));
	/* freeing kfifo */
	memset(&(p_exception_hdl_data
				->notify_cb_data.exception_kfifo), 0,
			sizeof(p_exception_hdl_data
				->notify_cb_data.exception_kfifo));
	kfree(p_exception_hdl_data);
	p_exception_hdl_data = NULL;
}

/* Configuration used for Exception Tests */
int configure_system_7(void)
{
	int res = 0;
	struct ipa_ep_cfg ipa_ep_cfg;

	struct ipa_sys_connect_params sys_in;
	unsigned long ipa_gsi_hdl;
	u32 ipa_pipe_num;

	memset(&ipa_ep_cfg, 0, sizeof(ipa_ep_cfg));

	res = exception_hdl_init();
	if (0 != res) {
		IPATEST_ERR("exception_hdl_init() failed (%d)\n", res);
		return res;
	}


	/* Connect first Rx IPA --> APPS MEM */
	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST2_CONS;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl, &ipa_pipe_num,
			&from_ipa_devs[0]->ipa_client_hdl,
			false))
		goto fail;

	res = connect_ipa_to_apps(&from_ipa_devs[0]->ep,
				  IPA_CLIENT_TEST2_CONS,
				  ipa_pipe_num,
				  ipa_gsi_hdl);
	if (res)
		goto fail;

	/* Connect second Rx IPA --> APPS MEM */
	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST3_CONS;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl, &ipa_pipe_num,
			&from_ipa_devs[1]->ipa_client_hdl,
			false))
		goto fail;

	res = connect_ipa_to_apps(&from_ipa_devs[1]->ep,
				  IPA_CLIENT_TEST3_CONS,
				  ipa_pipe_num,
				  ipa_gsi_hdl);
	if (res)
		goto fail;

	/* Connect third (Default) Rx IPA --> APPS MEM */
	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST4_CONS;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl, &ipa_pipe_num,
			&from_ipa_devs[2]->ipa_client_hdl,
			false))
		goto fail;

	res = connect_ipa_to_apps(&from_ipa_devs[2]->ep,
				  IPA_CLIENT_TEST4_CONS,
				  ipa_pipe_num,
				  ipa_gsi_hdl);
	if (res)
		goto fail;


	/* Connect Tx GSI -> IPA */
	/* Prepare an endpoint configuration structure */
	res = configure_ipa_endpoint(&ipa_ep_cfg, IPA_BASIC);
	if (res)
		goto fail;

	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST_PROD;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	sys_in.notify = &notify_upon_exception;
	sys_in.priv = &(p_exception_hdl_data->notify_cb_data);

	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl, &ipa_pipe_num,
			&to_ipa_devs[0]->ipa_client_hdl,
			false))
		goto fail;

	/* Connect APPS MEM --> Tx IPA */
	res = connect_apps_to_ipa(&to_ipa_devs[0]->ep,
				  IPA_CLIENT_TEST_PROD,
				  ipa_pipe_num,
				  &to_ipa_devs[0]->mem,
				  ipa_gsi_hdl);

	if (res)
		goto fail;
fail:
	/* cleanup and tear down goes here */
	return res;
}

void destroy_channel_device(struct channel_dev *channel_dev)
{
	int res = 0;
	struct device *pdev;

	IPATEST_DBG("Destroying device channel_dev = 0x%px,name %s.\n",
	       channel_dev, channel_dev->name);

	IPATEST_DBG("ep=0x%px gsi_chan_hdl=0x%lx\n", &channel_dev->ep, channel_dev->ep.gsi_chan_hdl);

	pdev = ipa3_get_pdev();

	if (channel_dev->ep.gsi_valid && pdev != NULL) {
		IPATEST_DBG("stopping channel 0x%lx\n", channel_dev->ep.gsi_chan_hdl);
		res = ipa_stop_gsi_channel(channel_dev->ipa_client_hdl);
		if (res != GSI_STATUS_SUCCESS)
			IPATEST_ERR("ipa_stop_gsi_channel failed %d\n\n", res);

		IPATEST_DBG("reset channel 0x%lx\n", channel_dev->ep.gsi_chan_hdl);
		res = gsi_reset_channel(channel_dev->ep.gsi_chan_hdl);
		if (res != GSI_STATUS_SUCCESS)
			IPATEST_ERR("gsi_reset_channel failed %d\n\n", res);

		IPATEST_DBG("deallocate channel 0x%lx\n", channel_dev->ep.gsi_chan_hdl);
		res = gsi_dealloc_channel(channel_dev->ep.gsi_chan_hdl);
		if (res != GSI_STATUS_SUCCESS)
			IPATEST_ERR("gsi_dealloc_channel failed %d\n\n", res);

		dma_free_coherent(pdev, channel_dev->ep.gsi_channel_props.ring_len, channel_dev->ep.gsi_channel_props.ring_base_vaddr, channel_dev->ep.gsi_channel_props.ring_base_addr);

		IPATEST_DBG("deallocate channel event ring 0x%lx\n", channel_dev->ep.gsi_evt_ring_hdl);
		res = gsi_dealloc_evt_ring(channel_dev->ep.gsi_evt_ring_hdl);
		if (res != GSI_STATUS_SUCCESS)
			IPATEST_ERR("gsi_dealloc_evt_ring failed %d\n\n", res);
		dma_free_coherent(pdev, channel_dev->ep.gsi_evt_ring_props.ring_len, channel_dev->ep.gsi_evt_ring_props.ring_base_vaddr, channel_dev->ep.gsi_evt_ring_props.ring_base_addr);

		res = ipa3_sys_teardown(channel_dev->ipa_client_hdl);
		if (res) {
			IPATEST_ERR("Failure on ipa_sys_teardown(),"
				" channel_dev = 0x%px, res = %d.\n",
				channel_dev, res);
		}
	}


	cdev_del(&channel_dev->cdev);
	memset(&channel_dev->cdev, 0, sizeof(channel_dev->cdev));
	device_destroy(channel_dev->class, channel_dev->dev_num);
	unregister_chrdev_region(channel_dev->dev_num, 1);
	class_destroy(channel_dev->class);
	test_free_mem(&channel_dev->mem);
	memset(channel_dev, 0, sizeof(struct channel_dev));
	kfree(channel_dev);
}

void destroy_channel_devices(void)
{
	IPATEST_DBG("-----Tear Down----\n");
	while (ipa_test->num_tx_channels > 0) {
		IPATEST_DBG("-- num_tx_channels = %d --\n", ipa_test->num_tx_channels);
		destroy_channel_device(ipa_test->tx_channels[--ipa_test->num_tx_channels]);
		ipa_test->tx_channels[ipa_test->num_tx_channels] = NULL;
	}

	while (ipa_test->num_rx_channels > 0) {
		IPATEST_DBG("-- num_rx_channels = %d --\n", ipa_test->num_rx_channels);
		destroy_channel_device(from_ipa_devs[--ipa_test->num_rx_channels]);
		from_ipa_devs[ipa_test->num_rx_channels] = NULL;
	}
}

int register_lan_interface(void)
{
	struct ipa_rx_intf rx_intf;
	struct ipa_tx_intf tx_intf;
	struct ipa_ioc_tx_intf_prop tx_prop;
	struct ipa_ioc_rx_intf_prop rx_prop;
	char *name = "rmnet1";
	int res;

	IPATEST_DBG(":new version\n");
	memset(&tx_prop, 0, sizeof(tx_prop));
	tx_prop.ip = IPA_IP_v6;
	tx_prop.dst_pipe = IPA_CLIENT_TEST_CONS;

	memset(&rx_prop, 0, sizeof(rx_prop));
	rx_prop.ip = IPA_IP_v6;
	rx_prop.src_pipe = IPA_CLIENT_TEST_PROD;

	memset(&rx_intf, 0, sizeof(rx_intf));
	rx_intf.num_props = 1;
	rx_intf.prop = &rx_prop;

	memset(&tx_intf, 0, sizeof(tx_intf));
	tx_intf.num_props = 1;
	tx_intf.prop = &tx_prop;

	res = ipa_register_intf(name, &tx_intf, &rx_intf);
	if (res != 0)
		goto fail;

	IPATEST_DBG(":registered interface %s !\n", name);

fail:
	return res;
}

/* add wlan header to ipa */

#define IPA_TO_WLAN_HEADER_NAME "wlan0"
int add_wlan_header(void)
{

#define IPA_TO_WLAN_HEADER_LEN  34

	uint8_t hdr[IPA_TO_WLAN_HEADER_LEN + 1] = {
		/* HTC Header - 6 bytes */
		0x00, 0x00,  /* Reserved */
		0x00, 0x00, /* length to be filled by IPA,
					after adding 32 with IP Payload
					length 32 will be
					programmed while
					intializing the header */
		0x00, 0x00,  /* Reserved */
		/* WMI header - 6 bytes*/
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

		/* 802.3 header - 14 bytes*/
		0x00, 0x03, 0x7f, 0x44, 0x33, 0x89,
		/* Des. MAC to be filled by IPA */
		0x00, 0x03, 0x7f, 0x17, 0x12, 0x69,
		/* Src. MAC to be filled by IPA */
		0x00, 0x00,
		/* length can be zero */

		/* LLC SNAP header - 8 bytes */
		0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00,
		0x08, 0x00  /* type value(2 bytes) to
						be filled by IPA, by reading
						from ethernet header */
		/* 0x0800 - IPV4, 0x86dd - IPV6 */
	};

	int ret = 0;
	int len = 0;
	struct ipa_ioc_add_hdr *ipa_to_wlan_header_partial;
	struct ipa_hdr_add ipa_to_wlan_header;

	memset(&ipa_to_wlan_header, 0, sizeof(ipa_to_wlan_header));
	/* Copy header name */
	memcpy(ipa_to_wlan_header.name,
				 IPA_TO_WLAN_HEADER_NAME,
				 sizeof(IPA_TO_WLAN_HEADER_NAME));

	/* poplate other fields of header add */
	ipa_to_wlan_header.hdr_len = IPA_TO_WLAN_HEADER_LEN;
	ipa_to_wlan_header.is_partial = 1;
	ipa_to_wlan_header.hdr_hdl = 0;
	ipa_to_wlan_header.status = -1;
	/* copy the parital header */
	memcpy(ipa_to_wlan_header.hdr, hdr, IPA_TO_WLAN_HEADER_LEN);

	/* Add wlan partial header to ipa */
	len = (sizeof(struct ipa_ioc_add_hdr)) +
			(1 * sizeof(struct ipa_hdr_add));
	ipa_to_wlan_header_partial = kmalloc(len, GFP_KERNEL);
	if (!ipa_to_wlan_header_partial) {
		IPATEST_ERR("Memory allocation failure");
		return false;
	}

	ipa_to_wlan_header_partial->commit = 1;
	ipa_to_wlan_header_partial->num_hdrs = 1;
	memcpy(&ipa_to_wlan_header_partial->hdr[0],
		       &ipa_to_wlan_header,
		       sizeof(ipa_to_wlan_header));
	ret = ipa_add_hdr(ipa_to_wlan_header_partial);
	if (ret) {
		IPATEST_ERR("unable to add wlan header %d", ret);
		goto fail;
	} else if (ipa_to_wlan_header_partial->hdr[0].status) {
		IPATEST_ERR("unable to add wlan header %d", ret);
		goto fail;
	}

	IPATEST_DBG("added wlan header successfully\n");

fail:
	kfree(ipa_to_wlan_header_partial);

	return ret;
}

/* Wlan interface has 4 rx and 1 Tx endpoint */
int register_wlan_interface(void)
{
	struct ipa_rx_intf rx_intf;
	struct ipa_tx_intf tx_intf;
	struct ipa_ioc_tx_intf_prop tx_prop[4];
	struct ipa_ioc_rx_intf_prop rx_prop;
	char *name = "eth0";
	int res, index = 0;

	res = add_wlan_header();
	if (res)
		return res;

	memset(&tx_prop, 0, 4 * sizeof(struct ipa_ioc_tx_intf_prop));

	index = 0;
	tx_prop[index].ip = IPA_IP_v6;
	tx_prop[index].dst_pipe = IPA_CLIENT_TEST1_CONS;
	memcpy(tx_prop[index].hdr_name, IPA_TO_WLAN_HEADER_NAME,
		  sizeof(IPA_TO_WLAN_HEADER_NAME));

	index++;
	tx_prop[index].ip = IPA_IP_v6;
	tx_prop[index].dst_pipe = IPA_CLIENT_TEST2_CONS;
	memcpy(tx_prop[index].hdr_name, IPA_TO_WLAN_HEADER_NAME,
			sizeof(IPA_TO_WLAN_HEADER_NAME));

	index++;
	tx_prop[index].ip = IPA_IP_v6;
	tx_prop[index].dst_pipe = IPA_CLIENT_TEST3_CONS;
	memcpy(tx_prop[index].hdr_name, IPA_TO_WLAN_HEADER_NAME,
			sizeof(IPA_TO_WLAN_HEADER_NAME));

	index++;
	tx_prop[index].ip = IPA_IP_v6;
	tx_prop[index].dst_pipe = IPA_CLIENT_TEST4_CONS;
	memcpy(tx_prop[index].hdr_name, IPA_TO_WLAN_HEADER_NAME,
			sizeof(IPA_TO_WLAN_HEADER_NAME));

	memset(&rx_prop, 0, sizeof(struct ipa_ioc_rx_intf_prop));
	rx_prop.ip = IPA_IP_v6;
	rx_prop.src_pipe = IPA_CLIENT_TEST1_PROD;

	memset(&rx_intf, 0, sizeof(rx_intf));
	rx_intf.num_props = 1;
	rx_intf.prop = &rx_prop;

	memset(&tx_intf, 0, sizeof(tx_intf));
	tx_intf.num_props = 4;
	tx_intf.prop = tx_prop;

	res = ipa_register_intf(name, &tx_intf, &rx_intf);
	if (res) {
		IPATEST_ERR("Unable to register interface %s, %d\n",
				name, res);
		return res;
	}

	IPATEST_DBG("Registered interface %s\n", name);
	return res;
}

/* Wan interface has 1 rx and 1 Tx endpoint */
int register_wan_interface(void)
{
	struct ipa_rx_intf rx_intf;
	struct ipa_tx_intf tx_intf;
	struct ipa_ioc_tx_intf_prop tx_prop;
	struct ipa_ioc_rx_intf_prop rx_prop;
	char *name = "rmnet0";
	int res;

	memset(&tx_prop, 0, sizeof(tx_prop));
	tx_prop.ip = IPA_IP_v6;
	tx_prop.dst_pipe = IPA_CLIENT_TEST3_CONS;

	memset(&rx_prop, 0, sizeof(rx_prop));
	rx_prop.ip = IPA_IP_v6;
	rx_prop.src_pipe = IPA_CLIENT_TEST3_PROD;

	memset(&rx_intf, 0, sizeof(rx_intf));
	rx_intf.num_props = 1;
	rx_intf.prop = &rx_prop;

	memset(&tx_intf, 0, sizeof(tx_intf));
	tx_intf.num_props = 1;
	tx_intf.prop = &tx_prop;

	res = ipa_register_intf(name, &tx_intf, &rx_intf);
	if (res != 0)
		goto fail;

	IPATEST_DBG("registered interface %s !\n", name);

fail:
	return res;
}

/*
 Configures the system with one input to IPA and 2 outputs.
 /dev/to_ipa_0 -> MEM -> GSI -> IPA |-> GSI
 -> MEM -> /dev/from_ipa_0
|-> GSI -> MEM -> /dev/from_ipa_1
*/
int configure_system_20(void)
{
	int res = 0;
	int index = 0;
	struct ipa_ep_cfg ipa_ep_cfg;

	struct ipa_sys_connect_params sys_in;
	unsigned long ipa_gsi_hdl;
	u32 ipa_pipe_num;

	memset(&ipa_ep_cfg, 0, sizeof(ipa_ep_cfg));


	/* Connect first Rx IPA --> AP MEM */
	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST2_CONS;
	if (ipa3_sys_setup(&sys_in,
		&ipa_gsi_hdl, &ipa_pipe_num,
		&from_ipa_devs[index]->ipa_client_hdl, false))
		goto fail;

	res = connect_ipa_to_apps(&from_ipa_devs[index]->ep,
		IPA_CLIENT_TEST2_CONS,
		ipa_pipe_num,
		ipa_gsi_hdl);
	if (res)
		goto fail;


	index++;

	/* Connect IPA -> 1 Tx GSI */
	/* RNDIS Aggregation with ETH2 header */
	memset(&ipa_ep_cfg, 0, sizeof(ipa_ep_cfg));
	ipa_ep_cfg.aggr.aggr_en = IPA_ENABLE_AGGR;
	ipa_ep_cfg.aggr.aggr = IPA_GENERIC;
	ipa_ep_cfg.aggr.aggr_byte_limit = 1;
	ipa_ep_cfg.aggr.aggr_time_limit = 0;
	ipa_ep_cfg.hdr.hdr_ofst_pkt_size_valid = true;
	ipa_ep_cfg.hdr.hdr_ofst_pkt_size = 12;
	ipa_ep_cfg.hdr.hdr_additional_const_len = 14;
	ipa_ep_cfg.hdr.hdr_len = 58;
	ipa_ep_cfg.hdr_ext.hdr_little_endian = true;
	ipa_ep_cfg.hdr_ext.hdr_total_len_or_pad_valid = true;
	ipa_ep_cfg.hdr_ext.hdr_total_len_or_pad = IPA_HDR_TOTAL_LEN;
	ipa_ep_cfg.hdr_ext.hdr_total_len_or_pad_offset = 4;

	/* Connect 1 Tx IPA --> AP MEM */
	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST3_CONS;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	if (ipa3_sys_setup(
		&sys_in,
		&ipa_gsi_hdl,
		&ipa_pipe_num,
		&from_ipa_devs[index]->ipa_client_hdl,
		false))
		goto fail;

	res = connect_ipa_to_apps(
		&from_ipa_devs[index]->ep,
		IPA_CLIENT_TEST3_CONS,
		ipa_pipe_num,
		ipa_gsi_hdl);
	if (res)
		goto fail;

	/* Connect Rx GSI -> IPA */
	/* Prepare an endpoint configuration structure */
	res = configure_ipa_endpoint(&ipa_ep_cfg, IPA_BASIC);
	if (res)
		goto fail;

	/* configure RNDIS+ETH2 header removal on Rx */
	/* configure RNDIS de-aggregation on Rx */
	ipa_ep_cfg.aggr.aggr_en = IPA_ENABLE_DEAGGR;
	ipa_ep_cfg.aggr.aggr = IPA_GENERIC;
	ipa_ep_cfg.deaggr.deaggr_hdr_len = 44; /* RNDIS hdr */
	ipa_ep_cfg.deaggr.packet_offset_valid = true;
	ipa_ep_cfg.deaggr.packet_offset_location = 8;
	ipa_ep_cfg.hdr.hdr_len = 14; /* Ethernet header */
	ipa_ep_cfg.hdr.hdr_ofst_pkt_size = 12;
	ipa_ep_cfg.hdr.hdr_remove_additional = false;
	ipa_ep_cfg.hdr_ext.hdr_little_endian = 1;
	ipa_ep_cfg.hdr_ext.hdr_total_len_or_pad_valid = 1;
	ipa_ep_cfg.hdr_ext.hdr_total_len_or_pad = IPA_HDR_TOTAL_LEN;
	ipa_ep_cfg.hdr_ext.hdr_payload_len_inc_padding = 0;
	ipa_ep_cfg.hdr_ext.hdr_total_len_or_pad_offset = 4;

	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST_PROD;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl, &ipa_pipe_num,
			&to_ipa_devs[0]->ipa_client_hdl, false))
		goto fail;

	/* Connect AP MEM --> Tx IPA */
	res = connect_apps_to_ipa(&to_ipa_devs[0]->ep,
				  IPA_CLIENT_TEST_PROD,
				  ipa_pipe_num,
				  &to_ipa_devs[0]->mem,
				  ipa_gsi_hdl);


	if (res)
		goto fail;

	/* Connect Tx GSI -> IPA */
	/* Prepare an endpoint configuration structure */
	res = configure_ipa_endpoint(&ipa_ep_cfg, IPA_BASIC);
	if (res)
		goto fail;

	/* configure ETH2+WLAN\ETH2_802_1Q (18B) header removal on Tx */
	ipa_ep_cfg.hdr.hdr_len = ETH_HLEN + IPA_TEST_ADDITIONAL_HDR_LEN;

	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST2_PROD;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl, &ipa_pipe_num,
			&to_ipa_devs[1]->ipa_client_hdl, false))
		goto fail;

	/* Connect AP MEM --> Tx IPA */
	res = connect_apps_to_ipa(&to_ipa_devs[1]->ep,
				  IPA_CLIENT_TEST2_PROD,
				  ipa_pipe_num,
				  &to_ipa_devs[1]->mem,
				  ipa_gsi_hdl);


	if (res)
		goto fail;

	/* Connect Tx GSI -> IPA */
	/* Prepare an endpoint configuration structure */
	res = configure_ipa_endpoint(&ipa_ep_cfg, IPA_BASIC);
	if (res)
		goto fail;

	/* configure ETH2 header removal on Tx */
	ipa_ep_cfg.hdr.hdr_len = ETH_HLEN;

	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = IPA_CLIENT_TEST3_PROD;
	sys_in.ipa_ep_cfg = ipa_ep_cfg;
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl, &ipa_pipe_num,
		&to_ipa_devs[2]->ipa_client_hdl, false))
		goto fail;

	/* Connect AP MEM --> Tx IPA */
	res = connect_apps_to_ipa(&to_ipa_devs[2]->ep,
		IPA_CLIENT_TEST3_PROD,
		ipa_pipe_num,
		&to_ipa_devs[2]->mem,
		ipa_gsi_hdl);


	if (res)
		goto fail;

fail:
	/* cleanup and tear down goes here */
	return res;
}

static char **str_split(char *str, const char *delim)
{
	char **res = NULL;
	char **tmp = NULL;
	char *p = strsep(&str, delim);
	int n_spaces = 0;

	/* split string and append tokens to 'res' */
	while (p) {
		tmp = krealloc(res, sizeof(char *) * ++n_spaces, GFP_KERNEL);

		if (tmp == NULL) {
			IPATEST_ERR("krealloc failed\n");
			goto fail; /* memory allocation failed */
		}

		res = tmp;
		res[n_spaces-1] = p;
		p = strsep(&str, delim);
	}
	/* realloc one extra element for the last NULL */
	tmp = krealloc(res, sizeof(char *) * (n_spaces+1), GFP_KERNEL);
	if (tmp == NULL) {
		IPATEST_ERR("krealloc failed\n");
		goto fail; /* memory allocation failed */
	}
	res = tmp;
	res[n_spaces] = 0;
	return res;
fail:
	kfree(res);
	return res;
}

/**
 * Write File.
 *
 * @note Configure the system by writing a configuration
 * index to the device node /dev/ipa_test
 */
ssize_t ipa_test_write(struct file *filp, const char __user *buf,
		       size_t size, loff_t *f_pos)
{
	int ret = 0;
	int index;

	unsigned long missing;
	char *str;
	char **params;

	str = kzalloc(size+1, GFP_KERNEL);
	if (str == NULL) {
		IPATEST_ERR("kzalloc err.\n");
		return -ENOMEM;
	}

	missing = copy_from_user(str, buf, size);
	if (missing) {
		kfree(str);
		return -EFAULT;
	}
	IPATEST_DBG("ipa_test_write: input string= %s\n", str);
	str[size] = '\0';

	params = str_split(str, " ");
	if (params == NULL) {
		kfree(str);
		return -EFAULT;
	}

	ret = kstrtos32(params[0], 10, (s32 *)&ipa_test->configuration_idx);
	if (ret) {
		IPATEST_ERR("kstrtoul() failed.\n");
		ret = -EFAULT;
		goto bail;
	}

	IPATEST_DBG(":Invoking configuration %d.\n",
			ipa_test->configuration_idx);

	/* Setup GSI */

	switch (ipa_test->configuration_idx) {
	case -1:
		destroy_channel_devices();
		/*exception_hdl_exit();TODO: hsnapy un-comment this*/
		break;

	case 2:
		index = 0;
		ret = create_channel_device(index,
					    "to_ipa", &to_ipa_devs[index],
					    TX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->tx_channels[ipa_test->num_tx_channels++] =
			to_ipa_devs[index];
		ret = create_channel_device(index, "from_ipa",
					    &from_ipa_devs[index],
					    RX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->rx_channels[ipa_test->num_rx_channels++] =
			from_ipa_devs[index];

		index++;
		ret = create_channel_device(index,
					    "to_ipa", &to_ipa_devs[index],
					    TX_SZ);
		if (ret) {
			IPATEST_ERR("channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->tx_channels[ipa_test->num_tx_channels++] =
			to_ipa_devs[index];
		ret = create_channel_device(index, "from_ipa",
					    &from_ipa_devs[index],
					    RX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->rx_channels[ipa_test->num_rx_channels++] =
			from_ipa_devs[index];

		index++;
		ret = create_channel_device(index, "from_ipa",
					    &from_ipa_devs[index],
					    RX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->rx_channels[ipa_test->num_rx_channels++] =
			from_ipa_devs[index];

		ret = configure_system_2();
		if (ret) {
			IPATEST_ERR("System configuration failed.");
			ret = -ENODEV;
			goto bail;
		}
		break;

	case 5:
		index = 0;
		ret = create_channel_device(index, "to_ipa",
						&to_ipa_devs[index],
					    TX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
				tx_channels[ipa_test->num_tx_channels++] =
						to_ipa_devs[index];

		ret = create_channel_device(index, "from_ipa",
						&from_ipa_devs[index],
					    RX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
				rx_channels[ipa_test->num_rx_channels++] =
						from_ipa_devs[index];

		index++;
		ret = create_channel_device(index, "from_ipa",
						&from_ipa_devs[index],
					    RX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
				rx_channels[ipa_test->num_rx_channels++] =
						from_ipa_devs[index];

		index++;
		ret = create_channel_device(index, "from_ipa",
						&from_ipa_devs[index],
					    RX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
				rx_channels[ipa_test->num_rx_channels++] =
						from_ipa_devs[index];

		ret = configure_system_5();
		if (ret) {
			IPATEST_ERR("System configuration failed.");
			ret = -ENODEV;
			goto bail;
		}
		break;

	case 6:
		index = 0;
		ret = create_channel_device(index, "to_ipa",
						&to_ipa_devs[index],
					    TX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
				tx_channels[ipa_test->num_tx_channels++] =
						to_ipa_devs[index];

		ret = create_channel_device(index, "from_ipa",
						&from_ipa_devs[index],
					    RX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
				rx_channels[ipa_test->num_rx_channels++] =
						from_ipa_devs[index];

		ret = configure_system_6();
		if (ret) {
			IPATEST_ERR("System configuration failed.");
			ret = -ENODEV;
			goto bail;
		}
		break;

	case 7:
		index = 0;
		ret = create_channel_device(index, "to_ipa",
						&to_ipa_devs[index],
					    TX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
				tx_channels[ipa_test->num_tx_channels++] =
						to_ipa_devs[index];

		ret = create_channel_device(index, "from_ipa",
						&from_ipa_devs[index],
					    RX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
				rx_channels[ipa_test->num_rx_channels++] =
						from_ipa_devs[index];

		index++;
		ret = create_channel_device(index, "from_ipa",
						&from_ipa_devs[index],
					    RX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
				rx_channels[ipa_test->num_rx_channels++] =
						from_ipa_devs[index];

		index++;
		ret = create_channel_device(index, "from_ipa",
						&from_ipa_devs[index],
					    RX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
				rx_channels[ipa_test->num_rx_channels++] =
						from_ipa_devs[index];

		ret = configure_system_7();
		if (ret) {
			IPATEST_ERR("System configuration failed.");
			ret = -ENODEV;
			goto bail;
		}
		break;

	case 8:
		index = 0;
		ret = create_channel_device(index, "to_ipa",
						&to_ipa_devs[index],
					    TX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
				tx_channels[ipa_test->num_tx_channels++] =
						to_ipa_devs[index];
		index++;

		ret = create_channel_device(index, "to_ipa",
						&to_ipa_devs[index],
					    TX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
				tx_channels[ipa_test->num_tx_channels++] =
						to_ipa_devs[index];
		index++;

		ret = create_channel_device(index, "to_ipa",
						&to_ipa_devs[index],
					    TX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
				tx_channels[ipa_test->num_tx_channels++] =
						to_ipa_devs[index];
		index++;

		ret = create_channel_device(index, "to_ipa",
						&to_ipa_devs[index],
					    TX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
				tx_channels[ipa_test->num_tx_channels++] =
						to_ipa_devs[index];

		index = 0;
		ret = create_channel_device(index, "from_ipa",
						&from_ipa_devs[index],
					    RX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
				rx_channels[ipa_test->num_rx_channels++] =
						from_ipa_devs[index];
		index++;

		ret = create_channel_device(index, "from_ipa",
						&from_ipa_devs[index],
					    RX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
				rx_channels[ipa_test->num_rx_channels++] =
						from_ipa_devs[index];
		index++;

		ret = create_channel_device(index, "from_ipa",
						&from_ipa_devs[index],
					    RX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
				rx_channels[ipa_test->num_rx_channels++] =
						from_ipa_devs[index];

		ret = configure_system_8();
		if (ret) {
			IPATEST_ERR("System configuration failed.\n");
			ret = -ENODEV;
			goto bail;
		}
		break;

	case 9:
		index = 0;
		ret = create_channel_device(index, "to_ipa",
						&to_ipa_devs[index],
					    TX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
				tx_channels[ipa_test->num_tx_channels++] =
						to_ipa_devs[index];
		index++;

		ret = create_channel_device(index, "to_ipa",
						&to_ipa_devs[index],
					    TX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
				tx_channels[ipa_test->num_tx_channels++] =
						to_ipa_devs[index];
		index++;

		ret = create_channel_device(index, "to_ipa",
						&to_ipa_devs[index],
					    TX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
				tx_channels[ipa_test->num_tx_channels++] =
						to_ipa_devs[index];
		index++;

		ret = create_channel_device(index, "to_ipa",
						&to_ipa_devs[index],
					    TX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
				tx_channels[ipa_test->num_tx_channels++] =
						to_ipa_devs[index];

		index = 0;
		ret = create_channel_device(index, "from_ipa",
						&from_ipa_devs[index],
					    RX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
				rx_channels[ipa_test->num_rx_channels++] =
						from_ipa_devs[index];
		index++;

		ret = create_channel_device(index, "from_ipa",
						&from_ipa_devs[index],
					    RX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
				rx_channels[ipa_test->num_rx_channels++] =
						from_ipa_devs[index];
		index++;

		ret = create_channel_device(index, "from_ipa",
						&from_ipa_devs[index],
					    RX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
				rx_channels[ipa_test->num_rx_channels++] =
						from_ipa_devs[index];

		ret = configure_system_9();
		if (ret) {
			IPATEST_ERR("System configuration failed.\n");
			ret = -ENODEV;
			goto bail;
		}
		break;

	case 10:
		index = 0;
		ret = create_channel_device(index, "to_ipa",
				&to_ipa_devs[index],
					    TX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
				tx_channels[ipa_test->num_tx_channels++] =
						to_ipa_devs[index];

		index = 0;
		ret = create_channel_device(index, "from_ipa",
						&from_ipa_devs[index],
					    RX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
				rx_channels[ipa_test->num_rx_channels++] =
						from_ipa_devs[index];

		ret = configure_system_10();
		if (ret) {
			IPATEST_ERR("System configuration failed.\n");
			ret = -ENODEV;
			goto bail;
		}
		break;

	case 12:
		index = 0;
		ret = create_channel_device(index, "to_ipa",
						&to_ipa_devs[index],
					    TX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
				tx_channels[ipa_test->num_tx_channels++] =
						to_ipa_devs[index];
		index++;

		ret = create_channel_device(index, "to_ipa",
						&to_ipa_devs[index],
					    TX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
				tx_channels[ipa_test->num_tx_channels++] =
						to_ipa_devs[index];

		index = 0;
		ret = create_channel_device(index, "from_ipa",
						&from_ipa_devs[index],
					    RX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
				rx_channels[ipa_test->num_rx_channels++] =
						from_ipa_devs[index];
		index++;

		ret = create_channel_device(index, "from_ipa",
						&from_ipa_devs[index],
					    RX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
				rx_channels[ipa_test->num_rx_channels++] =
						from_ipa_devs[index];
		index++;

		ret = create_channel_device(index, "from_ipa",
						&from_ipa_devs[index],
					    RX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
				rx_channels[ipa_test->num_rx_channels++] =
						from_ipa_devs[index];
		index++;

		ret = create_channel_device(index, "from_ipa",
						&from_ipa_devs[index],
					    RX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
				rx_channels[ipa_test->num_rx_channels++] =
						from_ipa_devs[index];

		ret = configure_system_12();
		if (ret) {
			IPATEST_ERR("System configuration failed.\n");
			ret = -ENODEV;
			goto bail;
		}
		break;

	case 18:
		index = 0;
		ret = create_channel_device(index, "to_ipa",
					&to_ipa_devs[index], TX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			return -ENODEV;
		}
		ipa_test->
			tx_channels[ipa_test->num_tx_channels++] =
						to_ipa_devs[index];

		index++;
		ret = create_channel_device_by_type(index, "to_ipa",
					&to_ipa_devs[index], RX_SZ,
					IPA_TEST_DATA_PATH_TEST_CHANNEL);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			return -ENODEV;
		}
		ipa_test->
			tx_channels[ipa_test->num_tx_channels++] =
					to_ipa_devs[index];

		index = 0;
		ret = create_channel_device(index, "from_ipa",
						&from_ipa_devs[index], RX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			return -ENODEV;
		}
		ipa_test->
			rx_channels[ipa_test->num_rx_channels++] =
				from_ipa_devs[index];

		ret = configure_system_18();
		if (ret) {
			IPATEST_ERR("System configuration failed.");
			return -ENODEV;
		}
		break;

	case 19:
		index = 0;
		/*Create the device on user space and allocate
		 * buffers for its GSI connection*/
		ret = create_channel_device(index, "to_ipa",
				&to_ipa_devs[index], TX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
			tx_channels[ipa_test->num_tx_channels++] =
					to_ipa_devs[index];

		/*Create the device on user space and allocate
		 *  buffers for its GSI connection*/
		ret = create_channel_device(index, "from_ipa",
						&from_ipa_devs[index],
					    RX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->rx_channels[ipa_test->num_rx_channels++] =
				from_ipa_devs[index];

		/*Make all the configurations required
		 * (IPA connect)*/
		ret = configure_system_19();
		if (ret) {
			IPATEST_ERR("System configuration failed.\n");
			ret = -ENODEV;
			goto bail;
		}
		break;

	case 20:

		/* Create producer channel 0 */
		index = 0;
		ret = create_channel_device(index,
			"to_ipa", &to_ipa_devs[index],
			TX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
			tx_channels[ipa_test->num_tx_channels++] =
			to_ipa_devs[index];


		/* Create producer channel 1 */
		index++;
		ret = create_channel_device(index,
			"to_ipa", &to_ipa_devs[index],
			TX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
			tx_channels[ipa_test->num_tx_channels++] =
			to_ipa_devs[index];

		/* Create producer channel 2 */
		index++;
		ret = create_channel_device(index,
			"to_ipa", &to_ipa_devs[index],
			TX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
			tx_channels[ipa_test->num_tx_channels++] =
			to_ipa_devs[index];

		/* Create consumer channel 0 */
		index = 0;
		ret = create_channel_device(index, "from_ipa",
			&from_ipa_devs[index],
			RX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
			rx_channels[ipa_test->num_rx_channels++] =
			from_ipa_devs[index];

		/* Create consumer channel 1 */
		index++;
		ret = create_channel_device(index, "from_ipa",
			&from_ipa_devs[index],
			RX_SZ);
		if (ret) {
			IPATEST_ERR("Channel device creation error.\n");
			ret = -ENODEV;
			goto bail;
		}
		ipa_test->
			rx_channels[ipa_test->num_rx_channels++] =
			from_ipa_devs[index];

		ret = configure_system_20();
		if (ret) {
			IPATEST_ERR("System configuration failed.");
			ret = -ENODEV;
			goto bail;
		}
		break;

	default:
		IPATEST_ERR("Unsupported configuration index.\n");
		break;
	}

	/* Insert descriptors into the receiving end(s) */
	ret = insert_descriptors_into_rx_endpoints(RX_BUFF_SIZE);
	if (ret) {
		IPATEST_DBG("Descriptor insertion into rx "
			"endpoints failed.\n");
		ret = -EINVAL;
		goto bail;
	}

	IPATEST_DBG("System configured !\n");

	/* Set the current configuration index */
	ipa_test->current_configuration_idx	=
		ipa_test->configuration_idx;

	ret = size;

bail:
	kfree(params);
	kfree(str);
	return ret;
}

static ssize_t ipa_test_read(struct file *filp,
		char __user *buf,
		size_t count, loff_t *f_pos)
{
	int res, len;
	char str[10];

	if (0 != *f_pos) {
		*f_pos = 0;
		return 0;
	}

	/* Convert the configuration index to a null terminated string */
	len = snprintf(str, 10, "%d",
			ipa_test->current_configuration_idx);

	IPATEST_DBG("str = %s, len = %d\n", str, len);

	/* Copy the result to the user buffer */
	res = copy_to_user(buf, str, len + 1);
	if (res < 0) {
		IPATEST_ERR("copy_to_user() failed.\n");
		return -EFAULT;
	}

	/* Increment the file position pointer */
	*f_pos += len;

	/* Return the number of bytes copied to the user */
	return len + 1;
}

static struct class *ipa_test_class;

//TODO make only one configuration function
static int configure_app_to_ipa_path(struct ipa_channel_config __user *to_ipa_user,
	bool isUlso)
{
	int retval;
	struct ipa_channel_config to_ipa_channel_config = {0};
	struct ipa_sys_connect_params sys_in;
	unsigned long ipa_gsi_hdl;
	u32 ipa_pipe_num;
	int index;

	IPATEST_DBG("copying from 0x%px %zu bytes\n",
		to_ipa_user,
		sizeof(struct ipa_channel_config));
	retval = copy_from_user(
		&to_ipa_channel_config,
		to_ipa_user,
		sizeof(struct ipa_channel_config));
	if (retval) {
		IPATEST_ERR("fail to copy from user - to_ipa_user\n");
		return -1;
	}

	index = to_ipa_channel_config.index;

	IPATEST_DBG("to_ipa head_marker value is 0x%x\n",
		to_ipa_channel_config.head_marker);

	IPATEST_DBG("to_ipa config_size value is %zu\n",
		to_ipa_channel_config.config_size);

	IPATEST_DBG("to_ipa index value is %d\n", index);

	IPATEST_DBG("client=%d\n",
		to_ipa_channel_config.client);

	IPATEST_DBG("to_ipa tail_marker value is 0x%x\n",
		to_ipa_channel_config.tail_marker);

	if (to_ipa_channel_config.head_marker != IPA_TEST_CHANNEL_CONFIG_MARKER) {
		IPATEST_ERR("bad head_marker - possible memory corruption\n");
		return -EFAULT;
	}

	if (to_ipa_channel_config.tail_marker != IPA_TEST_CHANNEL_CONFIG_MARKER) {
		IPATEST_ERR("bad tail_marker - possible memory corruption\n");
		return -EFAULT;
	}

	if (to_ipa_channel_config.config_size != sizeof(struct ipa_ep_cfg)) {
		IPATEST_ERR("bad config size (%zu.vs.%zu) update test struct?\n",
			to_ipa_channel_config.config_size,
			sizeof(struct ipa_ep_cfg));
		return -EFAULT;
	}

	/* Channel from which the userspace shall communicate to this pipe */
	if(isUlso){
		retval = create_channel_device_by_type(index, "to_ipa", &to_ipa_devs[index], TX_SZ,
		IPA_TEST_ULSO_DATA_PATH_TEST_CHANNEL);
	} else {
		retval = create_channel_device(index, "to_ipa", &to_ipa_devs[index], TX_SZ);
	}
	if (retval) {
		IPATEST_ERR("channel device creation error\n");
		return -1;
	}
	ipa_test->tx_channels[ipa_test->num_tx_channels++] = to_ipa_devs[index];
	if (isUlso)
	    return 0;

	/* Connect IPA --> Apps */
	memset(&sys_in, 0, sizeof(sys_in));
	sys_in.client = to_ipa_channel_config.client;
	IPATEST_DBG("copying from 0x%px\n", to_ipa_channel_config.cfg);
	retval = copy_from_user(&sys_in.ipa_ep_cfg, to_ipa_channel_config.cfg, to_ipa_channel_config.config_size);
	if (retval) {
		IPATEST_ERR("fail to copy cfg - from_ipa_user\n");
		return -1;
	}
	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl, &ipa_pipe_num, &to_ipa_devs[index]->ipa_client_hdl, false)) {
		IPATEST_ERR("setup sys pipe failed\n");
		return -1;
	}

	/* Connect APPS MEM --> Tx IPA */
	retval = connect_apps_to_ipa(&to_ipa_devs[index]->ep, to_ipa_channel_config.client, ipa_pipe_num,
				&to_ipa_devs[index]->mem, ipa_gsi_hdl);
	if (retval) {
		IPATEST_ERR("fail to connect ipa to apps\n");
		return -1;
	}

	return 0;
}

static int configure_app_from_ipa_path(struct ipa_channel_config __user *from_ipa_user, bool isUlso)
{
	int retval;
	struct ipa_channel_config from_ipa_channel_config = {0};
	struct ipa_sys_connect_params sys_in;
	unsigned long ipa_gsi_hdl;
	u32 ipa_pipe_num;
	int index;

	IPATEST_DBG("copying from 0x%px %zu bytes\n",
		from_ipa_user,
		sizeof(struct ipa_channel_config));
	retval = copy_from_user(
		&from_ipa_channel_config,
		from_ipa_user,
		sizeof(struct ipa_channel_config));
	if (retval) {
		IPATEST_ERR("fail to copy from user - from_ipa_user (%d.vs.%zu)\n",
			retval, sizeof(from_ipa_user));
		return -1;
	}

	index = from_ipa_channel_config.index;

	IPATEST_DBG("from_ipa head_marker value is 0x%x\n",
		from_ipa_channel_config.head_marker);

	IPATEST_DBG("from_ipa config_size value is %zu\n",
		from_ipa_channel_config.config_size);

	IPATEST_DBG("from_ipa index value is %d\n",
		index);

	IPATEST_DBG("client=%d\n",
		from_ipa_channel_config.client);

	IPATEST_DBG("from_ipa tail_marker value is 0x%x\n",
		from_ipa_channel_config.tail_marker);

	if (from_ipa_channel_config.head_marker !=
		IPA_TEST_CHANNEL_CONFIG_MARKER) {
		IPATEST_ERR("bad head_marker - possible memory corruption\n");
		return -EFAULT;
	}

	if (from_ipa_channel_config.tail_marker !=
		IPA_TEST_CHANNEL_CONFIG_MARKER) {
		IPATEST_ERR("bad tail_marker - possible memory corruption\n");
		return -EFAULT;
	}

	if (from_ipa_channel_config.config_size != sizeof(struct ipa_ep_cfg)) {
		IPATEST_ERR("bad config size (%zu.vs.%zu) update test struct?\n",
			from_ipa_channel_config.config_size,
			sizeof(struct ipa_ep_cfg));
		return -EFAULT;
	}

	/* Channel from which the userspace shall communicate to this pipe */
	retval = create_channel_device(index, "from_ipa",
			&from_ipa_devs[index], rx_size);
	if (retval) {
		IPATEST_ERR("channel device creation error\n");
		return -1;
	}
	ipa_test->rx_channels[ipa_test->num_rx_channels++] =
			from_ipa_devs[index];

	/* Connect IPA --> Apps */
	IPATEST_DBG("copying from 0x%px\n", from_ipa_channel_config.cfg);
	memset(&sys_in, 0, sizeof(sys_in));
	if (isUlso) {
		sys_in.notify = notify_ipa_write_done;
	}
	sys_in.client = from_ipa_channel_config.client;
	retval = copy_from_user(
		&sys_in.ipa_ep_cfg,
		from_ipa_channel_config.cfg,
		from_ipa_channel_config.config_size);
	if (retval) {
		IPATEST_ERR("fail to copy cfg - from_ipa_user\n");
		return -1;
	}

	if (ipa3_sys_setup(&sys_in, &ipa_gsi_hdl, &ipa_pipe_num,
			&from_ipa_devs[index]->ipa_client_hdl, from_ipa_channel_config.en_status)) {
			IPATEST_ERR("setup sys pipe failed\n");
			return -1;
	}

	retval = connect_ipa_to_apps(&from_ipa_devs[index]->ep,
				from_ipa_channel_config.client,
				ipa_pipe_num,
				ipa_gsi_hdl);
	if (retval) {
		IPATEST_ERR("fail to connect ipa to apps\n");
		return -1;
	}

	return 0;
}

static int configure_test_scenario(
		struct ipa_test_config_header *ipa_test_config_header,
		struct ipa_channel_config **from_ipa_channel_config_array,
		struct ipa_channel_config **to_ipa_channel_config_array,
		bool isUlso)
{
	int retval;
	int i;

	if (ipa_test->num_tx_channels > 0 || ipa_test->num_rx_channels >0) {
		IPATEST_DBG("cleanning previous configuration before new one\n");
		destroy_channel_devices();
	} else {
		IPATEST_DBG("system is clean, starting new configuration");
	}

	IPATEST_DBG("starting scenario configuration\n");

	IPATEST_DBG("head_marker value is 0x%x\n",
		ipa_test_config_header->head_marker);

	IPATEST_DBG("from_ipa_channels_num=%d\n\n",
		ipa_test_config_header->from_ipa_channels_num);

	IPATEST_DBG("to_ipa_channels_num=%d\n\n",
		ipa_test_config_header->to_ipa_channels_num);

	IPATEST_DBG("tail_marker value is 0x%x\n",
		ipa_test_config_header->tail_marker);

	if (ipa_test_config_header->head_marker != IPA_TEST_CONFIG_MARKER) {
		IPATEST_ERR("bad header marker - possible memory corruption\n");
		return -EFAULT;
	}

	if (ipa_test_config_header->tail_marker != IPA_TEST_CONFIG_MARKER) {
		IPATEST_ERR("bad tail marker - possible memory corruption\n");
		return -EFAULT;
	}

	for (i = 0 ; i < ipa_test_config_header->from_ipa_channels_num ; i++) {
		IPATEST_DBG("starting configuration of from_ipa_%d\n", i);
		retval = configure_app_from_ipa_path(from_ipa_channel_config_array[i], isUlso);
		if (retval) {
			IPATEST_ERR("fail to configure from_ipa_%d", i);
			goto fail;
		}
	}

	if (isUlso) {
		rx_size = RX_SZ_ULSO;
	} else {
		rx_size = RX_SZ;
	}

	retval = insert_descriptors_into_rx_endpoints(RX_BUFF_SIZE);
	if (retval) {
		IPATEST_ERR("RX descriptors failed\n");
		goto fail;
	}
	IPATEST_DBG("RX descriptors were added to RX pipes\n");

	for (i = 0 ; i < ipa_test_config_header->to_ipa_channels_num ; i++) {
		retval = configure_app_to_ipa_path(to_ipa_channel_config_array[i], isUlso);
		if (retval) {
			IPATEST_ERR("fail to configure to_ipa_%d", i);
			goto fail;
		}
	}

	/*
	 * This value is arbitrary, it is used in
	 * order to be able to cleanup
	 */
	ipa_test->current_configuration_idx = GENERIC_TEST_CONFIGURATION_IDX;

	IPATEST_DBG("finished scenario configuration\n");

	return 0;
fail:
	destroy_channel_devices();

	return retval;
}

static int handle_add_hdr_hpc(unsigned long ioctl_arg)
{
    struct ipa_ioc_add_hdr hdrs;
    struct ipa_hdr_add *hdr;
    int retval;

	IPATEST_ERR("copying from 0x%px\n", (u8 *)ioctl_arg);
	retval = copy_from_user(&hdrs, (u8 *)ioctl_arg, sizeof(hdrs) + sizeof(*hdr));
	if (retval) {
			IPATEST_ERR("failing copying header from user\n");
			return retval;
	}
    retval = ipa3_add_hdr_hpc(&hdrs);
    if (retval) {
        IPATEST_ERR("ipa3_add_hdr_hpc failed\n");
        return retval;
    }
	IPATEST_ERR("ELIAD: \n");
	hdr = &hdrs.hdr[0];
    if (hdr->status) {
        IPATEST_ERR("ipa3_add_hdr_hpc failed\n");
        return hdr->status;
    }
	IPATEST_ERR("ELIAD: \n");
    if (copy_to_user((void __user *)ioctl_arg, &hdrs, sizeof(hdrs) + sizeof(*hdr))) {
        retval = -EFAULT;
    }
	IPATEST_ERR("ELIAD: \n");

    return 0;
}

static int handle_pkt_init_ex_set_hdr_ofst_ioctl(unsigned long ioctl_arg)
{
	struct ipa_pkt_init_ex_hdr_ofst_set hdr_ofst;
	int retval;

	IPATEST_DBG("copying from 0x%px\n", (u8 *)ioctl_arg);
	retval = copy_from_user(&hdr_ofst, (u8 *)ioctl_arg, sizeof(hdr_ofst));
	if (retval) {
			IPATEST_ERR("failing copying header from user\n");
			return retval;
	}

	return ipa_set_pkt_init_ex_hdr_ofst(&hdr_ofst, true);
}

static int handle_configuration_ioctl(unsigned long ioctl_arg,
	bool isUlso)
{
	int retval;
	int needed_bytes;
	struct ipa_test_config_header test_header;
	struct ipa_channel_config **from_ipa_channel_config_array;
	struct ipa_channel_config **to_ipa_channel_config_array;

	/* header copy */
	IPATEST_DBG("copying from 0x%px\n", (u8 *)ioctl_arg);
	retval = copy_from_user(&test_header,
			(u8 *)ioctl_arg,
			sizeof(test_header));
	if (retval) {
		IPATEST_ERR("failing copying header from user\n");
		return retval;
	}

	/* allocate place for the configuration array for "from" */
	needed_bytes = test_header.from_ipa_channels_num*
		sizeof(*test_header.from_ipa_channel_config);

	from_ipa_channel_config_array = kzalloc(needed_bytes, GFP_KERNEL);
	if (!from_ipa_channel_config_array) {
		IPATEST_ERR("fail mem alloc for from_ipa\n");
		retval = -ENOMEM;
		goto fail_from_alloc;
	}

	/* copy the configuration array for "from" */
	IPATEST_DBG("copying from 0x%px\n", test_header.from_ipa_channel_config);
	retval = copy_from_user(from_ipa_channel_config_array,
			test_header.from_ipa_channel_config,
			needed_bytes);
	if (retval) {
		IPATEST_ERR("failing copying to_ipa from user\n");
		goto fail_copy_from;
	}

	/* allocate place for the configuration array for "from" */
	needed_bytes = test_header.to_ipa_channels_num*
		sizeof(*test_header.to_ipa_channel_config);

	to_ipa_channel_config_array = kzalloc(needed_bytes, GFP_KERNEL);
	if (!to_ipa_channel_config_array) {
		IPATEST_ERR("fail mem alloc for to_ipa\n");
		goto fail_to_alloc;
	}

	/* copy the configuration array for "to" */
	IPATEST_DBG("copying from 0x%px\n", test_header.to_ipa_channel_config);
	retval = copy_from_user(to_ipa_channel_config_array,
			test_header.to_ipa_channel_config,
			needed_bytes);
	if (retval) {
		IPATEST_ERR("failing copying to_ipa from user\n");
		goto fail_copy_to;
	}

	retval = configure_test_scenario(&test_header,
			from_ipa_channel_config_array,
			to_ipa_channel_config_array,
			isUlso);
	if (retval)
		IPATEST_ERR("fail to configure the system\n");

fail_copy_to:
	kfree(to_ipa_channel_config_array);
fail_to_alloc:
fail_copy_from:
	kfree(from_ipa_channel_config_array);
fail_from_alloc:
	return retval;

}

int handle_clean_ioctl(void)
{
	IPATEST_DBG("cleanning previous configuration\n");
	destroy_channel_devices();

	return 0;
}

static int handle_ep_ctrl_ioctl(unsigned long ioctl_arg)
{
	int retval = 0;
	struct ipa_ep_cfg_ctrl ep_ctrl;
	struct ipa_test_ep_ctrl test_ep_ctrl;

	retval = copy_from_user(&test_ep_ctrl,
			(u8 *)ioctl_arg,
			sizeof(test_ep_ctrl));
	if (retval) {
		IPATEST_ERR("failed copying ep_ctrl data from user\n");
		return retval;
	}

	ep_ctrl.ipa_ep_delay = test_ep_ctrl.ipa_ep_delay;
	ep_ctrl.ipa_ep_suspend = test_ep_ctrl.ipa_ep_suspend;

	/* pipe suspend is not supported in IPA_v4 or higher */
	if(ipa_get_hw_type() >= IPA_HW_v4_0){
		if(ep_ctrl.ipa_ep_suspend)
			retval = ipa_stop_gsi_channel(from_ipa_devs[test_ep_ctrl.from_dev_num]->ipa_client_hdl);
		else
			retval = gsi_start_channel(from_ipa_devs[test_ep_ctrl.from_dev_num]->ep.gsi_chan_hdl);

		if (retval) {
			IPATEST_ERR("failed closing/opening the GSI channel\n");
			return retval;
		}
		ep_ctrl.ipa_ep_suspend = false;
	}

	IPATEST_DBG("handle_ep_ctrl_ioctl: sending hdl %d\n\n",
			from_ipa_devs[test_ep_ctrl.from_dev_num]->ipa_client_hdl);

	return ipa_cfg_ep_ctrl(from_ipa_devs[test_ep_ctrl.from_dev_num]->ipa_client_hdl, &ep_ctrl);
}

static int handle_reg_suspend_handler(unsigned long ioctl_arg)
{
	int res = 0;
	struct ipa_tx_suspend_private_data *suspend_priv_data = NULL;
	struct ipa_test_reg_suspend_handler reg_data;

	res = copy_from_user(&reg_data,
			(u8 *)ioctl_arg,
			sizeof(reg_data));
	if (res) {
		IPATEST_ERR("failed copying ep_ctrl data from user\n");
		return res;
	}

	if (reg_data.reg) {
		if (reg_data.DevNum >= (MAX_CHANNEL_DEVS / 2))
		{
			res = -ENXIO;
			IPATEST_ERR("DevNum is incorrect %d\n", reg_data.DevNum);
			goto fail;
		}

		suspend_priv_data =
				kzalloc(sizeof(*suspend_priv_data), GFP_KERNEL);
		if (!suspend_priv_data) {
			IPATEST_ERR("failed allocating suspend_priv_data\n");
			res = -ENOMEM;
			goto fail;
		}

		suspend_priv_data->clnt_hdl = from_ipa_devs[reg_data.DevNum]->ipa_client_hdl;
		suspend_priv_data->gsi_chan_hdl = from_ipa_devs[reg_data.DevNum]->ep.gsi_chan_hdl;
		IPATEST_DBG("registering interrupt handle to clnt_hdl %d\n", suspend_priv_data->clnt_hdl);
		res = ipa_add_interrupt_handler(IPA_TX_SUSPEND_IRQ, suspend_handler,
				reg_data.deferred_flag, (void *)suspend_priv_data);
		if (res) {
			IPATEST_ERR("register handler for suspend interrupt failed\n");
			goto fail_allocated;
		}

	} else {
		res = ipa_restore_suspend_handler();
	}
fail:
	return res;
fail_allocated:
	kfree(suspend_priv_data);
	return res;
}

static int handle_holb_config_ioctl(unsigned long ioctl_arg)
{
	int retval = 0;
	int clnt_hdl;
	struct ipa_ep_cfg_holb holb_cfg = {0};
	struct ipa_test_holb_config test_holb_config;

	retval = copy_from_user(&test_holb_config,
			(u8 *)ioctl_arg,
			sizeof(test_holb_config));
	if (retval) {
		IPATEST_ERR("failed copying holb_config data from user\n");
		return retval;
	}

	clnt_hdl = ipa_get_ep_mapping(test_holb_config.client);

	IPATEST_ERR("Sending clnt_hdl %d", clnt_hdl);

	holb_cfg.en = test_holb_config.en;
	holb_cfg.tmr_val = test_holb_config.tmr_val;

	return ipa3_cfg_ep_holb(clnt_hdl, &holb_cfg);
}

static int ipa_test_get_mem_part(unsigned long ioctl_arg)
{
	unsigned long result;

	// Let's check that mirrored structure is of the same siz as the original
	BUILD_BUG_ON(sizeof(struct ipa_test_mem_partition) != sizeof(struct ipa3_mem_partition));

	result = copy_to_user((u8 *)ioctl_arg,
		ipa3_ctx->ctrl->mem_partition, sizeof(struct ipa3_mem_partition));

	if (result != 0)
		return -EACCES;

	return 0;
}

static long ipa_test_ioctl(struct file *filp,
	unsigned int cmd, unsigned long arg)
{
	int retval;

	IPATEST_DBG("cmd=%x nr=%d\n", cmd, _IOC_NR(cmd));
	if (_IOC_TYPE(cmd) != IPA_TEST_IOC_MAGIC)
		return -ENOTTY;

	switch (cmd) {
	case IPA_TEST_IOC_CONFIGURE:
		retval = handle_configuration_ioctl(arg, false);
		break;

	case IPA_TEST_IOC_CLEAN:
		retval = handle_clean_ioctl();
		break;

	case IPA_TEST_IOC_GET_HW_TYPE:
		retval = ipa_get_hw_type();
		break;
	case IPA_TEST_IOC_EP_CTRL:
		retval = handle_ep_ctrl_ioctl(arg);
		break;
	case IPA_TEST_IOC_REG_SUSPEND_HNDL:
		retval = handle_reg_suspend_handler(arg);
		break;
	case IPA_TEST_IOC_HOLB_CONFIG:
		retval = handle_holb_config_ioctl(arg);
		break;
	case IPA_TEST_IOC_IS_TEST_PROD_FLT_IN_SRAM:
		retval = ipa_is_test_prod_flt_in_sram_internal(arg);
	case IPA_TEST_IOC_GET_MEM_PART:
		retval = ipa_test_get_mem_part(arg);
		break;
	case IPA_TEST_IOC_ULSO_CONFIGURE:
		retval = handle_configuration_ioctl(arg, true);
		break;
    case IPA_TEST_IOC_ADD_HDR_HPC:
		retval = handle_add_hdr_hpc(arg);
		break;
	case IPA_TEST_IOC_PKT_INIT_EX_SET_HDR_OFST:
		retval = handle_pkt_init_ex_set_hdr_ofst_ioctl(arg);
		break;
	default:
		IPATEST_ERR("ioctl is not supported (%d)\n", cmd);
		return -ENOTTY;
	}

	return retval;
}

static int ipa_test_open(struct inode *inode, struct file *file)
{
	IPATEST_DBG("ipa_test module opened by %s\n", current->comm);
	return 0;
}

static int ipa_test_release(struct inode *inode, struct file *file)
{
	IPATEST_DBG("ipa_test module closed by %s\n", current->comm);
	return 0;
}


static const struct file_operations ipa_test_fops = {
	.owner = THIS_MODULE,
	.write = ipa_test_write,
	.read  = ipa_test_read,
	.open  = ipa_test_open,
	.release  = ipa_test_release,
	.unlocked_ioctl = ipa_test_ioctl,
};

/**
 * Module Init.
 */
static int __init ipa_test_init(void)
{
	int ret;

	IPATEST_DBG("IPA test driver load...\n");

	ipa_test = kzalloc(sizeof(*ipa_test), GFP_KERNEL);
	if (ipa_test == NULL) {
		IPATEST_ERR("kzalloc err.\n");
		return -ENOMEM;
	}
	ipa_test->signature = TEST_SIGNATURE;
	ipa_test->current_configuration_idx = -1;

	ipa_test_class = class_create(THIS_MODULE, IPA_TEST_DRV_NAME);

	ret = alloc_chrdev_region(&ipa_test->dev_num, 0, 1, IPA_TEST_DRV_NAME);
	if (ret) {
		IPATEST_ERR("alloc_chrdev_region err.\n");
		return -ENODEV;
	}

	ipa_test->dev = device_create(ipa_test_class, NULL,
					ipa_test->dev_num,
				      ipa_test, IPA_TEST_DRV_NAME);
	if (IS_ERR(ipa_test->dev)) {
		IPATEST_ERR("device_create err.\n");
		return -ENODEV;
	}

	ipa_test->cdev = cdev_alloc();
	if (ipa_test->cdev == NULL) {
		IPATEST_ERR("cdev_alloc err.\n");
		return -ENODEV;
	}

	cdev_init(ipa_test->cdev, &ipa_test_fops);
	ipa_test->cdev->owner = THIS_MODULE;

	ret = cdev_add(ipa_test->cdev, ipa_test->dev_num, 1);
	if (ret)
		IPATEST_ERR("cdev_add err=%d\n", -ret);

	if (ret == 0)
		IPATEST_DBG("IPA Test init OK, waiting for configuration index.\n");
	else
		IPATEST_DBG("IPA Test init FAIL.\n");

	ret = datapath_ds_init();
	if (ret != 0)
		IPATEST_DBG("datapath_ds_init() failed (%d)\n", ret);

	return ret;
}

/**
 * Module Exit.
 */
static void __exit ipa_test_exit(void)
{
	IPATEST_DBG("ipa_test_exit.\n");

	exception_hdl_exit(); /* Clear the Exception Device and KFIFO*/

	datapath_exit();

	destroy_channel_devices();

	cdev_del(ipa_test->cdev);
	device_destroy(ipa_test_class, ipa_test->dev_num);
	class_destroy(ipa_test_class);
	unregister_chrdev_region(ipa_test->dev_num, 1);

	kfree(ipa_test);

	IPATEST_DBG("ipa_test_exit complete.\n");
}

module_init(ipa_test_init);
module_exit(ipa_test_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("IPA Testing");
