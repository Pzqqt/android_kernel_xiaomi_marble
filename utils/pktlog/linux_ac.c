/*
 * Copyright (c) 2012-2019 The Linux Foundation. All rights reserved.
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
#include "i_host_diag_core_log.h"
#include "host_diag_core_log.h"
#include "ani_global.h"

#define PKTLOG_TAG              "ATH_PKTLOG"
#define PKTLOG_DEVNAME_SIZE     32
#define MAX_WLANDEV             1

#ifdef MULTI_IF_NAME
#define PKTLOG_PROC_DIR         "ath_pktlog" MULTI_IF_NAME
#else
#define PKTLOG_PROC_DIR         "ath_pktlog"
#endif

/* Permissions for creating proc entries */
#define PKTLOG_PROC_PERM        0444
#define PKTLOG_PROCSYS_DIR_PERM 0555
#define PKTLOG_PROCSYS_PERM     0644

#ifndef __MOD_INC_USE_COUNT
#define PKTLOG_MOD_INC_USE_COUNT	do {			\
	if (!try_module_get(THIS_MODULE)) {			\
		printk(KERN_WARNING "try_module_get failed\n");	\
	} } while (0)

#define PKTLOG_MOD_DEC_USE_COUNT        module_put(THIS_MODULE)
#else
#define PKTLOG_MOD_INC_USE_COUNT        MOD_INC_USE_COUNT
#define PKTLOG_MOD_DEC_USE_COUNT        MOD_DEC_USE_COUNT
#endif

static struct ath_pktlog_info *g_pktlog_info;

static struct proc_dir_entry *g_pktlog_pde;

static DEFINE_MUTEX(proc_mutex);

static int pktlog_attach(struct hif_opaque_softc *scn);
static void pktlog_detach(struct hif_opaque_softc *scn);
static int pktlog_open(struct inode *i, struct file *f);
static int pktlog_release(struct inode *i, struct file *f);
static ssize_t pktlog_read(struct file *file, char *buf, size_t nbytes,
			   loff_t *ppos);

static struct file_operations pktlog_fops = {
	open:  pktlog_open,
	release:pktlog_release,
	read : pktlog_read,
};

void pktlog_disable_adapter_logging(struct hif_opaque_softc *scn)
{
	struct pktlog_dev_t *pl_dev = get_pktlog_handle();
	if (pl_dev)
		pl_dev->pl_info->log_state = 0;
}

int pktlog_alloc_buf(struct hif_opaque_softc *scn)
{
	uint32_t page_cnt;
	unsigned long vaddr;
	struct page *vpg;
	struct pktlog_dev_t *pl_dev;
	struct ath_pktlog_info *pl_info;
	struct ath_pktlog_buf *buffer;

	pl_dev = get_pktlog_handle();

	if (!pl_dev) {
		printk(PKTLOG_TAG
		       "%s: Unable to allocate buffer pdev_txrx_handle or pdev_txrx_handle->pl_dev is null\n",
		       __func__);
		return -EINVAL;
	}

	pl_info = pl_dev->pl_info;

	page_cnt = (sizeof(*(pl_info->buf)) + pl_info->buf_size) / PAGE_SIZE;

	qdf_spin_lock_bh(&pl_info->log_lock);
	if (pl_info->buf) {
		qdf_spin_unlock_bh(&pl_info->log_lock);
		printk(PKTLOG_TAG "Buffer is already in use\n");
		return -EINVAL;
	}
	qdf_spin_unlock_bh(&pl_info->log_lock);

	buffer = vmalloc((page_cnt + 2) * PAGE_SIZE);
	if (!buffer) {
		printk(PKTLOG_TAG
		       "%s: Unable to allocate buffer "
		       "(%d pages)\n", __func__, page_cnt);
		return -ENOMEM;
	}

	buffer = (struct ath_pktlog_buf *)
		       (((unsigned long)(buffer) + PAGE_SIZE - 1)
			& PAGE_MASK);

	for (vaddr = (unsigned long)(buffer);
	     vaddr < ((unsigned long)(buffer) + (page_cnt * PAGE_SIZE));
	     vaddr += PAGE_SIZE) {
		vpg = vmalloc_to_page((const void *)vaddr);
		SetPageReserved(vpg);
	}

	qdf_spin_lock_bh(&pl_info->log_lock);
	if (pl_info->buf)
		pktlog_release_buf(scn);

	pl_info->buf =  buffer;
	qdf_spin_unlock_bh(&pl_info->log_lock);
	return 0;
}

void pktlog_release_buf(struct hif_opaque_softc *scn)
{
	unsigned long page_cnt;
	unsigned long vaddr;
	struct page *vpg;
	struct pktlog_dev_t *pl_dev;
	struct ath_pktlog_info *pl_info;

	pl_dev = get_pktlog_handle();

	if (!pl_dev) {
		qdf_print("%s: invalid pl_dev handle", __func__);
		return;
	}

	if (!pl_dev->pl_info) {
		qdf_print("%s: invalid pl_dev handle", __func__);
		return;
	}

	pl_info = pl_dev->pl_info;

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

static void pktlog_cleanup(struct ath_pktlog_info *pl_info)
{
	pl_info->log_state = 0;
	PKTLOG_LOCK_DESTROY(pl_info);
	mutex_destroy(&pl_info->pktlog_mutex);
}

/* sysctl procfs handler to enable pktlog */
static int
qdf_sysctl_decl(ath_sysctl_pktlog_enable, ctl, write, filp, buffer, lenp, ppos)
{
	int ret, enable;
	ol_ath_generic_softc_handle scn;
	struct pktlog_dev_t *pl_dev;

	mutex_lock(&proc_mutex);
	scn = (ol_ath_generic_softc_handle) ctl->extra1;

	if (!scn) {
		mutex_unlock(&proc_mutex);
		printk("%s: Invalid scn context\n", __func__);
		ASSERT(0);
		return -EINVAL;
	}

	pl_dev = get_pktlog_handle();

	if (!pl_dev) {
		mutex_unlock(&proc_mutex);
		printk("%s: Invalid pktlog context\n", __func__);
		ASSERT(0);
		return -ENODEV;
	}

	ctl->data = &enable;
	ctl->maxlen = sizeof(enable);

	if (write) {
		ret = QDF_SYSCTL_PROC_DOINTVEC(ctl, write, filp, buffer,
					       lenp, ppos);
		if (ret == 0) {
			ret = pl_dev->pl_funcs->pktlog_enable(
					(struct hif_opaque_softc *)scn, enable,
					cds_is_packet_log_enabled(), 0, 1);
		}
		else
			QDF_TRACE(QDF_MODULE_ID_SYS, QDF_TRACE_LEVEL_DEBUG,
				  "Line:%d %s:proc_dointvec failed reason %d",
				   __LINE__, __func__, ret);
	} else {
		ret = QDF_SYSCTL_PROC_DOINTVEC(ctl, write, filp, buffer,
					       lenp, ppos);
		if (ret)
			QDF_TRACE(QDF_MODULE_ID_SYS, QDF_TRACE_LEVEL_DEBUG,
				  "Line:%d %s:proc_dointvec failed reason %d",
				   __LINE__, __func__, ret);
	}

	ctl->data = NULL;
	ctl->maxlen = 0;
	mutex_unlock(&proc_mutex);

	return ret;
}

static int get_pktlog_bufsize(struct pktlog_dev_t *pl_dev)
{
	return pl_dev->pl_info->buf_size;
}

/* sysctl procfs handler to set/get pktlog size */
static int
qdf_sysctl_decl(ath_sysctl_pktlog_size, ctl, write, filp, buffer, lenp, ppos)
{
	int ret, size;
	ol_ath_generic_softc_handle scn;
	struct pktlog_dev_t *pl_dev;

	mutex_lock(&proc_mutex);
	scn = (ol_ath_generic_softc_handle) ctl->extra1;

	if (!scn) {
		mutex_unlock(&proc_mutex);
		printk("%s: Invalid scn context\n", __func__);
		ASSERT(0);
		return -EINVAL;
	}

	pl_dev = get_pktlog_handle();

	if (!pl_dev) {
		mutex_unlock(&proc_mutex);
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
	mutex_unlock(&proc_mutex);

	return ret;
}

/* Register sysctl table */
static int pktlog_sysctl_register(struct hif_opaque_softc *scn)
{
	struct pktlog_dev_t *pl_dev = get_pktlog_handle();
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
		return -EINVAL;
	}

	return 0;
}

/*
 * Initialize logging for system or adapter
 * Parameter scn should be NULL for system wide logging
 */
static int pktlog_attach(struct hif_opaque_softc *scn)
{
	struct pktlog_dev_t *pl_dev;
	struct ath_pktlog_info_lnx *pl_info_lnx;
	char *proc_name;
	struct proc_dir_entry *proc_entry;

	/* Allocate pktlog dev for later use */
	pl_dev = get_pktlog_handle();

	if (pl_dev) {
		pl_info_lnx = kmalloc(sizeof(*pl_info_lnx), GFP_KERNEL);
		if (!pl_info_lnx) {
			QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
				 "%s: Allocation failed for pl_info",
				 __func__);
			goto attach_fail1;
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
		return -EINVAL;
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

	if (!proc_entry) {
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

	return -EINVAL;
}

static void pktlog_sysctl_unregister(struct pktlog_dev_t *pl_dev)
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
	struct ath_pktlog_info *pl_info;
	struct pktlog_dev_t *pl_dev = get_pktlog_handle();

	if (!pl_dev) {
		printk("%s: Invalid pktlog context\n", __func__);
		ASSERT(0);
		return;
	}

	pl_info = pl_dev->pl_info;
	remove_proc_entry(WLANDEV_BASENAME, g_pktlog_pde);
	pktlog_sysctl_unregister(pl_dev);

	qdf_spin_lock_bh(&pl_info->log_lock);

	if (pl_info->buf) {
		pktlog_release_buf(scn);
		pl_dev->tgt_pktlog_alloced = false;
	}
	qdf_spin_unlock_bh(&pl_info->log_lock);
	pktlog_cleanup(pl_info);

	if (pl_dev) {
		kfree(pl_info);
		pl_dev->pl_info = NULL;
	}
}

static int __pktlog_open(struct inode *i, struct file *f)
{
	struct hif_opaque_softc *scn;
	struct pktlog_dev_t *pl_dev;
	struct ath_pktlog_info *pl_info;
	int ret = 0;

	PKTLOG_MOD_INC_USE_COUNT;
	pl_info = PDE_DATA(f->f_path.dentry->d_inode);
	if (!pl_info) {
		pr_err("%s: pl_info NULL", __func__);
		return -EINVAL;
	}

	if (pl_info->curr_pkt_state != PKTLOG_OPR_NOT_IN_PROGRESS) {
		pr_info("%s: plinfo state (%d) != PKTLOG_OPR_NOT_IN_PROGRESS",
			__func__, pl_info->curr_pkt_state);
		return -EBUSY;
	}

	pl_info->curr_pkt_state = PKTLOG_OPR_IN_PROGRESS_READ_START;
	scn = cds_get_context(QDF_MODULE_ID_HIF);
	if (!scn) {
		pl_info->curr_pkt_state = PKTLOG_OPR_NOT_IN_PROGRESS;
		qdf_print("%s: Invalid scn context", __func__);
		ASSERT(0);
		return -EINVAL;
	}

	pl_dev = get_pktlog_handle();

	if (!pl_dev) {
		pl_info->curr_pkt_state = PKTLOG_OPR_NOT_IN_PROGRESS;
		qdf_print("%s: Invalid pktlog handle", __func__);
		ASSERT(0);
		return -ENODEV;
	}

	pl_info->init_saved_state = pl_info->log_state;
	if (!pl_info->log_state) {
		/* Pktlog is already disabled.
		 * Proceed to read directly.
		 */
		pl_info->curr_pkt_state =
			PKTLOG_OPR_IN_PROGRESS_READ_START_PKTLOG_DISABLED;
		return ret;
	}
	/* Disbable the pktlog internally. */
	ret = pl_dev->pl_funcs->pktlog_disable(scn);
	pl_info->log_state = 0;
	pl_info->curr_pkt_state =
			PKTLOG_OPR_IN_PROGRESS_READ_START_PKTLOG_DISABLED;
	return ret;
}

static int pktlog_open(struct inode *i, struct file *f)
{
	struct qdf_op_sync *op_sync;
	int errno;

	errno = qdf_op_protect(&op_sync);
	if (errno)
		return errno;

	errno = __pktlog_open(i, f);

	qdf_op_unprotect(op_sync);

	return errno;
}

static int __pktlog_release(struct inode *i, struct file *f)
{
	struct hif_opaque_softc *scn;
	struct pktlog_dev_t *pl_dev;
	struct ath_pktlog_info *pl_info;
	int ret = 0;

	PKTLOG_MOD_DEC_USE_COUNT;

	pl_info = PDE_DATA(f->f_path.dentry->d_inode);
	if (!pl_info)
		return -EINVAL;

	scn = cds_get_context(QDF_MODULE_ID_HIF);
	if (!scn) {
		pl_info->curr_pkt_state = PKTLOG_OPR_NOT_IN_PROGRESS;
		qdf_print("%s: Invalid scn context", __func__);
		ASSERT(0);
		return -EINVAL;
	}

	pl_dev = get_pktlog_handle();

	if (!pl_dev) {
		pl_info->curr_pkt_state = PKTLOG_OPR_NOT_IN_PROGRESS;
		qdf_print("%s: Invalid pktlog handle", __func__);
		ASSERT(0);
		return -ENODEV;
	}

	pl_info->curr_pkt_state = PKTLOG_OPR_IN_PROGRESS_READ_COMPLETE;
	/*clear pktlog buffer.*/
	pktlog_clearbuff(scn, true);
	pl_info->log_state = pl_info->init_saved_state;
	pl_info->init_saved_state = 0;

	/*Enable pktlog again*/
	ret = pl_dev->pl_funcs->pktlog_enable(
			(struct hif_opaque_softc *)scn, pl_info->log_state,
			cds_is_packet_log_enabled(), 0, 1);

	if (ret != 0)
		pr_warn("%s: pktlog cannot be enabled. ret value %d\n",
			__func__, ret);

	pl_info->curr_pkt_state = PKTLOG_OPR_NOT_IN_PROGRESS;
	return ret;
}

static int pktlog_release(struct inode *i, struct file *f)
{
	struct qdf_op_sync *op_sync;
	int errno;

	errno = qdf_op_protect(&op_sync);
	if (errno)
		return errno;

	errno = __pktlog_release(i, f);

	qdf_op_unprotect(op_sync);

	return errno;
}

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

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
	struct ath_pktlog_buf *log_buf;

	qdf_spin_lock_bh(&pl_info->log_lock);
	log_buf = pl_info->buf;

	*read_complete = false;

	if (!log_buf) {
		*read_complete = true;
		qdf_spin_unlock_bh(&pl_info->log_lock);
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
	}
	qdf_spin_unlock_bh(&pl_info->log_lock);
	return ret_val;
}

static ssize_t
__pktlog_read(struct file *file, char *buf, size_t nbytes, loff_t *ppos)
{
	size_t bufhdr_size;
	size_t count = 0, ret_val = 0;
	int rem_len;
	int start_offset, end_offset;
	int fold_offset, ppos_data, cur_rd_offset;
	struct ath_pktlog_info *pl_info;
	struct ath_pktlog_buf *log_buf;

	pl_info = PDE_DATA(file->f_path.dentry->d_inode);
	if (!pl_info)
		return 0;

	qdf_spin_lock_bh(&pl_info->log_lock);
	log_buf = pl_info->buf;

	if (!log_buf) {
		qdf_spin_unlock_bh(&pl_info->log_lock);
		return 0;
	}

	if (pl_info->log_state) {
		/* Read is not allowed when write is going on
		 * When issuing cat command, ensure to send
		 * pktlog disable command first.
		 */
		qdf_spin_unlock_bh(&pl_info->log_lock);
		return -EINVAL;
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
		count = QDF_MIN((bufhdr_size - *ppos), rem_len);
		qdf_spin_unlock_bh(&pl_info->log_lock);
		if (copy_to_user(buf, ((char *)&log_buf->bufhdr) + *ppos,
				 count)) {
			return -EFAULT;
		}
		rem_len -= count;
		ret_val += count;
		qdf_spin_lock_bh(&pl_info->log_lock);
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
		qdf_spin_unlock_bh(&pl_info->log_lock);

		if (copy_to_user(buf + ret_val,
				 log_buf->log_data + ppos_data, count)) {
			return -EFAULT;
		}

		ret_val += count;
		rem_len -= count;
		qdf_spin_lock_bh(&pl_info->log_lock);
	} else {
		if (ppos_data <= fold_offset) {
			count = QDF_MIN(rem_len, (fold_offset - ppos_data + 1));
			qdf_spin_unlock_bh(&pl_info->log_lock);
			if (copy_to_user(buf + ret_val,
					 log_buf->log_data + ppos_data,
					 count)) {
				return -EFAULT;
			}
			ret_val += count;
			rem_len -= count;
			qdf_spin_lock_bh(&pl_info->log_lock);
		}

		if (rem_len == 0)
			goto rd_done;

		ppos_data =
			*ppos + ret_val - (bufhdr_size +
					   (fold_offset - start_offset + 1));

		if (ppos_data <= end_offset) {
			count = QDF_MIN(rem_len, (end_offset - ppos_data + 1));
			qdf_spin_unlock_bh(&pl_info->log_lock);
			if (copy_to_user(buf + ret_val,
					 log_buf->log_data + ppos_data,
					 count)) {
				return -EFAULT;
			}
			ret_val += count;
			rem_len -= count;
			qdf_spin_lock_bh(&pl_info->log_lock);
		}
	}

rd_done:
	if ((ret_val < nbytes) && pl_info->saved_state) {
		pl_info->log_state = pl_info->saved_state;
		pl_info->saved_state = 0;
	}
	*ppos += ret_val;

	qdf_spin_unlock_bh(&pl_info->log_lock);
	return ret_val;
}

static ssize_t
pktlog_read(struct file *file, char *buf, size_t nbytes, loff_t *ppos)
{
	struct ath_pktlog_info *info = PDE_DATA(file->f_path.dentry->d_inode);
	struct qdf_op_sync *op_sync;
	ssize_t err_size;

	if (!info)
		return 0;

	err_size = qdf_op_protect(&op_sync);
	if (err_size)
		return err_size;

	mutex_lock(&info->pktlog_mutex);
	err_size = __pktlog_read(file, buf, nbytes, ppos);
	mutex_unlock(&info->pktlog_mutex);

	qdf_op_unprotect(op_sync);

	return err_size;
}

int pktlogmod_init(void *context)
{
	int ret;

	/* create the proc directory entry */
	g_pktlog_pde = proc_mkdir(PKTLOG_PROC_DIR, NULL);

	if (!g_pktlog_pde) {
		printk(PKTLOG_TAG "%s: proc_mkdir failed\n", __func__);
		return -EPERM;
	}

	/* Attach packet log */
	ret = pktlog_attach((struct hif_opaque_softc *)context);

	/* If packet log init failed */
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
	if (!g_pktlog_pde)
		return;

	pktlog_detach((struct hif_opaque_softc *)context);

	/*
	 *  pdev kill needs to be implemented
	 */
	remove_proc_entry(PKTLOG_PROC_DIR, NULL);
}
#endif
