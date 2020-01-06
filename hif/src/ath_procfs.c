/*
 * Copyright (c) 2013-2014, 2016-2020 The Linux Foundation. All rights reserved.
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

#if defined(CONFIG_ATH_PROCFS_DIAG_SUPPORT)
#include <linux/module.h>       /* Specifically, a module */
#include <linux/kernel.h>       /* We're doing kernel work */
#include <linux/version.h>      /* We're doing kernel work */
#include <linux/proc_fs.h>      /* Necessary because we use the proc fs */
#include <linux/uaccess.h>        /* for copy_from_user */
#include "hif.h"
#include "hif_main.h"
#if defined(HIF_USB)
#include "if_usb.h"
#endif
#if defined(HIF_SDIO)
#include "if_sdio.h"
#endif
#include "hif_debug.h"
#include "pld_common.h"
#include "target_type.h"

#define PROCFS_NAME             "athdiagpfs"
#ifdef MULTI_IF_NAME
#define PROCFS_DIR              "cld" MULTI_IF_NAME
#else
#define PROCFS_DIR              "cld"
#endif

/**
 * This structure hold information about the /proc file
 *
 */
static struct proc_dir_entry *proc_file, *proc_dir;

static void *get_hif_hdl_from_file(struct file *file)
{
	struct hif_opaque_softc *scn;

	scn = (struct hif_opaque_softc *)PDE_DATA(file_inode(file));
	return (void *)scn;
}

static ssize_t ath_procfs_diag_read(struct file *file, char __user *buf,
				    size_t count, loff_t *pos)
{
	hif_handle_t hif_hdl;
	int rv;
	uint8_t *read_buffer = NULL;
	struct hif_softc *scn;
	uint32_t offset = 0, memtype = 0;
	struct hif_target_info *tgt_info;

	hif_hdl = get_hif_hdl_from_file(file);
	scn = HIF_GET_SOFTC(hif_hdl);

	if (scn->bus_ops.hif_addr_in_boundary(hif_hdl, (uint32_t)(*pos)))
		return -EINVAL;

	read_buffer = qdf_mem_malloc(count);
	if (!read_buffer)
		return -ENOMEM;

	HIF_DBG("rd buff 0x%pK cnt %zu offset 0x%x buf 0x%pK",
		 read_buffer, count, (int)*pos, buf);

	tgt_info = hif_get_target_info_handle(GET_HIF_OPAQUE_HDL(hif_hdl));
	if ((scn->bus_type == QDF_BUS_TYPE_SNOC) ||
	    (scn->bus_type ==  QDF_BUS_TYPE_PCI &&
	    ((tgt_info->target_type == TARGET_TYPE_QCA6290) ||
	     (tgt_info->target_type == TARGET_TYPE_QCA6390) ||
	     (tgt_info->target_type == TARGET_TYPE_QCA6490) ||
	     (tgt_info->target_type == TARGET_TYPE_QCA8074) ||
	     (tgt_info->target_type == TARGET_TYPE_QCA8074V2) ||
	     (tgt_info->target_type == TARGET_TYPE_QCN9000) ||
	     (tgt_info->target_type == TARGET_TYPE_QCA6018))) ||
	    (scn->bus_type ==  QDF_BUS_TYPE_IPCI &&
	     (tgt_info->target_type == TARGET_TYPE_QCA6750))) {
		memtype = ((uint32_t)(*pos) & 0xff000000) >> 24;
		offset = (uint32_t)(*pos) & 0xffffff;
		HIF_DBG("%s: offset 0x%x memtype 0x%x, datalen %zu\n",
			__func__, offset, memtype, count);
		rv = pld_athdiag_read(scn->qdf_dev->dev,
				      offset, memtype, count,
				      (uint8_t *)read_buffer);
		goto out;
	}

	if ((count == 4) && ((((uint32_t) (*pos)) & 3) == 0)) {
		/* reading a word? */
		rv = hif_diag_read_access(hif_hdl, (uint32_t)(*pos),
					  (uint32_t *)read_buffer);
	} else {
		rv = hif_diag_read_mem(hif_hdl, (uint32_t)(*pos),
				       (uint8_t *)read_buffer, count);
	}

out:
	if (rv) {
		qdf_mem_free(read_buffer);
		return -EIO;
	}

	if (copy_to_user(buf, read_buffer, count)) {
		qdf_mem_free(read_buffer);
		HIF_ERROR("%s: copy_to_user error in /proc/%s",
			__func__, PROCFS_NAME);
		return -EFAULT;
	}
	qdf_mem_free(read_buffer);
	return count;
}

static ssize_t ath_procfs_diag_write(struct file *file,
				     const char __user *buf,
				     size_t count, loff_t *pos)
{
	hif_handle_t hif_hdl;
	int rv;
	uint8_t *write_buffer = NULL;
	struct hif_softc *scn;
	uint32_t offset = 0, memtype = 0;
	struct hif_target_info *tgt_info;

	hif_hdl = get_hif_hdl_from_file(file);
	scn = HIF_GET_SOFTC(hif_hdl);

	if (scn->bus_ops.hif_addr_in_boundary(hif_hdl, (uint32_t)(*pos)))
		return -EINVAL;

	write_buffer = qdf_mem_malloc(count);
	if (!write_buffer)
		return -ENOMEM;

	if (copy_from_user(write_buffer, buf, count)) {
		qdf_mem_free(write_buffer);
		HIF_ERROR("%s: copy_to_user error in /proc/%s",
			__func__, PROCFS_NAME);
		return -EFAULT;
	}

	HIF_DBG("wr buff 0x%pK buf 0x%pK cnt %zu offset 0x%x value 0x%x",
		 write_buffer, buf, count,
		 (int)*pos, *((uint32_t *) write_buffer));

	tgt_info = hif_get_target_info_handle(GET_HIF_OPAQUE_HDL(hif_hdl));
	if ((scn->bus_type == QDF_BUS_TYPE_SNOC) ||
	    ((scn->bus_type ==  QDF_BUS_TYPE_PCI) &&
	     ((tgt_info->target_type == TARGET_TYPE_QCA6290) ||
	      (tgt_info->target_type == TARGET_TYPE_QCA6390) ||
	      (tgt_info->target_type == TARGET_TYPE_QCA6490) ||
	      (tgt_info->target_type == TARGET_TYPE_QCA8074) ||
	      (tgt_info->target_type == TARGET_TYPE_QCA8074V2) ||
	      (tgt_info->target_type == TARGET_TYPE_QCN9000) ||
	      (tgt_info->target_type == TARGET_TYPE_QCA6018))) ||
	    (scn->bus_type ==  QDF_BUS_TYPE_IPCI &&
	     (tgt_info->target_type == TARGET_TYPE_QCA6750))) {
		memtype = ((uint32_t)(*pos) & 0xff000000) >> 24;
		offset = (uint32_t)(*pos) & 0xffffff;
		HIF_DBG("%s: offset 0x%x memtype 0x%x, datalen %zu\n",
			__func__, offset, memtype, count);
		rv = pld_athdiag_write(scn->qdf_dev->dev,
				      offset, memtype, count,
				      (uint8_t *)write_buffer);
		goto out;
	}

	if ((count == 4) && ((((uint32_t) (*pos)) & 3) == 0)) {
		/* reading a word? */
		uint32_t value = *((uint32_t *)write_buffer);

		rv = hif_diag_write_access(hif_hdl, (uint32_t)(*pos), value);
	} else {
		rv = hif_diag_write_mem(hif_hdl, (uint32_t)(*pos),
					(uint8_t *)write_buffer, count);
	}

out:

	qdf_mem_free(write_buffer);
	if (rv == 0)
		return count;
	else
		return -EIO;
}

static const struct file_operations athdiag_fops = {
	.read = ath_procfs_diag_read,
	.write = ath_procfs_diag_write,
};

/*
 * This function is called when the module is loaded
 *
 */
int athdiag_procfs_init(void *scn)
{
	proc_dir = proc_mkdir(PROCFS_DIR, NULL);
	if (!proc_dir) {
		remove_proc_entry(PROCFS_DIR, NULL);
		HIF_ERROR("%s: Error: Could not initialize /proc/%s",
			__func__, PROCFS_DIR);
		return -ENOMEM;
	}

	proc_file = proc_create_data(PROCFS_NAME, 0600, proc_dir,
				     &athdiag_fops, (void *)scn);
	if (!proc_file) {
		remove_proc_entry(PROCFS_NAME, proc_dir);
		HIF_ERROR("%s: Could not initialize /proc/%s",
			__func__, PROCFS_NAME);
		return -ENOMEM;
	}

	HIF_DBG("/proc/%s/%s created", PROCFS_DIR, PROCFS_NAME);
	return 0;               /* everything is ok */
}

/*
 * This function is called when the module is unloaded
 *
 */
void athdiag_procfs_remove(void)
{
	if (proc_dir) {
		remove_proc_entry(PROCFS_NAME, proc_dir);
		HIF_DBG("/proc/%s/%s removed", PROCFS_DIR, PROCFS_NAME);
		remove_proc_entry(PROCFS_DIR, NULL);
		HIF_DBG("/proc/%s removed", PROCFS_DIR);
		proc_dir = NULL;
	}
}
#else
int athdiag_procfs_init(void *scn)
{
	return 0;
}
void athdiag_procfs_remove(void) {}
#endif
