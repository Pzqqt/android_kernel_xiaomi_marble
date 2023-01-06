/*
 * Copyright (c) 2013-2014, 2016-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * Get op_type, mem_type and offset fields from pos of procfs
 * It will reuse pos, which is long long type
 *
 * op_type:     4 bits
 * memtype:     8 bits
 * reserve1:    20 bits
 * offset:      32 bits
 */
#define OP_TYPE_LEGACY                  0
#define OP_TYPE_EXT_QMI                 1
#define OP_TYPE_EXT_DIRECT              2

#define ATH_DIAG_EXT_OP_TYPE_BITS        4
#define ATH_DIAG_EXT_OP_TYPE_INDEX       60
#define ATH_DIAG_EXT_MEM_TYPE_BITS       8
#define ATH_DIAG_EXT_MEM_TYPE_INDEX      52
#define ATH_DIAG_EXT_OFFSET_BITS         32
#define ATH_DIAG_EXT_OFFSET_INDEX        0

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

static ssize_t ath_procfs_diag_read_legacy(struct file *file,
					   char __user *buf,
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

	read_buffer = qdf_mem_malloc(count);
	if (!read_buffer)
		return -ENOMEM;

	hif_debug("rd buff 0x%pK cnt %zu offset 0x%x buf 0x%pK",
		 read_buffer, count, (int)*pos, buf);

	tgt_info = hif_get_target_info_handle(GET_HIF_OPAQUE_HDL(hif_hdl));
	if ((scn->bus_type == QDF_BUS_TYPE_SNOC) ||
	    (scn->bus_type ==  QDF_BUS_TYPE_PCI &&
	    ((tgt_info->target_type == TARGET_TYPE_QCA6290) ||
	     (tgt_info->target_type == TARGET_TYPE_QCA6390) ||
	     (tgt_info->target_type == TARGET_TYPE_QCA6490) ||
	     (tgt_info->target_type == TARGET_TYPE_QCA8074) ||
	     (tgt_info->target_type == TARGET_TYPE_QCA8074V2) ||
	     (tgt_info->target_type == TARGET_TYPE_QCA9574) ||
	     (tgt_info->target_type == TARGET_TYPE_QCN9000) ||
	     (tgt_info->target_type == TARGET_TYPE_QCN9224) ||
	     (tgt_info->target_type == TARGET_TYPE_QCN6122) ||
	     (tgt_info->target_type == TARGET_TYPE_QCA5018) ||
	     (tgt_info->target_type == TARGET_TYPE_QCA6018) ||
	     (tgt_info->target_type == TARGET_TYPE_QCN7605) ||
	     (tgt_info->target_type == TARGET_TYPE_KIWI))) ||
	    (scn->bus_type ==  QDF_BUS_TYPE_IPCI &&
	     (tgt_info->target_type == TARGET_TYPE_QCA6750)) ||
	    ((scn->bus_type ==  QDF_BUS_TYPE_USB) &&
	     (tgt_info->target_type == TARGET_TYPE_QCN7605))) {
		memtype = ((uint32_t)(*pos) & 0xff000000) >> 24;
		offset = (uint32_t)(*pos) & 0xffffff;
		hif_debug("offset 0x%x memtype 0x%x, datalen %zu",
			 offset, memtype, count);
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
		hif_err("copy_to_user error in /proc/%s", PROCFS_NAME);
		return -EFAULT;
	}
	qdf_mem_free(read_buffer);
	return count;
}

static ssize_t ath_procfs_diag_write_legacy(struct file *file,
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

	write_buffer = qdf_mem_malloc(count);
	if (!write_buffer)
		return -ENOMEM;

	if (copy_from_user(write_buffer, buf, count)) {
		qdf_mem_free(write_buffer);
		hif_err("copy_to_user error in /proc/%s", PROCFS_NAME);
		return -EFAULT;
	}

	hif_debug("wr buff 0x%pK buf 0x%pK cnt %zu offset 0x%x value 0x%x",
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
	      (tgt_info->target_type == TARGET_TYPE_QCA9574) ||
	      (tgt_info->target_type == TARGET_TYPE_QCN9000) ||
	      (tgt_info->target_type == TARGET_TYPE_QCN9224) ||
	      (tgt_info->target_type == TARGET_TYPE_QCN6122) ||
	      (tgt_info->target_type == TARGET_TYPE_QCA5018) ||
	      (tgt_info->target_type == TARGET_TYPE_QCA6018) ||
	      (tgt_info->target_type == TARGET_TYPE_QCN7605) ||
	      (tgt_info->target_type == TARGET_TYPE_KIWI))) ||
	    (scn->bus_type ==  QDF_BUS_TYPE_IPCI &&
	     (tgt_info->target_type == TARGET_TYPE_QCA6750)) ||
	    ((scn->bus_type ==  QDF_BUS_TYPE_USB) &&
	     (tgt_info->target_type == TARGET_TYPE_QCN7605))) {
		memtype = ((uint32_t)(*pos) & 0xff000000) >> 24;
		offset = (uint32_t)(*pos) & 0xffffff;
		hif_debug("offset 0x%x memtype 0x%x, datalen %zu",
			 offset, memtype, count);
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

#ifdef ATH_DIAG_EXT_DIRECT
/* Used to dump register or SRAM from target directly */
static int ath_procfs_direct_read(struct hif_softc *scn, uint32_t offset,
				  uint8_t *buf, size_t count)
{
	size_t remaining = count;
	uint32_t *p_val = (uint32_t *)buf;
	uint32_t val;
	uint8_t *buf_d, *buf_s;

	if (!scn->bus_ops.hif_reg_read32)
		return -EIO;

	while (remaining >= 4) {
		*p_val++ = scn->bus_ops.hif_reg_read32(scn,
						       offset);
		offset += 4;
		remaining -= 4;
	}

	if (remaining) {
		val = scn->bus_ops.hif_reg_read32(scn,
						  offset);
		buf_d = (uint8_t *)p_val;
		buf_s = (uint8_t *)&val;
		while (remaining) {
			*buf_d++ = *buf_s++;
			remaining--;
		}
	}

	return 0;
}

/* Used to write register or SRAM to target directly */
static int ath_procfs_direct_write(struct hif_softc *scn, uint32_t offset,
				   uint8_t *buf, size_t count)
{
	size_t remaining = count;
	uint32_t *p_val = (uint32_t *)buf;
	uint32_t val;
	uint8_t *buf_d, *buf_s;

	if (!scn->bus_ops.hif_reg_write32 || !scn->bus_ops.hif_reg_read32)
		return -EIO;

	while (remaining >= 4) {
		scn->bus_ops.hif_reg_write32(scn,
					     offset,
					     *p_val++);
		offset += 4;
		remaining -= 4;
	}

	if (remaining) {
		val = scn->bus_ops.hif_reg_read32(scn,
						  offset);
		buf_s = (uint8_t *)p_val;
		buf_d = (uint8_t *)&val;
		while (remaining) {
			*buf_d++ = *buf_s++;
			remaining--;
		}
		scn->bus_ops.hif_reg_write32(scn,
					     offset,
					     val);
	}

	return 0;
}
#else
static int ath_procfs_direct_read(struct hif_softc *scn, uint32_t offset,
				  uint8_t *buf, size_t count)
{
	return -EIO;
}

/* Used to write register or SRAM to target directly */
static int ath_procfs_direct_write(struct hif_softc *scn, uint32_t offset,
				   uint8_t *buf, size_t count)
{
	return -EIO;
}

#endif

static ssize_t ath_procfs_diag_read_ext(struct file *file, char __user *buf,
					size_t count,
					uint32_t op_type,
					uint32_t memtype,
					uint32_t offset)
{
	hif_handle_t hif_hdl = get_hif_hdl_from_file(file);
	int rv = -EINVAL;
	uint8_t *read_buffer;
	struct hif_softc *scn;
	struct hif_target_info *tgt_info;

	if (!hif_hdl)
		return -EINVAL;

	read_buffer = qdf_mem_malloc(count);
	if (!read_buffer)
		return -ENOMEM;

	scn = HIF_GET_SOFTC(hif_hdl);
	tgt_info = hif_get_target_info_handle(GET_HIF_OPAQUE_HDL(hif_hdl));
	switch (scn->bus_type) {
	case QDF_BUS_TYPE_PCI:
		switch (tgt_info->target_type) {
		case TARGET_TYPE_QCA6390:
		case TARGET_TYPE_QCA6490:
		case TARGET_TYPE_KIWI:
			if (op_type == OP_TYPE_EXT_DIRECT)
				rv = ath_procfs_direct_read(scn,
							    offset,
							    read_buffer,
							    count);
			else
				rv = pld_athdiag_read(scn->qdf_dev->dev,
						      offset,
						      memtype,
						      count,
						      read_buffer);
			break;
		default:
			hif_err("Unrecognized target type %d",
				tgt_info->target_type);
		}
		break;
	default:
		hif_err("Unrecognized bus type %d", scn->bus_type);
		break;
	}

	if (rv) {
		hif_err("fail to read from target %d", rv);
	} else {
		rv = count;
		if (copy_to_user(buf, read_buffer, count)) {
			hif_err("copy_to_user error in /proc/%s",
				PROCFS_NAME);
			rv = -EFAULT;
		}
	}

	qdf_mem_free(read_buffer);

	return rv;
}

static ssize_t ath_procfs_diag_write_ext(struct file *file,
					 const char __user *buf,
					 size_t count,
					 uint32_t op_type,
					 uint32_t memtype,
					 uint32_t offset)
{
	hif_handle_t hif_hdl = get_hif_hdl_from_file(file);
	int rv = -EINVAL;
	uint8_t *write_buffer;
	struct hif_softc *scn;
	struct hif_target_info *tgt_info;

	if (!hif_hdl)
		return -EINVAL;

	scn = HIF_GET_SOFTC(hif_hdl);

	write_buffer = qdf_mem_malloc(count);
	if (!write_buffer)
		return -ENOMEM;

	if (copy_from_user(write_buffer, buf, count)) {
		qdf_mem_free(write_buffer);
		hif_err("copy_to_user error in /proc/%s",
			PROCFS_NAME);
		return -EFAULT;
	}

	tgt_info = hif_get_target_info_handle(GET_HIF_OPAQUE_HDL(hif_hdl));

	switch (scn->bus_type) {
	case QDF_BUS_TYPE_PCI:
		switch (tgt_info->target_type) {
		case TARGET_TYPE_QCA6390:
		case TARGET_TYPE_QCA6490:
		case TARGET_TYPE_KIWI:
			if (op_type == OP_TYPE_EXT_DIRECT)
				rv = ath_procfs_direct_write(scn,
							     offset,
							     write_buffer,
							     count);
			else
				rv = pld_athdiag_write(scn->qdf_dev->dev,
						       offset,
						       memtype,
						       count,
						       write_buffer);
			break;
		default:
			hif_err("Unrecognized target type %d",
				tgt_info->target_type);
		}
		break;
	default:
		hif_err("Unrecognized bus type %d", scn->bus_type);
		break;
	}

	qdf_mem_free(write_buffer);

	return (rv == 0) ? count : -EIO;
}

static void get_fields_from_pos(loff_t pos,
				uint32_t *op_type,
				uint32_t *memtype,
				uint32_t *offset)
{
	*op_type = QDF_GET_BITS64(pos, ATH_DIAG_EXT_OP_TYPE_INDEX,
				  ATH_DIAG_EXT_OP_TYPE_BITS);
	*memtype = QDF_GET_BITS64(pos, ATH_DIAG_EXT_MEM_TYPE_INDEX,
				  ATH_DIAG_EXT_MEM_TYPE_BITS);
	*offset = QDF_GET_BITS64(pos, ATH_DIAG_EXT_OFFSET_INDEX,
				 ATH_DIAG_EXT_OFFSET_BITS);
}

static ssize_t ath_procfs_diag_read(struct file *file, char __user *buf,
				    size_t count, loff_t *pos)
{
	hif_handle_t hif_hdl = get_hif_hdl_from_file(file);
	int rv = -EINVAL;
	struct hif_softc *scn;
	uint32_t offset, memtype;
	uint32_t op_type;

	if (!hif_hdl)
		return -EINVAL;

	get_fields_from_pos(*pos, &op_type, &memtype, &offset);

	scn = HIF_GET_SOFTC(hif_hdl);
	if (scn->bus_ops.hif_addr_in_boundary(scn, offset))
		return -EINVAL;

	if (offset & 0x3)
		return -EINVAL;

	hif_info("rd cnt %zu offset 0x%x op_type %d type %d pos %llx",
		 count, offset, op_type, memtype, *pos);

	switch (op_type) {
	case OP_TYPE_LEGACY:
		rv = ath_procfs_diag_read_legacy(file, buf, count, pos);
		break;
	case OP_TYPE_EXT_QMI:
	case OP_TYPE_EXT_DIRECT:
		rv = ath_procfs_diag_read_ext(file, buf, count, op_type,
					      memtype, offset);
		break;
	default:
		hif_err("Unrecognized op type %d", op_type);
		break;
	}

	return rv;
}

static ssize_t ath_procfs_diag_write(struct file *file,
				     const char __user *buf,
				     size_t count, loff_t *pos)
{
	hif_handle_t hif_hdl = get_hif_hdl_from_file(file);
	int rv = -EINVAL;
	struct hif_softc *scn;
	uint32_t offset, memtype;
	uint32_t op_type;

	if (!hif_hdl)
		return -EINVAL;

	get_fields_from_pos(*pos, &op_type, &memtype, &offset);

	scn = HIF_GET_SOFTC(hif_hdl);
	if (scn->bus_ops.hif_addr_in_boundary(scn, offset))
		return -EINVAL;

	if (offset & 0x3)
		return -EINVAL;

	hif_info("wr cnt %zu offset 0x%x op_type %d mem_type %d",
		 count, offset, op_type, memtype);

	switch (op_type) {
	case OP_TYPE_LEGACY:
		rv = ath_procfs_diag_write_legacy(file, buf, count, pos);
		break;
	case OP_TYPE_EXT_QMI:
	case OP_TYPE_EXT_DIRECT:
		rv = ath_procfs_diag_write_ext(file, buf, count, op_type,
					       memtype, offset);
		break;
	default:
		hif_err("Unrecognized op type %d", op_type);
		break;
	}

	return rv;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
static const struct proc_ops athdiag_fops = {
	.proc_read = ath_procfs_diag_read,
	.proc_write = ath_procfs_diag_write,
	.proc_lseek = default_llseek,
};
#else
static const struct file_operations athdiag_fops = {
	.read = ath_procfs_diag_read,
	.write = ath_procfs_diag_write,
};
#endif

/*
 * This function is called when the module is loaded
 *
 */
int athdiag_procfs_init(void *scn)
{
	proc_dir = proc_mkdir(PROCFS_DIR, NULL);
	if (!proc_dir) {
		remove_proc_entry(PROCFS_DIR, NULL);
		hif_err("Could not initialize /proc/%s", PROCFS_DIR);
		return -ENOMEM;
	}

	proc_file = proc_create_data(PROCFS_NAME, 0600, proc_dir,
				     &athdiag_fops, (void *)scn);
	if (!proc_file) {
		remove_proc_entry(PROCFS_NAME, proc_dir);
		hif_err("Could not initialize /proc/%s", PROCFS_NAME);
		return -ENOMEM;
	}

	hif_debug("/proc/%s/%s created", PROCFS_DIR, PROCFS_NAME);
	return 0;
}

/*
 * This function is called when the module is unloaded
 *
 */
void athdiag_procfs_remove(void)
{
	if (proc_dir) {
		remove_proc_entry(PROCFS_NAME, proc_dir);
		hif_debug("/proc/%s/%s removed", PROCFS_DIR, PROCFS_NAME);
		remove_proc_entry(PROCFS_DIR, NULL);
		hif_debug("/proc/%s removed", PROCFS_DIR);
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
