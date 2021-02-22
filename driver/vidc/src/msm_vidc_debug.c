// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#include "msm_vidc_debug.h"
#include "msm_vidc_driver.h"
#include "msm_vidc_dt.h"
#include "msm_vidc.h"
#include "msm_vidc_core.h"
#include "msm_vidc_inst.h"
#include "msm_vidc_internal.h"

#define MAX_SSR_STRING_LEN         10
#define MAX_DEBUG_LEVEL_STRING_LEN 15

int msm_vidc_debug = VIDC_ERR | VIDC_PRINTK | FW_ERROR | FW_FATAL;
EXPORT_SYMBOL(msm_vidc_debug);

bool msm_vidc_lossless_encode = !true;
EXPORT_SYMBOL(msm_vidc_lossless_encode);

bool msm_vidc_syscache_disable = !true;
EXPORT_SYMBOL(msm_vidc_syscache_disable);

int msm_vidc_clock_voting = !1;

const char *level_str(u32 level)
{
	if (level & VIDC_ERR)
		return "err ";
	else if (level & VIDC_HIGH)
		return "high";
	else if (level & VIDC_LOW)
		return "low ";
	else if (level & VIDC_PERF)
		return "perf";
	else if (level & VIDC_PKT)
		return "pkt ";
	else if (level & VIDC_BUS)
		return "bus ";
	else
		return "????";
}

const char *codec_str(void *instance)
{
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		goto err_invalid_inst;
	}

	if (inst->codec == MSM_VIDC_H264)
		return "h264 ";
	else if (inst->codec == MSM_VIDC_HEVC)
		return "hevc ";
	else if (inst->codec == MSM_VIDC_VP9)
		return "vp9  ";
	else
		return "?????";

err_invalid_inst:
	return "null  ";
}

u32 get_sid(void *instance)
{
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		goto err_invalid_inst;
	}

	return inst->sid;

err_invalid_inst:
	return 0;
}

#define MAX_DBG_BUF_SIZE 4096

struct core_inst_pair {
	struct msm_vidc_core *core;
	struct msm_vidc_inst *inst;
};

/* debug fs support */

static inline void tic(struct msm_vidc_inst *i, enum profiling_points p,
				 char *b)
{
	return;
}

static inline void toc(struct msm_vidc_inst *i, enum profiling_points p)
{
	return;
}

static u32 write_str(char *buffer,
		size_t size, const char *fmt, ...)
{
	va_list args;
	u32 len;

	va_start(args, fmt);
	len = vscnprintf(buffer, size, fmt, args);
	va_end(args);
	return len;
}

static ssize_t core_info_read(struct file* file, char __user* buf,
	size_t count, loff_t* ppos)
{
	struct msm_vidc_core *core = file->private_data;
	char* dbuf, * cur, * end;
	ssize_t len = 0;

	if (!core || !core->dt) {
		d_vpr_e("%s: invalid params %pK\n", __func__, core);
		return 0;
	}

	dbuf = kzalloc(MAX_DBG_BUF_SIZE, GFP_KERNEL);
	if (!dbuf) {
		d_vpr_e("%s: Allocation failed!\n", __func__);
		return -ENOMEM;
	}
	cur = dbuf;
	end = cur + MAX_DBG_BUF_SIZE;

	cur += write_str(cur, end - cur, "Core state: %d\n", core->state);

	cur += write_str(cur, end - cur,
		"FW version : %s\n", core->fw_version);
	cur += write_str(cur, end - cur,
		"register_base: 0x%x\n", core->dt->register_base);
	cur += write_str(cur, end - cur,
		"register_size: %u\n", core->dt->register_size);
	cur += write_str(cur, end - cur, "irq: %u\n", core->dt->irq);

	len = simple_read_from_buffer(buf, count, ppos,
		dbuf, cur - dbuf);

	kfree(dbuf);
	return len;
}

static const struct file_operations core_info_fops = {
	.open = simple_open,
	.read = core_info_read,
};

static ssize_t trigger_ssr_write(struct file* filp, const char __user* buf,
	size_t count, loff_t* ppos)
{
	unsigned long ssr_trigger_val = 0;
	int rc = 0;
	struct msm_vidc_core* core = filp->private_data;
	size_t size = MAX_SSR_STRING_LEN;
	char kbuf[MAX_SSR_STRING_LEN + 1] = { 0 };

	if (!buf)
		return -EINVAL;

	if (!count)
		goto exit;

	if (count < size)
		size = count;

	if (copy_from_user(kbuf, buf, size)) {
		d_vpr_e("%s: User memory fault\n", __func__);
		rc = -EFAULT;
		goto exit;
	}

	rc = kstrtoul(kbuf, 0, &ssr_trigger_val);
	if (rc) {
		d_vpr_e("returning error err %d\n", rc);
		rc = -EINVAL;
	}
	else {
		msm_vidc_trigger_ssr(core, ssr_trigger_val);
		rc = count;
	}
exit:
	return rc;
}

static const struct file_operations ssr_fops = {
	.open = simple_open,
	.write = trigger_ssr_write,
};

static ssize_t debug_level_write(struct file* filp, const char __user* buf,
	size_t count, loff_t* ppos)
{
	int rc = 0;
	struct msm_vidc_core* core = filp->private_data;
	char kbuf[MAX_DEBUG_LEVEL_STRING_LEN] = { 0 };

	/* filter partial writes and invalid commands */
	if (*ppos != 0 || count >= sizeof(kbuf) || count == 0) {
		d_vpr_e("returning error - pos %d, count %d\n", *ppos, count);
		rc = -EINVAL;
	}

	rc = simple_write_to_buffer(kbuf, sizeof(kbuf) - 1, ppos, buf, count);
	if (rc < 0) {
		d_vpr_e("%s: User memory fault\n", __func__);
		rc = -EFAULT;
		goto exit;
	}

	rc = kstrtoint(kbuf, 0, &msm_vidc_debug);
	if (rc) {
		d_vpr_e("returning error err %d\n", rc);
		rc = -EINVAL;
		goto exit;
	}
	rc = count;
	if (core->capabilities) {
		core->capabilities[HW_RESPONSE_TIMEOUT].value =
			((msm_vidc_debug & 0xFF) >
			(VIDC_ERR | VIDC_HIGH)) ? 1500 : 1000;
		d_vpr_h("debug timeout updated to - %d ms\n",
			core->capabilities[HW_RESPONSE_TIMEOUT].value);
	}

exit:
	return rc;
}

static ssize_t debug_level_read(struct file* file, char __user* buf,
	size_t count, loff_t* ppos)
{
	size_t len;
	char kbuf[MAX_DEBUG_LEVEL_STRING_LEN];

	len = scnprintf(kbuf, sizeof(kbuf), "0x%08x\n", msm_vidc_debug);
	return simple_read_from_buffer(buf, count, ppos, kbuf, len);
}

static const struct file_operations debug_level_fops = {
	.open = simple_open,
	.write = debug_level_write,
	.read = debug_level_read,
};

struct dentry* msm_vidc_debugfs_init_drv()
{
	struct dentry *dir = NULL;

	dir = debugfs_create_dir("msm_vidc", NULL);
	if (IS_ERR_OR_NULL(dir)) {
		dir = NULL;
		goto failed_create_dir;
	}

	debugfs_create_u32("core_clock_voting", 0644, dir,
			&msm_vidc_clock_voting);
	debugfs_create_bool("disable_video_syscache", 0644, dir,
			&msm_vidc_syscache_disable);
	debugfs_create_bool("lossless_encoding", 0644, dir,
			&msm_vidc_lossless_encode);

	return dir;

failed_create_dir:
	if (dir)
		debugfs_remove_recursive(dir);

	return NULL;
}

struct dentry *msm_vidc_debugfs_init_core(void *core_in)
{
	struct dentry *dir = NULL;
	char debugfs_name[MAX_DEBUGFS_NAME];
	struct msm_vidc_core *core = (struct msm_vidc_core *) core_in;
	struct dentry *parent;

	if (!core || !core->debugfs_parent) {
		d_vpr_e("%s: invalid params\n", __func__);
		goto failed_create_dir;
	}
	parent = core->debugfs_parent;

	snprintf(debugfs_name, MAX_DEBUGFS_NAME, "core");
	dir = debugfs_create_dir(debugfs_name, parent);
	if (IS_ERR_OR_NULL(dir)) {
		dir = NULL;
		d_vpr_e("Failed to create debugfs for msm_vidc\n");
		goto failed_create_dir;
	}
	if (!debugfs_create_file("info", 0444, dir, core, &core_info_fops)) {
		d_vpr_e("debugfs_create_file: fail\n");
		goto failed_create_dir;
	}
	if (!debugfs_create_file("trigger_ssr", 0200,
			dir, core, &ssr_fops)) {
		d_vpr_e("debugfs_create_file: fail\n");
		goto failed_create_dir;
	}
	if (!debugfs_create_file("debug_level", 0644,
			parent, core, &debug_level_fops)) {
		d_vpr_e("debugfs_create_file: fail\n");
		goto failed_create_dir;
	}
failed_create_dir:
	return dir;
}

static int inst_info_open(struct inode *inode, struct file *file)
{
	d_vpr_l("Open inode ptr: %pK\n", inode->i_private);
	file->private_data = inode->i_private;
	return 0;
}

static int publish_unreleased_reference(struct msm_vidc_inst *inst,
		char **dbuf, char *end)
{
	return 0;
}

static ssize_t inst_info_read(struct file *file, char __user *buf,
		size_t count, loff_t *ppos)
{
	struct core_inst_pair *idata = file->private_data;
	struct msm_vidc_core *core;
	struct msm_vidc_inst *inst;
	char *dbuf, *cur, *end;
	int i, j;
	ssize_t len = 0;
	struct v4l2_format *f;

	if (!idata || !idata->core || !idata->inst ||
		!idata->inst->capabilities) {
		d_vpr_e("%s: invalid params %pK\n", __func__, idata);
		return 0;
	}

	core = idata->core;
	inst = idata->inst;

	inst = get_inst(core, inst->session_id);
	if (!inst) {
		d_vpr_h("%s: instance has become obsolete", __func__);
		return 0;
	}

	dbuf = kzalloc(MAX_DBG_BUF_SIZE, GFP_KERNEL);
	if (!dbuf) {
		i_vpr_e(inst, "%s: Allocation failed!\n", __func__);
		len = -ENOMEM;
		goto failed_alloc;
	}
	cur = dbuf;
	end = cur + MAX_DBG_BUF_SIZE;

	f = &inst->fmts[OUTPUT_PORT];
	cur += write_str(cur, end - cur, "==============================\n");
	cur += write_str(cur, end - cur, "INSTANCE: %pK (%s)\n", inst,
		inst->domain == MSM_VIDC_ENCODER ? "Encoder" : "Decoder");
	cur += write_str(cur, end - cur, "==============================\n");
	cur += write_str(cur, end - cur, "core: %pK\n", inst->core);
	cur += write_str(cur, end - cur, "height: %d\n", f->fmt.pix_mp.height);
	cur += write_str(cur, end - cur, "width: %d\n", f->fmt.pix_mp.width);
	cur += write_str(cur, end - cur, "fps: %d\n",
			inst->capabilities->cap[FRAME_RATE].value >> 16);
	cur += write_str(cur, end - cur, "state: %d\n", inst->state);
	cur += write_str(cur, end - cur, "secure: %d\n",
		!!(inst->flags & VIDC_SECURE));
	cur += write_str(cur, end - cur, "-----------Formats-------------\n");
	for (i = 0; i < MAX_PORT; i++) {
		if (i != INPUT_PORT && i != OUTPUT_PORT)
			continue;
		f = &inst->fmts[i];
		cur += write_str(cur, end - cur, "capability: %s\n",
			i == INPUT_PORT ? "Output" : "Capture");
		cur += write_str(cur, end - cur, "planes : %d\n",
			f->fmt.pix_mp.num_planes);
		cur += write_str(cur, end - cur,
			"type: %s\n", i == INPUT_PORT ?
			"Output" : "Capture");
		cur += write_str(cur, end - cur, "count: %u\n",
				inst->vb2q[i].num_buffers);

		for (j = 0; j < f->fmt.pix_mp.num_planes; j++)
			cur += write_str(cur, end - cur,
				"size for plane %d: %u\n",
				j, f->fmt.pix_mp.plane_fmt[j].sizeimage);

		cur += write_str(cur, end - cur, "\n");
	}
	cur += write_str(cur, end - cur, "-------------------------------\n");
	cur += write_str(cur, end - cur, "ETB Count: %d\n",
		inst->debug_count.etb);
	cur += write_str(cur, end - cur, "EBD Count: %d\n",
		inst->debug_count.ebd);
	cur += write_str(cur, end - cur, "FTB Count: %d\n",
		inst->debug_count.ftb);
	cur += write_str(cur, end - cur, "FBD Count: %d\n",
		inst->debug_count.fbd);

	publish_unreleased_reference(inst, &cur, end);
	len = simple_read_from_buffer(buf, count, ppos,
		dbuf, cur - dbuf);

	kfree(dbuf);
failed_alloc:
	put_inst(inst);
	return len;
}

static int inst_info_release(struct inode *inode, struct file *file)
{
	d_vpr_l("Release inode ptr: %pK\n", inode->i_private);
	file->private_data = NULL;
	return 0;
}

static const struct file_operations inst_info_fops = {
	.open = inst_info_open,
	.read = inst_info_read,
	.release = inst_info_release,
};

struct dentry *msm_vidc_debugfs_init_inst(void *instance, struct dentry *parent)
{
	struct dentry *dir = NULL, *info = NULL;
	char debugfs_name[MAX_DEBUGFS_NAME];
	struct core_inst_pair *idata = NULL;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		goto exit;
	}
	snprintf(debugfs_name, MAX_DEBUGFS_NAME, "inst_%d", inst->sid);

	idata = kzalloc(sizeof(struct core_inst_pair), GFP_KERNEL);
	if (!idata) {
		i_vpr_e(inst, "%s: Allocation failed!\n", __func__);
		goto exit;
	}

	idata->core = inst->core;
	idata->inst = inst;

	dir = debugfs_create_dir(debugfs_name, parent);
	if (IS_ERR_OR_NULL(dir)) {
		dir = NULL;
		i_vpr_e(inst,
			"%s: Failed to create debugfs for msm_vidc\n",
			__func__);
		goto failed_create_dir;
	}

	info = debugfs_create_file("info", 0444, dir,
			idata, &inst_info_fops);
	if (IS_ERR_OR_NULL(info)) {
		i_vpr_e(inst, "%s: debugfs_create_file: fail\n",
			__func__);
		goto failed_create_file;
	}

	dir->d_inode->i_private = info->d_inode->i_private;
	inst->debug.pdata[FRAME_PROCESSING].sampling = true;
	return dir;

failed_create_file:
	debugfs_remove_recursive(dir);
	dir = NULL;
failed_create_dir:
	kfree(idata);
exit:
	return dir;
}

void msm_vidc_debugfs_deinit_inst(void *instance)
{
	struct dentry *dentry = NULL;
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;

	if (!inst || !inst->debugfs_root)
		return;

	dentry = inst->debugfs_root;
	if (dentry->d_inode) {
		i_vpr_l(inst, "%s: Destroy %pK\n",
			__func__, dentry->d_inode->i_private);
		kfree(dentry->d_inode->i_private);
		dentry->d_inode->i_private = NULL;
	}
	debugfs_remove_recursive(dentry);
	inst->debugfs_root = NULL;
}

void msm_vidc_debugfs_update(void *instance,
	enum msm_vidc_debugfs_event e)
{
	struct msm_vidc_inst *inst = (struct msm_vidc_inst *) instance;
	struct msm_vidc_debug *d;
	char a[64] = "Frame processing";

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return;
	}
	d = &inst->debug;

	switch (e) {
	case MSM_VIDC_DEBUGFS_EVENT_ETB:
		inst->debug_count.etb++;
		if (inst->debug_count.ebd &&
			inst->debug_count.ftb > inst->debug_count.fbd) {
			d->pdata[FRAME_PROCESSING].name[0] = '\0';
			tic(inst, FRAME_PROCESSING, a);
		}
		break;
	case MSM_VIDC_DEBUGFS_EVENT_EBD:
		inst->debug_count.ebd++;
		if (inst->debug_count.ebd &&
			inst->debug_count.ebd == inst->debug_count.etb) {
			toc(inst, FRAME_PROCESSING);
			i_vpr_p(inst, "EBD: FW needs input buffers\n");
		}
		if (inst->debug_count.ftb == inst->debug_count.fbd)
			i_vpr_p(inst, "EBD: FW needs output buffers\n");
		break;
	case MSM_VIDC_DEBUGFS_EVENT_FTB:
		inst->debug_count.ftb++;
		if (inst->debug_count.ebd &&
			inst->debug_count.etb > inst->debug_count.ebd) {
			d->pdata[FRAME_PROCESSING].name[0] = '\0';
			tic(inst, FRAME_PROCESSING, a);
		}
		break;
	case MSM_VIDC_DEBUGFS_EVENT_FBD:
		inst->debug_count.fbd++;
		inst->debug.samples++;
		if (inst->debug_count.fbd &&
			inst->debug_count.fbd == inst->debug_count.ftb) {
			toc(inst, FRAME_PROCESSING);
			i_vpr_p(inst, "FBD: FW needs output buffers\n");
		}
		if (inst->debug_count.etb == inst->debug_count.ebd)
			i_vpr_p(inst, "FBD: FW needs input buffers\n");
		break;
	default:
		i_vpr_e(inst, "invalid event in debugfs: %d\n", e);
		break;
	}
}

int msm_vidc_check_ratelimit(void)
{
	static DEFINE_RATELIMIT_STATE(_rs,
				VIDC_DBG_SESSION_RATELIMIT_INTERVAL,
				VIDC_DBG_SESSION_RATELIMIT_BURST);
	return __ratelimit(&_rs);
}
