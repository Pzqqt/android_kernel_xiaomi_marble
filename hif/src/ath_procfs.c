/*
 * Copyright (c) 2013-2014 The Linux Foundation. All rights reserved.
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

#if defined(CONFIG_ATH_PROCFS_DIAG_SUPPORT)
#include <linux/module.h>       /* Specifically, a module */
#include <linux/kernel.h>       /* We're doing kernel work */
#include <linux/version.h>      /* We're doing kernel work */
#include <linux/proc_fs.h>      /* Necessary because we use the proc fs */
#include <asm/uaccess.h>        /* for copy_from_user */
#include "ol_if_athvar.h"
#include "hif.h"
#if defined(HIF_PCI)
#include "if_pci.h"
#elif defined(HIF_USB)
#include "if_usb.h"
#elif defined(HIF_SDIO)
#include "if_ath_sdio.h"
#endif
#include "cds_api.h"
#include "hif_debug.h"

#define PROCFS_NAME             "athdiagpfs"
#define PROCFS_DIR              "cld"

/**
 * This structure hold information about the /proc file
 *
 */
static struct proc_dir_entry *proc_file, *proc_dir;

static void *get_hif_hdl_from_file(struct file *file)
{
	struct ol_softc *scn;

	scn = (struct ol_softc *)PDE_DATA(file_inode(file));
	return (void *)scn;
}

static ssize_t ath_procfs_diag_read(struct file *file, char __user *buf,
				    size_t count, loff_t *pos)
{
	hif_handle_t hif_hdl;
	int rv;
	uint8_t *read_buffer = NULL;

	read_buffer = cdf_mem_malloc(count);
	if (NULL == read_buffer) {
		HIF_ERROR("%s: cdf_mem_alloc failed", __func__);
		return -ENOMEM;
	}

	hif_hdl = get_hif_hdl_from_file(file);
	HIF_DBG("rd buff 0x%p cnt %zu offset 0x%x buf 0x%p",
		 read_buffer, count, (int)*pos, buf);

	if ((count == 4) && ((((uint32_t) (*pos)) & 3) == 0)) {
		/* reading a word? */
		rv = hif_diag_read_access(hif_hdl, (uint32_t)(*pos),
					  (uint32_t *)read_buffer);
	} else {
		rv = hif_diag_read_mem(hif_hdl, (uint32_t)(*pos),
				       (uint8_t *)read_buffer, count);
	}

	if (copy_to_user(buf, read_buffer, count)) {
		cdf_mem_free(read_buffer);
		HIF_ERROR("%s: copy_to_user error in /proc/%s",
			__func__, PROCFS_NAME);
		return -EFAULT;
	} else
		cdf_mem_free(read_buffer);

	if (rv == 0) {
		return count;
	} else {
		return -EIO;
	}
}

static ssize_t ath_procfs_diag_write(struct file *file,
				     const char __user *buf,
				     size_t count, loff_t *pos)
{
	hif_handle_t hif_hdl;
	int rv;
	uint8_t *write_buffer = NULL;

	write_buffer = cdf_mem_malloc(count);
	if (NULL == write_buffer) {
		HIF_ERROR("%s: cdf_mem_alloc failed", __func__);
		return -ENOMEM;
	}
	if (copy_from_user(write_buffer, buf, count)) {
		cdf_mem_free(write_buffer);
		HIF_ERROR("%s: copy_to_user error in /proc/%s",
			__func__, PROCFS_NAME);
		return -EFAULT;
	}

	hif_hdl = get_hif_hdl_from_file(file);
	HIF_DBG("wr buff 0x%p buf 0x%p cnt %zu offset 0x%x value 0x%x",
		 write_buffer, buf, count,
		 (int)*pos, *((uint32_t *) write_buffer));

	if ((count == 4) && ((((uint32_t) (*pos)) & 3) == 0)) {
		/* reading a word? */
		uint32_t value = *((uint32_t *)write_buffer);
		rv = hif_diag_write_access(hif_hdl, (uint32_t)(*pos), value);
	} else {
		rv = hif_diag_write_mem(hif_hdl, (uint32_t)(*pos),
					(uint8_t *)write_buffer, count);
	}

	cdf_mem_free(write_buffer);
	if (rv == 0) {
		return count;
	} else {
		return -EIO;
	}
}

static const struct file_operations athdiag_fops = {
	.read = ath_procfs_diag_read,
	.write = ath_procfs_diag_write,
};

/**
   *This function is called when the module is loaded
 *
 */
int athdiag_procfs_init(void *scn)
{
	proc_dir = proc_mkdir(PROCFS_DIR, NULL);
	if (proc_dir == NULL) {
		remove_proc_entry(PROCFS_DIR, NULL);
		HIF_ERROR("%s: Error: Could not initialize /proc/%s",
			__func__, PROCFS_DIR);
		return -ENOMEM;
	}

	proc_file = proc_create_data(PROCFS_NAME,
				     S_IRUSR | S_IWUSR, proc_dir,
				     &athdiag_fops, (void *)scn);
	if (proc_file == NULL) {
		remove_proc_entry(PROCFS_NAME, proc_dir);
		HIF_ERROR("%s: Could not initialize /proc/%s",
			__func__, PROCFS_NAME);
		return -ENOMEM;
	}

	HIF_DBG("/proc/%s/%s created", PROCFS_DIR, PROCFS_NAME);
	return 0;               /* everything is ok */
}

/**
   *This function is called when the module is unloaded
 *
 */
void athdiag_procfs_remove(void)
{
	if (proc_dir != NULL) {
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
