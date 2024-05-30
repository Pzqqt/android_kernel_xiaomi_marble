// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 */
#define CREATE_TRACE_POINTS
#include "msm_vidc_debug.h"
#include "msm_vidc_driver.h"
#include "msm_vidc_dt.h"
#include "msm_vidc.h"
#include "msm_vidc_core.h"
#include "msm_vidc_inst.h"
#include "msm_vidc_internal.h"
#include "msm_vidc_events.h"

extern struct msm_vidc_core *g_core;

#define MAX_SSR_STRING_LEN         64
#define MAX_STABILITY_STRING_LEN   64
#define MAX_DEBUG_LEVEL_STRING_LEN 15
#define MSM_VIDC_MIN_STATS_DELAY_MS     200
#define MSM_VIDC_MAX_STATS_DELAY_MS     10000

unsigned int msm_vidc_debug = VIDC_ERR | VIDC_PRINTK | FW_ERROR | FW_FATAL | FW_PRINTK;

static int debug_level_set(const char *val,
	const struct kernel_param *kp)
{
	struct msm_vidc_core *core = NULL;
	unsigned int dvalue;
	int ret;

	if (!kp || !kp->arg || !val) {
		d_vpr_e("%s: Invalid params\n", __func__);
		return -EINVAL;
	}
	core = *(struct msm_vidc_core **)kp->arg;

	if (!core || !core->capabilities) {
		d_vpr_e("%s: Invalid core/capabilities\n", __func__);
		return -EINVAL;
	}

	ret = kstrtouint(val, 0, &dvalue);
	if (ret)
		return ret;

	msm_vidc_debug = dvalue;

	/* check only driver logmask */
	if ((dvalue & 0xFF) > (VIDC_ERR | VIDC_HIGH)) {
		core->capabilities[HW_RESPONSE_TIMEOUT].value = 2 * HW_RESPONSE_TIMEOUT_VALUE;
		core->capabilities[SW_PC_DELAY].value         = 2 * SW_PC_DELAY_VALUE;
		core->capabilities[FW_UNLOAD_DELAY].value     = 2 * FW_UNLOAD_DELAY_VALUE;
	} else {
		/* reset timeout values, if user reduces the logging */
		core->capabilities[HW_RESPONSE_TIMEOUT].value = HW_RESPONSE_TIMEOUT_VALUE;
		core->capabilities[SW_PC_DELAY].value         = SW_PC_DELAY_VALUE;
		core->capabilities[FW_UNLOAD_DELAY].value     = FW_UNLOAD_DELAY_VALUE;
	}

	d_vpr_h("timeout updated: hw_response %u, sw_pc %u, fw_unload %u, debug_level %#x\n",
		core->capabilities[HW_RESPONSE_TIMEOUT].value,
		core->capabilities[SW_PC_DELAY].value,
		core->capabilities[FW_UNLOAD_DELAY].value,
		msm_vidc_debug);

	return 0;
}

static int debug_level_get(char *buffer, const struct kernel_param *kp)
{
	return scnprintf(buffer, PAGE_SIZE, "%#x", msm_vidc_debug);
}

static const struct kernel_param_ops msm_vidc_debug_fops = {
	.set = debug_level_set,
	.get = debug_level_get,
};

module_param_cb(msm_vidc_debug, &msm_vidc_debug_fops, &g_core, 0644);

bool msm_vidc_lossless_encode = !true;
EXPORT_SYMBOL(msm_vidc_lossless_encode);

bool msm_vidc_syscache_disable = !true;
EXPORT_SYMBOL(msm_vidc_syscache_disable);

int msm_vidc_clock_voting = !1;
int msm_vidc_ddr_bw = !1;
int msm_vidc_llc_bw = !1;

bool msm_vidc_fw_dump = !true;
EXPORT_SYMBOL(msm_vidc_fw_dump);

unsigned int msm_vidc_enable_bugon = !1;
EXPORT_SYMBOL(msm_vidc_enable_bugon);

#define MAX_DBG_BUF_SIZE 4096

struct core_inst_pair {
	struct msm_vidc_core *core;
	struct msm_vidc_inst *inst;
};

/* debug fs support */
static inline void tic(struct msm_vidc_inst *i, enum profiling_points p,
				 char *b)
{
	if (!i->debug.pdata[p].name[0])
		memcpy(i->debug.pdata[p].name, b, 64);
	if (i->debug.pdata[p].sampling) {
		i->debug.pdata[p].start = ktime_get_ns() / 1000 / 1000;
		i->debug.pdata[p].sampling = false;
	}
}

static inline void toc(struct msm_vidc_inst *i, enum profiling_points p)
{
	if (!i->debug.pdata[p].sampling) {
		i->debug.pdata[p].stop = ktime_get_ns() / 1000 / 1000;
		i->debug.pdata[p].cumulative += i->debug.pdata[p].stop -
			i->debug.pdata[p].start;
		i->debug.pdata[p].sampling = true;
	}
}

void msm_vidc_show_stats(void *inst)
{
	int x;
	struct msm_vidc_inst *i = (struct msm_vidc_inst *) inst;

	if (!i) {
		d_vpr_e("%s: invalid params\n", __func__);
		return;
	}

	for (x = 0; x < MAX_PROFILING_POINTS; x++) {
		if (i->debug.pdata[x].name[0]) {
			if (i->debug.samples) {
				i_vpr_p(i, "%s averaged %d ms/sample\n",
						i->debug.pdata[x].name,
						i->debug.pdata[x].cumulative /
						i->debug.samples);
			}

			i_vpr_p(i, "%s Samples: %d\n",
				i->debug.pdata[x].name, i->debug.samples);
		}
	}
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
	char *cur, *end, *dbuf = NULL;
	ssize_t len = 0;
	int rc = 0;

	if (!core || !core->dt) {
		d_vpr_e("%s: invalid params %pK\n", __func__, core);
		return 0;
	}

	rc = msm_vidc_vmem_alloc(MAX_DBG_BUF_SIZE, (void **)&dbuf, __func__);
	if (rc)
		return rc;

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

	msm_vidc_vmem_free((void **)&dbuf);
	return len;
}

static const struct file_operations core_info_fops = {
	.open = simple_open,
	.read = core_info_read,
};

static ssize_t stats_delay_write_ms(struct file *filp, const char __user *buf,
		size_t count, loff_t *ppos)
{
	int rc = 0;
	struct msm_vidc_core *core = filp->private_data;
	char kbuf[MAX_DEBUG_LEVEL_STRING_LEN] = {0};
	u32 delay_ms = 0;

	/* filter partial writes and invalid commands */
	if (*ppos != 0 || count >= sizeof(kbuf) || count == 0) {
		d_vpr_e("returning error - pos %lld, count %lu\n", *ppos, count);
		rc = -EINVAL;
	}

	rc = simple_write_to_buffer(kbuf, sizeof(kbuf) - 1, ppos, buf, count);
	if (rc < 0) {
		d_vpr_e("%s: User memory fault\n", __func__);
		rc = -EFAULT;
		goto exit;
	}

	rc = kstrtoint(kbuf, 0, &delay_ms);
	if (rc) {
		d_vpr_e("returning error err %d\n", rc);
		rc = -EINVAL;
		goto exit;
	}
	delay_ms = clamp_t(u32, delay_ms, MSM_VIDC_MIN_STATS_DELAY_MS, MSM_VIDC_MAX_STATS_DELAY_MS);
	core->capabilities[STATS_TIMEOUT_MS].value = delay_ms;
	d_vpr_h("Stats delay is updated to - %d ms\n", delay_ms);

exit:
	return rc;
}

static ssize_t stats_delay_read_ms(struct file *file, char __user *buf,
		size_t count, loff_t *ppos)
{
	size_t len;
	char kbuf[MAX_DEBUG_LEVEL_STRING_LEN];
	struct msm_vidc_core *core = file->private_data;

	len = scnprintf(kbuf, sizeof(kbuf), "%u\n", core->capabilities[STATS_TIMEOUT_MS].value);
	return simple_read_from_buffer(buf, count, ppos, kbuf, len);
}

static const struct file_operations stats_delay_fops = {
	.open = simple_open,
	.write = stats_delay_write_ms,
	.read = stats_delay_read_ms,
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

static ssize_t trigger_stability_write(struct file *filp, const char __user *buf,
	size_t count, loff_t *ppos)
{
	unsigned long stability_trigger_val = 0;
	int rc = 0;
	struct msm_vidc_core *core = filp->private_data;
	size_t size = MAX_STABILITY_STRING_LEN;
	char kbuf[MAX_STABILITY_STRING_LEN + 1] = { 0 };

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

	rc = kstrtoul(kbuf, 0, &stability_trigger_val);
	if (rc) {
		d_vpr_e("%s: returning error err %d\n", __func__, rc);
		rc = -EINVAL;
	} else {
		msm_vidc_trigger_stability(core, stability_trigger_val);
		rc = count;
	}
exit:
	return rc;
}

static const struct file_operations stability_fops = {
	.open = simple_open,
	.write = trigger_stability_write,
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
	debugfs_create_u32("ddr_bw_kbps", 0644, dir,
			&msm_vidc_ddr_bw);
	debugfs_create_u32("llc_bw_kbps", 0644, dir,
			&msm_vidc_llc_bw);
	debugfs_create_bool("disable_video_syscache", 0644, dir,
			&msm_vidc_syscache_disable);
	debugfs_create_bool("lossless_encoding", 0644, dir,
			&msm_vidc_lossless_encode);
	debugfs_create_bool("msm_vidc_fw_dump", 0644, dir,
			&msm_vidc_fw_dump);
	debugfs_create_u32("enable_bugon", 0644, dir,
			&msm_vidc_enable_bugon);

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
	if (!debugfs_create_file("trigger_stability", 0200, dir, core, &stability_fops)) {
		d_vpr_e("trigger_stability debugfs_create_file: fail\n");
		goto failed_create_dir;
	}
	if (!debugfs_create_file("stats_delay_ms", 0644, dir, core, &stats_delay_fops)) {
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
	char *cur, *end, *dbuf = NULL;
	int i, j;
	ssize_t len = 0;
	struct v4l2_format *f;
	int rc = 0;

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

	rc = msm_vidc_vmem_alloc(MAX_DBG_BUF_SIZE, (void **)&dbuf, __func__);
	if (rc) {
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
		is_secure_session(inst));
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

	msm_vidc_vmem_free((void **)&dbuf);
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
	int rc = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		goto exit;
	}
	snprintf(debugfs_name, MAX_DEBUGFS_NAME, "inst_%d", inst->session_id);

	rc = msm_vidc_vmem_alloc(sizeof(struct core_inst_pair), (void **)&idata, __func__);
	if (rc)
		goto exit;

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
	msm_vidc_vmem_free((void **)&idata);
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
		msm_vidc_vmem_free(&dentry->d_inode->i_private);
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
		/*
		 * Host needs to ensure FW atleast have 2 buffers available always
		 * one for HW processing and another for fw processing in parallel
		 * to avoid FW starving for buffers
		 */
		if (inst->debug_count.etb < (inst->debug_count.ebd + 2)) {
			toc(inst, FRAME_PROCESSING);
			i_vpr_p(inst,
				"EBD: FW needs input buffers. Processed etb %llu ebd %llu ftb %llu fbd %llu\n",
				inst->debug_count.etb, inst->debug_count.ebd,
				inst->debug_count.ftb, inst->debug_count.fbd);
		}
		if (inst->debug_count.fbd &&
			inst->debug_count.ftb < (inst->debug_count.fbd + 2))
			i_vpr_p(inst,
				"EBD: FW needs output buffers. Processed etb %llu ebd %llu ftb %llu fbd %llu\n",
				inst->debug_count.etb, inst->debug_count.ebd,
				inst->debug_count.ftb, inst->debug_count.fbd);
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
		/*
		 * Host needs to ensure FW atleast have 2 buffers available always
		 * one for HW processing and another for fw processing in parallel
		 * to avoid FW starving for buffers
		 */
		if (inst->debug_count.ftb < (inst->debug_count.fbd + 2)) {
			toc(inst, FRAME_PROCESSING);
			i_vpr_p(inst,
				"FBD: FW needs output buffers. Processed etb %llu ebd %llu ftb %llu fbd %llu\n",
				inst->debug_count.etb, inst->debug_count.ebd,
				inst->debug_count.ftb, inst->debug_count.fbd);
		}
		if (inst->debug_count.ebd &&
			inst->debug_count.etb < (inst->debug_count.ebd + 2))
			i_vpr_p(inst,
				"FBD: FW needs input buffers. Processed etb %llu ebd %llu ftb %llu fbd %llu\n",
				inst->debug_count.etb, inst->debug_count.ebd,
				inst->debug_count.ftb, inst->debug_count.fbd);
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
