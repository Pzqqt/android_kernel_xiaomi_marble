// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#include <linux/clk.h>
#include <linux/regulator/consumer.h>
#include <linux/clk-provider.h>
#include <linux/iommu.h>
#include <linux/qcom_scm.h>
#include <linux/soc/qcom/smem.h>
#include <linux/irqreturn.h>
#include <linux/reset.h>
#include <linux/interconnect.h>
#include <soc/qcom/subsystem_restart.h>

#include "venus_hfi.h"
#include "msm_vidc_core.h"
#include "msm_vidc_bus.h"
#include "msm_vidc_dt.h"
#include "msm_vidc_platform.h"
#include "msm_vidc_memory.h"
#include "msm_vidc_driver.h"
#include "msm_vidc_debug.h"
#include "hfi_packet.h"
#include "venus_hfi_response.h"

#define MIN_PAYLOAD_SIZE 3

static int __resume(struct msm_vidc_core *core);
static int __suspend(struct msm_vidc_core *core);

struct tzbsp_memprot {
	u32 cp_start;
	u32 cp_size;
	u32 cp_nonpixel_start;
	u32 cp_nonpixel_size;
};

enum tzbsp_video_state {
	TZBSP_VIDEO_STATE_SUSPEND = 0,
	TZBSP_VIDEO_STATE_RESUME = 1,
	TZBSP_VIDEO_STATE_RESTORE_THRESHOLD = 2,
};

enum reset_state {
	INIT = 1,
	ASSERT,
	DEASSERT,
};

/* Less than 50MBps is treated as trivial BW change */
#define TRIVIAL_BW_THRESHOLD 50000
#define TRIVIAL_BW_CHANGE(a, b) \
	((a) > (b) ? (a) - (b) < TRIVIAL_BW_THRESHOLD : \
		(b) - (a) < TRIVIAL_BW_THRESHOLD)

/**
 * Utility function to enforce some of our assumptions.  Spam calls to this
 * in hotspots in code to double check some of the assumptions that we hold.
 */
struct lut const *__lut(int width, int height, int fps)
{
	int frame_size = height * width, c = 0;

	do {
		if (LUT[c].frame_size >= frame_size && LUT[c].frame_rate >= fps)
			return &LUT[c];
	} while (++c < ARRAY_SIZE(LUT));

	return &LUT[ARRAY_SIZE(LUT) - 1];
}

fp_t __compression_ratio(struct lut const *entry, int bpp)
{
	int c = 0;

	for (c = 0; c < COMPRESSION_RATIO_MAX; ++c) {
		if (entry->compression_ratio[c].bpp == bpp)
			return entry->compression_ratio[c].ratio;
	}

	WARN(true, "Shouldn't be here, LUT possibly corrupted?\n");
	return FP_ZERO; /* impossible */
}


void __dump(struct dump dump[], int len)
{
	int c = 0;

	for (c = 0; c < len; ++c) {
		char format_line[128] = "", formatted_line[128] = "";

		if (dump[c].val == DUMP_HEADER_MAGIC) {
			snprintf(formatted_line, sizeof(formatted_line), "%s\n",
					 dump[c].key);
		} else {
			bool fp_format = !strcmp(dump[c].format, DUMP_FP_FMT);

			if (!fp_format) {
				snprintf(format_line, sizeof(format_line),
						 "    %-35s: %s\n", dump[c].key,
						 dump[c].format);
				snprintf(formatted_line, sizeof(formatted_line),
						 format_line, dump[c].val);
			} else {
				size_t integer_part, fractional_part;

				integer_part = fp_int(dump[c].val);
				fractional_part = fp_frac(dump[c].val);
				snprintf(formatted_line, sizeof(formatted_line),
						 "    %-35s: %zd + %zd/%zd\n",
						 dump[c].key, integer_part,
						 fractional_part,
						 fp_frac_base());


			}
		}
		d_vpr_b("%s", formatted_line);
	}
}

static void __dump_packet(u8 *packet)
{
	u32 c = 0, packet_size = *(u32 *)packet;
	const int row_size = 32;
	/*
	 * row must contain enough for 0xdeadbaad * 8 to be converted into
	 * "de ad ba ab " * 8 + '\0'
	 */
	char row[3 * 32];

	for (c = 0; c * row_size < packet_size; ++c) {
		int bytes_to_read = ((c + 1) * row_size > packet_size) ?
			packet_size % row_size : row_size;
		hex_dump_to_buffer(packet + c * row_size, bytes_to_read,
				row_size, 4, row, sizeof(row), false);
		d_vpr_t("%s\n", row);
	}
}

static void __fatal_error(struct msm_vidc_core *core, bool fatal)
{
	return;
	fatal &= core->capabilities[HW_RESPONSE_TIMEOUT].value;
	MSM_VIDC_ERROR(fatal);
}

static void __strict_check(struct msm_vidc_core *core)
{
	__fatal_error(core, !mutex_is_locked(&core->lock));
}

bool __core_in_valid_state(struct msm_vidc_core *core)
{
	return core->state != MSM_VIDC_CORE_ERROR;
}

static bool is_sys_cache_present(struct msm_vidc_core *core)
{
	return core->dt->sys_cache_present;
}

void __write_register(struct msm_vidc_core *core,
		u32 reg, u32 value)
{
	u32 hwiosymaddr = reg;
	u8 *base_addr;

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return;
	}

	__strict_check(core);

	if (!core->power_enabled) {
		d_vpr_e("HFI Write register failed : Power is OFF\n");
		__fatal_error(core, true);
		return;
	}

	base_addr = core->register_base_addr;
	d_vpr_l("Base addr: %pK, writing to: %#x, Value: %#x...\n",
		base_addr, hwiosymaddr, value);
	base_addr += hwiosymaddr;
	writel_relaxed(value, base_addr);

	/*
	 * Memory barrier to make sure value is written into the register.
	 */
	wmb();
}

/*
 * Argument mask is used to specify which bits to update. In case mask is 0x11,
 * only bits 0 & 4 will be updated with corresponding bits from value. To update
 * entire register with value, set mask = 0xFFFFFFFF.
 */
void __write_register_masked(struct msm_vidc_core *core,
		u32 reg, u32 value, u32 mask)
{
	u32 prev_val, new_val;
	u8 *base_addr;

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return;
	}

	__strict_check(core);

	if (!core->power_enabled) {
		d_vpr_e("%s: register write failed, power is off\n",
			__func__);
		__fatal_error(core, true);
		return;
	}

	base_addr = core->register_base_addr;
	base_addr += reg;

	prev_val = readl_relaxed(base_addr);
	/*
	 * Memory barrier to ensure register read is correct
	 */
	rmb();

	new_val = (prev_val & ~mask) | (value & mask);
	d_vpr_l(
		"Base addr: %pK, writing to: %#x, previous-value: %#x, value: %#x, mask: %#x, new-value: %#x...\n",
		base_addr, reg, prev_val, value, mask, new_val);
	writel_relaxed(new_val, base_addr);
	/*
	 * Memory barrier to make sure value is written into the register.
	 */
	wmb();
}

int __read_register(struct msm_vidc_core *core, u32 reg)
{
	int rc = 0;
	u8 *base_addr;

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	__strict_check(core);

	if (!core->power_enabled) {
		d_vpr_e("HFI Read register failed : Power is OFF\n");
		__fatal_error(core, true);
		return -EINVAL;
	}

	base_addr = core->register_base_addr;

	rc = readl_relaxed(base_addr + reg);
	/*
	 * Memory barrier to make sure value is read correctly from the
	 * register.
	 */
	rmb();
	d_vpr_l("Base addr: %pK, read from: %#x, value: %#x...\n",
		base_addr, reg, rc);

	return rc;
}

static void __schedule_power_collapse_work(struct msm_vidc_core *core)
{
	if (!core || !core->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return;
	}
	if (!core->capabilities[SW_PC].value) {
		d_vpr_l("software power collapse not enabled\n");
		return;
	}

	cancel_delayed_work(&core->pm_work);
	if (!queue_delayed_work(core->pm_workq,
			&core->pm_work, msecs_to_jiffies(
			core->capabilities[SW_PC_DELAY].value))) {
		d_vpr_e("power collapse already scheduled\n");
	} else {
		d_vpr_l("power collapse scheduled for %d ms\n",
			core->capabilities[SW_PC_DELAY].value);
	}
}

static void __cancel_power_collapse_work(struct msm_vidc_core *core)
{
	if (!core || !core->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return;
	}
	if (!core->capabilities[SW_PC].value)
		return;

	cancel_delayed_work(&core->pm_work);
}

static int __acquire_regulator(struct msm_vidc_core *core,
	struct regulator_info *rinfo)
{
	int rc = 0;

	if (rinfo->has_hw_power_collapse) {
		if (regulator_get_mode(rinfo->regulator) ==
				REGULATOR_MODE_NORMAL) {
			d_vpr_h("Skip acquire regulator %s\n", rinfo->name);
			goto exit;
		}

		rc = regulator_set_mode(rinfo->regulator,
				REGULATOR_MODE_NORMAL);
		if (rc) {
			/*
			 * This is somewhat fatal, but nothing we can do
			 * about it. We can't disable the regulator w/o
			 * getting it back under s/w control
			 */
			d_vpr_e("Failed to acquire regulator control: %s\n",
				rinfo->name);
			goto exit;
		} else {

			d_vpr_h("Acquired regulator control from HW: %s\n",
					rinfo->name);

		}

		if (!regulator_is_enabled(rinfo->regulator)) {
			d_vpr_e("%s: Regulator is not enabled %s\n",
				__func__, rinfo->name);
			__fatal_error(core, true);
		}
	}

exit:
	return rc;
}

static int __hand_off_regulator(struct msm_vidc_core *core,
	struct regulator_info *rinfo)
{
	int rc = 0;

	if (rinfo->has_hw_power_collapse) {
		rc = regulator_set_mode(rinfo->regulator,
				REGULATOR_MODE_FAST);
		if (rc) {
			d_vpr_e("Failed to hand off regulator control: %s\n",
				rinfo->name);
			return rc;
		} else {
			d_vpr_h("Hand off regulator control to HW: %s\n",
					rinfo->name);
		}

		if (!regulator_is_enabled(rinfo->regulator)) {
			d_vpr_e("%s: Regulator is not enabled %s\n",
				__func__, rinfo->name);
			__fatal_error(core, true);
		}
	}

	return rc;
}

static int __hand_off_regulators(struct msm_vidc_core *core)
{
	struct regulator_info *rinfo;
	int rc = 0, c = 0;

	venus_hfi_for_each_regulator(core, rinfo) {
		rc = __hand_off_regulator(core, rinfo);
		/*
		 * If one regulator hand off failed, driver should take
		 * the control for other regulators back.
		 */
		if (rc)
			goto err_reg_handoff_failed;
		c++;
	}

	return rc;
err_reg_handoff_failed:
	venus_hfi_for_each_regulator_reverse_continue(core, rinfo, c)
		__acquire_regulator(core, rinfo);

	return rc;
}

static void __set_registers(struct msm_vidc_core *core)
{
	struct reg_set *reg_set;
	int i;

	if (!core || !core->dt) {
		d_vpr_e("core resources null, cannot set registers\n");
		return;
	}

	reg_set = &core->dt->reg_set;
	for (i = 0; i < reg_set->count; i++) {
		__write_register_masked(core, reg_set->reg_tbl[i].reg,
				reg_set->reg_tbl[i].value,
				reg_set->reg_tbl[i].mask);
	}
}

static int __vote_bandwidth(struct bus_info *bus,
	unsigned long bw_kbps)
{
	int rc = 0;

	d_vpr_p("Voting bus %s to ab %llu kbps\n", bus->name, bw_kbps);
	rc = icc_set_bw(bus->path, kbps_to_icc(bw_kbps), 0);
	if (rc)
		d_vpr_e("Failed voting bus %s to ab %llu, rc=%d\n",
				bus->name, bw_kbps, rc);

	return rc;
}

int __unvote_buses(struct msm_vidc_core *core)
{
	int rc = 0;
	struct bus_info *bus = NULL;

	core->power.bw_ddr = 0;
	core->power.bw_llcc = 0;

	venus_hfi_for_each_bus(core, bus) {
		rc = __vote_bandwidth(bus, 0);
		if (rc)
			goto err_unknown_device;
	}

err_unknown_device:
	return rc;
}

static int __vote_buses(struct msm_vidc_core *core,
		unsigned long bw_ddr, unsigned long bw_llcc)
{
	int rc = 0;
	struct bus_info *bus = NULL;
	unsigned long bw_kbps = 0, bw_prev = 0;
	enum vidc_bus_type type;

	venus_hfi_for_each_bus(core, bus) {
		if (bus && bus->path) {
			type = get_type_frm_name(bus->name);

			if (type == DDR) {
				bw_kbps = bw_ddr;
				bw_prev = core->power.bw_ddr;
			} else if (type == LLCC) {
				bw_kbps = bw_llcc;
				bw_prev = core->power.bw_llcc;
			} else {
				bw_kbps = bus->range[1];
				bw_prev = core->power.bw_ddr ?
						bw_kbps : 0;
			}

			/* ensure freq is within limits */
			bw_kbps = clamp_t(typeof(bw_kbps), bw_kbps,
				bus->range[0], bus->range[1]);

			if (TRIVIAL_BW_CHANGE(bw_kbps, bw_prev) && bw_prev) {
				d_vpr_l("Skip voting bus %s to %llu bps",
					bus->name, bw_kbps * 1000);
				continue;
			}

			rc = __vote_bandwidth(bus, bw_kbps);

			if (type == DDR)
				core->power.bw_ddr = bw_kbps;
			else if (type == LLCC)
				core->power.bw_llcc = bw_kbps;
		} else {
			d_vpr_e("No BUS to Vote\n");
		}
	}

	return rc;
}

static int __tzbsp_set_video_state(enum tzbsp_video_state state)
{
	int tzbsp_rsp = qcom_scm_set_remote_state(state, 0);

	d_vpr_l("Set state %d, resp %d\n", state, tzbsp_rsp);
	if (tzbsp_rsp) {
		d_vpr_e("Failed to set video core state to suspend: %d\n",
			tzbsp_rsp);
		return -EINVAL;
	}

	return 0;
}

static int __set_clk_rate(struct msm_vidc_core *core,
		struct clock_info *cl, u64 rate)
{
	int rc = 0;
	struct clk *clk = cl->clk;

	/* bail early if requested clk rate is not changed */
	if (rate == cl->prev)
		return 0;

	rc = clk_set_rate(clk, rate);
	if (rc) {
		d_vpr_e("%s: Failed to set clock rate %llu %s: %d\n",
			__func__, rate, cl->name, rc);
		return rc;
	}
	cl->prev = rate;
	return rc;
}

static int __set_clocks(struct msm_vidc_core *core, u32 freq)
{
	int rc = 0;
	struct clock_info *cl;

	venus_hfi_for_each_clock(core, cl) {
		if (cl->has_scaling) {/* has_scaling */
			d_vpr_h("Scaling clock %s to %u, prev %llu\n",
				cl->name, freq, cl->prev);
			rc = __set_clk_rate(core, cl, freq);
			if (rc)
				return rc;
		}
	}

	return 0;
}

static int __scale_clocks(struct msm_vidc_core *core)
{
	int rc = 0;
	struct allowed_clock_rates_table *allowed_clks_tbl;
	u32 freq = 0;

	allowed_clks_tbl = core->dt->allowed_clks_tbl;
	freq = core->power.clk_freq ? core->power.clk_freq :
		allowed_clks_tbl[0].clock_rate;

	rc = __set_clocks(core, freq);
	if (rc)
		return rc;

	core->power.clk_freq = freq;
	return 0;
}

static int __write_queue(struct msm_vidc_iface_q_info *qinfo, u8 *packet,
		bool *rx_req_is_set)
{
	struct hfi_queue_header *queue;
	u32 packet_size_in_words, new_write_idx;
	u32 empty_space, read_idx, write_idx;
	u32 *write_ptr;

	if (!qinfo || !packet) {
		d_vpr_e("%s: invalid params %pK %pK\n",
			__func__, qinfo, packet);
		return -EINVAL;
	} else if (!qinfo->q_array.align_virtual_addr) {
		d_vpr_e("Queues have already been freed\n");
		return -EINVAL;
	}

	queue = (struct hfi_queue_header *) qinfo->q_hdr;
	if (!queue) {
		d_vpr_e("queue not present\n");
		return -ENOENT;
	}

	if (msm_vidc_debug & VIDC_PKT) {
		d_vpr_t("%s: %pK\n", __func__, qinfo);
		__dump_packet(packet);
	}

	// TODO: handle writing packet
	//d_vpr_e("skip writing packet\n");
	//return 0;

	packet_size_in_words = (*(u32 *)packet) >> 2;
	if (!packet_size_in_words || packet_size_in_words >
		qinfo->q_array.mem_size>>2) {
		d_vpr_e("Invalid packet size\n");
		return -ENODATA;
	}

	read_idx = queue->qhdr_read_idx;
	write_idx = queue->qhdr_write_idx;

	empty_space = (write_idx >=  read_idx) ?
		((qinfo->q_array.mem_size>>2) - (write_idx -  read_idx)) :
		(read_idx - write_idx);
	if (empty_space <= packet_size_in_words) {
		queue->qhdr_tx_req =  1;
		d_vpr_e("Insufficient size (%d) to write (%d)\n",
					  empty_space, packet_size_in_words);
		return -ENOTEMPTY;
	}

	queue->qhdr_tx_req =  0;

	new_write_idx = write_idx + packet_size_in_words;
	write_ptr = (u32 *)((qinfo->q_array.align_virtual_addr) +
			(write_idx << 2));
	if (write_ptr < (u32 *)qinfo->q_array.align_virtual_addr ||
	    write_ptr > (u32 *)(qinfo->q_array.align_virtual_addr +
	    qinfo->q_array.mem_size)) {
		d_vpr_e("Invalid write index");
		return -ENODATA;
	}

	if (new_write_idx < (qinfo->q_array.mem_size >> 2)) {
		memcpy(write_ptr, packet, packet_size_in_words << 2);
	} else {
		new_write_idx -= qinfo->q_array.mem_size >> 2;
		memcpy(write_ptr, packet, (packet_size_in_words -
			new_write_idx) << 2);
		memcpy((void *)qinfo->q_array.align_virtual_addr,
			packet + ((packet_size_in_words - new_write_idx) << 2),
			new_write_idx  << 2);
	}

	/*
	 * Memory barrier to make sure packet is written before updating the
	 * write index
	 */
	mb();
	queue->qhdr_write_idx = new_write_idx;
	if (rx_req_is_set)
		*rx_req_is_set = queue->qhdr_rx_req == 1;
	/*
	 * Memory barrier to make sure write index is updated before an
	 * interrupt is raised on venus.
	 */
	mb();
	return 0;
}

static int __read_queue(struct msm_vidc_iface_q_info *qinfo, u8 *packet,
		u32 *pb_tx_req_is_set)
{
	struct hfi_queue_header *queue;
	u32 packet_size_in_words, new_read_idx;
	u32 *read_ptr;
	u32 receive_request = 0;
	u32 read_idx, write_idx;
	int rc = 0;
	u32 sid;

	if (!qinfo || !packet || !pb_tx_req_is_set) {
		d_vpr_e("%s: invalid params %pK %pK %pK\n",
			__func__, qinfo, packet, pb_tx_req_is_set);
		return -EINVAL;
	} else if (!qinfo->q_array.align_virtual_addr) {
		d_vpr_e("Queues have already been freed\n");
		return -EINVAL;
	}

	/*
	 * Memory barrier to make sure data is valid before
	 *reading it
	 */
	mb();
	queue = (struct hfi_queue_header *) qinfo->q_hdr;

	if (!queue) {
		d_vpr_e("Queue memory is not allocated\n");
		return -ENOMEM;
	}

	/*
	 * Do not set receive request for debug queue, if set,
	 * Venus generates interrupt for debug messages even
	 * when there is no response message available.
	 * In general debug queue will not become full as it
	 * is being emptied out for every interrupt from Venus.
	 * Venus will anyway generates interrupt if it is full.
	 */
	if (queue->qhdr_type & HFI_Q_ID_CTRL_TO_HOST_MSG_Q)
		receive_request = 1;

	read_idx = queue->qhdr_read_idx;
	write_idx = queue->qhdr_write_idx;

	if (read_idx == write_idx) {
		queue->qhdr_rx_req = receive_request;
		/*
		 * mb() to ensure qhdr is updated in main memory
		 * so that venus reads the updated header values
		 */
		mb();
		*pb_tx_req_is_set = 0;
		d_vpr_l(
			"%s queue is empty, rx_req = %u, tx_req = %u, read_idx = %u\n",
			receive_request ? "message" : "debug",
			queue->qhdr_rx_req, queue->qhdr_tx_req,
			queue->qhdr_read_idx);
		return -ENODATA;
	}

	read_ptr = (u32 *)((qinfo->q_array.align_virtual_addr) +
				(read_idx << 2));
	if (read_ptr < (u32 *)qinfo->q_array.align_virtual_addr ||
	    read_ptr > (u32 *)(qinfo->q_array.align_virtual_addr +
	    qinfo->q_array.mem_size - sizeof(*read_ptr))) {
		d_vpr_e("Invalid read index\n");
		return -ENODATA;
	}

	packet_size_in_words = (*read_ptr) >> 2;
	if (!packet_size_in_words) {
		d_vpr_e("Zero packet size\n");
		return -ENODATA;
	}

	new_read_idx = read_idx + packet_size_in_words;
	if (((packet_size_in_words << 2) <= VIDC_IFACEQ_VAR_HUGE_PKT_SIZE) &&
		read_idx <= (qinfo->q_array.mem_size >> 2)) {
		if (new_read_idx < (qinfo->q_array.mem_size >> 2)) {
			memcpy(packet, read_ptr,
					packet_size_in_words << 2);
		} else {
			new_read_idx -= (qinfo->q_array.mem_size >> 2);
			memcpy(packet, read_ptr,
			(packet_size_in_words - new_read_idx) << 2);
			memcpy(packet + ((packet_size_in_words -
					new_read_idx) << 2),
					(u8 *)qinfo->q_array.align_virtual_addr,
					new_read_idx << 2);
		}
	} else {
		d_vpr_e("BAD packet received, read_idx: %#x, pkt_size: %d\n",
			read_idx, packet_size_in_words << 2);
		d_vpr_e("Dropping this packet\n");
		new_read_idx = write_idx;
		rc = -ENODATA;
	}

	if (new_read_idx != write_idx)
		queue->qhdr_rx_req = 0;
	else
		queue->qhdr_rx_req = receive_request;

	queue->qhdr_read_idx = new_read_idx;
	/*
	 * mb() to ensure qhdr is updated in main memory
	 * so that venus reads the updated header values
	 */
	mb();

	*pb_tx_req_is_set = (queue->qhdr_tx_req == 1) ? 1 : 0;

	if ((msm_vidc_debug & VIDC_PKT) &&
		!(queue->qhdr_type & HFI_Q_ID_CTRL_TO_HOST_DEBUG_Q)) {
		sid = *((u32 *)packet + 2);
		d_vpr_t("%s: %pK\n", __func__, qinfo);
		__dump_packet(packet);
	}

	return rc;
}

/* Writes into cmdq without raising an interrupt */
static int __iface_cmdq_write_relaxed(struct msm_vidc_core *core,
		void *pkt, bool *requires_interrupt)
{
	struct msm_vidc_iface_q_info *q_info;
	//struct vidc_hal_cmd_pkt_hdr *cmd_packet;
	int result = -E2BIG;

	if (!core || !pkt) {
		d_vpr_e("%s: invalid params %pK %pK\n",
			__func__, core, pkt);
		return -EINVAL;
	}

	__strict_check(core);

	if (!__core_in_valid_state(core)) {
		d_vpr_e("%s: fw not in init state\n", __func__);
		result = -EINVAL;
		goto err_q_null;
	}

	//cmd_packet = (struct vidc_hal_cmd_pkt_hdr *)pkt;
	//core->last_packet_type = cmd_packet->packet_type;

	q_info = &core->iface_queues[VIDC_IFACEQ_CMDQ_IDX];
	if (!q_info) {
		d_vpr_e("cannot write to shared Q's\n");
		goto err_q_null;
	}

	if (!q_info->q_array.align_virtual_addr) {
		d_vpr_e("cannot write to shared CMD Q's\n");
		result = -ENODATA;
		goto err_q_null;
	}

	if (__resume(core)) {
		d_vpr_e("%s: Power on failed\n", __func__);
		goto err_q_write;
	}

	if (!__write_queue(q_info, (u8 *)pkt, requires_interrupt)) {
		__schedule_power_collapse_work(core);
		result = 0;
	} else {
		d_vpr_e("__iface_cmdq_write: queue full\n");
	}

err_q_write:
err_q_null:
	return result;
}

int __iface_cmdq_write(struct msm_vidc_core *core,
	void *pkt)
{
	bool needs_interrupt = false;
	int rc = __iface_cmdq_write_relaxed(core, pkt, &needs_interrupt);

	if (!rc && needs_interrupt)
		call_venus_op(core, raise_interrupt, core);

	return rc;
}

int __iface_msgq_read(struct msm_vidc_core *core, void *pkt)
{
	u32 tx_req_is_set = 0;
	int rc = 0;
	struct msm_vidc_iface_q_info *q_info;

	if (!pkt) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	__strict_check(core);

	if (!__core_in_valid_state(core)) {
		d_vpr_e("%s: fw not in init state\n", __func__);
		rc = -EINVAL;
		goto read_error_null;
	}

	q_info = &core->iface_queues[VIDC_IFACEQ_MSGQ_IDX];
	if (!q_info->q_array.align_virtual_addr) {
		d_vpr_e("cannot read from shared MSG Q's\n");
		rc = -ENODATA;
		goto read_error_null;
	}

	if (!__read_queue(q_info, (u8 *)pkt, &tx_req_is_set)) {
		if (tx_req_is_set)
			call_venus_op(core, raise_interrupt, core);
		rc = 0;
	} else {
		rc = -ENODATA;
	}

read_error_null:
	return rc;
}

int __iface_dbgq_read(struct msm_vidc_core *core, void *pkt)
{
	u32 tx_req_is_set = 0;
	int rc = 0;
	struct msm_vidc_iface_q_info *q_info;

	if (!pkt) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	__strict_check(core);

	q_info = &core->iface_queues[VIDC_IFACEQ_DBGQ_IDX];
	if (!q_info->q_array.align_virtual_addr) {
		d_vpr_e("cannot read from shared DBG Q's\n");
		rc = -ENODATA;
		goto dbg_error_null;
	}

	if (!__read_queue(q_info, (u8 *)pkt, &tx_req_is_set)) {
		if (tx_req_is_set)
			call_venus_op(core, raise_interrupt, core);
		rc = 0;
	} else
		rc = -ENODATA;

dbg_error_null:
	return rc;
}

// TODO: revisit once firmware updated to latest interface headers
struct hfi_packet_header {
	u32 size;
	u32 packet_type;
};

struct hfi_msg_sys_debug_packet {
	u32 size;
	u32 packet_type;
	u32 msg_type;
	u32 msg_size;
	u32 time_stamp_hi;
	u32 time_stamp_lo;
	u8 rg_msg_data[1];
};

static void __flush_debug_queue(struct msm_vidc_core *core, u8 *packet)
{
	bool local_packet = false;
	enum vidc_msg_prio log_level = msm_vidc_debug;

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return;
	}

	if (!packet) {
		packet = kzalloc(VIDC_IFACEQ_VAR_HUGE_PKT_SIZE, GFP_KERNEL);
		if (!packet) {
			d_vpr_e("%s: fail to allocate\n", __func__);
			return;
		}

		local_packet = true;

		/*
		 * Local packet is used when error occurred.
		 * It is good to print these logs to printk as well.
		 */
		log_level |= FW_PRINTK;
	}

#define SKIP_INVALID_PKT(pkt_size, payload_size, pkt_hdr_size) ({ \
		if (pkt_size < pkt_hdr_size || \
			payload_size < MIN_PAYLOAD_SIZE || \
			payload_size > \
			(pkt_size - pkt_hdr_size + sizeof(u8))) { \
			d_vpr_e("%s: invalid msg size - %d\n", \
				__func__, payload_size); \
			continue; \
		} \
	})

	while (!__iface_dbgq_read(core, packet)) {
		struct hfi_packet_header *pkt =
			(struct hfi_packet_header *) packet;

		if (pkt->size < sizeof(struct hfi_packet_header)) {
			d_vpr_e("Invalid pkt size - %s\n", __func__);
			continue;
		}

		if (1) {
			struct hfi_msg_sys_debug_packet *pkt =
				(struct hfi_msg_sys_debug_packet *) packet;

			SKIP_INVALID_PKT(pkt->size,
				pkt->msg_size, sizeof(*pkt));

			/*
			 * All fw messages starts with new line character. This
			 * causes dprintk to print this message in two lines
			 * in the kernel log. Ignoring the first character
			 * from the message fixes this to print it in a single
			 * line.
			 */
			pkt->rg_msg_data[pkt->msg_size-1] = '\0';
			dprintk_firmware(log_level, "%s", &pkt->rg_msg_data[1]);
		}
	}
#undef SKIP_INVALID_PKT

	if (local_packet)
		kfree(packet);
}

static int __sys_set_debug(struct msm_vidc_core *core, u32 debug)
{
	int rc = 0;

	rc = hfi_packet_sys_debug_config(core, core->packet,
			core->packet_size, debug);
	if (rc) {
		d_vpr_e("Debug mode setting to FW failed\n");
		return -ENOTEMPTY;
	}

	if (__iface_cmdq_write(core, core->packet))
		return -ENOTEMPTY;
	return 0;
}
/*
static int __sys_set_coverage(struct msm_vidc_core *core,
		u32 mode)
{
	int rc = 0;

	//rc = call_hfi_pkt_op(core, sys_coverage_config,	pkt, mode);
	if (rc) {
		d_vpr_e("Coverage mode setting to FW failed\n");
		return -ENOTEMPTY;
	}

	//if (__iface_cmdq_write(core, pkt, sid)) {
	//	d_vpr_e("Failed to send coverage pkt to f/w\n");
	//	return -ENOTEMPTY;
	//}

	return 0;
}

static int __sys_set_power_control(struct msm_vidc_core *core, bool enable)
{
	struct regulator_info *rinfo;
	bool supported = false;

	venus_hfi_for_each_regulator(core, rinfo) {
		if (rinfo->has_hw_power_collapse) {
			supported = true;
			break;
		}
	}

	if (!supported)
		return 0;

	//call_hfi_pkt_op(core, sys_power_control, pkt, enable);
	//if (__iface_cmdq_write(core, pkt, sid))
	//	return -ENOTEMPTY;
	return 0;
}
*/

int __prepare_pc(struct msm_vidc_core *core)
{
	int rc = 0;

	rc = hfi_packet_sys_pc_prep(core, core->packet, core->packet_size);
	if (rc) {
		d_vpr_e("Failed to create sys pc prep pkt\n");
		goto err_pc_prep;
	}

	if (__iface_cmdq_write(core, core->packet))
		rc = -ENOTEMPTY;
	if (rc)
		d_vpr_e("Failed to prepare venus for power off");
err_pc_prep:
	return rc;
}

static int __power_collapse(struct msm_vidc_core *core, bool force)
{
	int rc = 0;

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	if (!core->power_enabled) {
		d_vpr_h("%s: Power already disabled\n", __func__);
		goto exit;
	}

	if (!__core_in_valid_state(core)) {
		d_vpr_e("%s: Core not in init state\n", __func__);
		return -EINVAL;
	}

	rc = call_venus_op(core, prepare_pc, core);
	if (rc)
		goto skip_power_off;

	__flush_debug_queue(core, core->packet);

	rc = __suspend(core);
	if (rc)
		d_vpr_e("Failed __suspend\n");

exit:
	return rc;

skip_power_off:
	d_vpr_e("%s: skipped\n", __func__);
	return -EAGAIN;
}

static int __protect_cp_mem(struct msm_vidc_core *core)
{
	struct tzbsp_memprot memprot;
	int rc = 0;
	struct context_bank_info *cb;

	if (!core)
		return -EINVAL;

	memprot.cp_start = 0x0;
	memprot.cp_size = 0x0;
	memprot.cp_nonpixel_start = 0x0;
	memprot.cp_nonpixel_size = 0x0;

	list_for_each_entry(cb, &core->dt->context_banks, list) {
		if (!strcmp(cb->name, "venus_ns")) {
			memprot.cp_size = cb->addr_range.start;

			d_vpr_h("%s: memprot.cp_size: %#x\n",
				__func__, memprot.cp_size);
		}

		if (!strcmp(cb->name, "venus_sec_non_pixel")) {
			memprot.cp_nonpixel_start = cb->addr_range.start;
			memprot.cp_nonpixel_size = cb->addr_range.size;

			d_vpr_h("%s: cp_nonpixel_start: %#x size: %#x\n",
				__func__, memprot.cp_nonpixel_start,
				memprot.cp_nonpixel_size);
		}
	}

	rc = qcom_scm_mem_protect_video(memprot.cp_start, memprot.cp_size,
			memprot.cp_nonpixel_start, memprot.cp_nonpixel_size);

	if (rc)
		d_vpr_e("Failed to protect memory(%d)\n", rc);

	return rc;
}
#if 0 // TODO
static int __core_set_resource(struct msm_vidc_core *core,
		struct vidc_resource_hdr *resource_hdr, void *resource_value)
{
	int rc = 0;

	if (!core || !resource_hdr || !resource_value) {
		d_vpr_e("%s: invalid params %pK %pK %pK\n", __func__,
			core, resource_hdr, resource_value);
		return -EINVAL;
	}

	//rc = hfi_packet_sys_set_resource(core, core->packet, core->packet_size,
	//		resource_hdr, resource_value);
	if (rc) {
		d_vpr_e("set_res: failed to create packet\n");
		goto err_create_pkt;
	}

	//rc = __iface_cmdq_write(core, core->packet);
	if (rc)
		rc = -ENOTEMPTY;

err_create_pkt:
	return rc;
}

static int __core_release_resource(struct msm_vidc_core *core,
		struct vidc_resource_hdr *resource_hdr)
{
	int rc = 0;

	if (!core || !resource_hdr) {
		d_vpr_e("%s: invalid params %pK %pK\n",
			__func__, core, resource_hdr);
		return -EINVAL;
	}

	//rc = hfi_packet_sys_release_resource(core, core->packet, core->packet_size, resource_hdr);
	if (rc) {
		d_vpr_e("release_res: failed to create packet\n");
		goto err_create_pkt;
	}

	//rc = __iface_cmdq_write(core, core->packet);
	if (rc)
		rc = -ENOTEMPTY;

err_create_pkt:
	return rc;
}
#endif



static void __deinit_clocks(struct msm_vidc_core *core)
{
	struct clock_info *cl;

	core->power.clk_freq = 0;
	venus_hfi_for_each_clock_reverse(core, cl) {
		if (cl->clk) {
			clk_put(cl->clk);
			cl->clk = NULL;
		}
	}
}

static int __init_clocks(struct msm_vidc_core *core)
{
	int rc = 0;
	struct clock_info *cl = NULL;

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	venus_hfi_for_each_clock(core, cl) {
		d_vpr_h("%s: scalable? %d, count %d\n",
				cl->name, cl->has_scaling, cl->count);
	}

	venus_hfi_for_each_clock(core, cl) {
		if (!cl->clk) {
			cl->clk = clk_get(&core->pdev->dev, cl->name);
			if (IS_ERR_OR_NULL(cl->clk)) {
				d_vpr_e("Failed to get clock: %s\n", cl->name);
				rc = PTR_ERR(cl->clk) ?
					PTR_ERR(cl->clk) : -EINVAL;
				cl->clk = NULL;
				goto err_clk_get;
			}
		}
	}
	core->power.clk_freq = 0;
	return 0;

err_clk_get:
	__deinit_clocks(core);
	return rc;
}

static int __handle_reset_clk(struct msm_vidc_core *core,
			int reset_index, enum reset_state state)
{
	int rc = 0;
	struct msm_vidc_dt *dt = core->dt;
	struct reset_control *rst;
	struct reset_set *rst_set = &dt->reset_set;

	if (!rst_set->reset_tbl)
		return 0;

	rst = rst_set->reset_tbl[reset_index].rst;
	d_vpr_h("reset_clk: name %s reset_state %d rst %pK\n",
		rst_set->reset_tbl[reset_index].name, state, rst);

	switch (state) {
	case INIT:
		if (rst)
			goto skip_reset_init;

		rst = devm_reset_control_get(&core->pdev->dev,
				rst_set->reset_tbl[reset_index].name);
		if (IS_ERR(rst))
			rc = PTR_ERR(rst);

		rst_set->reset_tbl[reset_index].rst = rst;
		break;
	case ASSERT:
		if (!rst) {
			rc = PTR_ERR(rst);
			goto failed_to_reset;
		}

		rc = reset_control_assert(rst);
		break;
	case DEASSERT:
		if (!rst) {
			rc = PTR_ERR(rst);
			goto failed_to_reset;
		}
		rc = reset_control_deassert(rst);
		break;
	default:
		d_vpr_e("%s: invalid reset request\n", __func__);
		if (rc)
			goto failed_to_reset;
	}

	return 0;

skip_reset_init:
failed_to_reset:
	return rc;
}

void __disable_unprepare_clks(struct msm_vidc_core *core)
{
	struct clock_info *cl;

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return;
	}

	venus_hfi_for_each_clock_reverse(core, cl) {
		d_vpr_h("Clock: %s disable and unprepare\n",
				cl->name);

		if (!__clk_is_enabled(cl->clk))
			d_vpr_e("%s: clock %s already disabled\n",
				__func__, cl->name);

		clk_disable_unprepare(cl->clk);
		cl->prev = 0;

		if (__clk_is_enabled(cl->clk))
			d_vpr_e("%s: clock %s not disabled\n",
				__func__, cl->name);
	}
}

int __reset_ahb2axi_bridge(struct msm_vidc_core *core)
{
	int rc, i;

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < core->dt->reset_set.count; i++) {
		rc = __handle_reset_clk(core, i, ASSERT);
		if (rc) {
			d_vpr_e("failed to assert reset clocks\n");
			goto failed_to_reset;
		}

		/* wait for deassert */
		usleep_range(400, 500);

		rc = __handle_reset_clk(core, i, DEASSERT);
		if (rc) {
			d_vpr_e("failed to deassert reset clocks\n");
			goto failed_to_reset;
		}
	}

	return 0;

failed_to_reset:
	return rc;
}

static int __prepare_enable_clks(struct msm_vidc_core *core)
{
	struct clock_info *cl = NULL, *cl_fail = NULL;
	int rc = 0, c = 0;

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	venus_hfi_for_each_clock(core, cl) {
		/*
		 * For the clocks we control, set the rate prior to preparing
		 * them.  Since we don't really have a load at this point, scale
		 * it to the lowest frequency possible
		 */
		if (cl->has_scaling)
			__set_clk_rate(core, cl,
					clk_round_rate(cl->clk, 0));

		if (__clk_is_enabled(cl->clk))
			d_vpr_e("%s: clock %s already enabled\n",
				__func__, cl->name);

		rc = clk_prepare_enable(cl->clk);
		if (rc) {
			d_vpr_e("Failed to enable clocks\n");
			cl_fail = cl;
			goto fail_clk_enable;
		}

		if (!__clk_is_enabled(cl->clk))
			d_vpr_e("%s: clock %s not enabled\n",
				__func__, cl->name);

		c++;
		d_vpr_h("Clock: %s prepared and enabled\n", cl->name);
	}

	call_venus_op(core, clock_config_on_enable, core);
	return rc;

fail_clk_enable:
	venus_hfi_for_each_clock_reverse_continue(core, cl, c) {
		d_vpr_e("Clock: %s disable and unprepare\n",
			cl->name);
		clk_disable_unprepare(cl->clk);
		cl->prev = 0;
	}

	return rc;
}

static void __deinit_bus(struct msm_vidc_core *core)
{
	struct bus_info *bus = NULL;

	if (!core)
		return;

	core->power.bw_ddr = 0;
	core->power.bw_llcc = 0;

	venus_hfi_for_each_bus_reverse(core, bus) {
		icc_put(bus->path);
		bus->path = NULL;
	}
}

static int __init_bus(struct msm_vidc_core *core)
{
	struct bus_info *bus = NULL;
	int rc = 0;

	if (!core)
		return -EINVAL;

	venus_hfi_for_each_bus(core, bus) {
		if (!strcmp(bus->name, "venus-llcc")) {
			if (msm_vidc_syscache_disable) {
				d_vpr_h("Skipping LLC bus init: %s\n",
					bus->name);
				continue;
			}
		}
		bus->path = of_icc_get(bus->dev, bus->name);
		if (IS_ERR_OR_NULL(bus->path)) {
			rc = PTR_ERR(bus->path) ?
				PTR_ERR(bus->path) : -EBADHANDLE;

			d_vpr_e("Failed to register bus %s: %d\n",
					bus->name, rc);
			bus->path = NULL;
			goto err_add_dev;
		}
	}

	return 0;

err_add_dev:
	__deinit_bus(core);
	return rc;
}

static void __deinit_regulators(struct msm_vidc_core *core)
{
	struct regulator_info *rinfo = NULL;

	venus_hfi_for_each_regulator_reverse(core, rinfo) {
		if (rinfo->regulator) {
			regulator_put(rinfo->regulator);
			rinfo->regulator = NULL;
		}
	}
}

static int __init_regulators(struct msm_vidc_core *core)
{
	int rc = 0;
	struct regulator_info *rinfo = NULL;

	venus_hfi_for_each_regulator(core, rinfo) {
		rinfo->regulator = regulator_get(&core->pdev->dev,
				rinfo->name);
		if (IS_ERR_OR_NULL(rinfo->regulator)) {
			rc = PTR_ERR(rinfo->regulator) ?
				PTR_ERR(rinfo->regulator) : -EBADHANDLE;
			d_vpr_e("Failed to get regulator: %s\n", rinfo->name);
			rinfo->regulator = NULL;
			goto err_reg_get;
		}
	}

	return 0;

err_reg_get:
	__deinit_regulators(core);
	return rc;
}

static void __deinit_subcaches(struct msm_vidc_core *core)
{
	struct subcache_info *sinfo = NULL;

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		goto exit;
	}

	if (!is_sys_cache_present(core))
		goto exit;

	venus_hfi_for_each_subcache_reverse(core, sinfo) {
		if (sinfo->subcache) {
			d_vpr_h("deinit_subcaches: %s\n", sinfo->name);
			llcc_slice_putd(sinfo->subcache);
			sinfo->subcache = NULL;
		}
	}

exit:
	return;
}

static int __init_subcaches(struct msm_vidc_core *core)
{
	int rc = 0;
	struct subcache_info *sinfo = NULL;

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (!is_sys_cache_present(core))
		return 0;

	venus_hfi_for_each_subcache(core, sinfo) {
		if (!strcmp("vidsc0", sinfo->name)) {
			sinfo->subcache = llcc_slice_getd(LLCC_VIDSC0);
		} else if (!strcmp("vidsc1", sinfo->name)) {
			sinfo->subcache = llcc_slice_getd(LLCC_VIDSC1);
		} else if (!strcmp("vidscfw", sinfo->name)) {
			sinfo->subcache = llcc_slice_getd(LLCC_VIDFW);
		} else {
			d_vpr_e("Invalid subcache name %s\n",
					sinfo->name);
		}
		if (IS_ERR_OR_NULL(sinfo->subcache)) {
			rc = PTR_ERR(sinfo->subcache) ?
				PTR_ERR(sinfo->subcache) : -EBADHANDLE;
			d_vpr_e("init_subcaches: invalid subcache: %s rc %d\n",
				sinfo->name, rc);
			sinfo->subcache = NULL;
			goto err_subcache_get;
		}
		d_vpr_h("init_subcaches: %s\n", sinfo->name);
	}

	return 0;

err_subcache_get:
	__deinit_subcaches(core);
	return rc;
}

static int __init_resources(struct msm_vidc_core *core)
{
	int i, rc = 0;

	rc = __init_regulators(core);
	if (rc) {
		d_vpr_e("Failed to get all regulators\n");
		return -ENODEV;
	}

	rc = __init_clocks(core);
	if (rc) {
		d_vpr_e("Failed to init clocks\n");
		rc = -ENODEV;
		goto err_init_clocks;
	}

	for (i = 0; i < core->dt->reset_set.count; i++) {
		rc = __handle_reset_clk(core, i, INIT);
		if (rc) {
			d_vpr_e("Failed to init reset clocks\n");
			rc = -ENODEV;
			goto err_init_reset_clk;
		}
	}

	rc = __init_bus(core);
	if (rc) {
		d_vpr_e("Failed to init bus: %d\n", rc);
		goto err_init_bus;
	}

	rc = __init_subcaches(core);
	if (rc)
		d_vpr_e("Failed to init subcaches: %d\n", rc);

	return rc;

err_init_reset_clk:
err_init_bus:
	__deinit_clocks(core);
err_init_clocks:
	__deinit_regulators(core);
	return rc;
}

static void __deinit_resources(struct msm_vidc_core *core)
{
	__deinit_subcaches(core);
	__deinit_bus(core);
	__deinit_clocks(core);
	__deinit_regulators(core);
}

static int __disable_regulator(struct regulator_info *rinfo,
				struct msm_vidc_core *core)
{
	int rc = 0;

	d_vpr_h("Disabling regulator %s\n", rinfo->name);

	/*
	 * This call is needed. Driver needs to acquire the control back
	 * from HW in order to disable the regualtor. Else the behavior
	 * is unknown.
	 */

	rc = __acquire_regulator(core, rinfo);
	if (rc) {
		/*
		 * This is somewhat fatal, but nothing we can do
		 * about it. We can't disable the regulator w/o
		 * getting it back under s/w control
		 */
		d_vpr_e("Failed to acquire control on %s\n",
			rinfo->name);

		goto disable_regulator_failed;
	}

	if (!regulator_is_enabled(rinfo->regulator))
		d_vpr_e("%s: regulator %s already disabled\n",
			__func__, rinfo->name);

	rc = regulator_disable(rinfo->regulator);
	if (rc) {
		d_vpr_e("Failed to disable %s: %d\n",
			rinfo->name, rc);
		goto disable_regulator_failed;
	}

	if (regulator_is_enabled(rinfo->regulator))
		d_vpr_e("%s: regulator %s not disabled\n",
			__func__, rinfo->name);

	return 0;
disable_regulator_failed:

	/* Bring attention to this issue */
	__fatal_error(core, true);
	return rc;
}

static int __enable_hw_power_collapse(struct msm_vidc_core *core)
{
	int rc = 0;

	// TODO: skip if hardwar power control feature is not present
	d_vpr_e("%s: skip hand off regulators\n", __func__);
	return 0;

	rc = __hand_off_regulators(core);
	if (rc)
		d_vpr_e("%s: Failed to enable HW power collapse %d\n",
				__func__, rc);
	return rc;
}

static int __enable_regulators(struct msm_vidc_core *core)
{
	int rc = 0, c = 0;
	struct regulator_info *rinfo;

	d_vpr_h("Enabling regulators\n");

	venus_hfi_for_each_regulator(core, rinfo) {
		if (regulator_is_enabled(rinfo->regulator))
			d_vpr_e("%s: regulator %s already enabled\n",
				__func__, rinfo->name);

		rc = regulator_enable(rinfo->regulator);
		if (rc) {
			d_vpr_e("Failed to enable %s: %d\n",
					rinfo->name, rc);
			goto err_reg_enable_failed;
		}

		if (!regulator_is_enabled(rinfo->regulator))
			d_vpr_e("%s: regulator %s not enabled\n",
				__func__, rinfo->name);

		d_vpr_h("Enabled regulator %s\n",
				rinfo->name);
		c++;
	}

	return 0;

err_reg_enable_failed:
	venus_hfi_for_each_regulator_reverse_continue(core, rinfo, c)
		__disable_regulator(rinfo, core);

	return rc;
}

int __disable_regulators(struct msm_vidc_core *core)
{
	struct regulator_info *rinfo;

	d_vpr_h("Disabling regulators\n");
	venus_hfi_for_each_regulator_reverse(core, rinfo)
		__disable_regulator(rinfo, core);

	return 0;
}

static int __release_subcaches(struct msm_vidc_core *core)
{
#if 0 // TODO
	struct subcache_info *sinfo;
	int rc = 0;
	u32 c = 0;
	u32 resource[VIDC_MAX_SUBCACHE_SIZE];
	struct hfi_resource_syscache_info_type *sc_res_info;
	struct hfi_resource_subcache_type *sc_res;
	struct vidc_resource_hdr rhdr;

	if (msm_vidc_syscache_disable || !is_sys_cache_present(core))
		return 0;

	memset((void *)resource, 0x0, (sizeof(u32) * VIDC_MAX_SUBCACHE_SIZE));

	sc_res_info = (struct hfi_resource_syscache_info_type *)resource;
	sc_res = &(sc_res_info->rg_subcache_entries[0]);

	/* Release resource command to Venus */
	venus_hfi_for_each_subcache_reverse(core, sinfo) {
		if (sinfo->isset) {
			/* Update the entry */
			sc_res[c].size = sinfo->subcache->slice_size;
			sc_res[c].sc_id = sinfo->subcache->slice_id;
			c++;
			sinfo->isset = false;
		}
	}

	if (c > 0) {
		d_vpr_h("Releasing %d subcaches\n", c);
		rhdr.resource_handle = sc_res_info; /* cookie */
		rhdr.resource_id = VIDC_RESOURCE_SYSCACHE;

		rc = __core_release_resource(core, &rhdr);
		if (rc)
			d_vpr_e("Failed to release %d subcaches\n", c);
	}

	core->dt->sys_cache_res_set = false;
#endif
	return 0;
}

static int __disable_subcaches(struct msm_vidc_core *core)
{
	struct subcache_info *sinfo;
	int rc = 0;

	if (msm_vidc_syscache_disable || !is_sys_cache_present(core))
		return 0;

	/* De-activate subcaches */
	venus_hfi_for_each_subcache_reverse(core, sinfo) {
		if (sinfo->isactive) {
			d_vpr_h("De-activate subcache %s\n",
				sinfo->name);
			rc = llcc_slice_deactivate(sinfo->subcache);
			if (rc) {
				d_vpr_e("Failed to de-activate %s: %d\n",
					sinfo->name, rc);
			}
			sinfo->isactive = false;
		}
	}

	return 0;
}

static int __enable_subcaches(struct msm_vidc_core *core)
{
	int rc = 0;
	u32 c = 0;
	struct subcache_info *sinfo;

	if (msm_vidc_syscache_disable || !is_sys_cache_present(core))
		return 0;

	/* Activate subcaches */
	venus_hfi_for_each_subcache(core, sinfo) {
		rc = llcc_slice_activate(sinfo->subcache);
		if (rc) {
			d_vpr_e("Failed to activate %s: %d\n",
				sinfo->name, rc);
			__fatal_error(core, true);
			goto err_activate_fail;
		}
		sinfo->isactive = true;
		d_vpr_h("Activated subcache %s\n", sinfo->name);
		c++;
	}

	d_vpr_h("Activated %d Subcaches to Venus\n", c);

	return 0;

err_activate_fail:
	__release_subcaches(core);
	__disable_subcaches(core);
	return 0;
}

static int __set_subcaches(struct msm_vidc_core *core)
{
#if 0 // TODO
	int rc = 0;
	u32 c = 0;
	struct subcache_info *sinfo;
	u32 resource[VIDC_MAX_SUBCACHE_SIZE];
	struct hfi_resource_syscache_info_type *sc_res_info;
	struct hfi_resource_subcache_type *sc_res;
	struct vidc_resource_hdr rhdr;

	if (core->dt->sys_cache_res_set) {
		d_vpr_h("Subcaches already set to Venus\n");
		return 0;
	}

	memset((void *)resource, 0x0, (sizeof(u32) * VIDC_MAX_SUBCACHE_SIZE));

	sc_res_info = (struct hfi_resource_syscache_info_type *)resource;
	sc_res = &(sc_res_info->rg_subcache_entries[0]);

	venus_hfi_for_each_subcache(core, sinfo) {
		if (sinfo->isactive) {
			sc_res[c].size = sinfo->subcache->slice_size;
			sc_res[c].sc_id = sinfo->subcache->slice_id;
			c++;
		}
	}

	/* Set resource to Venus for activated subcaches */
	if (c) {
		d_vpr_h("Setting %d Subcaches\n", c);

		rhdr.resource_handle = sc_res_info; /* cookie */
		rhdr.resource_id = VIDC_RESOURCE_SYSCACHE;

		sc_res_info->num_entries = c;

		rc = __core_set_resource(core, &rhdr, (void *)sc_res_info);
		if (rc) {
			d_vpr_e("Failed to set subcaches %d\n", rc);
			goto err_fail_set_subacaches;
		}

		venus_hfi_for_each_subcache(core, sinfo) {
			if (sinfo->isactive)
				sinfo->isset = true;
		}

		d_vpr_h("Set Subcaches done to Venus\n");
		core->dt->sys_cache_res_set = true;
	}

	return 0;

err_fail_set_subacaches:
	__disable_subcaches(core);
#endif
	return 0;
}
/*
static int __set_ubwc_config(struct msm_vidc_core *core)
{
	int rc = 0;

	if (!core->platform->data.ubwc_config) {
		d_vpr_h("%s: invalid ubwc config\n", __func__);
		return -EINVAL;
	}

	//rc = hfi_packet_sys_ubwc_config(core, core->packet, core->packet_size);
	if (rc)
		return rc;

	//rc = __iface_cmdq_write(core, core->packet));
	if (rc)
		return rc;

	d_vpr_h("Configured UBWC Config\n");
	return rc;
}
*/
static int __venus_power_on(struct msm_vidc_core *core)
{
	int rc = 0;

	if (core->power_enabled)
		return 0;

	core->power_enabled = true;
	/* Vote for all hardware resources */
	rc = __vote_buses(core, INT_MAX, INT_MAX);
	if (rc) {
		d_vpr_e("Failed to vote buses, err: %d\n", rc);
		goto fail_vote_buses;
	}

	rc = __enable_regulators(core);
	if (rc) {
		d_vpr_e("Failed to enable GDSC, err = %d\n", rc);
		goto fail_enable_gdsc;
	}

	rc = call_venus_op(core, reset_ahb2axi_bridge, core);
	if (rc) {
		d_vpr_e("Failed to reset ahb2axi: %d\n", rc);
		goto fail_enable_clks;
	}

	rc = __prepare_enable_clks(core);
	if (rc) {
		d_vpr_e("Failed to enable clocks: %d\n", rc);
		goto fail_enable_clks;
	}

	rc = __scale_clocks(core);
	if (rc) {
		d_vpr_e("Failed to scale clocks, performance might be affected\n");
		rc = 0;
	}

	/*
	 * Re-program all of the registers that get reset as a result of
	 * regulator_disable() and _enable()
	 */
	__set_registers(core);

	call_venus_op(core, interrupt_init, core);
	core->intr_status = 0;
	enable_irq(core->dt->irq);

	return rc;

fail_enable_clks:
	__disable_regulators(core);
fail_enable_gdsc:
	__unvote_buses(core);
fail_vote_buses:
	core->power_enabled = false;
	return rc;
}

static int __suspend(struct msm_vidc_core *core)
{
	int rc = 0;

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	} else if (!core->power_enabled) {
		d_vpr_h("Power already disabled\n");
		return 0;
	}

	d_vpr_h("Entering suspend\n");

	rc = __tzbsp_set_video_state(TZBSP_VIDEO_STATE_SUSPEND);
	if (rc) {
		d_vpr_e("Failed to suspend video core %d\n", rc);
		goto err_tzbsp_suspend;
	}

	__disable_subcaches(core);

	call_venus_op(core, power_off, core);
	d_vpr_h("Venus power off\n");
	return rc;

err_tzbsp_suspend:
	return rc;
}

static int __resume(struct msm_vidc_core *core)
{
	int rc = 0;

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	} else if (core->power_enabled) {
		goto exit;
	} else if (!__core_in_valid_state(core)) {
		d_vpr_e("%s: core in deinit state\n", __func__);
		return -EINVAL;
	}

	d_vpr_h("Resuming from power collapse\n");
	rc = __venus_power_on(core);
	if (rc) {
		d_vpr_e("Failed to power on venus\n");
		goto err_venus_power_on;
	}

	/* Reboot the firmware */
	rc = __tzbsp_set_video_state(TZBSP_VIDEO_STATE_RESUME);
	if (rc) {
		d_vpr_e("Failed to resume video core %d\n", rc);
		goto err_set_video_state;
	}

	/*
	 * Hand off control of regulators to h/w _after_ loading fw.
	 * Note that the GDSC will turn off when switching from normal
	 * (s/w triggered) to fast (HW triggered) unless the h/w vote is
	 * present.
	 */
	if (__enable_hw_power_collapse(core))
		d_vpr_e("Failed to enabled inter-frame PC\n");

	call_venus_op(core, setup_ucregion_memmap, core);

	/* Wait for boot completion */
	rc = call_venus_op(core, boot_firmware, core);
	if (rc) {
		d_vpr_e("Failed to reset venus core\n");
		goto err_reset_core;
	}

	__sys_set_debug(core, (msm_vidc_debug & FW_LOGMASK) >> FW_LOGSHIFT);
	__enable_subcaches(core);
	__set_subcaches(core);

	d_vpr_h("Resumed from power collapse\n");
exit:
	/* Don't reset skip_pc_count for SYS_PC_PREP cmd */
	//if (core->last_packet_type != HFI_CMD_SYS_PC_PREP)
	//	core->skip_pc_count = 0;
	return rc;
err_reset_core:
	__tzbsp_set_video_state(TZBSP_VIDEO_STATE_SUSPEND);
err_set_video_state:
	call_venus_op(core, power_off, core);
err_venus_power_on:
	d_vpr_e("Failed to resume from power collapse\n");
	return rc;
}

static void __set_queue_hdr_defaults(struct hfi_queue_header *q_hdr)
{
	q_hdr->qhdr_status = 0x1;
	q_hdr->qhdr_type = VIDC_IFACEQ_DFLT_QHDR;
	q_hdr->qhdr_q_size = VIDC_IFACEQ_QUEUE_SIZE / 4;
	q_hdr->qhdr_pkt_size = 0;
	q_hdr->qhdr_rx_wm = 0x1;
	q_hdr->qhdr_tx_wm = 0x1;
	q_hdr->qhdr_rx_req = 0x1;
	q_hdr->qhdr_tx_req = 0x0;
	q_hdr->qhdr_rx_irq_status = 0x0;
	q_hdr->qhdr_tx_irq_status = 0x0;
	q_hdr->qhdr_read_idx = 0x0;
	q_hdr->qhdr_write_idx = 0x0;
}

static void __interface_queues_release(struct msm_vidc_core *core)
{
	int i;

	d_vpr_h("%s()\n", __func__);

	msm_vidc_memory_unmap(core, &core->iface_q_table.map);
	msm_vidc_memory_free(core, &core->iface_q_table.alloc);
	msm_vidc_memory_unmap(core, &core->sfr.map);
	msm_vidc_memory_free(core, &core->sfr.alloc);

	for (i = 0; i < VIDC_IFACEQ_NUMQ; i++) {
		core->iface_queues[i].q_hdr = NULL;
		core->iface_queues[i].q_array.align_virtual_addr = NULL;
		core->iface_queues[i].q_array.align_device_addr = 0;
	}

	core->iface_q_table.align_virtual_addr = NULL;
	core->iface_q_table.align_device_addr = 0;

	core->sfr.align_virtual_addr = NULL;
	core->sfr.align_device_addr = 0;
}

static int __interface_queues_init(struct msm_vidc_core *core)
{
	int rc = 0;
	struct hfi_queue_table_header *q_tbl_hdr;
	struct hfi_queue_header *q_hdr;
	struct msm_vidc_iface_q_info *iface_q;
	struct msm_vidc_alloc alloc;
	struct msm_vidc_map map;
	int offset = 0;
	u32 q_size;
	u32 i;

	d_vpr_h("%s()\n", __func__);
	q_size = SHARED_QSIZE - ALIGNED_SFR_SIZE - ALIGNED_QDSS_SIZE;

	memset(&alloc, 0, sizeof(alloc));
	alloc.type       = MSM_VIDC_BUF_QUEUE;
	alloc.region     = MSM_VIDC_NON_SECURE;
	alloc.size       = q_size;
	alloc.cached     = false;
	alloc.secure     = false;
	alloc.map_kernel = true;
	rc = msm_vidc_memory_alloc(core, &alloc);
	if (rc) {
		d_vpr_e("%s: alloc failed\n", __func__);
		goto fail_alloc_queue;
	}

	memset(&map, 0, sizeof(map));
	map.type         = alloc.type;
	map.region       = alloc.region;
	map.dmabuf       = alloc.dmabuf;
	rc = msm_vidc_memory_map(core, &map);
	if (rc) {
		d_vpr_e("%s: alloc failed\n", __func__);
		goto fail_alloc_queue;
	}

	core->iface_q_table.align_virtual_addr = alloc.kvaddr;
	core->iface_q_table.align_device_addr = map.device_addr;
	core->iface_q_table.mem_size = VIDC_IFACEQ_TABLE_SIZE;
	core->iface_q_table.alloc = alloc;
	core->iface_q_table.map = map;
	offset += core->iface_q_table.mem_size;

	for (i = 0; i < VIDC_IFACEQ_NUMQ; i++) {
		iface_q = &core->iface_queues[i];
		iface_q->q_array.align_device_addr = map.device_addr + offset;
		iface_q->q_array.align_virtual_addr = alloc.kvaddr + offset;
		iface_q->q_array.mem_size = VIDC_IFACEQ_QUEUE_SIZE;
		offset += iface_q->q_array.mem_size;
		iface_q->q_hdr = VIDC_IFACEQ_GET_QHDR_START_ADDR(
				core->iface_q_table.align_virtual_addr, i);
		__set_queue_hdr_defaults(iface_q->q_hdr);
	}

	q_tbl_hdr = (struct hfi_queue_table_header *)
			core->iface_q_table.align_virtual_addr;
	q_tbl_hdr->qtbl_version = 0;
	q_tbl_hdr->device_addr = (void *)core;
	strlcpy(q_tbl_hdr->name, "msm_v4l2_vidc", sizeof(q_tbl_hdr->name));
	q_tbl_hdr->qtbl_size = VIDC_IFACEQ_TABLE_SIZE;
	q_tbl_hdr->qtbl_qhdr0_offset = sizeof(struct hfi_queue_table_header);
	q_tbl_hdr->qtbl_qhdr_size = sizeof(struct hfi_queue_header);
	q_tbl_hdr->qtbl_num_q = VIDC_IFACEQ_NUMQ;
	q_tbl_hdr->qtbl_num_active_q = VIDC_IFACEQ_NUMQ;

	iface_q = &core->iface_queues[VIDC_IFACEQ_CMDQ_IDX];
	q_hdr = iface_q->q_hdr;
	q_hdr->qhdr_start_addr = iface_q->q_array.align_device_addr;
	q_hdr->qhdr_type |= HFI_Q_ID_HOST_TO_CTRL_CMD_Q;

	iface_q = &core->iface_queues[VIDC_IFACEQ_MSGQ_IDX];
	q_hdr = iface_q->q_hdr;
	q_hdr->qhdr_start_addr = iface_q->q_array.align_device_addr;
	q_hdr->qhdr_type |= HFI_Q_ID_CTRL_TO_HOST_MSG_Q;

	iface_q = &core->iface_queues[VIDC_IFACEQ_DBGQ_IDX];
	q_hdr = iface_q->q_hdr;
	q_hdr->qhdr_start_addr = iface_q->q_array.align_device_addr;
	q_hdr->qhdr_type |= HFI_Q_ID_CTRL_TO_HOST_DEBUG_Q;
	/*
	 * Set receive request to zero on debug queue as there is no
	 * need of interrupt from video hardware for debug messages
	 */
	q_hdr->qhdr_rx_req = 0;

	/* sfr buffer */
	memset(&alloc, 0, sizeof(alloc));
	alloc.type       = MSM_VIDC_BUF_QUEUE;
	alloc.region     = MSM_VIDC_NON_SECURE;
	alloc.size       = ALIGNED_SFR_SIZE;
	alloc.cached     = false;
	alloc.secure     = false;
	alloc.map_kernel = true;
	rc = msm_vidc_memory_alloc(core, &alloc);
	if (rc) {
		d_vpr_e("%s: sfr alloc failed\n", __func__);
		goto fail_alloc_queue;
	}
	memset(&map, 0, sizeof(map));
	map.type         = alloc.type;
	map.region       = alloc.region;
	map.dmabuf       = alloc.dmabuf;
	rc = msm_vidc_memory_map(core, &map);
	if (rc) {
		d_vpr_e("%s: sfr map failed\n", __func__);
		goto fail_alloc_queue;
	}
	core->sfr.align_device_addr = map.device_addr;
	core->sfr.align_virtual_addr = alloc.kvaddr;
	core->sfr.mem_size = ALIGNED_SFR_SIZE;
	core->sfr.alloc = alloc;
	core->sfr.map = map;
	/* write sfr buffer size in first word */
	*((u32 *)core->sfr.align_virtual_addr) = ALIGNED_SFR_SIZE;

	rc = call_venus_op(core, setup_ucregion_memmap, core);
	if (rc)
		return rc;

	return 0;
fail_alloc_queue:
	return -ENOMEM;
}

static int __load_fw(struct msm_vidc_core *core)
{
	int rc = 0;

	rc = __init_resources(core);
	if (rc) {
		d_vpr_e("Failed to init resources: %d\n", rc);
		goto fail_init_res;
	}

	rc = __venus_power_on(core);
	if (rc) {
		d_vpr_e("%s: power on failed\n", __func__);
		goto fail_venus_power_on;
	}

	if (!core->dt->fw_cookie) {
		core->dt->fw_cookie = subsystem_get_with_fwname("venus",
								core->dt->fw_name);
		if (IS_ERR_OR_NULL(core->dt->fw_cookie)) {
			d_vpr_e("%s: firmware download failed\n", __func__);
			core->dt->fw_cookie = NULL;
			rc = -ENOMEM;
			goto fail_load_fw;
		}
	}

	rc = __protect_cp_mem(core);
	if (rc) {
		d_vpr_e("%s: protect memory failed\n", __func__);
		goto fail_protect_mem;
	}
	/*
	* Hand off control of regulators to h/w _after_ loading fw.
	* Note that the GDSC will turn off when switching from normal
	* (s/w triggered) to fast (HW triggered) unless the h/w vote is
	* present.
	*/
	if (__enable_hw_power_collapse(core))
		d_vpr_e("%s: hardware power collapse unsuccessful\n", __func__);

	return rc;
fail_protect_mem:
	if (core->dt->fw_cookie)
		subsystem_put(core->dt->fw_cookie);
	core->dt->fw_cookie = NULL;
fail_load_fw:
	call_venus_op(core, power_off, core);
fail_venus_power_on:
	__deinit_resources(core);
fail_init_res:
	return rc;
}

static void __unload_fw(struct msm_vidc_core *core)
{
	if (!core->dt->fw_cookie)
		return;

	cancel_delayed_work(&core->pm_work);
	if (core->state != MSM_VIDC_CORE_DEINIT)
		flush_workqueue(core->pm_workq);

	subsystem_put(core->dt->fw_cookie);
	__interface_queues_release(core);
	call_venus_op(core, power_off, core);
	core->dt->fw_cookie = NULL;
	__deinit_resources(core);

	d_vpr_h("Firmware unloaded successfully\n");
}

static int __response_handler(struct msm_vidc_core *core)
{
	int rc = 0;

	while (!__iface_msgq_read(core, core->response_packet)) {
		rc = handle_response(core, core->response_packet);
		if (rc)
			continue;
		/* check for system error */
		if (core->state != MSM_VIDC_CORE_INIT)
			break;
	}

	__schedule_power_collapse_work(core);
	__flush_debug_queue(core, core->response_packet);

	return rc;
}

irqreturn_t venus_hfi_isr(int irq, void *data)
{
	struct msm_vidc_core *core = data;

	d_vpr_e("%s()\n", __func__);

	disable_irq_nosync(irq);
	queue_work(core->device_workq, &core->device_work);

	return IRQ_HANDLED;
}

void venus_hfi_work_handler(struct work_struct *work)
{
	struct msm_vidc_core *core;
	int num_responses = 0;

	core = container_of(work, struct msm_vidc_core, device_work);
	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return;
	}
	d_vpr_e("%s(): core %pK\n", __func__, core);

	mutex_lock(&core->lock);
	if (__resume(core)) {
		d_vpr_e("%s: Power on failed\n", __func__);
		goto err_no_work;
	}

	call_venus_op(core, clear_interrupt, core);
	mutex_unlock(&core->lock);

	num_responses = __response_handler(core);

err_no_work:
	mutex_unlock(&core->lock);
	if (!call_venus_op(core, watchdog, core, core->intr_status))
		enable_irq(core->dt->irq);
}

void venus_hfi_pm_work_handler(struct work_struct *work)
{
	int rc = 0;
	struct msm_vidc_core *core;

	core = container_of(work, struct msm_vidc_core, pm_work.work);
	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return;
	}

	/*
	 * It is ok to check this variable outside the lock since
	 * it is being updated in this context only
	 */
	if (core->skip_pc_count >= VIDC_MAX_PC_SKIP_COUNT) {
		d_vpr_e("Failed to PC for %d times\n",
				core->skip_pc_count);
		core->skip_pc_count = 0;
		//__process_fatal_error(core);
		return;
	}

	mutex_lock(&core->lock);
	rc = __power_collapse(core, false);
	switch (rc) {
	case 0:
		core->skip_pc_count = 0;
		/* Cancel pending delayed works if any */
		__cancel_power_collapse_work(core);
		d_vpr_h("%s: power collapse successful!\n", __func__);
		break;
	case -EBUSY:
		core->skip_pc_count = 0;
		d_vpr_h("%s: retry PC as dsp is busy\n", __func__);
		__schedule_power_collapse_work(core);
		break;
	case -EAGAIN:
		core->skip_pc_count++;
		d_vpr_e("%s: retry power collapse (count %d)\n",
			__func__, core->skip_pc_count);
		__schedule_power_collapse_work(core);
		break;
	default:
		d_vpr_e("%s: power collapse failed\n", __func__);
		break;
	}
	mutex_unlock(&core->lock);
}

static int __sys_init(struct msm_vidc_core *core)
{
	int rc = 0;

	rc =  hfi_packet_sys_init(core, core->packet, core->packet_size);
	if (rc)
		return rc;

	rc = __iface_cmdq_write(core, core->packet);
	if (rc)
		return rc;

	return 0;
}

static int __sys_image_version(struct msm_vidc_core *core)
{
	int rc = 0;

	rc = hfi_packet_image_version(core, core->packet, core->packet_size);
	if (rc)
		return rc;

	rc = __iface_cmdq_write(core, core->packet);
	if (rc)
		return rc;

	return 0;
}

int venus_hfi_core_init(struct msm_vidc_core *core)
{
	int rc = 0;

	d_vpr_h("%s(): core %p\n", __func__, core);

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	core->packet_size = 4096;
	core->packet = kzalloc(core->packet_size, GFP_KERNEL);
	if (!core->packet) {
		d_vpr_e("%s(): core packet allocation failed\n", __func__);
		return -ENOMEM;
	}

	core->response_packet = kzalloc(core->packet_size, GFP_KERNEL);
	if (!core->response_packet) {
		d_vpr_e("%s(): core response packet allocation failed\n",
			__func__);
		kfree(core->packet);
		return -ENOMEM;
	}

	rc = __load_fw(core);
	if (rc)
		return rc;

	rc = __interface_queues_init(core);
	if (rc)
		goto error;

	rc = call_venus_op(core, boot_firmware, core);
	if (rc)
		goto error;

	__sys_init(core);
	__sys_image_version(core);
	__sys_set_debug(core, (msm_vidc_debug & FW_LOGMASK) >> FW_LOGSHIFT);
	__enable_subcaches(core);
	__set_subcaches(core);

	d_vpr_h("%s(): successful\n", __func__);
	return 0;

error:
	d_vpr_h("%s(): failed\n", __func__);
	__unload_fw(core);
	kfree(core->response_packet);
	kfree(core->packet);
	return rc;
}

int venus_hfi_core_release(struct msm_vidc_core *core)
{
	d_vpr_h("%s(): core %p\n", __func__, core);

	return 0;
}

int venus_hfi_suspend(struct msm_vidc_core *core)
{
	int rc = 0;

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	d_vpr_h("Suspending Venus\n");
	rc = __power_collapse(core, true);
	if (!rc) {
		/* Cancel pending delayed works if any */
		__cancel_power_collapse_work(core);
	} else {
		d_vpr_e("%s: Venus is busy\n", __func__);
		rc = -EBUSY;
	}

	return rc;
}

int venus_hfi_session_open(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	inst->packet_size = 4096;
	inst->packet = kzalloc(inst->packet_size, GFP_KERNEL);
	if (!inst->packet) {
		d_vpr_e("%s(): inst packet allocation failed\n", __func__);
		return -ENOMEM;
	}

	rc = hfi_packet_session_command(inst,
				HFI_CMD_OPEN,
				(HFI_HOST_FLAGS_RESPONSE_REQUIRED |
				HFI_HOST_FLAGS_INTR_REQUIRED),
				HFI_PORT_NONE,
				0, /* session_id */
				HFI_PAYLOAD_U32,
				&inst->session_id, /* payload */
				sizeof(u32));
	if (rc)
		goto error;

	rc = __iface_cmdq_write(inst->core, inst->packet);
	if (rc)
		goto error;
error:
	return rc;
}

int venus_hfi_session_set_codec(struct msm_vidc_inst *inst)
{
	int rc = 0;
	u32 codec;

	if (!inst || !inst->packet) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	codec = get_hfi_codec(inst);
	rc = venus_hfi_session_property(inst,
				HFI_PROP_CODEC,
				HFI_HOST_FLAGS_NONE,
				HFI_PORT_NONE,
				HFI_PAYLOAD_U32_ENUM,
				&codec,
				sizeof(u32));
	if (rc)
		goto error;

error:
	return rc;
}

int venus_hfi_session_property(struct msm_vidc_inst *inst,
	u32 pkt_type, u32 flags, u32 port, u32 payload_type,
	void *payload, u32 payload_size)
{
	int rc = 0;
	struct msm_vidc_core *core;

	if (!inst || !inst->core || !inst->packet) {
		d_vpr_e("%s: Invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	rc = hfi_create_header(inst->packet, inst->packet_size,
				inst->session_id, core->header_id++);
	if (rc)
		goto err_prop;
	rc = hfi_create_packet(inst->packet, inst->packet_size,
				pkt_type,
				flags,
				payload_type,
				port,
				core->packet_id++,
				payload,
				payload_size);
	if (rc)
		goto err_prop;

	rc = __iface_cmdq_write(inst->core, inst->packet);
	if (rc)
		goto err_prop;

	return rc;

err_prop:
	d_vpr_e("%s: create packet failed\n", __func__);
	return rc;
}

int venus_hfi_session_close(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst || !inst->packet) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = hfi_packet_session_command(inst,
				HFI_CMD_CLOSE,
				(HFI_HOST_FLAGS_RESPONSE_REQUIRED |
				HFI_HOST_FLAGS_INTR_REQUIRED |
				HFI_HOST_FLAGS_NON_DISCARDABLE),
				HFI_PORT_NONE,
				inst->session_id,
				HFI_PAYLOAD_NONE,
				NULL,
				0);
	if (!rc)
		__iface_cmdq_write(inst->core, inst->packet);

	kfree(inst->packet);

	return rc;
}

int venus_hfi_start(struct msm_vidc_inst *inst, enum msm_vidc_port_type port)
{
	int rc = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	if (port != INPUT_PORT && port != OUTPUT_PORT) {
		s_vpr_e(inst->sid, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	rc = hfi_packet_session_command(inst,
				HFI_CMD_START,
				(HFI_HOST_FLAGS_RESPONSE_REQUIRED |
				HFI_HOST_FLAGS_INTR_REQUIRED),
				get_hfi_port(inst, port),
				inst->session_id,
				HFI_PAYLOAD_NONE,
				NULL,
				0);
	if (rc)
		return rc;

	rc = __iface_cmdq_write(inst->core, inst->packet);
	if (rc)
		return rc;

	return rc;
}

int venus_hfi_stop(struct msm_vidc_inst *inst, enum msm_vidc_port_type port)
{
	int rc = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	if (port != INPUT_PORT && port != OUTPUT_PORT) {
		s_vpr_e(inst->sid, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	rc = hfi_packet_session_command(inst,
				HFI_CMD_STOP,
				(HFI_HOST_FLAGS_RESPONSE_REQUIRED |
				HFI_HOST_FLAGS_INTR_REQUIRED |
				HFI_HOST_FLAGS_NON_DISCARDABLE),
				get_hfi_port(inst, port),
				inst->session_id,
				HFI_PAYLOAD_NONE,
				NULL,
				0);
	if (rc)
		return rc;

	rc = __iface_cmdq_write(inst->core, inst->packet);
	if (rc)
		return rc;

	return rc;
}

int venus_hfi_session_command(struct msm_vidc_inst *inst,
	u32 cmd, enum msm_vidc_port_type port, u32 payload_type,
	void *payload, u32 payload_size)
{
	int rc = 0;
	struct msm_vidc_core *core;

	if (!inst || !inst->packet || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	rc = hfi_create_header(inst->packet, inst->packet_size,
			inst->session_id,
			core->header_id++);
	if (rc)
		return rc;

	rc = hfi_create_packet(inst->packet, inst->packet_size,
			cmd,
			(HFI_HOST_FLAGS_RESPONSE_REQUIRED |
			HFI_HOST_FLAGS_INTR_REQUIRED),
			payload_type,
			get_hfi_port(inst, port),
			core->packet_id++,
			payload,
			payload_size);

	if (rc)
		goto err_cmd;

	s_vpr_h(inst->sid, "Command packet 0x%x created\n", cmd);
	__iface_cmdq_write(inst->core, inst->packet);

	return rc;

err_cmd:
	s_vpr_e(inst->sid, "%s: create packet failed for cmd: %d\n",
		__func__, cmd);
	return rc;
}

int venus_hfi_queue_buffer(struct msm_vidc_inst *inst,
	struct msm_vidc_buffer *buffer, struct msm_vidc_buffer *metabuf)
{
	int rc = 0;
	struct msm_vidc_core *core;
	struct hfi_buffer hfi_buffer;

	d_vpr_h("%s(): inst %p\n", __func__, inst);
	if (!inst || !inst->core || !inst->packet) {
		d_vpr_e("%s: Invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	rc = get_hfi_buffer(inst, buffer, &hfi_buffer);
	if (rc)
		return rc;

	rc = hfi_create_header(inst->packet, inst->packet_size,
			   inst->session_id, core->header_id++);
	if (rc)
		return rc;

	rc = hfi_create_packet(inst->packet,
			inst->packet_size,
			HFI_CMD_BUFFER,
			(HFI_HOST_FLAGS_RESPONSE_REQUIRED |
			HFI_HOST_FLAGS_INTR_REQUIRED),
			HFI_PAYLOAD_STRUCTURE,
			get_hfi_port_from_buffer_type(inst, buffer->type),
			core->packet_id++,
			&hfi_buffer,
			sizeof(hfi_buffer));
	if (rc)
		return rc;

	if (metabuf) {
		rc = get_hfi_buffer(inst, metabuf, &hfi_buffer);
		if (rc)
			return rc;
		rc = hfi_create_packet(inst->packet,
			inst->packet_size,
			HFI_CMD_BUFFER,
			HFI_HOST_FLAGS_NONE,
			HFI_PAYLOAD_STRUCTURE,
			get_hfi_port_from_buffer_type(inst, metabuf->type),
			core->packet_id++,
			&hfi_buffer,
			sizeof(hfi_buffer));
		if (rc)
			return rc;
	}

	rc = __iface_cmdq_write(inst->core, inst->packet);
	if (rc)
		return rc;

	return rc;
}

int venus_hfi_release_buffer(struct msm_vidc_inst *inst,
	struct msm_vidc_buffer *buffer)
{
	int rc = 0;
	struct msm_vidc_core *core;
	struct hfi_buffer hfi_buffer;

	d_vpr_h("%s(): inst %p\n", __func__, inst);
	if (!inst || !buffer) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	if (!is_internal_buffer(buffer->type)) {
		s_vpr_e(inst->sid, "release not allowed for buffer type %d\n",
			buffer->type);
		return 0;
	}
	core = inst->core;

	rc = get_hfi_buffer(inst, buffer, &hfi_buffer);
	if (rc)
		return rc;

	/* add pending release flag */
	hfi_buffer.flags |= HFI_BUF_HOST_FLAG_RELEASE;

	rc = hfi_create_header(inst->packet, inst->packet_size,
			   inst->session_id, core->header_id++);
	if (rc)
		return rc;

	rc = hfi_create_packet(inst->packet,
			inst->packet_size,
			HFI_CMD_BUFFER,
			(HFI_HOST_FLAGS_RESPONSE_REQUIRED |
			HFI_HOST_FLAGS_INTR_REQUIRED),
			HFI_PAYLOAD_STRUCTURE,
			get_hfi_port_from_buffer_type(inst, buffer->type),
			core->packet_id++,
			&hfi_buffer,
			sizeof(hfi_buffer));
	if (rc)
		return rc;

	rc = __iface_cmdq_write(inst->core, inst->packet);
	if (rc)
		return rc;

	return rc;
}
