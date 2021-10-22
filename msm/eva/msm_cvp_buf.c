// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 */

#include <linux/pid.h>
#include <linux/fdtable.h>
#include <linux/rcupdate.h>
#include <linux/fs.h>
#include <linux/dma-buf.h>
#include <linux/sched/task.h>
#include <linux/version.h>
#include "msm_cvp_common.h"
#include "cvp_hfi_api.h"
#include "msm_cvp_debug.h"
#include "msm_cvp_core.h"
#include "msm_cvp_dsp.h"

#define CLEAR_USE_BITMAP(idx, inst) \
	do { \
		clear_bit(idx, &inst->dma_cache.usage_bitmap); \
		dprintk(CVP_MEM, "clear %x bit %d dma_cache bitmap 0x%llx\n", \
			hash32_ptr(inst->session), smem->bitmap_index, \
			inst->dma_cache.usage_bitmap); \
	} while (0)

#define SET_USE_BITMAP(idx, inst) \
	do { \
		set_bit(idx, &inst->dma_cache.usage_bitmap); \
		dprintk(CVP_MEM, "Set %x bit %d dma_cache bitmap 0x%llx\n", \
			hash32_ptr(inst->session), idx, \
			inst->dma_cache.usage_bitmap); \
	} while (0)


int print_smem(u32 tag, const char *str, struct msm_cvp_inst *inst,
		struct msm_cvp_smem *smem)
{
	if (!(tag & msm_cvp_debug))
		return 0;

	if (!inst || !smem) {
		dprintk(CVP_ERR, "Invalid inst 0x%llx or smem 0x%llx\n",
				inst, smem);
		return -EINVAL;
	}

	if (smem->dma_buf) {
		dprintk(tag,
			"%s: %x : %s size %d flags %#x iova %#x idx %d ref %d",
			str, hash32_ptr(inst->session), smem->dma_buf->name,
			smem->size, smem->flags, smem->device_addr,
			smem->bitmap_index, smem->refcount);
	}
	return 0;
}

static void print_internal_buffer(u32 tag, const char *str,
		struct msm_cvp_inst *inst, struct cvp_internal_buf *cbuf)
{
	if (!(tag & msm_cvp_debug) || !inst || !cbuf)
		return;

	if (cbuf->smem->dma_buf) {
		dprintk(tag,
		"%s: %x : fd %d off %d %s size %d iova %#x",
		str, hash32_ptr(inst->session), cbuf->fd,
		cbuf->offset, cbuf->smem->dma_buf->name, cbuf->size,
		cbuf->smem->device_addr);
	} else {
		dprintk(tag,
		"%s: %x : idx %2d fd %d off %d size %d iova %#x",
		str, hash32_ptr(inst->session), cbuf->fd,
		cbuf->offset, cbuf->size, cbuf->smem->device_addr);
	}
}

void print_cvp_buffer(u32 tag, const char *str, struct msm_cvp_inst *inst,
		struct cvp_internal_buf *cbuf)
{
	dprintk(tag, "%s addr: %x size %u\n", str,
			cbuf->smem->device_addr, cbuf->size);
}

static void _log_smem(struct inst_snapshot *snapshot, struct msm_cvp_inst *inst,
		struct msm_cvp_smem *smem, bool logging)
{

	if (print_smem(CVP_ERR, "bufdump", inst, smem))
		return;
	if (!logging || !snapshot)
		return;
	if (snapshot && snapshot->smem_index < MAX_ENTRIES) {
		struct smem_data *s;
		s = &snapshot->smem_log[snapshot->smem_index];
		snapshot->smem_index++;
		s->size = smem->size;
		s->flags = smem->flags;
		s->device_addr = smem->device_addr;
		s->bitmap_index = smem->bitmap_index;
		s->refcount = atomic_read(&smem->refcount);
	}
}

static void _log_buf(struct inst_snapshot *snapshot, enum smem_prop prop,
		struct msm_cvp_inst *inst, struct cvp_internal_buf *cbuf,
		bool logging)
{
	struct cvp_buf_data *buf = NULL;
	u32 index;
	print_cvp_buffer(CVP_ERR, "bufdump", inst, cbuf);
	if (!logging)
		return;
	if (snapshot) {
		if (prop == SMEM_ADSP && snapshot->dsp_index < MAX_ENTRIES) {
			index = snapshot->dsp_index;
			buf = &snapshot->dsp_buf_log[index];
			snapshot->dsp_index++;
		} else if (prop == SMEM_PERSIST &&
				snapshot->persist_index < MAX_ENTRIES) {
			index = snapshot->persist_index;
			buf = &snapshot->persist_buf_log[index];
			snapshot->persist_index++;
		}
		if (buf) {
			buf->device_addr = cbuf->smem->device_addr;
			buf->size = cbuf->size;
		}
	}
}

void print_client_buffer(u32 tag, const char *str,
		struct msm_cvp_inst *inst, struct eva_kmd_buffer *cbuf)
{
	if (!(tag & msm_cvp_debug) || !inst || !cbuf)
		return;

	dprintk(tag,
		"%s: %x : idx %2d fd %d off %d size %d type %d flags 0x%x\n",
		str, hash32_ptr(inst->session), cbuf->index, cbuf->fd,
		cbuf->offset, cbuf->size, cbuf->type, cbuf->flags);
}

static bool __is_buf_valid(struct msm_cvp_inst *inst,
		struct eva_kmd_buffer *buf)
{
	struct cvp_hal_session *session;
	struct cvp_internal_buf *cbuf = NULL;
	bool found = false;

	if (!inst || !inst->core || !buf) {
		dprintk(CVP_ERR, "%s: invalid params\n", __func__);
		return false;
	}

	if (buf->fd < 0) {
		dprintk(CVP_ERR, "%s: Invalid fd = %d", __func__, buf->fd);
		return false;
	}

	if (buf->offset) {
		dprintk(CVP_ERR,
			"%s: offset is deprecated, set to 0.\n",
			__func__);
		return false;
	}

	session = (struct cvp_hal_session *)inst->session;

	mutex_lock(&inst->cvpdspbufs.lock);
	list_for_each_entry(cbuf, &inst->cvpdspbufs.list, list) {
		if (cbuf->fd == buf->fd) {
			if (cbuf->size != buf->size) {
				dprintk(CVP_ERR, "%s: buf size mismatch\n",
					__func__);
				mutex_unlock(&inst->cvpdspbufs.lock);
				return false;
			}
			found = true;
			break;
		}
	}
	mutex_unlock(&inst->cvpdspbufs.lock);
	if (found) {
		print_internal_buffer(CVP_ERR, "duplicate", inst, cbuf);
		return false;
	}

	return true;
}

static struct file *msm_cvp_fget(unsigned int fd, struct task_struct *task,
			fmode_t mask, unsigned int refs)
{
	struct files_struct *files = task->files;
	struct file *file;

	if (!files)
		return NULL;

	rcu_read_lock();
loop:
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 13, 0))
	file = fcheck_files(files, fd);
#else
	file = files_lookup_fd_rcu(files, fd);
#endif
	if (file) {
		/* File object ref couldn't be taken.
		 * dup2() atomicity guarantee is the reason
		 * we loop to catch the new file (or NULL pointer)
		 */
		if (file->f_mode & mask)
			file = NULL;
		else if (!get_file_rcu_many(file, refs))
			goto loop;
	}
	rcu_read_unlock();

	return file;
}

static struct dma_buf *cvp_dma_buf_get(struct file *file, int fd,
			struct task_struct *task)
{
	if (file->f_op != gfa_cv.dmabuf_f_op) {
		dprintk(CVP_WARN, "fd doesn't refer to dma_buf\n");
		return ERR_PTR(-EINVAL);
	}

	return file->private_data;
}

int msm_cvp_map_buf_dsp(struct msm_cvp_inst *inst, struct eva_kmd_buffer *buf)
{
	int rc = 0;
	struct cvp_internal_buf *cbuf = NULL;
	struct msm_cvp_smem *smem = NULL;
	struct dma_buf *dma_buf = NULL;
	struct file *file;

	if (!__is_buf_valid(inst, buf))
		return -EINVAL;

	if (!inst->task)
		return -EINVAL;

	file = msm_cvp_fget(buf->fd, inst->task, FMODE_PATH, 1);
	if (file == NULL) {
		dprintk(CVP_WARN, "%s fail to get file from fd\n", __func__);
		return -EINVAL;
	}

	dma_buf = cvp_dma_buf_get(
			file,
			buf->fd,
			inst->task);
	if (dma_buf == ERR_PTR(-EINVAL)) {
		dprintk(CVP_ERR, "%s: Invalid fd = %d", __func__, buf->fd);
		rc = -EINVAL;
		goto exit;
	}

	dprintk(CVP_MEM, "dma_buf from internal %llu\n", dma_buf);

	cbuf = kmem_cache_zalloc(cvp_driver->buf_cache, GFP_KERNEL);
	if (!cbuf) {
		rc = -ENOMEM;
		goto exit;
	}

	smem = kmem_cache_zalloc(cvp_driver->smem_cache, GFP_KERNEL);
	if (!smem) {
		rc = -ENOMEM;
		goto exit;
	}

	smem->dma_buf = dma_buf;
	smem->bitmap_index = MAX_DMABUF_NUMS;
	dprintk(CVP_MEM, "%s: dma_buf = %llx\n", __func__, dma_buf);
	rc = msm_cvp_map_smem(inst, smem, "map dsp");
	if (rc) {
		print_client_buffer(CVP_ERR, "map failed", inst, buf);
		goto exit;
	}

	cbuf->smem = smem;
	cbuf->fd = buf->fd;
	cbuf->size = buf->size;
	cbuf->offset = buf->offset;
	cbuf->ownership = CLIENT;
	cbuf->index = buf->index;

	buf->reserved[0] = (uint32_t)smem->device_addr;

	mutex_lock(&inst->cvpdspbufs.lock);
	list_add_tail(&cbuf->list, &inst->cvpdspbufs.list);
	mutex_unlock(&inst->cvpdspbufs.lock);

	return rc;

exit:
	fput(file);
	if (smem) {
		if (smem->device_addr) {
			msm_cvp_unmap_smem(inst, smem, "unmap dsp");
			msm_cvp_smem_put_dma_buf(smem->dma_buf);
		}
		kmem_cache_free(cvp_driver->smem_cache, smem);
	}
	if (cbuf)
		kmem_cache_free(cvp_driver->buf_cache, cbuf);
	return rc;
}

int msm_cvp_unmap_buf_dsp(struct msm_cvp_inst *inst, struct eva_kmd_buffer *buf)
{
	int rc = 0;
	bool found;
	struct cvp_internal_buf *cbuf;
	struct cvp_hal_session *session;

	if (!inst || !inst->core || !buf) {
		dprintk(CVP_ERR, "%s: invalid params\n", __func__);
		return -EINVAL;
	}

	session = (struct cvp_hal_session *)inst->session;
	if (!session) {
		dprintk(CVP_ERR, "%s: invalid session\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&inst->cvpdspbufs.lock);
	found = false;
	list_for_each_entry(cbuf, &inst->cvpdspbufs.list, list) {
		if (cbuf->fd == buf->fd) {
			found = true;
			break;
		}
	}
	mutex_unlock(&inst->cvpdspbufs.lock);
	if (!found) {
		print_client_buffer(CVP_ERR, "invalid", inst, buf);
		return -EINVAL;
	}

	if (cbuf->smem->device_addr) {
		msm_cvp_unmap_smem(inst, cbuf->smem, "unmap dsp");
		msm_cvp_smem_put_dma_buf(cbuf->smem->dma_buf);
	}

	mutex_lock(&inst->cvpdspbufs.lock);
	list_del(&cbuf->list);
	mutex_unlock(&inst->cvpdspbufs.lock);

	kmem_cache_free(cvp_driver->smem_cache, cbuf->smem);
	kmem_cache_free(cvp_driver->buf_cache, cbuf);
	return rc;
}


void msm_cvp_cache_operations(struct msm_cvp_smem *smem, u32 type,
				u32 offset, u32 size)
{
	enum smem_cache_ops cache_op;

	if (msm_cvp_cacheop_disabled)
		return;

	if (!smem) {
		dprintk(CVP_ERR, "%s: invalid params\n", __func__);
		return;
	}

	switch (type) {
	case EVA_KMD_BUFTYPE_INPUT:
		cache_op = SMEM_CACHE_CLEAN;
		break;
	case EVA_KMD_BUFTYPE_OUTPUT:
		cache_op = SMEM_CACHE_INVALIDATE;
		break;
	default:
		cache_op = SMEM_CACHE_CLEAN_INVALIDATE;
	}

	dprintk(CVP_MEM,
		"%s: cache operation enabled for dma_buf: %llx, cache_op: %d, offset: %d, size: %d\n",
		__func__, smem->dma_buf, cache_op, offset, size);
	msm_cvp_smem_cache_operations(smem->dma_buf, cache_op, offset, size);
}

static struct msm_cvp_smem *msm_cvp_session_find_smem(struct msm_cvp_inst *inst,
				struct dma_buf *dma_buf)
{
	struct msm_cvp_smem *smem;
	int i;

	if (inst->dma_cache.nr > MAX_DMABUF_NUMS)
		return NULL;

	mutex_lock(&inst->dma_cache.lock);
	for (i = 0; i < inst->dma_cache.nr; i++)
		if (inst->dma_cache.entries[i]->dma_buf == dma_buf) {
			SET_USE_BITMAP(i, inst);
			smem = inst->dma_cache.entries[i];
			smem->bitmap_index = i;
			atomic_inc(&smem->refcount);
			/*
			 * If we find it, it means we already increased
			 * refcount before, so we put it to avoid double
			 * incremental.
			 */
			msm_cvp_smem_put_dma_buf(smem->dma_buf);
			mutex_unlock(&inst->dma_cache.lock);
			print_smem(CVP_MEM, "found", inst, smem);
			return smem;
		}

	mutex_unlock(&inst->dma_cache.lock);

	return NULL;
}

static int msm_cvp_session_add_smem(struct msm_cvp_inst *inst,
				struct msm_cvp_smem *smem)
{
	unsigned int i;
	struct msm_cvp_smem *smem2;

	mutex_lock(&inst->dma_cache.lock);
	if (inst->dma_cache.nr < MAX_DMABUF_NUMS) {
		inst->dma_cache.entries[inst->dma_cache.nr] = smem;
		SET_USE_BITMAP(inst->dma_cache.nr, inst);
		smem->bitmap_index = inst->dma_cache.nr;
		inst->dma_cache.nr++;
		i = smem->bitmap_index;
	} else {
		i = find_first_zero_bit(&inst->dma_cache.usage_bitmap,
				MAX_DMABUF_NUMS);
		if (i < MAX_DMABUF_NUMS) {
			smem2 = inst->dma_cache.entries[i];
			msm_cvp_unmap_smem(inst, smem2, "unmap cpu");
			msm_cvp_smem_put_dma_buf(smem2->dma_buf);
			kmem_cache_free(cvp_driver->smem_cache, smem2);

			inst->dma_cache.entries[i] = smem;
			smem->bitmap_index = i;
			SET_USE_BITMAP(i, inst);
		} else {
			dprintk(CVP_WARN, "%s: not enough memory\n", __func__);
			mutex_unlock(&inst->dma_cache.lock);
			return -ENOMEM;
		}
	}

	atomic_inc(&smem->refcount);
	mutex_unlock(&inst->dma_cache.lock);
	dprintk(CVP_MEM, "Add entry %d into cache\n", i);

	return 0;
}

static struct msm_cvp_smem *msm_cvp_session_get_smem(struct msm_cvp_inst *inst,
						struct cvp_buf_type *buf)
{
	int rc = 0, found = 1;
	struct msm_cvp_smem *smem = NULL;
	struct dma_buf *dma_buf = NULL;

	if (buf->fd < 0) {
		dprintk(CVP_ERR, "%s: Invalid fd = %d", __func__, buf->fd);
		return NULL;
	}

	dma_buf = msm_cvp_smem_get_dma_buf(buf->fd);
	if (!dma_buf) {
		dprintk(CVP_ERR, "%s: Invalid fd = %d", __func__, buf->fd);
		return NULL;
	}

	smem = msm_cvp_session_find_smem(inst, dma_buf);
	if (!smem) {
		found = 0;
		smem = kmem_cache_zalloc(cvp_driver->smem_cache, GFP_KERNEL);
		if (!smem)
			return NULL;

		smem->dma_buf = dma_buf;
		smem->bitmap_index = MAX_DMABUF_NUMS;
		rc = msm_cvp_map_smem(inst, smem, "map cpu");
		if (rc)
			goto exit;
		if (buf->size > smem->size || buf->size > smem->size - buf->offset) {
			dprintk(CVP_ERR, "%s: invalid offset %d or size %d for a new entry\n",
				__func__, buf->offset, buf->size);
			goto exit2;
		}
		rc = msm_cvp_session_add_smem(inst, smem);
		if (rc && rc != -ENOMEM)
			goto exit2;
	}

	if (buf->size > smem->size || buf->size > smem->size - buf->offset) {
		dprintk(CVP_ERR, "%s: invalid offset %d or size %d\n",
			__func__, buf->offset, buf->size);
		if (found) {
			mutex_lock(&inst->dma_cache.lock);
			atomic_dec(&smem->refcount);
			mutex_unlock(&inst->dma_cache.lock);
			return NULL;
		}
		goto exit2;
	}

	return smem;

exit2:
	msm_cvp_unmap_smem(inst, smem, "unmap cpu");
exit:
	msm_cvp_smem_put_dma_buf(dma_buf);
	kmem_cache_free(cvp_driver->smem_cache, smem);
	smem = NULL;
	return smem;
}

static u32 msm_cvp_map_user_persist_buf(struct msm_cvp_inst *inst,
				struct cvp_buf_type *buf)
{
	u32 iova = 0;
	struct msm_cvp_smem *smem = NULL;
	struct cvp_internal_buf *pbuf;

	if (!inst) {
		dprintk(CVP_ERR, "%s: invalid params\n", __func__);
		return -EINVAL;
	}

	pbuf = kmem_cache_zalloc(cvp_driver->buf_cache, GFP_KERNEL);
	if (!pbuf)
		return 0;

	smem = msm_cvp_session_get_smem(inst, buf);
	if (!smem)
		goto exit;

	smem->flags |= SMEM_PERSIST;
	pbuf->smem = smem;
	pbuf->fd = buf->fd;
	pbuf->size = buf->size;
	pbuf->offset = buf->offset;
	pbuf->ownership = CLIENT;

	mutex_lock(&inst->persistbufs.lock);
	list_add_tail(&pbuf->list, &inst->persistbufs.list);
	mutex_unlock(&inst->persistbufs.lock);

	print_internal_buffer(CVP_MEM, "map persist", inst, pbuf);

	iova = smem->device_addr + buf->offset;

	return iova;

exit:
	kmem_cache_free(cvp_driver->buf_cache, pbuf);
	return 0;
}

u32 msm_cvp_map_frame_buf(struct msm_cvp_inst *inst,
			struct cvp_buf_type *buf,
			struct msm_cvp_frame *frame)
{
	u32 iova = 0;
	struct msm_cvp_smem *smem = NULL;
	u32 nr;
	u32 type;

	if (!inst || !frame) {
		dprintk(CVP_ERR, "%s: invalid params\n", __func__);
		return 0;
	}

	nr = frame->nr;
	if (nr == MAX_FRAME_BUFFER_NUMS) {
		dprintk(CVP_ERR, "%s: max frame buffer reached\n", __func__);
		return 0;
	}

	smem = msm_cvp_session_get_smem(inst, buf);
	if (!smem)
		return 0;

	frame->bufs[nr].fd = buf->fd;
	frame->bufs[nr].smem = smem;
	frame->bufs[nr].size = buf->size;
	frame->bufs[nr].offset = buf->offset;

	print_internal_buffer(CVP_MEM, "map cpu", inst, &frame->bufs[nr]);

	frame->nr++;

	type = EVA_KMD_BUFTYPE_INPUT | EVA_KMD_BUFTYPE_OUTPUT;
	msm_cvp_cache_operations(smem, type, buf->offset, buf->size);

	iova = smem->device_addr + buf->offset;

	return iova;
}

static void msm_cvp_unmap_frame_buf(struct msm_cvp_inst *inst,
			struct msm_cvp_frame *frame)
{
	u32 i;
	u32 type;
	struct msm_cvp_smem *smem = NULL;
	struct cvp_internal_buf *buf;

	type = EVA_KMD_BUFTYPE_OUTPUT;

	for (i = 0; i < frame->nr; ++i) {
		buf = &frame->bufs[i];
		smem = buf->smem;
		msm_cvp_cache_operations(smem, type, buf->offset, buf->size);

		if (smem->bitmap_index >= MAX_DMABUF_NUMS) {
			/* smem not in dmamap cache */
			msm_cvp_unmap_smem(inst, smem, "unmap cpu");
			dma_heap_buffer_free(smem->dma_buf);
			kmem_cache_free(cvp_driver->smem_cache, smem);
			buf->smem = NULL;
		} else {
			mutex_lock(&inst->dma_cache.lock);
			if (atomic_dec_and_test(&smem->refcount)) {
				CLEAR_USE_BITMAP(smem->bitmap_index, inst);
				print_smem(CVP_MEM, "Map dereference",
					inst, smem);
			}
			mutex_unlock(&inst->dma_cache.lock);
		}
	}

	kmem_cache_free(cvp_driver->frame_cache, frame);
}

void msm_cvp_unmap_frame(struct msm_cvp_inst *inst, u64 ktid)
{
	struct msm_cvp_frame *frame, *dummy1;
	bool found;

	if (!inst) {
		dprintk(CVP_ERR, "%s: invalid params\n", __func__);
		return;
	}

	ktid &= (FENCE_BIT - 1);
	dprintk(CVP_MEM, "%s: (%#x) unmap frame %llu\n",
			__func__, hash32_ptr(inst->session), ktid);

	found = false;
	mutex_lock(&inst->frames.lock);
	list_for_each_entry_safe(frame, dummy1, &inst->frames.list, list) {
		if (frame->ktid == ktid) {
			found = true;
			list_del(&frame->list);
			break;
		}
	}
	mutex_unlock(&inst->frames.lock);

	if (found)
		msm_cvp_unmap_frame_buf(inst, frame);
	else
		dprintk(CVP_WARN, "%s frame %llu not found!\n", __func__, ktid);
}

int msm_cvp_unmap_user_persist(struct msm_cvp_inst *inst,
				struct eva_kmd_hfi_packet *in_pkt,
				unsigned int offset, unsigned int buf_num)
{
	struct cvp_hfi_cmd_session_hdr *cmd_hdr;
	struct cvp_internal_buf *pbuf, *dummy;
	u64 ktid;
	int rc = 0;
	struct msm_cvp_smem *smem = NULL;

	if (!offset || !buf_num)
		return rc;

	cmd_hdr = (struct cvp_hfi_cmd_session_hdr *)in_pkt;
	ktid = cmd_hdr->client_data.kdata & (FENCE_BIT - 1);

	mutex_lock(&inst->persistbufs.lock);
	list_for_each_entry_safe(pbuf, dummy, &inst->persistbufs.list, list) {
		if (pbuf->ktid == ktid && pbuf->ownership == CLIENT) {
			list_del(&pbuf->list);
			smem = pbuf->smem;

			dprintk(CVP_MEM, "unmap persist: %x %d %d %#x",
				hash32_ptr(inst->session), pbuf->fd,
				pbuf->size, smem->device_addr);

			if (smem->bitmap_index >= MAX_DMABUF_NUMS) {
				/* smem not in dmamap cache */
				msm_cvp_unmap_smem(inst, smem,
						"unmap cpu");
				dma_heap_buffer_free(smem->dma_buf);
				kmem_cache_free(
					cvp_driver->smem_cache,
					smem);
				pbuf->smem = NULL;
			} else {
				mutex_lock(&inst->dma_cache.lock);
				if (atomic_dec_and_test(&smem->refcount))
					CLEAR_USE_BITMAP(
						smem->bitmap_index,
						inst);
				mutex_unlock(&inst->dma_cache.lock);
			}

			kmem_cache_free(cvp_driver->buf_cache, pbuf);
		}
	}
	mutex_unlock(&inst->persistbufs.lock);
	return rc;
}

int msm_cvp_mark_user_persist(struct msm_cvp_inst *inst,
			struct eva_kmd_hfi_packet *in_pkt,
			unsigned int offset, unsigned int buf_num)
{
	struct cvp_hfi_cmd_session_hdr *cmd_hdr;
	struct cvp_internal_buf *pbuf, *dummy;
	u64 ktid;
	struct cvp_buf_type *buf;
	int i, rc = 0;

	if (!offset || !buf_num)
		return 0;

	cmd_hdr = (struct cvp_hfi_cmd_session_hdr *)in_pkt;
	ktid = atomic64_inc_return(&inst->core->kernel_trans_id);
	ktid &= (FENCE_BIT - 1);
	cmd_hdr->client_data.kdata = ktid;

	for (i = 0; i < buf_num; i++) {
		buf = (struct cvp_buf_type *)&in_pkt->pkt_data[offset];
		offset += sizeof(*buf) >> 2;

		if (buf->fd < 0 || !buf->size)
			continue;

		mutex_lock(&inst->persistbufs.lock);
		list_for_each_entry_safe(pbuf, dummy, &inst->persistbufs.list,
				list) {
			if (pbuf->ownership == CLIENT) {
				if (pbuf->fd == buf->fd &&
					pbuf->size == buf->size)
					buf->fd = pbuf->smem->device_addr;
				rc = 1;
				break;
			}
		}
		mutex_unlock(&inst->persistbufs.lock);
		if (!rc) {
			dprintk(CVP_ERR, "%s No persist buf %d found\n",
				__func__, buf->fd);
			rc = -EFAULT;
			break;
		}
		pbuf->ktid = ktid;
		rc = 0;
	}
	return rc;
}

int msm_cvp_map_user_persist(struct msm_cvp_inst *inst,
			struct eva_kmd_hfi_packet *in_pkt,
			unsigned int offset, unsigned int buf_num)
{
	struct cvp_buf_type *buf;
	int i;
	u32 iova;

	if (!offset || !buf_num)
		return 0;

	for (i = 0; i < buf_num; i++) {
		buf = (struct cvp_buf_type *)&in_pkt->pkt_data[offset];
		offset += sizeof(*buf) >> 2;

		if (buf->fd < 0 || !buf->size)
			continue;

		iova = msm_cvp_map_user_persist_buf(inst, buf);
		if (!iova) {
			dprintk(CVP_ERR,
				"%s: buf %d register failed.\n",
				__func__, i);

			return -EINVAL;
		}
		buf->fd = iova;
	}
	return 0;
}

int msm_cvp_map_frame(struct msm_cvp_inst *inst,
		struct eva_kmd_hfi_packet *in_pkt,
		unsigned int offset, unsigned int buf_num)
{
	struct cvp_buf_type *buf;
	int i;
	u32 iova;
	u64 ktid;
	struct msm_cvp_frame *frame;
	struct cvp_hfi_cmd_session_hdr *cmd_hdr;

	if (!offset || !buf_num)
		return 0;

	cmd_hdr = (struct cvp_hfi_cmd_session_hdr *)in_pkt;
	ktid = atomic64_inc_return(&inst->core->kernel_trans_id);
	ktid &= (FENCE_BIT - 1);
	cmd_hdr->client_data.kdata = ktid;

	frame = kmem_cache_zalloc(cvp_driver->frame_cache, GFP_KERNEL);
	if (!frame)
		return -ENOMEM;

	frame->ktid = ktid;
	frame->nr = 0;
	frame->pkt_type = cmd_hdr->packet_type;

	for (i = 0; i < buf_num; i++) {
		buf = (struct cvp_buf_type *)&in_pkt->pkt_data[offset];
		offset += sizeof(*buf) >> 2;

		if (buf->fd < 0 || !buf->size)
			continue;

		iova = msm_cvp_map_frame_buf(inst, buf, frame);
		if (!iova) {
			dprintk(CVP_ERR,
				"%s: buf %d register failed.\n",
				__func__, i);

			msm_cvp_unmap_frame_buf(inst, frame);
			return -EINVAL;
		}
		buf->fd = iova;
	}

	mutex_lock(&inst->frames.lock);
	list_add_tail(&frame->list, &inst->frames.list);
	mutex_unlock(&inst->frames.lock);
	dprintk(CVP_MEM, "%s: map frame %llu\n", __func__, ktid);

	return 0;
}

int msm_cvp_session_deinit_buffers(struct msm_cvp_inst *inst)
{
	int rc = 0, i;
	struct cvp_internal_buf *cbuf, *dummy;
	struct msm_cvp_frame *frame, *dummy1;
	struct msm_cvp_smem *smem;
	struct cvp_hal_session *session;

	session = (struct cvp_hal_session *)inst->session;

	mutex_lock(&inst->frames.lock);
	list_for_each_entry_safe(frame, dummy1, &inst->frames.list, list) {
		list_del(&frame->list);
		msm_cvp_unmap_frame_buf(inst, frame);
	}
	mutex_unlock(&inst->frames.lock);

	mutex_lock(&inst->dma_cache.lock);
	for (i = 0; i < inst->dma_cache.nr; i++) {
		smem = inst->dma_cache.entries[i];
		if (atomic_read(&smem->refcount) == 0) {
			print_smem(CVP_MEM, "free", inst, smem);
		} else if (!(smem->flags & SMEM_PERSIST)) {
			print_smem(CVP_WARN, "in use", inst, smem);
		}
		msm_cvp_unmap_smem(inst, smem, "unmap cpu");
		msm_cvp_smem_put_dma_buf(smem->dma_buf);
		kmem_cache_free(cvp_driver->smem_cache, smem);
		inst->dma_cache.entries[i] = NULL;
	}
	mutex_unlock(&inst->dma_cache.lock);

	mutex_lock(&inst->cvpdspbufs.lock);
	list_for_each_entry_safe(cbuf, dummy, &inst->cvpdspbufs.list, list) {
		print_internal_buffer(CVP_MEM, "remove dspbufs", inst, cbuf);
		if (cbuf->ownership == CLIENT) {
			rc = cvp_dsp_deregister_buffer(hash32_ptr(session),
				cbuf->fd, cbuf->smem->dma_buf->size, cbuf->size,
				cbuf->offset, cbuf->index,
				(uint32_t)cbuf->smem->device_addr);
			if (rc)
				dprintk(CVP_ERR,
				"%s: failed dsp deregistration fd=%d rc=%d",
				__func__, cbuf->fd, rc);
			msm_cvp_unmap_smem(inst, cbuf->smem, "unmap dsp");
			msm_cvp_smem_put_dma_buf(cbuf->smem->dma_buf);
		} else if (cbuf->ownership == DSP) {
			rc = cvp_dsp_fastrpc_unmap(inst->process_id, cbuf);
			if (rc)
				dprintk(CVP_ERR,
				"%s: failed to unmap buf from DSP\n",
				__func__);

			rc = cvp_release_dsp_buffers(inst, cbuf);
			if (rc)
				dprintk(CVP_ERR,
					"%s Fail to free buffer 0x%x\n",
					__func__, rc);
		}
		list_del(&cbuf->list);
		kmem_cache_free(cvp_driver->buf_cache, cbuf);
	}
	mutex_unlock(&inst->cvpdspbufs.lock);

	return rc;
}

void msm_cvp_print_inst_bufs(struct msm_cvp_inst *inst, bool log)
{
	struct cvp_internal_buf *buf;
	struct msm_cvp_core *core;
	struct inst_snapshot *snap = NULL;
	int i;

	core = list_first_entry(&cvp_driver->cores, struct msm_cvp_core, list);
	if (log && core->log.snapshot_index < 16) {
		snap = &core->log.snapshot[core->log.snapshot_index];
		snap->session = inst->session;
		core->log.snapshot_index++;
	}

	if (!inst) {
		dprintk(CVP_ERR, "%s - invalid param %pK\n",
			__func__, inst);
		return;
	}

	dprintk(CVP_ERR,
			"---Buffer details for inst: %pK of type: %d---\n",
			inst, inst->session_type);
	mutex_lock(&inst->dma_cache.lock);
	dprintk(CVP_ERR, "dma cache: %d\n", inst->dma_cache.nr);
	if (inst->dma_cache.nr <= MAX_DMABUF_NUMS)
		for (i = 0; i < inst->dma_cache.nr; i++)
			_log_smem(snap, inst, inst->dma_cache.entries[i], log);
	mutex_unlock(&inst->dma_cache.lock);

	mutex_lock(&inst->cvpdspbufs.lock);
	dprintk(CVP_ERR, "dsp buffer list:\n");
	list_for_each_entry(buf, &inst->cvpdspbufs.list, list)
		_log_buf(snap, SMEM_ADSP, inst, buf, log);
	mutex_unlock(&inst->cvpdspbufs.lock);

	mutex_lock(&inst->persistbufs.lock);
	dprintk(CVP_ERR, "persist buffer list:\n");
	list_for_each_entry(buf, &inst->persistbufs.list, list)
		_log_buf(snap, SMEM_PERSIST, inst, buf, log);
	mutex_unlock(&inst->persistbufs.lock);
}

struct cvp_internal_buf *cvp_allocate_arp_bufs(struct msm_cvp_inst *inst,
			u32 buffer_size)
{
	struct cvp_internal_buf *buf;
	struct msm_cvp_list *buf_list;
	u32 smem_flags = SMEM_UNCACHED;
	int rc = 0;

	if (!inst) {
		dprintk(CVP_ERR, "%s Invalid input\n", __func__);
		return NULL;
	}

	buf_list = &inst->persistbufs;

	if (!buffer_size)
		return NULL;

	/* PERSIST buffer requires secure mapping
	 * Disable and wait for hyp_assign available
	 */
	 smem_flags |= SMEM_SECURE | SMEM_NON_PIXEL;

	buf = kmem_cache_zalloc(cvp_driver->buf_cache, GFP_KERNEL);
	if (!buf) {
		dprintk(CVP_ERR, "%s Out of memory\n", __func__);
		goto fail_kzalloc;
	}

	buf->smem = kmem_cache_zalloc(cvp_driver->smem_cache, GFP_KERNEL);
	if (!buf->smem) {
		dprintk(CVP_ERR, "%s Out of memory\n", __func__);
		goto fail_kzalloc;
	}

	buf->smem->flags = smem_flags;
	rc = msm_cvp_smem_alloc(buffer_size, 1, 0,
		&(inst->core->resources), buf->smem);
	if (rc) {
		dprintk(CVP_ERR, "Failed to allocate ARP memory\n");
		goto err_no_mem;
	}

	buf->size = buf->smem->size;
	buf->type = HFI_BUFFER_INTERNAL_PERSIST_1;
	buf->ownership = DRIVER;

	mutex_lock(&buf_list->lock);
	list_add_tail(&buf->list, &buf_list->list);
	mutex_unlock(&buf_list->lock);
	return buf;

err_no_mem:
	kmem_cache_free(cvp_driver->buf_cache, buf);
fail_kzalloc:
	return NULL;
}

int cvp_release_arp_buffers(struct msm_cvp_inst *inst)
{
	struct msm_cvp_smem *smem;
	struct list_head *ptr, *next;
	struct cvp_internal_buf *buf;
	int rc = 0;
	struct msm_cvp_core *core;
	struct cvp_hfi_device *hdev;

	if (!inst) {
		dprintk(CVP_ERR, "Invalid instance pointer = %pK\n", inst);
		return -EINVAL;
	}

	core = inst->core;
	if (!core) {
		dprintk(CVP_ERR, "Invalid core pointer = %pK\n", core);
		return -EINVAL;
	}
	hdev = core->device;
	if (!hdev) {
		dprintk(CVP_ERR, "Invalid device pointer = %pK\n", hdev);
		return -EINVAL;
	}

	dprintk(CVP_MEM, "release persist buffer!\n");

	mutex_lock(&inst->persistbufs.lock);
	/* Workaround for FW: release buffer means release all */
	if (inst->state <= MSM_CVP_CLOSE_DONE) {
		rc = call_hfi_op(hdev, session_release_buffers,
				(void *)inst->session);
		if (!rc) {
			mutex_unlock(&inst->persistbufs.lock);
			rc = wait_for_sess_signal_receipt(inst,
				HAL_SESSION_RELEASE_BUFFER_DONE);
			if (rc)
				dprintk(CVP_WARN,
				"%s: wait for signal failed, rc %d\n",
				__func__, rc);
			mutex_lock(&inst->persistbufs.lock);
		} else {
			dprintk(CVP_WARN, "Fail to send Rel prst buf\n");
		}
	}

	list_for_each_safe(ptr, next, &inst->persistbufs.list) {
		buf = list_entry(ptr, struct cvp_internal_buf, list);
		smem = buf->smem;
		if (!smem) {
			dprintk(CVP_ERR, "%s invalid smem\n", __func__);
			mutex_unlock(&inst->persistbufs.lock);
			return -EINVAL;
		}

		list_del(&buf->list);

		if (buf->ownership == DRIVER) {
			dprintk(CVP_MEM,
			"%s: %x : fd %d %s size %d",
			"free arp", hash32_ptr(inst->session), buf->fd,
			smem->dma_buf->name, buf->size);
			msm_cvp_smem_free(smem);
			kmem_cache_free(cvp_driver->smem_cache, smem);
		}
		buf->smem = NULL;
		kmem_cache_free(cvp_driver->buf_cache, buf);
	}
	mutex_unlock(&inst->persistbufs.lock);
	return rc;
}

int cvp_allocate_dsp_bufs(struct msm_cvp_inst *inst,
			struct cvp_internal_buf *buf,
			u32 buffer_size,
			u32 secure_type)
{
	u32 smem_flags = SMEM_UNCACHED;
	int rc = 0;

	if (!inst) {
		dprintk(CVP_ERR, "%s Invalid input\n", __func__);
		return -EINVAL;
	}

	if (!buf)
		return -EINVAL;

	if (!buffer_size)
		return -EINVAL;

	switch (secure_type) {
	case 0:
		break;
	case 1:
		smem_flags |= SMEM_SECURE | SMEM_PIXEL;
		break;
	case 2:
		smem_flags |= SMEM_SECURE | SMEM_NON_PIXEL;
		break;
	default:
		dprintk(CVP_ERR, "%s Invalid secure_type %d\n",
			__func__, secure_type);
		return -EINVAL;
	}

	dprintk(CVP_MEM, "%s smem_flags 0x%x\n", __func__, smem_flags);
	buf->smem = kmem_cache_zalloc(cvp_driver->smem_cache, GFP_KERNEL);
	if (!buf->smem) {
		dprintk(CVP_ERR, "%s Out of memory\n", __func__);
		goto fail_kzalloc_smem_cache;
	}

	buf->smem->flags = smem_flags;
	rc = msm_cvp_smem_alloc(buffer_size, 1, 0,
			&(inst->core->resources), buf->smem);
	if (rc) {
		dprintk(CVP_ERR, "Failed to allocate ARP memory\n");
		goto err_no_mem;
	}

	dprintk(CVP_MEM, "%s dma_buf %pK\n", __func__, buf->smem->dma_buf);

	buf->size = buf->smem->size;
	buf->type = HFI_BUFFER_INTERNAL_PERSIST_1;
	buf->ownership = DSP;

	return rc;

err_no_mem:
	kmem_cache_free(cvp_driver->smem_cache, buf->smem);
fail_kzalloc_smem_cache:
	return rc;
}

int cvp_release_dsp_buffers(struct msm_cvp_inst *inst,
			struct cvp_internal_buf *buf)
{
	struct msm_cvp_smem *smem;
	int rc = 0;

	if (!inst) {
		dprintk(CVP_ERR, "Invalid instance pointer = %pK\n", inst);
		return -EINVAL;
	}

	if (!buf) {
		dprintk(CVP_ERR, "Invalid buffer pointer = %pK\n", inst);
		return -EINVAL;
	}

	smem = buf->smem;
	if (!smem) {
		dprintk(CVP_ERR, "%s invalid smem\n", __func__);
		return -EINVAL;
	}

	if (buf->ownership == DSP) {
		dprintk(CVP_MEM,
			"%s: %x : fd %x %s size %d",
			__func__, hash32_ptr(inst->session), buf->fd,
			smem->dma_buf->name, buf->size);
		msm_cvp_smem_free(smem);
		kmem_cache_free(cvp_driver->smem_cache, smem);
	} else {
		dprintk(CVP_ERR,
			"%s: wrong owner %d %x : fd %x %s size %d",
			__func__, buf->ownership, hash32_ptr(inst->session),
			buf->fd, smem->dma_buf->name, buf->size);
	}

	return rc;
}

int msm_cvp_register_buffer(struct msm_cvp_inst *inst,
		struct eva_kmd_buffer *buf)
{
	struct cvp_hfi_device *hdev;
	struct cvp_hal_session *session;
	struct msm_cvp_inst *s;
	int rc = 0;

	if (!inst || !inst->core || !buf) {
		dprintk(CVP_ERR, "%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (!buf->index)
		return 0;

	s = cvp_get_inst_validate(inst->core, inst);
	if (!s)
		return -ECONNRESET;

	session = (struct cvp_hal_session *)inst->session;
	if (!session) {
		dprintk(CVP_ERR, "%s: invalid session\n", __func__);
		rc = -EINVAL;
		goto exit;
	}
	hdev = inst->core->device;
	print_client_buffer(CVP_HFI, "register", inst, buf);

	rc = msm_cvp_map_buf_dsp(inst, buf);
	dprintk(CVP_DSP, "%s: fd %d, iova 0x%x\n", __func__,
			buf->fd, buf->reserved[0]);
exit:
	cvp_put_inst(s);
	return rc;
}

int msm_cvp_unregister_buffer(struct msm_cvp_inst *inst,
		struct eva_kmd_buffer *buf)
{
	struct msm_cvp_inst *s;
	int rc = 0;

	if (!inst || !inst->core || !buf) {
		dprintk(CVP_ERR, "%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (!buf->index)
		return 0;

	s = cvp_get_inst_validate(inst->core, inst);
	if (!s)
		return -ECONNRESET;

	print_client_buffer(CVP_HFI, "unregister", inst, buf);

	rc = msm_cvp_unmap_buf_dsp(inst, buf);
	cvp_put_inst(s);
	return rc;
}



