/*
 * Copyright (c) 2016-2020 The Linux Foundation. All rights reserved.
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

#include "hal_hw_headers.h"
#include "hal_api.h"
#include "target_type.h"
#include "wcss_version.h"
#include "qdf_module.h"

#ifdef QCA_WIFI_QCA8074
void hal_qca6290_attach(struct hal_soc *hal);
#endif
#ifdef QCA_WIFI_QCA8074
void hal_qca8074_attach(struct hal_soc *hal);
#endif
#if defined(QCA_WIFI_QCA8074V2) || defined(QCA_WIFI_QCA6018)
void hal_qca8074v2_attach(struct hal_soc *hal);
#endif
#ifdef QCA_WIFI_QCA6390
void hal_qca6390_attach(struct hal_soc *hal);
#endif
#ifdef QCA_WIFI_QCA6490
void hal_qca6490_attach(struct hal_soc *hal);
#endif
#ifdef QCA_WIFI_QCN9000
void hal_qcn9000_attach(struct hal_soc *hal);
#endif
#ifdef QCA_WIFI_QCN9100
void hal_qcn9100_attach(struct hal_soc *hal);
#endif
#ifdef QCA_WIFI_QCA6750
void hal_qca6750_attach(struct hal_soc *hal);
#endif
#ifdef QCA_WIFI_QCA5018
void hal_qca5018_attach(struct hal_soc *hal);
#endif

#ifdef ENABLE_VERBOSE_DEBUG
bool is_hal_verbose_debug_enabled;
#endif

#ifdef ENABLE_HAL_REG_WR_HISTORY
struct hal_reg_write_fail_history hal_reg_wr_hist;

void hal_reg_wr_fail_history_add(struct hal_soc *hal_soc,
				 uint32_t offset,
				 uint32_t wr_val, uint32_t rd_val)
{
	struct hal_reg_write_fail_entry *record;
	int idx;

	idx = hal_history_get_next_index(&hal_soc->reg_wr_fail_hist->index,
					 HAL_REG_WRITE_HIST_SIZE);

	record = &hal_soc->reg_wr_fail_hist->record[idx];

	record->timestamp = qdf_get_log_timestamp();
	record->reg_offset = offset;
	record->write_val = wr_val;
	record->read_val = rd_val;
}

static void hal_reg_write_fail_history_init(struct hal_soc *hal)
{
	hal->reg_wr_fail_hist = &hal_reg_wr_hist;

	qdf_atomic_set(&hal->reg_wr_fail_hist->index, -1);
}
#else
static void hal_reg_write_fail_history_init(struct hal_soc *hal)
{
}
#endif

/**
 * hal_get_srng_ring_id() - get the ring id of a descriped ring
 * @hal: hal_soc data structure
 * @ring_type: type enum describing the ring
 * @ring_num: which ring of the ring type
 * @mac_id: which mac does the ring belong to (or 0 for non-lmac rings)
 *
 * Return: the ring id or -EINVAL if the ring does not exist.
 */
static int hal_get_srng_ring_id(struct hal_soc *hal, int ring_type,
				int ring_num, int mac_id)
{
	struct hal_hw_srng_config *ring_config =
		HAL_SRNG_CONFIG(hal, ring_type);
	int ring_id;

	if (ring_num >= ring_config->max_rings) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO,
			  "%s: ring_num exceeded maximum no. of supported rings",
			  __func__);
		/* TODO: This is a programming error. Assert if this happens */
		return -EINVAL;
	}

	if (ring_config->lmac_ring) {
		ring_id = ring_config->start_ring_id + ring_num +
			(mac_id * HAL_MAX_RINGS_PER_LMAC);
	} else {
		ring_id = ring_config->start_ring_id + ring_num;
	}

	return ring_id;
}

static struct hal_srng *hal_get_srng(struct hal_soc *hal, int ring_id)
{
	/* TODO: Should we allocate srng structures dynamically? */
	return &(hal->srng_list[ring_id]);
}

#define HP_OFFSET_IN_REG_START 1
#define OFFSET_FROM_HP_TO_TP 4
static void hal_update_srng_hp_tp_address(struct hal_soc *hal_soc,
					  int shadow_config_index,
					  int ring_type,
					  int ring_num)
{
	struct hal_srng *srng;
	int ring_id;
	struct hal_hw_srng_config *ring_config =
		HAL_SRNG_CONFIG(hal_soc, ring_type);

	ring_id = hal_get_srng_ring_id(hal_soc, ring_type, ring_num, 0);
	if (ring_id < 0)
		return;

	srng = hal_get_srng(hal_soc, ring_id);

	if (ring_config->ring_dir == HAL_SRNG_DST_RING) {
		srng->u.dst_ring.tp_addr = SHADOW_REGISTER(shadow_config_index)
			+ hal_soc->dev_base_addr;
		hal_debug("tp_addr=%pK dev base addr %pK index %u",
			  srng->u.dst_ring.tp_addr, hal_soc->dev_base_addr,
			  shadow_config_index);
	} else {
		srng->u.src_ring.hp_addr = SHADOW_REGISTER(shadow_config_index)
			+ hal_soc->dev_base_addr;
		hal_debug("hp_addr=%pK dev base addr %pK index %u",
			  srng->u.src_ring.hp_addr,
			  hal_soc->dev_base_addr, shadow_config_index);
	}

}

#ifdef GENERIC_SHADOW_REGISTER_ACCESS_ENABLE
void hal_set_one_target_reg_config(struct hal_soc *hal,
				   uint32_t target_reg_offset,
				   int list_index)
{
	int i = list_index;

	qdf_assert_always(i < MAX_GENERIC_SHADOW_REG);
	hal->list_shadow_reg_config[i].target_register =
		target_reg_offset;
	hal->num_generic_shadow_regs_configured++;
}

qdf_export_symbol(hal_set_one_target_reg_config);

#define REO_R0_DESTINATION_RING_CTRL_ADDR_OFFSET 0x4
#define MAX_REO_REMAP_SHADOW_REGS 4
QDF_STATUS hal_set_shadow_regs(void *hal_soc)
{
	uint32_t target_reg_offset;
	struct hal_soc *hal = (struct hal_soc *)hal_soc;
	int i;
	struct hal_hw_srng_config *srng_config =
		&hal->hw_srng_table[WBM2SW_RELEASE];

	target_reg_offset =
		HWIO_REO_R0_DESTINATION_RING_CTRL_IX_0_ADDR(
			SEQ_WCSS_UMAC_REO_REG_OFFSET);

	for (i = 0; i < MAX_REO_REMAP_SHADOW_REGS; i++) {
		hal_set_one_target_reg_config(hal, target_reg_offset, i);
		target_reg_offset += REO_R0_DESTINATION_RING_CTRL_ADDR_OFFSET;
	}

	target_reg_offset = srng_config->reg_start[HP_OFFSET_IN_REG_START];
	target_reg_offset += (srng_config->reg_size[HP_OFFSET_IN_REG_START]
			      * HAL_IPA_TX_COMP_RING_IDX);

	hal_set_one_target_reg_config(hal, target_reg_offset, i);
	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(hal_set_shadow_regs);

QDF_STATUS hal_construct_shadow_regs(void *hal_soc)
{
	struct hal_soc *hal = (struct hal_soc *)hal_soc;
	int shadow_config_index = hal->num_shadow_registers_configured;
	int i;
	int num_regs = hal->num_generic_shadow_regs_configured;

	for (i = 0; i < num_regs; i++) {
		qdf_assert_always(shadow_config_index < MAX_SHADOW_REGISTERS);
		hal->shadow_config[shadow_config_index].addr =
			hal->list_shadow_reg_config[i].target_register;
		hal->list_shadow_reg_config[i].shadow_config_index =
			shadow_config_index;
		hal->list_shadow_reg_config[i].va =
			SHADOW_REGISTER(shadow_config_index) +
			(uintptr_t)hal->dev_base_addr;
		hal_debug("target_reg %x, shadow register 0x%x shadow_index 0x%x",
			  hal->shadow_config[shadow_config_index].addr,
			  SHADOW_REGISTER(shadow_config_index),
			  shadow_config_index);
		shadow_config_index++;
		hal->num_shadow_registers_configured++;
	}
	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(hal_construct_shadow_regs);
#endif

QDF_STATUS hal_set_one_shadow_config(void *hal_soc,
				     int ring_type,
				     int ring_num)
{
	uint32_t target_register;
	struct hal_soc *hal = (struct hal_soc *)hal_soc;
	struct hal_hw_srng_config *srng_config = &hal->hw_srng_table[ring_type];
	int shadow_config_index = hal->num_shadow_registers_configured;

	if (shadow_config_index >= MAX_SHADOW_REGISTERS) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_RESOURCES;
	}

	hal->num_shadow_registers_configured++;

	target_register = srng_config->reg_start[HP_OFFSET_IN_REG_START];
	target_register += (srng_config->reg_size[HP_OFFSET_IN_REG_START]
			    *ring_num);

	/* if the ring is a dst ring, we need to shadow the tail pointer */
	if (srng_config->ring_dir == HAL_SRNG_DST_RING)
		target_register += OFFSET_FROM_HP_TO_TP;

	hal->shadow_config[shadow_config_index].addr = target_register;

	/* update hp/tp addr in the hal_soc structure*/
	hal_update_srng_hp_tp_address(hal_soc, shadow_config_index, ring_type,
				      ring_num);

	hal_debug("target_reg %x, shadow register 0x%x shadow_index 0x%x, ring_type %d, ring num %d",
		  target_register,
		  SHADOW_REGISTER(shadow_config_index),
		  shadow_config_index,
		  ring_type, ring_num);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(hal_set_one_shadow_config);

QDF_STATUS hal_construct_srng_shadow_regs(void *hal_soc)
{
	int ring_type, ring_num;
	struct hal_soc *hal = (struct hal_soc *)hal_soc;

	for (ring_type = 0; ring_type < MAX_RING_TYPES; ring_type++) {
		struct hal_hw_srng_config *srng_config =
			&hal->hw_srng_table[ring_type];

		if (ring_type == CE_SRC ||
		    ring_type == CE_DST ||
		    ring_type == CE_DST_STATUS)
			continue;

		if (srng_config->lmac_ring)
			continue;

		for (ring_num = 0; ring_num < srng_config->max_rings;
		     ring_num++)
			hal_set_one_shadow_config(hal_soc, ring_type, ring_num);
	}

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(hal_construct_srng_shadow_regs);

void hal_get_shadow_config(void *hal_soc,
	struct pld_shadow_reg_v2_cfg **shadow_config,
	int *num_shadow_registers_configured)
{
	struct hal_soc *hal = (struct hal_soc *)hal_soc;

	*shadow_config = hal->shadow_config;
	*num_shadow_registers_configured =
		hal->num_shadow_registers_configured;
}

qdf_export_symbol(hal_get_shadow_config);


static void hal_validate_shadow_register(struct hal_soc *hal,
				  uint32_t *destination,
				  uint32_t *shadow_address)
{
	unsigned int index;
	uint32_t *shadow_0_offset = SHADOW_REGISTER(0) + hal->dev_base_addr;
	int destination_ba_offset =
		((char *)destination) - (char *)hal->dev_base_addr;

	index =	shadow_address - shadow_0_offset;

	if (index >= MAX_SHADOW_REGISTERS) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: index %x out of bounds", __func__, index);
		goto error;
	} else if (hal->shadow_config[index].addr != destination_ba_offset) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: sanity check failure, expected %x, found %x",
			__func__, destination_ba_offset,
			hal->shadow_config[index].addr);
		goto error;
	}
	return;
error:
	qdf_print("baddr %pK, desination %pK, shadow_address %pK s0offset %pK index %x",
		  hal->dev_base_addr, destination, shadow_address,
		  shadow_0_offset, index);
	QDF_BUG(0);
	return;
}

static void hal_target_based_configure(struct hal_soc *hal)
{
	/**
	 * Indicate Initialization of srngs to avoid force wake
	 * as umac power collapse is not enabled yet
	 */
	hal->init_phase = true;

	switch (hal->target_type) {
#ifdef QCA_WIFI_QCA6290
	case TARGET_TYPE_QCA6290:
		hal->use_register_windowing = true;
		hal_qca6290_attach(hal);
	break;
#endif
#ifdef QCA_WIFI_QCA6390
	case TARGET_TYPE_QCA6390:
		hal->use_register_windowing = true;
		hal_qca6390_attach(hal);
	break;
#endif
#ifdef QCA_WIFI_QCA6490
	case TARGET_TYPE_QCA6490:
		hal->use_register_windowing = true;
		hal_qca6490_attach(hal);
		hal->init_phase = false;
	break;
#endif
#ifdef QCA_WIFI_QCA6750
		case TARGET_TYPE_QCA6750:
			hal->use_register_windowing = true;
			hal->static_window_map = true;
			hal_qca6750_attach(hal);
		break;
#endif
#if defined(QCA_WIFI_QCA8074) && defined(WIFI_TARGET_TYPE_3_0)
	case TARGET_TYPE_QCA8074:
		hal_qca8074_attach(hal);
	break;
#endif

#if defined(QCA_WIFI_QCA8074V2)
	case TARGET_TYPE_QCA8074V2:
		hal_qca8074v2_attach(hal);
	break;
#endif

#if defined(QCA_WIFI_QCA6018)
	case TARGET_TYPE_QCA6018:
		hal_qca8074v2_attach(hal);
	break;
#endif

#if defined(QCA_WIFI_QCN9100)
	case TARGET_TYPE_QCN9100:
		hal->use_register_windowing = true;
		/*
		 * Static window map  is enabled for qcn9000 to use 2mb bar
		 * size and use multiple windows to write into registers.
		 */
		hal->static_window_map = true;
		hal_qcn9100_attach(hal);
		break;
#endif

#ifdef QCA_WIFI_QCN9000
	case TARGET_TYPE_QCN9000:
		hal->use_register_windowing = true;
		/*
		 * Static window map  is enabled for qcn9000 to use 2mb bar
		 * size and use multiple windows to write into registers.
		 */
		hal->static_window_map = true;
		hal_qcn9000_attach(hal);
	break;
#endif
#ifdef QCA_WIFI_QCA5018
	case TARGET_TYPE_QCA5018:
		hal->use_register_windowing = true;
		hal->static_window_map = true;
		hal_qca5018_attach(hal);
	break;
#endif
	default:
	break;
	}
}

uint32_t hal_get_target_type(hal_soc_handle_t hal_soc_hdl)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;
	struct hif_target_info *tgt_info =
		hif_get_target_info_handle(hal_soc->hif_handle);

	return tgt_info->target_type;
}

qdf_export_symbol(hal_get_target_type);

#ifdef FEATURE_HAL_DELAYED_REG_WRITE
#ifdef MEMORY_DEBUG
/*
 * Length of the queue(array) used to hold delayed register writes.
 * Must be a multiple of 2.
 */
#define HAL_REG_WRITE_QUEUE_LEN 128
#else
#define HAL_REG_WRITE_QUEUE_LEN 32
#endif

/**
 * hal_is_reg_write_tput_level_high() - throughput level for delayed reg writes
 * @hal: hal_soc pointer
 *
 * Return: true if throughput is high, else false.
 */
static inline bool hal_is_reg_write_tput_level_high(struct hal_soc *hal)
{
	int bw_level = hif_get_bandwidth_level(hal->hif_handle);

	return (bw_level >= PLD_BUS_WIDTH_MEDIUM) ? true : false;
}

/**
 * hal_process_reg_write_q_elem() - process a regiter write queue element
 * @hal: hal_soc pointer
 * @q_elem: pointer to hal regiter write queue element
 *
 * Return: The value which was written to the address
 */
static uint32_t
hal_process_reg_write_q_elem(struct hal_soc *hal,
			     struct hal_reg_write_q_elem *q_elem)
{
	struct hal_srng *srng = q_elem->srng;
	uint32_t write_val;

	SRNG_LOCK(&srng->lock);

	srng->reg_write_in_progress = false;
	srng->wstats.dequeues++;

	if (srng->ring_dir == HAL_SRNG_SRC_RING) {
		q_elem->dequeue_val = srng->u.src_ring.hp;
		hal_write_address_32_mb(hal,
					srng->u.src_ring.hp_addr,
					srng->u.src_ring.hp, false);
		write_val = srng->u.src_ring.hp;
	} else {
		q_elem->dequeue_val = srng->u.dst_ring.tp;
		hal_write_address_32_mb(hal,
					srng->u.dst_ring.tp_addr,
					srng->u.dst_ring.tp, false);
		write_val = srng->u.dst_ring.tp;
	}

	q_elem->valid = 0;
	SRNG_UNLOCK(&srng->lock);

	return write_val;
}

/**
 * hal_reg_write_fill_sched_delay_hist() - fill reg write delay histogram in hal
 * @hal: hal_soc pointer
 * @delay: delay in us
 *
 * Return: None
 */
static inline void hal_reg_write_fill_sched_delay_hist(struct hal_soc *hal,
						       uint64_t delay_us)
{
	uint32_t *hist;

	hist = hal->stats.wstats.sched_delay;

	if (delay_us < 100)
		hist[REG_WRITE_SCHED_DELAY_SUB_100us]++;
	else if (delay_us < 1000)
		hist[REG_WRITE_SCHED_DELAY_SUB_1000us]++;
	else if (delay_us < 5000)
		hist[REG_WRITE_SCHED_DELAY_SUB_5000us]++;
	else
		hist[REG_WRITE_SCHED_DELAY_GT_5000us]++;
}

/**
 * hal_reg_write_work() - Worker to process delayed writes
 * @arg: hal_soc pointer
 *
 * Return: None
 */
static void hal_reg_write_work(void *arg)
{
	int32_t q_depth, write_val;
	struct hal_soc *hal = arg;
	struct hal_reg_write_q_elem *q_elem;
	uint64_t delta_us;
	uint8_t ring_id;
	uint32_t *addr;

	q_elem = &hal->reg_write_queue[(hal->read_idx)];
	q_elem->work_scheduled_time = qdf_get_log_timestamp();

	/* Make sure q_elem consistent in the memory for multi-cores */
	qdf_rmb();
	if (!q_elem->valid)
		return;

	q_depth = qdf_atomic_read(&hal->stats.wstats.q_depth);
	if (q_depth > hal->stats.wstats.max_q_depth)
		hal->stats.wstats.max_q_depth =  q_depth;

	if (hif_prevent_link_low_power_states(hal->hif_handle)) {
		hal->stats.wstats.prevent_l1_fails++;
		return;
	}

	while (true) {
		qdf_rmb();
		if (!q_elem->valid)
			break;

		q_elem->dequeue_time = qdf_get_log_timestamp();
		ring_id = q_elem->srng->ring_id;
		addr = q_elem->addr;
		delta_us = qdf_log_timestamp_to_usecs(q_elem->dequeue_time -
						      q_elem->enqueue_time);
		hal_reg_write_fill_sched_delay_hist(hal, delta_us);

		hal->stats.wstats.dequeues++;
		qdf_atomic_dec(&hal->stats.wstats.q_depth);

		write_val = hal_process_reg_write_q_elem(hal, q_elem);
		hal_verbose_debug("read_idx %u srng 0x%x, addr 0x%pK dequeue_val %u sched delay %llu us",
				  hal->read_idx, ring_id, addr, write_val, delta_us);

		qdf_atomic_dec(&hal->active_work_cnt);
		hal->read_idx = (hal->read_idx + 1) &
					(HAL_REG_WRITE_QUEUE_LEN - 1);
		q_elem = &hal->reg_write_queue[(hal->read_idx)];
	}

	hif_allow_link_low_power_states(hal->hif_handle);
}

static void __hal_flush_reg_write_work(struct hal_soc *hal)
{
	qdf_cancel_work(&hal->reg_write_work);
	qdf_flush_work(&hal->reg_write_work);
	qdf_flush_workqueue(0, hal->reg_write_wq);
}

void hal_flush_reg_write_work(hal_soc_handle_t hal_handle)
{	__hal_flush_reg_write_work((struct hal_soc *)hal_handle);
}

/**
 * hal_reg_write_enqueue() - enqueue register writes into kworker
 * @hal_soc: hal_soc pointer
 * @srng: srng pointer
 * @addr: iomem address of regiter
 * @value: value to be written to iomem address
 *
 * This function executes from within the SRNG LOCK
 *
 * Return: None
 */
static void hal_reg_write_enqueue(struct hal_soc *hal_soc,
				  struct hal_srng *srng,
				  void __iomem *addr,
				  uint32_t value)
{
	struct hal_reg_write_q_elem *q_elem;
	uint32_t write_idx;

	if (srng->reg_write_in_progress) {
		hal_verbose_debug("Already in progress srng ring id 0x%x addr 0x%pK val %u",
				  srng->ring_id, addr, value);
		qdf_atomic_inc(&hal_soc->stats.wstats.coalesces);
		srng->wstats.coalesces++;
		return;
	}

	write_idx = qdf_atomic_inc_return(&hal_soc->write_idx);

	write_idx = write_idx & (HAL_REG_WRITE_QUEUE_LEN - 1);

	q_elem = &hal_soc->reg_write_queue[write_idx];

	if (q_elem->valid) {
		hal_err("queue full");
		QDF_BUG(0);
		return;
	}

	qdf_atomic_inc(&hal_soc->stats.wstats.enqueues);
	srng->wstats.enqueues++;

	qdf_atomic_inc(&hal_soc->stats.wstats.q_depth);

	q_elem->srng = srng;
	q_elem->addr = addr;
	q_elem->enqueue_val = value;
	q_elem->enqueue_time = qdf_get_log_timestamp();

	/*
	 * Before the valid flag is set to true, all the other
	 * fields in the q_elem needs to be updated in memory.
	 * Else there is a chance that the dequeuing worker thread
	 * might read stale entries and process incorrect srng.
	 */
	qdf_wmb();
	q_elem->valid = true;

	/*
	 * After all other fields in the q_elem has been updated
	 * in memory successfully, the valid flag needs to be updated
	 * in memory in time too.
	 * Else there is a chance that the dequeuing worker thread
	 * might read stale valid flag and the work will be bypassed
	 * for this round. And if there is no other work scheduled
	 * later, this hal register writing won't be updated any more.
	 */
	qdf_wmb();

	srng->reg_write_in_progress  = true;
	qdf_atomic_inc(&hal_soc->active_work_cnt);

	hal_verbose_debug("write_idx %u srng ring id 0x%x addr 0x%pK val %u",
			  write_idx, srng->ring_id, addr, value);

	qdf_queue_work(hal_soc->qdf_dev, hal_soc->reg_write_wq,
		       &hal_soc->reg_write_work);
}

void hal_delayed_reg_write(struct hal_soc *hal_soc,
			   struct hal_srng *srng,
			   void __iomem *addr,
			   uint32_t value)
{
	if (pld_is_device_awake(hal_soc->qdf_dev->dev) ||
	    hal_is_reg_write_tput_level_high(hal_soc)) {
		qdf_atomic_inc(&hal_soc->stats.wstats.direct);
		srng->wstats.direct++;
		hal_write_address_32_mb(hal_soc, addr, value, false);
	} else {
		hal_reg_write_enqueue(hal_soc, srng, addr, value);
	}
}

/**
 * hal_delayed_reg_write_init() - Initialization function for delayed reg writes
 * @hal_soc: hal_soc pointer
 *
 * Initialize main data structures to process register writes in a delayed
 * workqueue.
 *
 * Return: QDF_STATUS_SUCCESS on success else a QDF error.
 */
static QDF_STATUS hal_delayed_reg_write_init(struct hal_soc *hal)
{
	hal->reg_write_wq =
		qdf_alloc_high_prior_ordered_workqueue("hal_register_write_wq");
	qdf_create_work(0, &hal->reg_write_work, hal_reg_write_work, hal);
	hal->reg_write_queue = qdf_mem_malloc(HAL_REG_WRITE_QUEUE_LEN *
					      sizeof(*hal->reg_write_queue));
	if (!hal->reg_write_queue) {
		hal_err("unable to allocate memory");
		QDF_BUG(0);
		return QDF_STATUS_E_NOMEM;
	}

	/* Initial value of indices */
	hal->read_idx = 0;
	qdf_atomic_set(&hal->write_idx, -1);
	return QDF_STATUS_SUCCESS;
}

/**
 * hal_delayed_reg_write_deinit() - De-Initialize delayed reg write processing
 * @hal_soc: hal_soc pointer
 *
 * De-initialize main data structures to process register writes in a delayed
 * workqueue.
 *
 * Return: None
 */
static void hal_delayed_reg_write_deinit(struct hal_soc *hal)
{
	__hal_flush_reg_write_work(hal);
	qdf_destroy_workqueue(0, hal->reg_write_wq);
	qdf_mem_free(hal->reg_write_queue);
}

static inline
char *hal_fill_reg_write_srng_stats(struct hal_srng *srng,
				    char *buf, qdf_size_t size)
{
	qdf_scnprintf(buf, size, "enq %u deq %u coal %u direct %u",
		      srng->wstats.enqueues, srng->wstats.dequeues,
		      srng->wstats.coalesces, srng->wstats.direct);
	return buf;
}

/* bytes for local buffer */
#define HAL_REG_WRITE_SRNG_STATS_LEN 100

void hal_dump_reg_write_srng_stats(hal_soc_handle_t hal_soc_hdl)
{
	struct hal_srng *srng;
	char buf[HAL_REG_WRITE_SRNG_STATS_LEN];
	struct hal_soc *hal = (struct hal_soc *)hal_soc_hdl;

	srng = hal_get_srng(hal, HAL_SRNG_SW2TCL1);
	hal_debug("SW2TCL1: %s",
		  hal_fill_reg_write_srng_stats(srng, buf, sizeof(buf)));

	srng = hal_get_srng(hal, HAL_SRNG_WBM2SW0_RELEASE);
	hal_debug("WBM2SW0: %s",
		  hal_fill_reg_write_srng_stats(srng, buf, sizeof(buf)));

	srng = hal_get_srng(hal, HAL_SRNG_REO2SW1);
	hal_debug("REO2SW1: %s",
		  hal_fill_reg_write_srng_stats(srng, buf, sizeof(buf)));

	srng = hal_get_srng(hal, HAL_SRNG_REO2SW2);
	hal_debug("REO2SW2: %s",
		  hal_fill_reg_write_srng_stats(srng, buf, sizeof(buf)));

	srng = hal_get_srng(hal, HAL_SRNG_REO2SW3);
	hal_debug("REO2SW3: %s",
		  hal_fill_reg_write_srng_stats(srng, buf, sizeof(buf)));
}

void hal_dump_reg_write_stats(hal_soc_handle_t hal_soc_hdl)
{
	uint32_t *hist;
	struct hal_soc *hal = (struct hal_soc *)hal_soc_hdl;

	hist = hal->stats.wstats.sched_delay;

	hal_debug("enq %u deq %u coal %u direct %u q_depth %u max_q %u sched-delay hist %u %u %u %u",
		  qdf_atomic_read(&hal->stats.wstats.enqueues),
		  hal->stats.wstats.dequeues,
		  qdf_atomic_read(&hal->stats.wstats.coalesces),
		  qdf_atomic_read(&hal->stats.wstats.direct),
		  qdf_atomic_read(&hal->stats.wstats.q_depth),
		  hal->stats.wstats.max_q_depth,
		  hist[REG_WRITE_SCHED_DELAY_SUB_100us],
		  hist[REG_WRITE_SCHED_DELAY_SUB_1000us],
		  hist[REG_WRITE_SCHED_DELAY_SUB_5000us],
		  hist[REG_WRITE_SCHED_DELAY_GT_5000us]);
}

int hal_get_reg_write_pending_work(void *hal_soc)
{
	struct hal_soc *hal = (struct hal_soc *)hal_soc;

	return qdf_atomic_read(&hal->active_work_cnt);
}

#else
static inline QDF_STATUS hal_delayed_reg_write_init(struct hal_soc *hal)
{
	return QDF_STATUS_SUCCESS;
}

static inline void hal_delayed_reg_write_deinit(struct hal_soc *hal)
{
}
#endif

/**
 * hal_attach - Initialize HAL layer
 * @hif_handle: Opaque HIF handle
 * @qdf_dev: QDF device
 *
 * Return: Opaque HAL SOC handle
 *		 NULL on failure (if given ring is not available)
 *
 * This function should be called as part of HIF initialization (for accessing
 * copy engines). DP layer will get hal_soc handle using hif_get_hal_handle()
 *
 */
void *hal_attach(struct hif_opaque_softc *hif_handle, qdf_device_t qdf_dev)
{
	struct hal_soc *hal;
	int i;

	hal = qdf_mem_malloc(sizeof(*hal));

	if (!hal) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: hal_soc allocation failed", __func__);
		goto fail0;
	}
	hal->hif_handle = hif_handle;
	hal->dev_base_addr = hif_get_dev_ba(hif_handle); /* UMAC */
	hal->dev_base_addr_ce = hif_get_dev_ba_ce(hif_handle); /* CE */
	hal->qdf_dev = qdf_dev;
	hal->shadow_rdptr_mem_vaddr = (uint32_t *)qdf_mem_alloc_consistent(
		qdf_dev, qdf_dev->dev, sizeof(*(hal->shadow_rdptr_mem_vaddr)) *
		HAL_SRNG_ID_MAX, &(hal->shadow_rdptr_mem_paddr));
	if (!hal->shadow_rdptr_mem_paddr) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: hal->shadow_rdptr_mem_paddr allocation failed",
			__func__);
		goto fail1;
	}
	qdf_mem_zero(hal->shadow_rdptr_mem_vaddr,
		     sizeof(*(hal->shadow_rdptr_mem_vaddr)) * HAL_SRNG_ID_MAX);

	hal->shadow_wrptr_mem_vaddr =
		(uint32_t *)qdf_mem_alloc_consistent(qdf_dev, qdf_dev->dev,
		sizeof(*(hal->shadow_wrptr_mem_vaddr)) * HAL_MAX_LMAC_RINGS,
		&(hal->shadow_wrptr_mem_paddr));
	if (!hal->shadow_wrptr_mem_vaddr) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: hal->shadow_wrptr_mem_vaddr allocation failed",
			__func__);
		goto fail2;
	}
	qdf_mem_zero(hal->shadow_wrptr_mem_vaddr,
		sizeof(*(hal->shadow_wrptr_mem_vaddr)) * HAL_MAX_LMAC_RINGS);

	for (i = 0; i < HAL_SRNG_ID_MAX; i++) {
		hal->srng_list[i].initialized = 0;
		hal->srng_list[i].ring_id = i;
	}

	qdf_spinlock_create(&hal->register_access_lock);
	hal->register_window = 0;
	hal->target_type = hal_get_target_type(hal_soc_to_hal_soc_handle(hal));

	hal_target_based_configure(hal);

	hal_reg_write_fail_history_init(hal);

	qdf_minidump_log(hal, sizeof(*hal), "hal_soc");

	qdf_atomic_init(&hal->active_work_cnt);
	hal_delayed_reg_write_init(hal);

	return (void *)hal;

fail2:
	qdf_mem_free_consistent(qdf_dev, qdf_dev->dev,
		sizeof(*(hal->shadow_rdptr_mem_vaddr)) * HAL_SRNG_ID_MAX,
		hal->shadow_rdptr_mem_vaddr, hal->shadow_rdptr_mem_paddr, 0);
fail1:
	qdf_mem_free(hal);
fail0:
	return NULL;
}
qdf_export_symbol(hal_attach);

/**
 * hal_mem_info - Retrieve hal memory base address
 *
 * @hal_soc: Opaque HAL SOC handle
 * @mem: pointer to structure to be updated with hal mem info
 */
void hal_get_meminfo(hal_soc_handle_t hal_soc_hdl, struct hal_mem_info *mem)
{
	struct hal_soc *hal = (struct hal_soc *)hal_soc_hdl;
	mem->dev_base_addr = (void *)hal->dev_base_addr;
        mem->shadow_rdptr_mem_vaddr = (void *)hal->shadow_rdptr_mem_vaddr;
	mem->shadow_wrptr_mem_vaddr = (void *)hal->shadow_wrptr_mem_vaddr;
        mem->shadow_rdptr_mem_paddr = (void *)hal->shadow_rdptr_mem_paddr;
	mem->shadow_wrptr_mem_paddr = (void *)hal->shadow_wrptr_mem_paddr;
	hif_read_phy_mem_base((void *)hal->hif_handle,
			      (qdf_dma_addr_t *)&mem->dev_base_paddr);
	return;
}
qdf_export_symbol(hal_get_meminfo);

/**
 * hal_detach - Detach HAL layer
 * @hal_soc: HAL SOC handle
 *
 * Return: Opaque HAL SOC handle
 *		 NULL on failure (if given ring is not available)
 *
 * This function should be called as part of HIF initialization (for accessing
 * copy engines). DP layer will get hal_soc handle using hif_get_hal_handle()
 *
 */
extern void hal_detach(void *hal_soc)
{
	struct hal_soc *hal = (struct hal_soc *)hal_soc;

	hal_delayed_reg_write_deinit(hal);

	qdf_mem_free_consistent(hal->qdf_dev, hal->qdf_dev->dev,
		sizeof(*(hal->shadow_rdptr_mem_vaddr)) * HAL_SRNG_ID_MAX,
		hal->shadow_rdptr_mem_vaddr, hal->shadow_rdptr_mem_paddr, 0);
	qdf_mem_free_consistent(hal->qdf_dev, hal->qdf_dev->dev,
		sizeof(*(hal->shadow_wrptr_mem_vaddr)) * HAL_MAX_LMAC_RINGS,
		hal->shadow_wrptr_mem_vaddr, hal->shadow_wrptr_mem_paddr, 0);
	qdf_minidump_remove(hal);
	qdf_mem_free(hal);

	return;
}
qdf_export_symbol(hal_detach);

/**
 * hal_ce_dst_setup - Initialize CE destination ring registers
 * @hal_soc: HAL SOC handle
 * @srng: SRNG ring pointer
 */
static inline void hal_ce_dst_setup(struct hal_soc *hal, struct hal_srng *srng,
				    int ring_num)
{
	uint32_t reg_val = 0;
	uint32_t reg_addr;
	struct hal_hw_srng_config *ring_config =
		HAL_SRNG_CONFIG(hal, CE_DST);

	/* set DEST_MAX_LENGTH according to ce assignment */
	reg_addr = HWIO_WFSS_CE_CHANNEL_DST_R0_DEST_CTRL_ADDR(
			ring_config->reg_start[R0_INDEX] +
			(ring_num * ring_config->reg_size[R0_INDEX]));

	reg_val = HAL_REG_READ(hal, reg_addr);
	reg_val &= ~HWIO_WFSS_CE_CHANNEL_DST_R0_DEST_CTRL_DEST_MAX_LENGTH_BMSK;
	reg_val |= srng->u.dst_ring.max_buffer_length &
		HWIO_WFSS_CE_CHANNEL_DST_R0_DEST_CTRL_DEST_MAX_LENGTH_BMSK;
	HAL_REG_WRITE(hal, reg_addr, reg_val);

	if (srng->prefetch_timer) {
		reg_addr = HWIO_WFSS_CE_CHANNEL_DST_R0_DEST_RING_CONSUMER_PREFETCH_TIMER_ADDR(
				ring_config->reg_start[R0_INDEX] +
				(ring_num * ring_config->reg_size[R0_INDEX]));

		reg_val = HAL_REG_READ(hal, reg_addr);
		reg_val &= ~HWIO_WFSS_CE_CHANNEL_DST_R0_DEST_RING_CONSUMER_PREFETCH_TIMER_RMSK;
		reg_val |= srng->prefetch_timer;
		HAL_REG_WRITE(hal, reg_addr, reg_val);
		reg_val = HAL_REG_READ(hal, reg_addr);
	}

}

/**
 * hal_reo_read_write_ctrl_ix - Read or write REO_DESTINATION_RING_CTRL_IX
 * @hal: HAL SOC handle
 * @read: boolean value to indicate if read or write
 * @ix0: pointer to store IX0 reg value
 * @ix1: pointer to store IX1 reg value
 * @ix2: pointer to store IX2 reg value
 * @ix3: pointer to store IX3 reg value
 */
void hal_reo_read_write_ctrl_ix(hal_soc_handle_t hal_soc_hdl, bool read,
				uint32_t *ix0, uint32_t *ix1,
				uint32_t *ix2, uint32_t *ix3)
{
	uint32_t reg_offset;
	struct hal_soc *hal = (struct hal_soc *)hal_soc_hdl;

	if (read) {
		if (ix0) {
			reg_offset =
				HWIO_REO_R0_DESTINATION_RING_CTRL_IX_0_ADDR(
						SEQ_WCSS_UMAC_REO_REG_OFFSET);
			*ix0 = HAL_REG_READ(hal, reg_offset);
		}

		if (ix1) {
			reg_offset =
				HWIO_REO_R0_DESTINATION_RING_CTRL_IX_1_ADDR(
						SEQ_WCSS_UMAC_REO_REG_OFFSET);
			*ix1 = HAL_REG_READ(hal, reg_offset);
		}

		if (ix2) {
			reg_offset =
				HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_ADDR(
						SEQ_WCSS_UMAC_REO_REG_OFFSET);
			*ix2 = HAL_REG_READ(hal, reg_offset);
		}

		if (ix3) {
			reg_offset =
				HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_ADDR(
						SEQ_WCSS_UMAC_REO_REG_OFFSET);
			*ix3 = HAL_REG_READ(hal, reg_offset);
		}
	} else {
		if (ix0) {
			reg_offset =
				HWIO_REO_R0_DESTINATION_RING_CTRL_IX_0_ADDR(
						SEQ_WCSS_UMAC_REO_REG_OFFSET);
			HAL_REG_WRITE_CONFIRM_RETRY(hal, reg_offset,
						    *ix0, true);
		}

		if (ix1) {
			reg_offset =
				HWIO_REO_R0_DESTINATION_RING_CTRL_IX_1_ADDR(
						SEQ_WCSS_UMAC_REO_REG_OFFSET);
			HAL_REG_WRITE_CONFIRM_RETRY(hal, reg_offset,
						    *ix1, true);
		}

		if (ix2) {
			reg_offset =
				HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_ADDR(
						SEQ_WCSS_UMAC_REO_REG_OFFSET);
			HAL_REG_WRITE_CONFIRM_RETRY(hal, reg_offset,
						    *ix2, true);
		}

		if (ix3) {
			reg_offset =
				HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_ADDR(
						SEQ_WCSS_UMAC_REO_REG_OFFSET);
			HAL_REG_WRITE_CONFIRM_RETRY(hal, reg_offset,
						    *ix3, true);
		}
	}
}

/**
 * hal_srng_dst_set_hp_paddr() - Set physical address to dest ring head pointer
 * @srng: sring pointer
 * @paddr: physical address
 */
void hal_srng_dst_set_hp_paddr(struct hal_srng *srng,
			       uint64_t paddr)
{
	SRNG_DST_REG_WRITE(srng, HP_ADDR_LSB,
			   paddr & 0xffffffff);
	SRNG_DST_REG_WRITE(srng, HP_ADDR_MSB,
			   paddr >> 32);
}

/**
 * hal_srng_dst_init_hp() - Initialize destination ring head
 * pointer
 * @hal_soc: hal_soc handle
 * @srng: sring pointer
 * @vaddr: virtual address
 */
void hal_srng_dst_init_hp(struct hal_soc_handle *hal_soc,
			  struct hal_srng *srng,
			  uint32_t *vaddr)
{
	uint32_t reg_offset;
	struct hal_soc *hal = (struct hal_soc *)hal_soc;

	if (!srng)
		return;

	srng->u.dst_ring.hp_addr = vaddr;
	reg_offset = SRNG_DST_ADDR(srng, HP) - hal->dev_base_addr;
	HAL_REG_WRITE_CONFIRM_RETRY(
		hal, reg_offset, srng->u.dst_ring.cached_hp, true);

	if (vaddr) {
		*srng->u.dst_ring.hp_addr = srng->u.dst_ring.cached_hp;
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "hp_addr=%pK, cached_hp=%d, hp=%d",
			  (void *)srng->u.dst_ring.hp_addr,
			  srng->u.dst_ring.cached_hp,
			  *srng->u.dst_ring.hp_addr);
	}
}

/**
 * hal_srng_hw_init - Private function to initialize SRNG HW
 * @hal_soc: HAL SOC handle
 * @srng: SRNG ring pointer
 */
static inline void hal_srng_hw_init(struct hal_soc *hal,
	struct hal_srng *srng)
{
	if (srng->ring_dir == HAL_SRNG_SRC_RING)
		hal_srng_src_hw_init(hal, srng);
	else
		hal_srng_dst_hw_init(hal, srng);
}

#ifdef CONFIG_SHADOW_V2
#define ignore_shadow false
#define CHECK_SHADOW_REGISTERS true
#else
#define ignore_shadow true
#define CHECK_SHADOW_REGISTERS false
#endif

/**
 * hal_srng_setup - Initialize HW SRNG ring.
 * @hal_soc: Opaque HAL SOC handle
 * @ring_type: one of the types from hal_ring_type
 * @ring_num: Ring number if there are multiple rings of same type (staring
 * from 0)
 * @mac_id: valid MAC Id should be passed if ring type is one of lmac rings
 * @ring_params: SRNG ring params in hal_srng_params structure.

 * Callers are expected to allocate contiguous ring memory of size
 * 'num_entries * entry_size' bytes and pass the physical and virtual base
 * addresses through 'ring_base_paddr' and 'ring_base_vaddr' in
 * hal_srng_params structure. Ring base address should be 8 byte aligned
 * and size of each ring entry should be queried using the API
 * hal_srng_get_entrysize
 *
 * Return: Opaque pointer to ring on success
 *		 NULL on failure (if given ring is not available)
 */
void *hal_srng_setup(void *hal_soc, int ring_type, int ring_num,
	int mac_id, struct hal_srng_params *ring_params)
{
	int ring_id;
	struct hal_soc *hal = (struct hal_soc *)hal_soc;
	struct hal_srng *srng;
	struct hal_hw_srng_config *ring_config =
		HAL_SRNG_CONFIG(hal, ring_type);
	void *dev_base_addr;
	int i;

	ring_id = hal_get_srng_ring_id(hal_soc, ring_type, ring_num, mac_id);
	if (ring_id < 0)
		return NULL;

	hal_verbose_debug("mac_id %d ring_id %d", mac_id, ring_id);

	srng = hal_get_srng(hal_soc, ring_id);

	if (srng->initialized) {
		hal_verbose_debug("Ring (ring_type, ring_num) already initialized");
		return NULL;
	}

	dev_base_addr = hal->dev_base_addr;
	srng->ring_id = ring_id;
	srng->ring_dir = ring_config->ring_dir;
	srng->ring_base_paddr = ring_params->ring_base_paddr;
	srng->ring_base_vaddr = ring_params->ring_base_vaddr;
	srng->entry_size = ring_config->entry_size;
	srng->num_entries = ring_params->num_entries;
	srng->ring_size = srng->num_entries * srng->entry_size;
	srng->ring_size_mask = srng->ring_size - 1;
	srng->msi_addr = ring_params->msi_addr;
	srng->msi_data = ring_params->msi_data;
	srng->intr_timer_thres_us = ring_params->intr_timer_thres_us;
	srng->intr_batch_cntr_thres_entries =
		ring_params->intr_batch_cntr_thres_entries;
	srng->prefetch_timer = ring_params->prefetch_timer;
	srng->hal_soc = hal_soc;

	for (i = 0 ; i < MAX_SRNG_REG_GROUPS; i++) {
		srng->hwreg_base[i] = dev_base_addr + ring_config->reg_start[i]
			+ (ring_num * ring_config->reg_size[i]);
	}

	/* Zero out the entire ring memory */
	qdf_mem_zero(srng->ring_base_vaddr, (srng->entry_size *
		srng->num_entries) << 2);

	srng->flags = ring_params->flags;
#ifdef BIG_ENDIAN_HOST
		/* TODO: See if we should we get these flags from caller */
	srng->flags |= HAL_SRNG_DATA_TLV_SWAP;
	srng->flags |= HAL_SRNG_MSI_SWAP;
	srng->flags |= HAL_SRNG_RING_PTR_SWAP;
#endif

	if (srng->ring_dir == HAL_SRNG_SRC_RING) {
		srng->u.src_ring.hp = 0;
		srng->u.src_ring.reap_hp = srng->ring_size -
			srng->entry_size;
		srng->u.src_ring.tp_addr =
			&(hal->shadow_rdptr_mem_vaddr[ring_id]);
		srng->u.src_ring.low_threshold =
			ring_params->low_threshold * srng->entry_size;
		if (ring_config->lmac_ring) {
			/* For LMAC rings, head pointer updates will be done
			 * through FW by writing to a shared memory location
			 */
			srng->u.src_ring.hp_addr =
				&(hal->shadow_wrptr_mem_vaddr[ring_id -
					HAL_SRNG_LMAC1_ID_START]);
			srng->flags |= HAL_SRNG_LMAC_RING;
		} else if (ignore_shadow || (srng->u.src_ring.hp_addr == 0)) {
			srng->u.src_ring.hp_addr =
				hal_get_window_address(hal,
						SRNG_SRC_ADDR(srng, HP));

			if (CHECK_SHADOW_REGISTERS) {
				QDF_TRACE(QDF_MODULE_ID_TXRX,
				    QDF_TRACE_LEVEL_ERROR,
				    "%s: Ring (%d, %d) missing shadow config",
				    __func__, ring_type, ring_num);
			}
		} else {
			hal_validate_shadow_register(hal,
						     SRNG_SRC_ADDR(srng, HP),
						     srng->u.src_ring.hp_addr);
		}
	} else {
		/* During initialization loop count in all the descriptors
		 * will be set to zero, and HW will set it to 1 on completing
		 * descriptor update in first loop, and increments it by 1 on
		 * subsequent loops (loop count wraps around after reaching
		 * 0xffff). The 'loop_cnt' in SW ring state is the expected
		 * loop count in descriptors updated by HW (to be processed
		 * by SW).
		 */
		srng->u.dst_ring.loop_cnt = 1;
		srng->u.dst_ring.tp = 0;
		srng->u.dst_ring.hp_addr =
			&(hal->shadow_rdptr_mem_vaddr[ring_id]);
		if (ring_config->lmac_ring) {
			/* For LMAC rings, tail pointer updates will be done
			 * through FW by writing to a shared memory location
			 */
			srng->u.dst_ring.tp_addr =
				&(hal->shadow_wrptr_mem_vaddr[ring_id -
				HAL_SRNG_LMAC1_ID_START]);
			srng->flags |= HAL_SRNG_LMAC_RING;
		} else if (ignore_shadow || srng->u.dst_ring.tp_addr == 0) {
			srng->u.dst_ring.tp_addr =
				hal_get_window_address(hal,
						SRNG_DST_ADDR(srng, TP));

			if (CHECK_SHADOW_REGISTERS) {
				QDF_TRACE(QDF_MODULE_ID_TXRX,
				    QDF_TRACE_LEVEL_ERROR,
				    "%s: Ring (%d, %d) missing shadow config",
				    __func__, ring_type, ring_num);
			}
		} else {
			hal_validate_shadow_register(hal,
						     SRNG_DST_ADDR(srng, TP),
						     srng->u.dst_ring.tp_addr);
		}
	}

	if (!(ring_config->lmac_ring)) {
		hal_srng_hw_init(hal, srng);

		if (ring_type == CE_DST) {
			srng->u.dst_ring.max_buffer_length = ring_params->max_buffer_length;
			hal_ce_dst_setup(hal, srng, ring_num);
		}
	}

	SRNG_LOCK_INIT(&srng->lock);

	srng->srng_event = 0;

	srng->initialized = true;

	return (void *)srng;
}
qdf_export_symbol(hal_srng_setup);

/**
 * hal_srng_cleanup - Deinitialize HW SRNG ring.
 * @hal_soc: Opaque HAL SOC handle
 * @hal_srng: Opaque HAL SRNG pointer
 */
void hal_srng_cleanup(void *hal_soc, hal_ring_handle_t hal_ring_hdl)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;
	SRNG_LOCK_DESTROY(&srng->lock);
	srng->initialized = 0;
}
qdf_export_symbol(hal_srng_cleanup);

/**
 * hal_srng_get_entrysize - Returns size of ring entry in bytes
 * @hal_soc: Opaque HAL SOC handle
 * @ring_type: one of the types from hal_ring_type
 *
 */
uint32_t hal_srng_get_entrysize(void *hal_soc, int ring_type)
{
	struct hal_soc *hal = (struct hal_soc *)hal_soc;
	struct hal_hw_srng_config *ring_config =
		HAL_SRNG_CONFIG(hal, ring_type);
	return ring_config->entry_size << 2;
}
qdf_export_symbol(hal_srng_get_entrysize);

/**
 * hal_srng_max_entries - Returns maximum possible number of ring entries
 * @hal_soc: Opaque HAL SOC handle
 * @ring_type: one of the types from hal_ring_type
 *
 * Return: Maximum number of entries for the given ring_type
 */
uint32_t hal_srng_max_entries(void *hal_soc, int ring_type)
{
	struct hal_soc *hal = (struct hal_soc *)hal_soc;
	struct hal_hw_srng_config *ring_config =
		HAL_SRNG_CONFIG(hal, ring_type);

	return ring_config->max_size / ring_config->entry_size;
}
qdf_export_symbol(hal_srng_max_entries);

enum hal_srng_dir hal_srng_get_dir(void *hal_soc, int ring_type)
{
	struct hal_soc *hal = (struct hal_soc *)hal_soc;
	struct hal_hw_srng_config *ring_config =
		HAL_SRNG_CONFIG(hal, ring_type);

	return ring_config->ring_dir;
}

/**
 * hal_srng_dump - Dump ring status
 * @srng: hal srng pointer
 */
void hal_srng_dump(struct hal_srng *srng)
{
	if (srng->ring_dir == HAL_SRNG_SRC_RING) {
		hal_debug("=== SRC RING %d ===", srng->ring_id);
		hal_debug("hp %u, reap_hp %u, tp %u, cached tp %u",
			  srng->u.src_ring.hp,
			  srng->u.src_ring.reap_hp,
			  *srng->u.src_ring.tp_addr,
			  srng->u.src_ring.cached_tp);
	} else {
		hal_debug("=== DST RING %d ===", srng->ring_id);
		hal_debug("tp %u, hp %u, cached tp %u, loop_cnt %u",
			  srng->u.dst_ring.tp,
			  *srng->u.dst_ring.hp_addr,
			  srng->u.dst_ring.cached_hp,
			  srng->u.dst_ring.loop_cnt);
	}
}

/**
 * hal_get_srng_params - Retrieve SRNG parameters for a given ring from HAL
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring: Ring pointer (Source or Destination ring)
 * @ring_params: SRNG parameters will be returned through this structure
 */
extern void hal_get_srng_params(hal_soc_handle_t hal_soc_hdl,
				hal_ring_handle_t hal_ring_hdl,
				struct hal_srng_params *ring_params)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;
	int i =0;
	ring_params->ring_id = srng->ring_id;
	ring_params->ring_dir = srng->ring_dir;
	ring_params->entry_size = srng->entry_size;

	ring_params->ring_base_paddr = srng->ring_base_paddr;
	ring_params->ring_base_vaddr = srng->ring_base_vaddr;
	ring_params->num_entries = srng->num_entries;
	ring_params->msi_addr = srng->msi_addr;
	ring_params->msi_data = srng->msi_data;
	ring_params->intr_timer_thres_us = srng->intr_timer_thres_us;
	ring_params->intr_batch_cntr_thres_entries =
		srng->intr_batch_cntr_thres_entries;
	ring_params->low_threshold = srng->u.src_ring.low_threshold;
	ring_params->flags = srng->flags;
	ring_params->ring_id = srng->ring_id;
	for (i = 0 ; i < MAX_SRNG_REG_GROUPS; i++)
		ring_params->hwreg_base[i] = srng->hwreg_base[i];
}
qdf_export_symbol(hal_get_srng_params);

void hal_set_low_threshold(hal_ring_handle_t hal_ring_hdl,
				 uint32_t low_threshold)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;
	srng->u.src_ring.low_threshold = low_threshold * srng->entry_size;
}
qdf_export_symbol(hal_set_low_threshold);


#ifdef FORCE_WAKE
void hal_set_init_phase(hal_soc_handle_t soc, bool init_phase)
{
	struct hal_soc *hal_soc = (struct hal_soc *)soc;

	hal_soc->init_phase = init_phase;
}
#endif /* FORCE_WAKE */
