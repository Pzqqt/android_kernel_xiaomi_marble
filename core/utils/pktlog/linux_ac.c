/*
 * Copyright (c) 2012-2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

#ifndef REMOVE_PKT_LOG
#ifndef EXPORT_SYMTAB
#define EXPORT_SYMTAB
#endif
#ifndef __KERNEL__
#define __KERNEL__
#endif
/*
 * Linux specific implementation of Pktlogs for 802.11ac
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/proc_fs.h>
#include <pktlog_ac_i.h>
#include <pktlog_ac_fmt.h>
#include <pktlog_ac.h>
#include "i_host_diag_core_log.h"
#include "host_diag_core_log.h"
#include "ani_global.h"

#define PKTLOG_TAG              "ATH_PKTLOG"
#define PKTLOG_DEVNAME_SIZE     32
#define MAX_WLANDEV             1

#define PKTLOG_PROC_DIR         "ath_pktlog"

/* Permissions for creating proc entries */
#define PKTLOG_PROC_PERM        0444
#define PKTLOG_PROCSYS_DIR_PERM 0555
#define PKTLOG_PROCSYS_PERM     0644

#ifndef __MOD_INC_USE_COUNT
#define PKTLOG_MOD_INC_USE_COUNT				\
	if (!try_module_get(THIS_MODULE)) {			\
		printk(KERN_WARNING "try_module_get failed\n");	\
	}

#define PKTLOG_MOD_DEC_USE_COUNT        module_put(THIS_MODULE)
#else
#define PKTLOG_MOD_INC_USE_COUNT        MOD_INC_USE_COUNT
#define PKTLOG_MOD_DEC_USE_COUNT        MOD_DEC_USE_COUNT
#endif

static struct ath_pktlog_info *g_pktlog_info;

static struct proc_dir_entry *g_pktlog_pde;

static int pktlog_attach(struct hif_opaque_softc *sc);
static void pktlog_detach(struct hif_opaque_softc *sc);
static int pktlog_open(struct inode *i, struct file *f);
static int pktlog_release(struct inode *i, struct file *f);
static int pktlog_mmap(struct file *f, struct vm_area_struct *vma);
static ssize_t pktlog_read(struct file *file, char *buf, size_t nbytes,
			   loff_t *ppos);

static struct file_operations pktlog_fops = {
	open:  pktlog_open,
	release:pktlog_release,
	mmap : pktlog_mmap,
	read : pktlog_read,
};

/*
 * Linux implementation of helper functions
 */

static struct ol_pktlog_dev_t *get_pl_handle(struct hif_opaque_softc *scn)
{
	ol_txrx_pdev_handle pdev_txrx_handle;
	pdev_txrx_handle = cds_get_context(QDF_MODULE_ID_TXRX);
	if (!pdev_txrx_handle)
		return NULL;
	return pdev_txrx_handle->pl_dev;
}

void ol_pl_set_name(hif_opaque_softc_handle scn, net_device_handle dev)
{
	ol_txrx_pdev_handle pdev_txrx_handle;
	pdev_txrx_handle = cds_get_context(QDF_MODULE_ID_TXRX);
	if (pdev_txrx_handle && pdev_txrx_handle->pl_dev && dev)
		pdev_txrx_handle->pl_dev->name = dev->name;
}

void pktlog_disable_adapter_logging(struct hif_opaque_softc *scn)
{
	struct ol_pktlog_dev_t *pl_dev = get_pl_handle(scn);
	if (pl_dev)
		pl_dev->pl_info->log_state = 0;
}

int pktlog_alloc_buf(struct hif_opaque_softc *scn)
{
	uint32_t page_cnt;
	unsigned long vaddr;
	struct page *vpg;
	struct ath_pktlog_info *pl_info;
	ol_txrx_pdev_handle pdev_txrx_handle;
	pdev_txrx_handle = cds_get_context(QDF_MODULE_ID_TXRX);

	if (!pdev_txrx_handle || !pdev_txrx_handle->pl_dev) {
		printk(PKTLOG_TAG
		       "%s: Unable to allocate buffer "
		       "scn or scn->pdev_txrx_handle->pl_dev is null\n",
		       __func__);
		return -EINVAL;
	}

	pl_info = pdev_txrx_handle->pl_dev->pl_info;

	page_cnt = (sizeof(*(pl_info->buf)) + pl_info->buf_size) / PAGE_SIZE;

	pl_info->buf = vmalloc((page_cnt + 2) * PAGE_SIZE);
	if (pl_info->buf == NULL) {
		printk(PKTLOG_TAG
		       "%s: Unable to allocate buffer "
		       "(%d pages)\n", __func__, page_cnt);
		return -ENOMEM;
	}

	pl_info->buf = (struct ath_pktlog_buf *)
		       (((unsigned long)(pl_info->buf) + PAGE_SIZE - 1)
			& PAGE_MASK);

	for (vaddr = (unsigned long)(pl_info->buf);
	     vaddr < ((unsigned long)(pl_info->buf) + (page_cnt * PAGE_SIZE));
	     vaddr += PAGE_SIZE) {
		vpg = vmalloc_to_page((const void *)vaddr);
		SetPageReserved(vpg);
	}

	return 0;
}

void pktlog_release_buf(struct hif_opaque_softc *scn)
{
	unsigned long page_cnt;
	unsigned long vaddr;
	struct page *vpg;
	struct ath_pktlog_info *pl_info;
	ol_txrx_pdev_handle pdev_txrx_handle;
	pdev_txrx_handle = cds_get_context(QDF_MODULE_ID_TXRX);

	if (!pdev_txrx_handle || !pdev_txrx_handle->pl_dev) {
		printk(PKTLOG_TAG
		       "%s: Unable to allocate buffer"
		       "scn or scn->pdev_txrx_handle->pl_dev is null\n",
		       __func__);
		return;
	}

	pl_info = pdev_txrx_handle->pl_dev->pl_info;

	page_cnt = ((sizeof(*(pl_info->buf)) + pl_info->buf_size) /
		    PAGE_SIZE) + 1;

	for (vaddr = (unsigned long)(pl_info->buf);
	     vaddr < (unsigned long)(pl_info->buf) + (page_cnt * PAGE_SIZE);
	     vaddr += PAGE_SIZE) {
		vpg = vmalloc_to_page((const void *)vaddr);
		ClearPageReserved(vpg);
	}

	vfree(pl_info->buf);
	pl_info->buf = NULL;
}

void pktlog_cleanup(struct ath_pktlog_info *pl_info)
{
	pl_info->log_state = 0;
	PKTLOG_LOCK_DESTROY(pl_info);
}

/* sysctl procfs handler to enable pktlog */
static int
qdf_sysctl_decl(ath_sysctl_pktlog_enable, ctl, write, filp, buffer, lenp, ppos)
{
	int ret, enable;
	ol_ath_generic_softc_handle scn;
	struct ol_pktlog_dev_t *pl_dev;

	scn = (ol_ath_generic_softc_handle) ctl->extra1;

	if (!scn) {
		printk("%s: Invalid scn context\n", __func__);
		ASSERT(0);
		return -EINVAL;
	}

	pl_dev = get_pl_handle((struct hif_opaque_softc *)scn);

	if (!pl_dev) {
		printk("%s: Invalid pktlog context\n", __func__);
		ASSERT(0);
		return -ENODEV;
	}

	ctl->data = &enable;
	ctl->maxlen = sizeof(enable);

	if (write) {
		ret = QDF_SYSCTL_PROC_DOINTVEC(ctl, write, filp, buffer,
					       lenp, ppos);
		if (ret == 0)
			ret = pl_dev->pl_funcs->pktlog_enable(
					(struct hif_opaque_softc *)scn, enable,
						cds_is_packet_log_enabled(), 0);
		else
			printk(PKTLOG_TAG "%s:proc_dointvec failed\n",
			       __func__);
	} else {
		ret = QDF_SYSCTL_PROC_DOINTVEC(ctl, write, filp, buffer,
					       lenp, ppos);
		if (ret)
			printk(PKTLOG_TAG "%s:proc_dointvec failed\n",
			       __func__);
	}

	ctl->data = NULL;
	ctl->maxlen = 0;

	return ret;
}

static int get_pktlog_bufsize(struct ol_pktlog_dev_t *pl_dev)
{
	return pl_dev->pl_info->buf_size;
}

/* sysctl procfs handler to set/get pktlog size */
static int
qdf_sysctl_decl(ath_sysctl_pktlog_size, ctl, write, filp, buffer, lenp, ppos)
{
	int ret, size;
	ol_ath_generic_softc_handle scn;
	struct ol_pktlog_dev_t *pl_dev;

	scn = (ol_ath_generic_softc_handle) ctl->extra1;

	if (!scn) {
		printk("%s: Invalid scn context\n", __func__);
		ASSERT(0);
		return -EINVAL;
	}

	pl_dev = get_pl_handle((struct hif_opaque_softc *)scn);

	if (!pl_dev) {
		printk("%s: Invalid pktlog handle\n", __func__);
		ASSERT(0);
		return -ENODEV;
	}

	ctl->data = &size;
	ctl->maxlen = sizeof(size);

	if (write) {
		ret = QDF_SYSCTL_PROC_DOINTVEC(ctl, write, filp, buffer,
					       lenp, ppos);
		if (ret == 0)
			ret = pl_dev->pl_funcs->pktlog_setsize(
					(struct hif_opaque_softc *)scn, size);
	} else {
		size = get_pktlog_bufsize(pl_dev);
		ret = QDF_SYSCTL_PROC_DOINTVEC(ctl, write, filp, buffer,
					       lenp, ppos);
	}

	ctl->data = NULL;
	ctl->maxlen = 0;

	return ret;
}

/* Register sysctl table */
static int pktlog_sysctl_register(struct hif_opaque_softc *scn)
{
	struct ol_pktlog_dev_t *pl_dev = get_pl_handle(scn);
	struct ath_pktlog_info_lnx *pl_info_lnx;
	char *proc_name;

	if (pl_dev) {
		pl_info_lnx = PL_INFO_LNX(pl_dev->pl_info);
		proc_name = pl_dev->name;
	} else {
		pl_info_lnx = PL_INFO_LNX(g_pktlog_info);
		proc_name = PKTLOG_PROC_SYSTEM;
	}

	/*
	 * Setup the sysctl table for creating the following sysctl entries:
	 * /proc/sys/PKTLOG_PROC_DIR/<adapter>/enable for enabling/disabling
	 * pktlog
	 * /proc/sys/PKTLOG_PROC_DIR/<adapter>/size for changing the buffer size
	 */
	memset(pl_info_lnx->sysctls, 0, sizeof(pl_info_lnx->sysctls));
	pl_info_lnx->sysctls[0].procname = PKTLOG_PROC_DIR;
	pl_info_lnx->sysctls[0].mode = PKTLOG_PROCSYS_DIR_PERM;
	pl_info_lnx->sysctls[0].child = &pl_info_lnx->sysctls[2];

	/* [1] is NULL terminator */
	pl_info_lnx->sysctls[2].procname = proc_name;
	pl_info_lnx->sysctls[2].mode = PKTLOG_PROCSYS_DIR_PERM;
	pl_info_lnx->sysctls[2].child = &pl_info_lnx->sysctls[4];

	/* [3] is NULL terminator */
	pl_info_lnx->sysctls[4].procname = "enable";
	pl_info_lnx->sysctls[4].mode = PKTLOG_PROCSYS_PERM;
	pl_info_lnx->sysctls[4].proc_handler = ath_sysctl_pktlog_enable;
	pl_info_lnx->sysctls[4].extra1 = scn;

	pl_info_lnx->sysctls[5].procname = "size";
	pl_info_lnx->sysctls[5].mode = PKTLOG_PROCSYS_PERM;
	pl_info_lnx->sysctls[5].proc_handler = ath_sysctl_pktlog_size;
	pl_info_lnx->sysctls[5].extra1 = scn;

	pl_info_lnx->sysctls[6].procname = "options";
	pl_info_lnx->sysctls[6].mode = PKTLOG_PROCSYS_PERM;
	pl_info_lnx->sysctls[6].proc_handler = proc_dointvec;
	pl_info_lnx->sysctls[6].data = &pl_info_lnx->info.options;
	pl_info_lnx->sysctls[6].maxlen = sizeof(pl_info_lnx->info.options);

	pl_info_lnx->sysctls[7].procname = "sack_thr";
	pl_info_lnx->sysctls[7].mode = PKTLOG_PROCSYS_PERM;
	pl_info_lnx->sysctls[7].proc_handler = proc_dointvec;
	pl_info_lnx->sysctls[7].data = &pl_info_lnx->info.sack_thr;
	pl_info_lnx->sysctls[7].maxlen = sizeof(pl_info_lnx->info.sack_thr);

	pl_info_lnx->sysctls[8].procname = "tail_length";
	pl_info_lnx->sysctls[8].mode = PKTLOG_PROCSYS_PERM;
	pl_info_lnx->sysctls[8].proc_handler = proc_dointvec;
	pl_info_lnx->sysctls[8].data = &pl_info_lnx->info.tail_length;
	pl_info_lnx->sysctls[8].maxlen = sizeof(pl_info_lnx->info.tail_length);

	pl_info_lnx->sysctls[9].procname = "thruput_thresh";
	pl_info_lnx->sysctls[9].mode = PKTLOG_PROCSYS_PERM;
	pl_info_lnx->sysctls[9].proc_handler = proc_dointvec;
	pl_info_lnx->sysctls[9].data = &pl_info_lnx->info.thruput_thresh;
	pl_info_lnx->sysctls[9].maxlen =
		sizeof(pl_info_lnx->info.thruput_thresh);

	pl_info_lnx->sysctls[10].procname = "phyerr_thresh";
	pl_info_lnx->sysctls[10].mode = PKTLOG_PROCSYS_PERM;
	pl_info_lnx->sysctls[10].proc_handler = proc_dointvec;
	pl_info_lnx->sysctls[10].data = &pl_info_lnx->info.phyerr_thresh;
	pl_info_lnx->sysctls[10].maxlen =
		sizeof(pl_info_lnx->info.phyerr_thresh);

	pl_info_lnx->sysctls[11].procname = "per_thresh";
	pl_info_lnx->sysctls[11].mode = PKTLOG_PROCSYS_PERM;
	pl_info_lnx->sysctls[11].proc_handler = proc_dointvec;
	pl_info_lnx->sysctls[11].data = &pl_info_lnx->info.per_thresh;
	pl_info_lnx->sysctls[11].maxlen = sizeof(pl_info_lnx->info.per_thresh);

	pl_info_lnx->sysctls[12].procname = "trigger_interval";
	pl_info_lnx->sysctls[12].mode = PKTLOG_PROCSYS_PERM;
	pl_info_lnx->sysctls[12].proc_handler = proc_dointvec;
	pl_info_lnx->sysctls[12].data = &pl_info_lnx->info.trigger_interval;
	pl_info_lnx->sysctls[12].maxlen =
		sizeof(pl_info_lnx->info.trigger_interval);
	/* [13] is NULL terminator */

	/* and register everything */
	/* register_sysctl_table changed from 2.6.21 onwards */
	pl_info_lnx->sysctl_header =
		register_sysctl_table(pl_info_lnx->sysctls);

	if (!pl_info_lnx->sysctl_header) {
		printk("%s: failed to register sysctls!\n", proc_name);
		return -1;
	}

	return 0;
}

/*
 * Initialize logging for system or adapter
 * Parameter scn should be NULL for system wide logging
 */
static int pktlog_attach(struct hif_opaque_softc *scn)
{
	struct ol_pktlog_dev_t *pl_dev;
	struct ath_pktlog_info_lnx *pl_info_lnx;
	char *proc_name;
	struct proc_dir_entry *proc_entry;

	pl_dev = get_pl_handle(scn);

	if (pl_dev != NULL) {
		pl_info_lnx = kmalloc(sizeof(*pl_info_lnx), GFP_KERNEL);
		if (pl_info_lnx == NULL) {
			printk(PKTLOG_TAG "%s:allocation failed for pl_info\n",
			       __func__);
			return -ENOMEM;
		}
		pl_dev->pl_info = &pl_info_lnx->info;
		pl_dev->name = WLANDEV_BASENAME;
		proc_name = pl_dev->name;
		if (!pl_dev->pl_funcs)
			pl_dev->pl_funcs = &ol_pl_funcs;

		/*
		 * Valid for both direct attach and offload architecture
		 */
		pl_dev->pl_funcs->pktlog_init(scn);
	} else {
		return -1;
	}

	/*
	 * initialize log info
	 * might be good to move to pktlog_init
	 */
	/* pl_dev->tgt_pktlog_alloced = false; */
	pl_info_lnx->proc_entry = NULL;
	pl_info_lnx->sysctl_header = NULL;

	proc_entry = proc_create_data(proc_name, PKTLOG_PROC_PERM,
				      g_pktlog_pde, &pktlog_fops,
				      &pl_info_lnx->info);

	if (proc_entry == NULL) {
		printk(PKTLOG_TAG "%s: create_proc_entry failed for %s\n",
		       __func__, proc_name);
		goto attach_fail1;
	}

	pl_info_lnx->proc_entry = proc_entry;

	if (pktlog_sysctl_register(scn)) {
		printk(PKTLOG_TAG "%s: sysctl register failed for %s\n",
		       __func__, proc_name);
		goto attach_fail2;
	}
	return 0;

attach_fail2:
	remove_proc_entry(proc_name, g_pktlog_pde);

attach_fail1:
	if (pl_dev)
		kfree(pl_dev->pl_info);
	return -1;
}

static void pktlog_sysctl_unregister(struct ol_pktlog_dev_t *pl_dev)
{
	struct ath_pktlog_info_lnx *pl_info_lnx;

	if (!pl_dev) {
		printk("%s: Invalid pktlog context\n", __func__);
		ASSERT(0);
		return;
	}

	pl_info_lnx = (pl_dev) ? PL_INFO_LNX(pl_dev->pl_info) :
		      PL_INFO_LNX(g_pktlog_info);

	if (pl_info_lnx->sysctl_header) {
		unregister_sysctl_table(pl_info_lnx->sysctl_header);
		pl_info_lnx->sysctl_header = NULL;
	}
}

static void pktlog_detach(struct hif_opaque_softc *scn)
{
	struct ol_txrx_pdev_t *txrx_pdev;
	struct ol_pktlog_dev_t *pl_dev;
	struct ath_pktlog_info *pl_info;

	txrx_pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	if (!txrx_pdev) {
		printk("%s: Invalid txrx_pdev context\n", __func__);
		ASSERT(0);
		return;
	}

	pl_dev = txrx_pdev->pl_dev;
	if (!pl_dev) {
		printk("%s: Invalid pktlog context\n", __func__);
		ASSERT(0);
		return;
	}

	pl_info = pl_dev->pl_info;
	remove_proc_entry(WLANDEV_BASENAME, g_pktlog_pde);
	pktlog_sysctl_unregister(pl_dev);
	pktlog_cleanup(pl_info);

	if (pl_info->buf) {
		pktlog_release_buf(scn);
		pl_dev->tgt_pktlog_alloced = false;
	}

	if (pl_dev) {
		kfree(pl_info);
		pl_dev->pl_info = NULL;
	}
}

static int pktlog_open(struct inode *i, struct file *f)
{
	PKTLOG_MOD_INC_USE_COUNT;
	return 0;
}

static int pktlog_release(struct inode *i, struct file *f)
{
	PKTLOG_MOD_DEC_USE_COUNT;
	return 0;
}

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

/**
 * pktlog_send_per_pkt_stats_to_user() - This function is used to send the per
 * packet statistics to the user
 *
 * This function is used to send the per packet statistics to the user
 *
 * Return: Success if the message is posted to user
 *
 */
int pktlog_send_per_pkt_stats_to_user(void)
{
	ssize_t ret_val;
	struct host_log_pktlog_info *pktlog = NULL;
	ol_txrx_pdev_handle txrx_pdev =
		cds_get_context(QDF_MODULE_ID_TXRX);
	struct ath_pktlog_info *pl_info;
	bool read_complete;
	uint32_t num_bytes_read = 0;

	/*
	 * We do not want to do this packet stats related processing when
	 * packet log tool is run. i.e., we want this processing to be
	 * done only when start logging command of packet stats is initiated.
	 */
	if ((cds_get_ring_log_level(RING_ID_PER_PACKET_STATS) <
				WLAN_LOG_LEVEL_ACTIVE)) {
		printk(PKTLOG_TAG " %s: Shouldnt happen. Logging not started\n",
				__func__);
		return -EINVAL;
	}

	if (!txrx_pdev) {
		printk(PKTLOG_TAG " %s: Invalid TxRx handle\n", __func__);
		return -EINVAL;
	}

	pl_info = txrx_pdev->pl_dev->pl_info;

	if (!pl_info || !pl_info->buf) {
		printk(PKTLOG_TAG " %s: Shouldnt happen. pl_info is invalid\n",
				__func__);
		return -EINVAL;
	}

	if (pl_info->buf->rd_offset == -1) {
		printk(PKTLOG_TAG " %s: Shouldnt happen. No write yet!\n",
			__func__);
		return -EINVAL;
	}

	do {
		pktlog = (struct host_log_pktlog_info *)
			qdf_mem_malloc(sizeof(struct host_log_pktlog_info) +
					VOS_LOG_PKT_LOG_SIZE);
		if (!pktlog) {
			printk(PKTLOG_TAG " %s: Memory allocation failed\n",
				__func__);
			return -ENOMEM;
		}

		qdf_mem_zero(pktlog, VOS_LOG_PKT_LOG_SIZE);
		host_diag_log_set_code(pktlog, LOG_WLAN_PKT_LOG_INFO_C);

		pktlog->buf_len = 0;
		pktlog->version = VERSION_LOG_WLAN_PKT_LOG_INFO_C;

		/*
		 * @ret_val: ret_val gives the actual data read from the buffer.
		 * When there is no more data to read, this value will be zero
		 * @offset: offset in the ring buffer. Initially it is zero and
		 * is incremented during every read based on number of bytes
		 * read
		 */
		ret_val = pktlog_read_proc_entry(pktlog->buf,
				VOS_LOG_PKT_LOG_SIZE,
				&pl_info->buf->offset,
				pl_info, &read_complete);
		if (ret_val) {
			int index = 0;
			struct ath_pktlog_hdr *temp;
			while (1) {
				if ((ret_val - index) <
						sizeof(struct ath_pktlog_hdr)) {
					/* Partial header */
					pl_info->buf->offset -=
							(ret_val - index);
					ret_val = index;
					break;
				}
				temp = (struct ath_pktlog_hdr *)
					(pktlog->buf + index);
				if ((ret_val - index) < (temp->size +
					sizeof(struct ath_pktlog_hdr))) {
					/* Partial record payload */
					pl_info->buf->offset -=
							(ret_val - index);
					ret_val = index;
					break;
				}
				index += temp->size +
					sizeof(struct ath_pktlog_hdr);
			}
		}

		/* Data will include message index/seq number and buf length */
		pktlog->buf_len = ret_val;
		if (ret_val) {
			host_diag_log_set_length(pktlog, ret_val +
					sizeof(struct host_log_pktlog_info));
			pktlog->seq_no = pl_info->buf->msg_index++;
			WLAN_HOST_DIAG_LOG_REPORT(pktlog);
		} else {
			qdf_mem_free(pktlog);
		}
		num_bytes_read += ret_val;

		/*
		 * If the logger thread is scheduled late and the proc entry
		 * is having too much data to be read, we might start to starve
		 * the other threads if we continuously keep reading the proc
		 * entry. So, having a threshold to break this read from proc
		 * entry.
		 */
		if (num_bytes_read > HOST_LOG_PKT_LOG_THRESHOLD) {
			read_complete = true;
			printk(PKTLOG_TAG " %s: Break read to prevent starve\n",
				__func__);
		}
	} while (read_complete == false);

	return 0;
}

/**
 * pktlog_read_proc_entry() - This function is used to read data from the
 * proc entry into the readers buffer
 * @buf:     Readers buffer
 * @nbytes:  Number of bytes to read
 * @ppos:    Offset within the drivers buffer
 * @pl_info: Packet log information pointer
 * @read_complete: Boolean value indication whether read is complete
 *
 * This function is used to read data from the proc entry into the readers
 * buffer. Its functionality is similar to 'pktlog_read' which does
 * copy to user to the user space buffer
 *
 * Return: Number of bytes read from the buffer
 *
 */
	ssize_t
pktlog_read_proc_entry(char *buf, size_t nbytes, loff_t *ppos,
		struct ath_pktlog_info *pl_info, bool *read_complete)
{
	size_t bufhdr_size;
	size_t count = 0, ret_val = 0;
	int rem_len;
	int start_offset, end_offset;
	int fold_offset, ppos_data, cur_rd_offset, cur_wr_offset;
	struct ath_pktlog_buf *log_buf = pl_info->buf;
	*read_complete = false;

	if (log_buf == NULL) {
		*read_complete = true;
		return 0;
	}

	if (*ppos == 0 && pl_info->log_state) {
		pl_info->saved_state = pl_info->log_state;
		pl_info->log_state = 0;
	}

	bufhdr_size = sizeof(log_buf->bufhdr);

	/* copy valid log entries from circular buffer into user space */
	rem_len = nbytes;
	count = 0;

	if (*ppos < bufhdr_size) {
		count = MIN((bufhdr_size - *ppos), rem_len);
		qdf_mem_copy(buf, ((char *)&log_buf->bufhdr) + *ppos,
				count);
		rem_len -= count;
		ret_val += count;
	}

	start_offset = log_buf->rd_offset;
	cur_wr_offset = log_buf->wr_offset;

	if ((rem_len == 0) || (start_offset < 0))
		goto rd_done;

	fold_offset = -1;
	cur_rd_offset = start_offset;

	/* Find the last offset and fold-offset if the buffer is folded */
	do {
		struct ath_pktlog_hdr *log_hdr;
		int log_data_offset;

		log_hdr = (struct ath_pktlog_hdr *) (log_buf->log_data +
				cur_rd_offset);

		log_data_offset = cur_rd_offset + sizeof(struct ath_pktlog_hdr);

		if ((fold_offset == -1)
				&& ((pl_info->buf_size - log_data_offset)
					<= log_hdr->size))
			fold_offset = log_data_offset - 1;

		PKTLOG_MOV_RD_IDX(cur_rd_offset, log_buf, pl_info->buf_size);

		if ((fold_offset == -1) && (cur_rd_offset == 0)
				&& (cur_rd_offset != cur_wr_offset))
			fold_offset = log_data_offset + log_hdr->size - 1;

		end_offset = log_data_offset + log_hdr->size - 1;
	} while (cur_rd_offset != cur_wr_offset);

	ppos_data = *ppos + ret_val - bufhdr_size + start_offset;

	if (fold_offset == -1) {
		if (ppos_data > end_offset)
			goto rd_done;

		count = MIN(rem_len, (end_offset - ppos_data + 1));
		qdf_mem_copy(buf + ret_val,
				log_buf->log_data + ppos_data,
				count);
		ret_val += count;
		rem_len -= count;
	} else {
		if (ppos_data <= fold_offset) {
			count = MIN(rem_len, (fold_offset - ppos_data + 1));
			qdf_mem_copy(buf + ret_val,
					log_buf->log_data + ppos_data,
					count);
			ret_val += count;
			rem_len -= count;
		}

		if (rem_len == 0)
			goto rd_done;

		ppos_data =
			*ppos + ret_val - (bufhdr_size +
					(fold_offset - start_offset + 1));

		if (ppos_data <= end_offset) {
			count = MIN(rem_len, (end_offset - ppos_data + 1));
			qdf_mem_copy(buf + ret_val,
					log_buf->log_data + ppos_data,
					count);
			ret_val += count;
			rem_len -= count;
		}
	}

rd_done:
	if ((ret_val < nbytes) && pl_info->saved_state) {
		pl_info->log_state = pl_info->saved_state;
		pl_info->saved_state = 0;
	}
	*ppos += ret_val;

	if (ret_val == 0) {
		PKTLOG_LOCK(pl_info);
		/* Write pointer might have been updated during the read.
		 * So, if some data is written into, lets not reset the pointers
		 * We can continue to read from the offset position
		 */
		if (cur_wr_offset != log_buf->wr_offset) {
			*read_complete = false;
		} else {
			pl_info->buf->rd_offset = -1;
			pl_info->buf->wr_offset = 0;
			pl_info->buf->bytes_written = 0;
			pl_info->buf->offset = PKTLOG_READ_OFFSET;
			*read_complete = true;
		}
		PKTLOG_UNLOCK(pl_info);
	}

	return ret_val;
}

static ssize_t
pktlog_read(struct file *file, char *buf, size_t nbytes, loff_t *ppos)
{
	size_t bufhdr_size;
	size_t count = 0, ret_val = 0;
	int rem_len;
	int start_offset, end_offset;
	int fold_offset, ppos_data, cur_rd_offset;
	struct ath_pktlog_info *pl_info;
	struct ath_pktlog_buf *log_buf;

	pl_info = (struct ath_pktlog_info *)
					PDE_DATA(file->f_path.dentry->d_inode);
	if (!pl_info)
		return 0;

	log_buf = pl_info->buf;

	if (log_buf == NULL)
		return 0;

	if (*ppos == 0 && pl_info->log_state) {
		pl_info->saved_state = pl_info->log_state;
		pl_info->log_state = 0;
	}

	bufhdr_size = sizeof(log_buf->bufhdr);

	/* copy valid log entries from circular buffer into user space */
	rem_len = nbytes;
	count = 0;

	if (*ppos < bufhdr_size) {
		count = QDF_MIN((bufhdr_size - *ppos), rem_len);
		if (copy_to_user(buf, ((char *)&log_buf->bufhdr) + *ppos,
				 count))
			return -EFAULT;
		rem_len -= count;
		ret_val += count;
	}

	start_offset = log_buf->rd_offset;

	if ((rem_len == 0) || (start_offset < 0))
		goto rd_done;

	fold_offset = -1;
	cur_rd_offset = start_offset;

	/* Find the last offset and fold-offset if the buffer is folded */
	do {
		struct ath_pktlog_hdr *log_hdr;
		int log_data_offset;

		log_hdr = (struct ath_pktlog_hdr *)(log_buf->log_data +
						    cur_rd_offset);

		log_data_offset = cur_rd_offset + sizeof(struct ath_pktlog_hdr);

		if ((fold_offset == -1)
		    && ((pl_info->buf_size - log_data_offset)
			<= log_hdr->size))
			fold_offset = log_data_offset - 1;

		PKTLOG_MOV_RD_IDX(cur_rd_offset, log_buf, pl_info->buf_size);

		if ((fold_offset == -1) && (cur_rd_offset == 0)
		    && (cur_rd_offset != log_buf->wr_offset))
			fold_offset = log_data_offset + log_hdr->size - 1;

		end_offset = log_data_offset + log_hdr->size - 1;
	} while (cur_rd_offset != log_buf->wr_offset);

	ppos_data = *ppos + ret_val - bufhdr_size + start_offset;

	if (fold_offset == -1) {
		if (ppos_data > end_offset)
			goto rd_done;

		count = QDF_MIN(rem_len, (end_offset - ppos_data + 1));
		if (copy_to_user(buf + ret_val,
				 log_buf->log_data + ppos_data, count))
			return -EFAULT;
		ret_val += count;
		rem_len -= count;
	} else {
		if (ppos_data <= fold_offset) {
			count = QDF_MIN(rem_len, (fold_offset - ppos_data + 1));
			if (copy_to_user(buf + ret_val,
					 log_buf->log_data + ppos_data, count))
				return -EFAULT;
			ret_val += count;
			rem_len -= count;
		}

		if (rem_len == 0)
			goto rd_done;

		ppos_data =
			*ppos + ret_val - (bufhdr_size +
					   (fold_offset - start_offset + 1));

		if (ppos_data <= end_offset) {
			count = QDF_MIN(rem_len, (end_offset - ppos_data + 1));
			if (copy_to_user(buf + ret_val,
					 log_buf->log_data + ppos_data, count))
				return -EFAULT;
			ret_val += count;
			rem_len -= count;
		}
	}

rd_done:
	if ((ret_val < nbytes) && pl_info->saved_state) {
		pl_info->log_state = pl_info->saved_state;
		pl_info->saved_state = 0;
	}
	*ppos += ret_val;

	return ret_val;
}

#ifndef VMALLOC_VMADDR
#define VMALLOC_VMADDR(x) ((unsigned long)(x))
#endif

/* vma operations for mapping vmalloced area to user space */
static void pktlog_vopen(struct vm_area_struct *vma)
{
	PKTLOG_MOD_INC_USE_COUNT;
}

static void pktlog_vclose(struct vm_area_struct *vma)
{
	PKTLOG_MOD_DEC_USE_COUNT;
}

int pktlog_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
	unsigned long address = (unsigned long)vmf->virtual_address;

	if (address == 0UL)
		return VM_FAULT_NOPAGE;

	if (vmf->pgoff > vma->vm_end)
		return VM_FAULT_SIGBUS;

	get_page(virt_to_page((void *)address));
	vmf->page = virt_to_page((void *)address);
	return VM_FAULT_MINOR;
}
static struct vm_operations_struct pktlog_vmops = {
	open:  pktlog_vopen,
	close:pktlog_vclose,
	fault:pktlog_fault,
};

static int pktlog_mmap(struct file *file, struct vm_area_struct *vma)
{
	struct ath_pktlog_info *pl_info;

	pl_info = (struct ath_pktlog_info *)
					PDE_DATA(file->f_path.dentry->d_inode);

	if (vma->vm_pgoff != 0) {
		/* Entire buffer should be mapped */
		return -EINVAL;
	}

	if (!pl_info->buf) {
		printk(PKTLOG_TAG "%s: Log buffer unavailable\n", __func__);
		return -ENOMEM;
	}

	vma->vm_flags |= VM_LOCKED;
	vma->vm_ops = &pktlog_vmops;
	pktlog_vopen(vma);
	return 0;
}

int pktlogmod_init(void *context)
{
	int ret;

	/* create the proc directory entry */
	g_pktlog_pde = proc_mkdir(PKTLOG_PROC_DIR, NULL);

	if (g_pktlog_pde == NULL) {
		printk(PKTLOG_TAG "%s: proc_mkdir failed\n", __func__);
		return -1;
	}

	/* Attach packet log */
	ret = pktlog_attach((struct hif_opaque_softc *)context);

	if (ret)
		goto attach_fail;

	return ret;

attach_fail:
	remove_proc_entry(PKTLOG_PROC_DIR, NULL);
	g_pktlog_pde = NULL;
	return ret;
}

void pktlogmod_exit(void *context)
{
	struct hif_opaque_softc *scn = (struct hif_opaque_softc *)context;
	struct ol_pktlog_dev_t *pl_dev;

	if (!scn)
		return;

	pl_dev = get_pl_handle(scn);

	if (!pl_dev || g_pktlog_pde == NULL)
		return;

	pktlog_detach(scn);
	/*
	 *  pdev kill needs to be implemented
	 */
	remove_proc_entry(PKTLOG_PROC_DIR, NULL);
}
#endif
