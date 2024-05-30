/*
 * Copyright (c) 2015-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

/**
 * DOC: hif_napi.c
 *
 * HIF NAPI interface implementation
 */

#include <linux/string.h> /* memset */

/* Linux headers */
#include <linux/cpumask.h>
#include <linux/cpufreq.h>
#include <linux/cpu.h>
#include <linux/topology.h>
#include <linux/interrupt.h>
#ifdef CONFIG_SCHED_CORE_CTL
#include <linux/sched/core_ctl.h>
#endif
#include <pld_common.h>
#include <linux/pm.h>

/* Driver headers */
#include <hif_napi.h>
#include <hif_debug.h>
#include <hif_io32.h>
#include <ce_api.h>
#include <ce_internal.h>
#include <hif_irq_affinity.h>
#include "qdf_cpuhp.h"
#include "qdf_module.h"
#include "qdf_net_if.h"
#include "qdf_dev.h"
#include "qdf_irq.h"

enum napi_decision_vector {
	HIF_NAPI_NOEVENT = 0,
	HIF_NAPI_INITED  = 1,
	HIF_NAPI_CONF_UP = 2
};
#define ENABLE_NAPI_MASK (HIF_NAPI_INITED | HIF_NAPI_CONF_UP)

#ifdef RECEIVE_OFFLOAD
/**
 * hif_rxthread_napi_poll() - dummy napi poll for rx_thread NAPI
 * @napi: Rx_thread NAPI
 * @budget: NAPI BUDGET
 *
 * Return: 0 as it is not supposed to be polled at all as it is not scheduled.
 */
static int hif_rxthread_napi_poll(struct napi_struct *napi, int budget)
{
	hif_err("This napi_poll should not be polled as we don't schedule it");
	QDF_ASSERT(0);
	return 0;
}

/**
 * hif_init_rx_thread_napi() - Initialize dummy Rx_thread NAPI
 * @napii: Handle to napi_info holding rx_thread napi
 *
 * Return: None
 */
static void hif_init_rx_thread_napi(struct qca_napi_info *napii)
{
	struct qdf_net_if *nd = (struct qdf_net_if *)&napii->rx_thread_netdev;

	qdf_net_if_create_dummy_if(nd);
	netif_napi_add_ni(&napii->rx_thread_netdev, &napii->rx_thread_napi,
		       hif_rxthread_napi_poll, 64);
	napi_enable(&napii->rx_thread_napi);
}

/**
 * hif_deinit_rx_thread_napi() - Deinitialize dummy Rx_thread NAPI
 * @napii: Handle to napi_info holding rx_thread napi
 *
 * Return: None
 */
static void hif_deinit_rx_thread_napi(struct qca_napi_info *napii)
{
	netif_napi_del(&napii->rx_thread_napi);
}
#else /* RECEIVE_OFFLOAD */
static void hif_init_rx_thread_napi(struct qca_napi_info *napii)
{
}

static void hif_deinit_rx_thread_napi(struct qca_napi_info *napii)
{
}
#endif

/**
 * hif_napi_create() - creates the NAPI structures for a given CE
 * @hif    : pointer to hif context
 * @pipe_id: the CE id on which the instance will be created
 * @poll   : poll function to be used for this NAPI instance
 * @budget : budget to be registered with the NAPI instance
 * @scale  : scale factor on the weight (to scaler budget to 1000)
 * @flags  : feature flags
 *
 * Description:
 *    Creates NAPI instances. This function is called
 *    unconditionally during initialization. It creates
 *    napi structures through the proper HTC/HIF calls.
 *    The structures are disabled on creation.
 *    Note that for each NAPI instance a separate dummy netdev is used
 *
 * Return:
 * < 0: error
 * = 0: <should never happen>
 * > 0: id of the created object (for multi-NAPI, number of objects created)
 */
int hif_napi_create(struct hif_opaque_softc   *hif_ctx,
		    int (*poll)(struct napi_struct *, int),
		    int                budget,
		    int                scale,
		    uint8_t            flags)
{
	int i;
	struct qca_napi_data *napid;
	struct qca_napi_info *napii;
	struct CE_state      *ce_state;
	struct hif_softc *hif = HIF_GET_SOFTC(hif_ctx);
	int    rc = 0;

	NAPI_DEBUG("-->(budget=%d, scale=%d)",
		   budget, scale);
	NAPI_DEBUG("hif->napi_data.state = 0x%08x",
		   hif->napi_data.state);
	NAPI_DEBUG("hif->napi_data.ce_map = 0x%08x",
		   hif->napi_data.ce_map);

	napid = &(hif->napi_data);
	if (0 == (napid->state &  HIF_NAPI_INITED)) {
		memset(napid, 0, sizeof(struct qca_napi_data));
		qdf_spinlock_create(&(napid->lock));

		napid->state |= HIF_NAPI_INITED;
		napid->flags = flags;

		rc = hif_napi_cpu_init(hif_ctx);
		if (rc != 0 && rc != -EALREADY) {
			hif_err("NAPI_initialization failed(rc=%d)", rc);
			rc = napid->ce_map;
			goto hnc_err;
		} else
			rc = 0;

		hif_debug("NAPI structures initialized, rc=%d", rc);
	}
	for (i = 0; i < hif->ce_count; i++) {
		ce_state = hif->ce_id_to_state[i];
		NAPI_DEBUG("ce %d: htt_rx=%d htt_tx=%d",
			   i, ce_state->htt_rx_data,
			   ce_state->htt_tx_data);
		if (ce_srng_based(hif))
			continue;

		if (!ce_state->htt_rx_data)
			continue;

		/* Now this is a CE where we need NAPI on */
		NAPI_DEBUG("Creating NAPI on pipe %d", i);
		napii = qdf_mem_malloc(sizeof(*napii));
		napid->napis[i] = napii;
		if (!napii) {
			rc = -ENOMEM;
			goto napii_free;
		}
	}

	for (i = 0; i < hif->ce_count; i++) {
		napii = napid->napis[i];
		if (!napii)
			continue;

		NAPI_DEBUG("initializing NAPI for pipe %d", i);
		memset(napii, 0, sizeof(struct qca_napi_info));
		napii->scale = scale;
		napii->id    = NAPI_PIPE2ID(i);
		napii->hif_ctx = hif_ctx;
		napii->irq   = pld_get_irq(hif->qdf_dev->dev, i);

		if (napii->irq < 0)
			hif_warn("bad IRQ value for CE %d: %d", i, napii->irq);

		qdf_net_if_create_dummy_if((struct qdf_net_if *)&napii->netdev);

		NAPI_DEBUG("adding napi=%pK to netdev=%pK (poll=%pK, bdgt=%d)",
			   &(napii->napi), &(napii->netdev), poll, budget);
		netif_napi_add_ni(&(napii->netdev), &(napii->napi),
				      poll, budget);

		NAPI_DEBUG("after napi_add");
		NAPI_DEBUG("napi=0x%pK, netdev=0x%pK",
			   &(napii->napi), &(napii->netdev));
		NAPI_DEBUG("napi.dev_list.prev=0x%pK, next=0x%pK",
			   napii->napi.dev_list.prev,
			   napii->napi.dev_list.next);
		NAPI_DEBUG("dev.napi_list.prev=0x%pK, next=0x%pK",
			   napii->netdev.napi_list.prev,
			   napii->netdev.napi_list.next);

		hif_init_rx_thread_napi(napii);
		napii->lro_ctx = qdf_lro_init();
		NAPI_DEBUG("Registering LRO for ce_id %d NAPI callback for %d lro_ctx %pK\n",
				i, napii->id, napii->lro_ctx);

		/* It is OK to change the state variable below without
		 * protection as there should be no-one around yet
		 */
		napid->ce_map |= (0x01 << i);
		hif_debug("NAPI id %d created for pipe %d", napii->id, i);
	}

	/* no ces registered with the napi */
	if (!ce_srng_based(hif) && napid->ce_map == 0) {
		hif_warn("no napis created for copy engines");
		rc = -EFAULT;
		goto napii_free;
	}

	NAPI_DEBUG("napi map = %x", napid->ce_map);
	NAPI_DEBUG("NAPI ids created for all applicable pipes");
	return napid->ce_map;

napii_free:
	for (i = 0; i < hif->ce_count; i++) {
		napii = napid->napis[i];
		napid->napis[i] = NULL;
		if (napii)
			qdf_mem_free(napii);
	}

hnc_err:
	NAPI_DEBUG("<--napi_instances_map=%x]", napid->ce_map);
	return rc;
}
qdf_export_symbol(hif_napi_create);

#ifdef RECEIVE_OFFLOAD
void hif_napi_rx_offld_flush_cb_register(struct hif_opaque_softc *hif_hdl,
					 void (offld_flush_handler)(void *))
{
	int i;
	struct CE_state *ce_state;
	struct hif_softc *scn = HIF_GET_SOFTC(hif_hdl);
	struct qca_napi_data *napid;
	struct qca_napi_info *napii;

	if (!scn) {
		hif_err("hif_state NULL!");
		QDF_ASSERT(0);
		return;
	}

	napid = hif_napi_get_all(hif_hdl);
	for (i = 0; i < scn->ce_count; i++) {
		ce_state = scn->ce_id_to_state[i];
		if (ce_state && (ce_state->htt_rx_data)) {
			napii = napid->napis[i];
			napii->offld_flush_cb = offld_flush_handler;
			hif_debug("Registering offload for ce_id %d NAPI callback for %d flush_cb %pK",
				i, napii->id, napii->offld_flush_cb);
		}
	}
}

void hif_napi_rx_offld_flush_cb_deregister(struct hif_opaque_softc *hif_hdl)
{
	int i;
	struct CE_state *ce_state;
	struct hif_softc *scn = HIF_GET_SOFTC(hif_hdl);
	struct qca_napi_data *napid;
	struct qca_napi_info *napii;

	if (!scn) {
		hif_err("hif_state NULL!");
		QDF_ASSERT(0);
		return;
	}

	napid = hif_napi_get_all(hif_hdl);
	for (i = 0; i < scn->ce_count; i++) {
		ce_state = scn->ce_id_to_state[i];
		if (ce_state && (ce_state->htt_rx_data)) {
			napii = napid->napis[i];
			hif_debug("deRegistering offld for ce_id %d NAPI callback for %d flush_cb %pK",
				 i, napii->id, napii->offld_flush_cb);
			/* Not required */
			napii->offld_flush_cb = NULL;
		}
	}
}
#endif /* RECEIVE_OFFLOAD */

/**
 *
 * hif_napi_destroy() - destroys the NAPI structures for a given instance
 * @hif   : pointer to hif context
 * @ce_id : the CE id whose napi instance will be destroyed
 * @force : if set, will destroy even if entry is active (de-activates)
 *
 * Description:
 *    Destroy a given NAPI instance. This function is called
 *    unconditionally during cleanup.
 *    Refuses to destroy an entry of it is still enabled (unless force=1)
 *    Marks the whole napi_data invalid if all instances are destroyed.
 *
 * Return:
 * -EINVAL: specific entry has not been created
 * -EPERM : specific entry is still active
 * 0 <    : error
 * 0 =    : success
 */
int hif_napi_destroy(struct hif_opaque_softc *hif_ctx,
		     uint8_t          id,
		     int              force)
{
	uint8_t ce = NAPI_ID2PIPE(id);
	int rc = 0;
	struct hif_softc *hif = HIF_GET_SOFTC(hif_ctx);

	NAPI_DEBUG("-->(id=%d, force=%d)", id, force);

	if (0 == (hif->napi_data.state & HIF_NAPI_INITED)) {
		hif_err("NAPI not initialized or entry %d not created", id);
		rc = -EINVAL;
	} else if (0 == (hif->napi_data.ce_map & (0x01 << ce))) {
		hif_err("NAPI instance %d (pipe %d) not created", id, ce);
		if (hif->napi_data.napis[ce])
			hif_err("memory allocated but ce_map not set %d (pipe %d)",
				id, ce);
		rc = -EINVAL;
	} else {
		struct qca_napi_data *napid;
		struct qca_napi_info *napii;

		napid = &(hif->napi_data);
		napii = napid->napis[ce];
		if (!napii) {
			if (napid->ce_map & (0x01 << ce))
				hif_err("napii & ce_map out of sync(ce %d)", ce);
			return -EINVAL;
		}


		if (hif->napi_data.state == HIF_NAPI_CONF_UP) {
			if (force) {
				napi_disable(&(napii->napi));
				hif_debug("NAPI entry %d force disabled", id);
				NAPI_DEBUG("NAPI %d force disabled", id);
			} else {
				hif_err("Cannot destroy active NAPI %d", id);
				rc = -EPERM;
			}
		}
		if (0 == rc) {
			NAPI_DEBUG("before napi_del");
			NAPI_DEBUG("napi.dlist.prv=0x%pK, next=0x%pK",
				  napii->napi.dev_list.prev,
				  napii->napi.dev_list.next);
			NAPI_DEBUG("dev.napi_l.prv=0x%pK, next=0x%pK",
				   napii->netdev.napi_list.prev,
				   napii->netdev.napi_list.next);

			qdf_lro_deinit(napii->lro_ctx);
			netif_napi_del(&(napii->napi));
			hif_deinit_rx_thread_napi(napii);

			napid->ce_map &= ~(0x01 << ce);
			napid->napis[ce] = NULL;
			napii->scale  = 0;
			qdf_mem_free(napii);
			hif_debug("NAPI %d destroyed", id);

			/* if there are no active instances and
			 * if they are all destroyed,
			 * set the whole structure to uninitialized state
			 */
			if (napid->ce_map == 0) {
				rc = hif_napi_cpu_deinit(hif_ctx);
				/* caller is tolerant to receiving !=0 rc */

				qdf_spinlock_destroy(&(napid->lock));
				memset(napid,
				       0, sizeof(struct qca_napi_data));
				hif_debug("no NAPI instances. Zapped");
			}
		}
	}

	return rc;
}
qdf_export_symbol(hif_napi_destroy);

#ifdef FEATURE_LRO
void *hif_napi_get_lro_info(struct hif_opaque_softc *hif_hdl, int napi_id)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_hdl);
	struct qca_napi_data *napid;
	struct qca_napi_info *napii;

	napid = &(scn->napi_data);
	napii = napid->napis[NAPI_ID2PIPE(napi_id)];

	if (napii)
		return napii->lro_ctx;
	return 0;
}
#endif

/**
 *
 * hif_napi_get_all() - returns the address of the whole HIF NAPI structure
 * @hif: pointer to hif context
 *
 * Description:
 *    Returns the address of the whole structure
 *
 * Return:
 *  <addr>: address of the whole HIF NAPI structure
 */
inline struct qca_napi_data *hif_napi_get_all(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *hif = HIF_GET_SOFTC(hif_ctx);

	return &(hif->napi_data);
}

struct qca_napi_info *hif_get_napi(int napi_id, struct qca_napi_data *napid)
{
	int id = NAPI_ID2PIPE(napi_id);

	return napid->napis[id];
}

/**
 *
 * hif_napi_event() - reacts to events that impact NAPI
 * @hif : pointer to hif context
 * @evnt: event that has been detected
 * @data: more data regarding the event
 *
 * Description:
 *   This function handles two types of events:
 *   1- Events that change the state of NAPI (enabled/disabled):
 *      {NAPI_EVT_INI_FILE, NAPI_EVT_CMD_STATE}
 *      The state is retrievable by "hdd_napi_enabled(-1)"
 *    - NAPI will be on if either INI file is on and it has not been disabled
 *                                by a subsequent vendor CMD,
 *                         or     it has been enabled by a vendor CMD.
 *   2- Events that change the CPU affinity of a NAPI instance/IRQ:
 *      {NAPI_EVT_TPUT_STATE, NAPI_EVT_CPU_STATE}
 *    - NAPI will support a throughput mode (HI/LO), kept at napid->napi_mode
 *    - NAPI will switch throughput mode based on hdd_napi_throughput_policy()
 *    - In LO tput mode, NAPI will yield control if its interrupts to the system
 *      management functions. However in HI throughput mode, NAPI will actively
 *      manage its interrupts/instances (by trying to disperse them out to
 *      separate performance cores).
 *    - CPU eligibility is kept up-to-date by NAPI_EVT_CPU_STATE events.
 *
 *    + In some cases (roaming peer management is the only case so far), a
 *      a client can trigger a "SERIALIZE" event. Basically, this means that the
 *      users is asking NAPI to go into a truly single execution context state.
 *      So, NAPI indicates to msm-irqbalancer that it wants to be blacklisted,
 *      (if called for the first time) and then moves all IRQs (for NAPI
 *      instances) to be collapsed to a single core. If called multiple times,
 *      it will just re-collapse the CPUs. This is because blacklist-on() API
 *      is reference-counted, and because the API has already been called.
 *
 *      Such a user, should call "DESERIALIZE" (NORMAL) event, to set NAPI to go
 *      to its "normal" operation. Optionally, they can give a timeout value (in
 *      multiples of BusBandwidthCheckPeriod -- 100 msecs by default). In this
 *      case, NAPI will just set the current throughput state to uninitialized
 *      and set the delay period. Once policy handler is called, it would skip
 *      applying the policy delay period times, and otherwise apply the policy.
 *
 * Return:
 *  < 0: some error
 *  = 0: event handled successfully
 */
int hif_napi_event(struct hif_opaque_softc *hif_ctx, enum qca_napi_event event,
		   void *data)
{
	int      rc = 0;
	uint32_t prev_state;
	int      i;
	bool state_changed;
	struct napi_struct *napi;
	struct hif_softc *hif = HIF_GET_SOFTC(hif_ctx);
	struct qca_napi_data *napid = &(hif->napi_data);
	enum qca_napi_tput_state tput_mode = QCA_NAPI_TPUT_UNINITIALIZED;
	enum {
		BLACKLIST_NOT_PENDING,
		BLACKLIST_ON_PENDING,
		BLACKLIST_OFF_PENDING
	     } blacklist_pending = BLACKLIST_NOT_PENDING;

	NAPI_DEBUG("%s: -->(event=%d, aux=%pK)", __func__, event, data);

	if (ce_srng_based(hif))
		return hif_exec_event(hif_ctx, event, data);

	if ((napid->state & HIF_NAPI_INITED) == 0) {
		NAPI_DEBUG("%s: got event when NAPI not initialized",
			   __func__);
		return -EINVAL;
	}
	qdf_spin_lock_bh(&(napid->lock));
	prev_state = napid->state;
	switch (event) {
	case NAPI_EVT_INI_FILE:
	case NAPI_EVT_CMD_STATE:
	case NAPI_EVT_INT_STATE: {
		int on = (data != ((void *)0));

		hif_debug("recved evnt: STATE_CMD %d; v = %d (state=0x%0x)",
			 event, on, prev_state);
		if (on)
			if (prev_state & HIF_NAPI_CONF_UP) {
				hif_debug("Duplicate NAPI conf ON msg");
			} else {
				hif_debug("Setting state to ON");
				napid->state |= HIF_NAPI_CONF_UP;
			}
		else /* off request */
			if (prev_state & HIF_NAPI_CONF_UP) {
				hif_debug("Setting state to OFF");
				napid->state &= ~HIF_NAPI_CONF_UP;
			} else {
				hif_debug("Duplicate NAPI conf OFF msg");
			}
		break;
	}
	/* case NAPI_INIT_FILE/CMD_STATE */

	case NAPI_EVT_CPU_STATE: {
		int cpu = ((unsigned long int)data >> 16);
		int val = ((unsigned long int)data & 0x0ff);

		NAPI_DEBUG("%s: evt=CPU_STATE on CPU %d value=%d",
			   __func__, cpu, val);

		/* state has already been set by hnc_cpu_notify_cb */
		if ((val == QCA_NAPI_CPU_DOWN) &&
		    (napid->napi_mode == QCA_NAPI_TPUT_HI) && /* we manage */
		    (napid->napi_cpu[cpu].napis != 0)) {
			NAPI_DEBUG("%s: Migrating NAPIs out of cpu %d",
				   __func__, cpu);
			rc = hif_napi_cpu_migrate(napid,
						  cpu,
						  HNC_ACT_RELOCATE);
			napid->napi_cpu[cpu].napis = 0;
		}
		/* in QCA_NAPI_TPUT_LO case, napis MUST == 0 */
		break;
	}

	case NAPI_EVT_TPUT_STATE: {
		tput_mode = (enum qca_napi_tput_state)data;
		if (tput_mode == QCA_NAPI_TPUT_LO) {
			/* from TPUT_HI -> TPUT_LO */
			NAPI_DEBUG("%s: Moving to napi_tput_LO state",
				   __func__);
			blacklist_pending = BLACKLIST_OFF_PENDING;
			/*
			 * Ideally we should "collapse" interrupts here, since
			 * we are "dispersing" interrupts in the "else" case.
			 * This allows the possibility that our interrupts may
			 * still be on the perf cluster the next time we enter
			 * high tput mode. However, the irq_balancer is free
			 * to move our interrupts to power cluster once
			 * blacklisting has been turned off in the "else" case.
			 */
		} else {
			/* from TPUT_LO -> TPUT->HI */
			NAPI_DEBUG("%s: Moving to napi_tput_HI state",
				   __func__);
			rc = hif_napi_cpu_migrate(napid,
						  HNC_ANY_CPU,
						  HNC_ACT_DISPERSE);

			blacklist_pending = BLACKLIST_ON_PENDING;
		}
		napid->napi_mode = tput_mode;
		break;
	}

	case NAPI_EVT_USR_SERIAL: {
		unsigned long users = (unsigned long)data;

		NAPI_DEBUG("%s: User forced SERIALIZATION; users=%ld",
			   __func__, users);

		rc = hif_napi_cpu_migrate(napid,
					  HNC_ANY_CPU,
					  HNC_ACT_COLLAPSE);
		if ((users == 0) && (rc == 0))
			blacklist_pending = BLACKLIST_ON_PENDING;
		break;
	}
	case NAPI_EVT_USR_NORMAL: {
		NAPI_DEBUG("%s: User forced DE-SERIALIZATION", __func__);
		if (!napid->user_cpu_affin_mask)
			blacklist_pending = BLACKLIST_OFF_PENDING;
		/*
		 * Deserialization timeout is handled at hdd layer;
		 * just mark current mode to uninitialized to ensure
		 * it will be set when the delay is over
		 */
		napid->napi_mode = QCA_NAPI_TPUT_UNINITIALIZED;
		break;
	}
	default: {
		hif_err("Unknown event: %d (data=0x%0lx)",
			event, (unsigned long) data);
		break;
	} /* default */
	}; /* switch */


	switch (blacklist_pending) {
	case BLACKLIST_ON_PENDING:
		/* assume the control of WLAN IRQs */
		hif_napi_cpu_blacklist(napid, BLACKLIST_ON);
		break;
	case BLACKLIST_OFF_PENDING:
		/* yield the control of WLAN IRQs */
		hif_napi_cpu_blacklist(napid, BLACKLIST_OFF);
		break;
	default: /* nothing to do */
		break;
	} /* switch blacklist_pending */

	/* we want to perform the comparison in lock:
	 * there is a possiblity of hif_napi_event get called
	 * from two different contexts (driver unload and cpu hotplug
	 * notification) and napid->state get changed
	 * in driver unload context and can lead to race condition
	 * in cpu hotplug context. Therefore, perform the napid->state
	 * comparison before releasing lock.
	 */
	state_changed = (prev_state != napid->state);
	qdf_spin_unlock_bh(&(napid->lock));

	if (state_changed) {
		if (napid->state == ENABLE_NAPI_MASK) {
			rc = 1;
			for (i = 0; i < CE_COUNT_MAX; i++) {
				struct qca_napi_info *napii = napid->napis[i];
				if (napii) {
					napi = &(napii->napi);
					NAPI_DEBUG("%s: enabling NAPI %d",
						   __func__, i);
					napi_enable(napi);
				}
			}
		} else {
			rc = 0;
			for (i = 0; i < CE_COUNT_MAX; i++) {
				struct qca_napi_info *napii = napid->napis[i];
				if (napii) {
					napi = &(napii->napi);
					NAPI_DEBUG("%s: disabling NAPI %d",
						   __func__, i);
					napi_disable(napi);
					/* in case it is affined, remove it */
					qdf_dev_set_irq_affinity(napii->irq,
								 NULL);
				}
			}
		}
	} else {
		hif_debug("no change in hif napi state (still %d)", prev_state);
	}

	NAPI_DEBUG("<--[rc=%d]", rc);
	return rc;
}
qdf_export_symbol(hif_napi_event);

/**
 * hif_napi_enabled() - checks whether NAPI is enabled for given ce or not
 * @hif: hif context
 * @ce : CE instance (or -1, to check if any CEs are enabled)
 *
 * Return: bool
 */
int hif_napi_enabled(struct hif_opaque_softc *hif_ctx, int ce)
{
	int rc;
	struct hif_softc *hif = HIF_GET_SOFTC(hif_ctx);

	if (-1 == ce)
		rc = ((hif->napi_data.state == ENABLE_NAPI_MASK));
	else
		rc = ((hif->napi_data.state == ENABLE_NAPI_MASK) &&
		      (hif->napi_data.ce_map & (0x01 << ce)));
	return rc;
}
qdf_export_symbol(hif_napi_enabled);

/**
 * hif_napi_created() - checks whether NAPI is created for given ce or not
 * @hif: hif context
 * @ce : CE instance
 *
 * Return: bool
 */
bool hif_napi_created(struct hif_opaque_softc *hif_ctx, int ce)
{
	int rc;
	struct hif_softc *hif = HIF_GET_SOFTC(hif_ctx);

	rc = (hif->napi_data.ce_map & (0x01 << ce));

	return !!rc;
}
qdf_export_symbol(hif_napi_created);

/**
 * hif_napi_enable_irq() - enables bus interrupts after napi_complete
 *
 * @hif: hif context
 * @id : id of NAPI instance calling this (used to determine the CE)
 *
 * Return: void
 */
inline void hif_napi_enable_irq(struct hif_opaque_softc *hif, int id)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif);

	hif_irq_enable(scn, NAPI_ID2PIPE(id));
}


/**
 * hif_napi_schedule() - schedules napi, updates stats
 * @scn:  hif context
 * @ce_id: index of napi instance
 *
 * Return: false if napi didn't enable or already scheduled, otherwise true
 */
bool hif_napi_schedule(struct hif_opaque_softc *hif_ctx, int ce_id)
{
	int cpu = smp_processor_id();
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	struct qca_napi_info *napii;

	napii = scn->napi_data.napis[ce_id];
	if (qdf_unlikely(!napii)) {
		hif_err("scheduling unallocated napi (ce:%d)", ce_id);
		qdf_atomic_dec(&scn->active_tasklet_cnt);
		return false;
	}

	if (test_bit(NAPI_STATE_SCHED, &napii->napi.state)) {
		NAPI_DEBUG("napi scheduled, return");
		qdf_atomic_dec(&scn->active_tasklet_cnt);
		return false;
	}

	hif_record_ce_desc_event(scn,  ce_id, NAPI_SCHEDULE,
				 NULL, NULL, 0, 0);
	napii->stats[cpu].napi_schedules++;
	NAPI_DEBUG("scheduling napi %d (ce:%d)", napii->id, ce_id);
	napi_schedule(&(napii->napi));

	return true;
}
qdf_export_symbol(hif_napi_schedule);

/**
 * hif_napi_correct_cpu() - correct the interrupt affinity for napi if needed
 * @napi_info: pointer to qca_napi_info for the napi instance
 *
 * Return: true  => interrupt already on correct cpu, no correction needed
 *         false => interrupt on wrong cpu, correction done for cpu affinity
 *                   of the interrupt
 */
static inline
bool hif_napi_correct_cpu(struct qca_napi_info *napi_info)
{
	bool right_cpu = true;
	int rc = 0;
	int cpu;
	struct qca_napi_data *napid;
	QDF_STATUS ret;

	napid = hif_napi_get_all(GET_HIF_OPAQUE_HDL(napi_info->hif_ctx));

	if (napid->flags & QCA_NAPI_FEATURE_CPU_CORRECTION) {

		cpu = qdf_get_cpu();
		if (unlikely((hif_napi_cpu_blacklist(napid,
						BLACKLIST_QUERY) > 0) &&
						(cpu != napi_info->cpu))) {
			right_cpu = false;

			NAPI_DEBUG("interrupt on wrong CPU, correcting");
			napi_info->cpumask.bits[0] = (0x01 << napi_info->cpu);

			qdf_dev_modify_irq_status(napi_info->irq,
						  QDF_IRQ_NO_BALANCING, 0);
			ret = qdf_dev_set_irq_affinity(napi_info->irq,
						       (struct qdf_cpu_mask *)
						       &napi_info->cpumask);
			rc = qdf_status_to_os_return(ret);
			qdf_dev_modify_irq_status(napi_info->irq, 0,
						  QDF_IRQ_NO_BALANCING);

			if (rc)
				hif_err("Setting irq affinity hint: %d", rc);
			else
				napi_info->stats[cpu].cpu_corrected++;
		}
	}
	return right_cpu;
}

#ifdef RECEIVE_OFFLOAD
/**
 * hif_napi_offld_flush_cb() - Call upper layer flush callback
 * @napi_info: Handle to hif_napi_info
 *
 * Return: None
 */
static void hif_napi_offld_flush_cb(struct qca_napi_info *napi_info)
{
	if (napi_info->offld_flush_cb)
		napi_info->offld_flush_cb(napi_info);
}
#else
static void hif_napi_offld_flush_cb(struct qca_napi_info *napi_info)
{
}
#endif

/**
 * hif_napi_poll() - NAPI poll routine
 * @napi  : pointer to NAPI struct as kernel holds it
 * @budget:
 *
 * This is the body of the poll function.
 * The poll function is called by kernel. So, there is a wrapper
 * function in HDD, which in turn calls this function.
 * Two main reasons why the whole thing is not implemented in HDD:
 * a) references to things like ce_service that HDD is not aware of
 * b) proximity to the implementation of ce_tasklet, which the body
 *    of this function should be very close to.
 *
 * NOTE TO THE MAINTAINER:
 *  Consider this function and ce_tasklet very tightly coupled pairs.
 *  Any changes to ce_tasklet or this function may likely need to be
 *  reflected in the counterpart.
 *
 * Returns:
 *  int: the amount of work done in this poll (<= budget)
 */
int hif_napi_poll(struct hif_opaque_softc *hif_ctx,
		  struct napi_struct *napi,
		  int budget)
{
	int    rc = 0; /* default: no work done, also takes care of error */
	int    normalized = 0;
	int    bucket;
	int    cpu = smp_processor_id();
	bool poll_on_right_cpu;
	struct hif_softc      *hif = HIF_GET_SOFTC(hif_ctx);
	struct qca_napi_info *napi_info;
	struct CE_state *ce_state = NULL;

	if (unlikely(!hif)) {
		hif_err("hif context is NULL");
		QDF_ASSERT(0);
		goto out;
	}

	napi_info = (struct qca_napi_info *)
		container_of(napi, struct qca_napi_info, napi);

	NAPI_DEBUG("%s -->(napi(%d, irq=%d), budget=%d)",
		   __func__, napi_info->id, napi_info->irq, budget);

	napi_info->stats[cpu].napi_polls++;

	hif_record_ce_desc_event(hif, NAPI_ID2PIPE(napi_info->id),
				 NAPI_POLL_ENTER, NULL, NULL, cpu, 0);

	rc = ce_per_engine_service(hif, NAPI_ID2PIPE(napi_info->id));
	NAPI_DEBUG("%s: ce_per_engine_service processed %d msgs",
		    __func__, rc);

	hif_napi_offld_flush_cb(napi_info);

	/* do not return 0, if there was some work done,
	 * even if it is below the scale
	 */
	if (rc) {
		napi_info->stats[cpu].napi_workdone += rc;
		normalized = (rc / napi_info->scale);
		if (normalized == 0)
			normalized++;
		bucket = (normalized - 1) /
				(QCA_NAPI_BUDGET / QCA_NAPI_NUM_BUCKETS);
		if (bucket >= QCA_NAPI_NUM_BUCKETS) {
			bucket = QCA_NAPI_NUM_BUCKETS - 1;
			hif_err("Bad bucket#(%d) > QCA_NAPI_NUM_BUCKETS(%d)"
				" normalized %d, napi budget %d",
				bucket, QCA_NAPI_NUM_BUCKETS,
				normalized, QCA_NAPI_BUDGET);
		}
		napi_info->stats[cpu].napi_budget_uses[bucket]++;
	} else {
	/* if ce_per engine reports 0, then poll should be terminated */
		NAPI_DEBUG("%s:%d: nothing processed by CE. Completing NAPI",
			   __func__, __LINE__);
	}

	ce_state = hif->ce_id_to_state[NAPI_ID2PIPE(napi_info->id)];

	/*
	 * Not using the API hif_napi_correct_cpu directly in the if statement
	 * below since the API may not get evaluated if put at the end if any
	 * prior condition would evaluate to be true. The CPU correction
	 * check should kick in every poll.
	 */
#ifdef NAPI_YIELD_BUDGET_BASED
	if (ce_state && (ce_state->force_break || 0 == rc)) {
#else
	poll_on_right_cpu = hif_napi_correct_cpu(napi_info);
	if ((ce_state) &&
	    (!ce_check_rx_pending(ce_state) || (0 == rc) ||
	     !poll_on_right_cpu)) {
#endif
		napi_info->stats[cpu].napi_completes++;
#ifdef NAPI_YIELD_BUDGET_BASED
		ce_state->force_break = 0;
#endif

		hif_record_ce_desc_event(hif, ce_state->id, NAPI_COMPLETE,
					 NULL, NULL, 0, 0);
		if (normalized >= budget)
			normalized = budget - 1;

		napi_complete(napi);
		/* enable interrupts */
		hif_napi_enable_irq(hif_ctx, napi_info->id);
		/* support suspend/resume */
		qdf_atomic_dec(&(hif->active_tasklet_cnt));

		NAPI_DEBUG("%s:%d: napi_complete + enabling the interrupts",
			   __func__, __LINE__);
	} else {
		/* 4.4 kernel NAPI implementation requires drivers to
		 * return full work when they ask to be re-scheduled,
		 * or napi_complete and re-start with a fresh interrupt
		 */
		normalized = budget;
	}

	hif_record_ce_desc_event(hif, NAPI_ID2PIPE(napi_info->id),
				 NAPI_POLL_EXIT, NULL, NULL, normalized, 0);

	NAPI_DEBUG("%s <--[normalized=%d]", __func__, normalized);
	return normalized;
out:
	return rc;
}
qdf_export_symbol(hif_napi_poll);

void hif_update_napi_max_poll_time(struct CE_state *ce_state,
				   int ce_id,
				   int cpu_id)
{
	struct hif_softc *hif;
	struct qca_napi_info *napi_info;
	unsigned long long napi_poll_time = qdf_time_sched_clock() -
					ce_state->ce_service_start_time;

	hif = ce_state->scn;
	napi_info = hif->napi_data.napis[ce_id];
	if (napi_poll_time >
			napi_info->stats[cpu_id].napi_max_poll_time)
		napi_info->stats[cpu_id].napi_max_poll_time = napi_poll_time;
}
qdf_export_symbol(hif_update_napi_max_poll_time);

#ifdef HIF_IRQ_AFFINITY
/**
 *
 * hif_napi_update_yield_stats() - update NAPI yield related stats
 * @cpu_id: CPU ID for which stats needs to be updates
 * @ce_id: Copy Engine ID for which yield stats needs to be updates
 * @time_limit_reached: indicates whether the time limit was reached
 * @rxpkt_thresh_reached: indicates whether rx packet threshold was reached
 *
 * Return: None
 */
void hif_napi_update_yield_stats(struct CE_state *ce_state,
				 bool time_limit_reached,
				 bool rxpkt_thresh_reached)
{
	struct hif_softc *hif;
	struct qca_napi_data *napi_data = NULL;
	int ce_id = 0;
	int cpu_id = 0;

	if (unlikely(!ce_state)) {
		QDF_ASSERT(ce_state);
		return;
	}

	hif = ce_state->scn;

	if (unlikely(!hif)) {
		QDF_ASSERT(hif);
		return;
	}
	napi_data = &(hif->napi_data);
	if (unlikely(!napi_data)) {
		QDF_ASSERT(napi_data);
		return;
	}

	ce_id = ce_state->id;
	cpu_id = qdf_get_cpu();

	if (unlikely(!napi_data->napis[ce_id])) {
		return;
	}

	if (time_limit_reached)
		napi_data->napis[ce_id]->stats[cpu_id].time_limit_reached++;
	else
		napi_data->napis[ce_id]->stats[cpu_id].rxpkt_thresh_reached++;

	hif_update_napi_max_poll_time(ce_state, ce_id,
				      cpu_id);
}

/**
 *
 * hif_napi_stats() - display NAPI CPU statistics
 * @napid: pointer to qca_napi_data
 *
 * Description:
 *    Prints the various CPU cores on which the NAPI instances /CEs interrupts
 *    are being executed. Can be called from outside NAPI layer.
 *
 * Return: None
 */
void hif_napi_stats(struct qca_napi_data *napid)
{
	int i;
	struct qca_napi_cpu *cpu;

	if (!napid) {
		qdf_debug("%s: napiid struct is null", __func__);
		return;
	}

	cpu = napid->napi_cpu;
	qdf_debug("NAPI CPU TABLE");
	qdf_debug("lilclhead=%d, bigclhead=%d",
		  napid->lilcl_head, napid->bigcl_head);
	for (i = 0; i < NR_CPUS; i++) {
		qdf_debug("CPU[%02d]: state:%d crid=%02d clid=%02d crmk:0x%0lx thmk:0x%0lx frq:%d napi = 0x%08x lnk:%d",
			  i,
			  cpu[i].state, cpu[i].core_id, cpu[i].cluster_id,
			  cpu[i].core_mask.bits[0],
			  cpu[i].thread_mask.bits[0],
			  cpu[i].max_freq, cpu[i].napis,
			  cpu[i].cluster_nxt);
	}
}

#ifdef FEATURE_NAPI_DEBUG
/*
 * Local functions
 * - no argument checks, all internal/trusted callers
 */
static void hnc_dump_cpus(struct qca_napi_data *napid)
{
	hif_napi_stats(napid);
}
#else
static void hnc_dump_cpus(struct qca_napi_data *napid) { /* no-op */ };
#endif /* FEATURE_NAPI_DEBUG */
/**
 * hnc_link_clusters() - partitions to cpu table into clusters
 * @napid: pointer to NAPI data
 *
 * Takes in a CPU topology table and builds two linked lists
 * (big cluster cores, list-head at bigcl_head, and little cluster
 * cores, list-head at lilcl_head) out of it.
 *
 * If there are more than two clusters:
 * - bigcl_head and lilcl_head will be different,
 * - the cluster with highest cpufreq will be considered the "big" cluster.
 *   If there are more than one with the highest frequency, the *last* of such
 *   clusters will be designated as the "big cluster"
 * - the cluster with lowest cpufreq will be considered the "li'l" cluster.
 *   If there are more than one clusters with the lowest cpu freq, the *first*
 *   of such clusters will be designated as the "little cluster"
 * - We only support up to 32 clusters
 * Return: 0 : OK
 *         !0: error (at least one of lil/big clusters could not be found)
 */
#define HNC_MIN_CLUSTER 0
#define HNC_MAX_CLUSTER 1
static int hnc_link_clusters(struct qca_napi_data *napid)
{
	int rc = 0;

	int i;
	int it = 0;
	uint32_t cl_done = 0x0;
	int cl, curcl, curclhead = 0;
	int more;
	unsigned int lilfrq = INT_MAX;
	unsigned int bigfrq = 0;
	unsigned int clfrq = 0;
	int prev = 0;
	struct qca_napi_cpu *cpus = napid->napi_cpu;

	napid->lilcl_head = napid->bigcl_head = -1;

	do {
		more = 0;
		it++; curcl = -1;
		for (i = 0; i < NR_CPUS; i++) {
			cl = cpus[i].cluster_id;
			NAPI_DEBUG("Processing cpu[%d], cluster=%d\n",
				   i, cl);
			if ((cl < HNC_MIN_CLUSTER) || (cl > HNC_MAX_CLUSTER)) {
				NAPI_DEBUG("Bad cluster (%d). SKIPPED\n", cl);
				/* continue if ASSERTs are disabled */
				continue;
			};
			if (cpumask_weight(&(cpus[i].core_mask)) == 0) {
				NAPI_DEBUG("Core mask 0. SKIPPED\n");
				continue;
			}
			if (cl_done & (0x01 << cl)) {
				NAPI_DEBUG("Cluster already processed. SKIPPED\n");
				continue;
			} else {
				if (more == 0) {
					more = 1;
					curcl = cl;
					curclhead = i; /* row */
					clfrq = cpus[i].max_freq;
					prev = -1;
				};
				if ((curcl >= 0) && (curcl != cl)) {
					NAPI_DEBUG("Entry cl(%d) != curcl(%d). SKIPPED\n",
						   cl, curcl);
					continue;
				}
				if (cpus[i].max_freq != clfrq)
					NAPI_DEBUG("WARN: frq(%d)!=clfrq(%d)\n",
						   cpus[i].max_freq, clfrq);
				if (clfrq >= bigfrq) {
					bigfrq = clfrq;
					napid->bigcl_head  = curclhead;
					NAPI_DEBUG("bigcl=%d\n", curclhead);
				}
				if (clfrq < lilfrq) {
					lilfrq = clfrq;
					napid->lilcl_head = curclhead;
					NAPI_DEBUG("lilcl=%d\n", curclhead);
				}
				if (prev != -1)
					cpus[prev].cluster_nxt = i;

				prev = i;
			}
		}
		if (curcl >= 0)
			cl_done |= (0x01 << curcl);

	} while (more);

	if (qdf_unlikely((napid->lilcl_head < 0) && (napid->bigcl_head < 0)))
		rc = -EFAULT;

	hnc_dump_cpus(napid); /* if NAPI_DEBUG */
	return rc;
}
#undef HNC_MIN_CLUSTER
#undef HNC_MAX_CLUSTER

/*
 * hotplug function group
 */

/**
 * hnc_cpu_online_cb() - handles CPU hotplug "up" events
 * @context: the associated HIF context
 * @cpu: the CPU Id of the CPU the event happened on
 *
 * Return: None
 */
static void hnc_cpu_online_cb(void *context, uint32_t cpu)
{
	struct hif_softc *hif = context;
	struct qca_napi_data *napid = &hif->napi_data;

	if (cpu >= NR_CPUS)
		return;

	NAPI_DEBUG("-->%s(act=online, cpu=%u)", __func__, cpu);

	napid->napi_cpu[cpu].state = QCA_NAPI_CPU_UP;
	NAPI_DEBUG("%s: CPU %u marked %d",
		   __func__, cpu, napid->napi_cpu[cpu].state);

	NAPI_DEBUG("<--%s", __func__);
}

/**
 * hnc_cpu_before_offline_cb() - handles CPU hotplug "prepare down" events
 * @context: the associated HIF context
 * @cpu: the CPU Id of the CPU the event happened on
 *
 * On transtion to offline, we act on PREP events, because we may need to move
 * the irqs/NAPIs to another CPU before it is actually off-lined.
 *
 * Return: None
 */
static void hnc_cpu_before_offline_cb(void *context, uint32_t cpu)
{
	struct hif_softc *hif = context;
	struct qca_napi_data *napid = &hif->napi_data;

	if (cpu >= NR_CPUS)
		return;

	NAPI_DEBUG("-->%s(act=before_offline, cpu=%u)", __func__, cpu);

	napid->napi_cpu[cpu].state = QCA_NAPI_CPU_DOWN;

	NAPI_DEBUG("%s: CPU %u marked %d; updating affinity",
		   __func__, cpu, napid->napi_cpu[cpu].state);

	/**
	 * we need to move any NAPIs on this CPU out.
	 * if we are in LO throughput mode, then this is valid
	 * if the CPU is the the low designated CPU.
	 */
	hif_napi_event(GET_HIF_OPAQUE_HDL(hif),
		       NAPI_EVT_CPU_STATE,
		       (void *)
		       ((size_t)cpu << 16 | napid->napi_cpu[cpu].state));

	NAPI_DEBUG("<--%s", __func__);
}

static int hnc_hotplug_register(struct hif_softc *hif_sc)
{
	QDF_STATUS status;

	NAPI_DEBUG("-->%s", __func__);

	status = qdf_cpuhp_register(&hif_sc->napi_data.cpuhp_handler,
				    hif_sc,
				    hnc_cpu_online_cb,
				    hnc_cpu_before_offline_cb);

	NAPI_DEBUG("<--%s [%d]", __func__, status);

	return qdf_status_to_os_return(status);
}

static void hnc_hotplug_unregister(struct hif_softc *hif_sc)
{
	NAPI_DEBUG("-->%s", __func__);

	if (hif_sc->napi_data.cpuhp_handler)
		qdf_cpuhp_unregister(&hif_sc->napi_data.cpuhp_handler);

	NAPI_DEBUG("<--%s", __func__);
}

/**
 * hnc_install_tput() - installs a callback in the throughput detector
 * @register: !0 => register; =0: unregister
 *
 * installs a callback to be called when wifi driver throughput (tx+rx)
 * crosses a threshold. Currently, we are using the same criteria as
 * TCP ack suppression (500 packets/100ms by default).
 *
 * Return: 0 : success
 *         <0: failure
 */

static int hnc_tput_hook(int install)
{
	int rc = 0;

	/*
	 * Nothing, until the bw_calculation accepts registration
	 * it is now hardcoded in the wlan_hdd_main.c::hdd_bus_bw_compute_cbk
	 *   hdd_napi_throughput_policy(...)
	 */
	return rc;
}

/*
 * Implementation of hif_napi_cpu API
 */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
static inline void record_sibling_cpumask(struct qca_napi_cpu *cpus, int i)
{
	cpumask_copy(&(cpus[i].thread_mask),
			     topology_sibling_cpumask(i));
}
#else
static inline void record_sibling_cpumask(struct qca_napi_cpu *cpus, int i)
{
}
#endif


/**
 * hif_napi_cpu_init() - initialization of irq affinity block
 * @ctx: pointer to qca_napi_data
 *
 * called by hif_napi_create, after the first instance is called
 * - builds napi_rss_cpus table from cpu topology
 * - links cores of the same clusters together
 * - installs hot-plug notifier
 * - installs throughput trigger notifier (when such mechanism exists)
 *
 * Return: 0: OK
 *         <0: error code
 */
int hif_napi_cpu_init(struct hif_opaque_softc *hif)
{
	int rc = 0;
	int i;
	struct qca_napi_data *napid = &HIF_GET_SOFTC(hif)->napi_data;
	struct qca_napi_cpu *cpus = napid->napi_cpu;

	NAPI_DEBUG("--> ");

	if (cpus[0].state != QCA_NAPI_CPU_UNINITIALIZED) {
		NAPI_DEBUG("NAPI RSS table already initialized.\n");
		rc = -EALREADY;
		goto lab_rss_init;
	}

	/* build CPU topology table */
	for_each_possible_cpu(i) {
		cpus[i].state       = ((cpumask_test_cpu(i, cpu_online_mask)
					? QCA_NAPI_CPU_UP
					: QCA_NAPI_CPU_DOWN));
		cpus[i].core_id     = topology_core_id(i);
		cpus[i].cluster_id  = topology_physical_package_id(i);
		cpumask_copy(&(cpus[i].core_mask),
			     topology_core_cpumask(i));
		record_sibling_cpumask(cpus, i);
		cpus[i].max_freq    = cpufreq_quick_get_max(i);
		cpus[i].napis       = 0x0;
		cpus[i].cluster_nxt = -1; /* invalid */
	}

	/* link clusters together */
	rc = hnc_link_clusters(napid);
	if (0 != rc)
		goto lab_err_topology;

	/* install hotplug notifier */
	rc = hnc_hotplug_register(HIF_GET_SOFTC(hif));
	if (0 != rc)
		goto lab_err_hotplug;

	/* install throughput notifier */
	rc = hnc_tput_hook(1);
	if (0 == rc)
		goto lab_rss_init;

lab_err_hotplug:
	hnc_tput_hook(0);
	hnc_hotplug_unregister(HIF_GET_SOFTC(hif));
lab_err_topology:
	memset(napid->napi_cpu, 0, sizeof(struct qca_napi_cpu) * NR_CPUS);
lab_rss_init:
	NAPI_DEBUG("<-- [rc=%d]", rc);
	return rc;
}

/**
 * hif_napi_cpu_deinit() - clean-up of irq affinity block
 *
 * called by hif_napi_destroy, when the last instance is removed
 * - uninstalls throughput and hotplug notifiers
 * - clears cpu topology table
 * Return: 0: OK
 */
int hif_napi_cpu_deinit(struct hif_opaque_softc *hif)
{
	int rc = 0;
	struct qca_napi_data *napid = &HIF_GET_SOFTC(hif)->napi_data;

	NAPI_DEBUG("-->%s(...)", __func__);

	/* uninstall tput notifier */
	rc = hnc_tput_hook(0);

	/* uninstall hotplug notifier */
	hnc_hotplug_unregister(HIF_GET_SOFTC(hif));

	/* clear the topology table */
	memset(napid->napi_cpu, 0, sizeof(struct qca_napi_cpu) * NR_CPUS);

	NAPI_DEBUG("<--%s[rc=%d]", __func__, rc);

	return rc;
}

/**
 * hncm_migrate_to() - migrates a NAPI to a CPU
 * @napid: pointer to NAPI block
 * @ce_id: CE_id of the NAPI instance
 * @didx : index in the CPU topology table for the CPU to migrate to
 *
 * Migrates NAPI (identified by the CE_id) to the destination core
 * Updates the napi_map of the destination entry
 *
 * Return:
 *  =0 : success
 *  <0 : error
 */
static int hncm_migrate_to(struct qca_napi_data *napid,
			   int                   napi_ce,
			   int                   didx)
{
	int rc = 0;
	QDF_STATUS status;

	NAPI_DEBUG("-->%s(napi_cd=%d, didx=%d)", __func__, napi_ce, didx);

	if (!napid->napis[napi_ce])
		return -EINVAL;

	napid->napis[napi_ce]->cpumask.bits[0] = (1 << didx);

	qdf_dev_modify_irq_status(napid->napis[napi_ce]->irq,
				  QDF_IRQ_NO_BALANCING, 0);
	status = qdf_dev_set_irq_affinity(napid->napis[napi_ce]->irq,
					  (struct qdf_cpu_mask *)
					  &napid->napis[napi_ce]->cpumask);
	rc = qdf_status_to_os_return(status);

	/* unmark the napis bitmap in the cpu table */
	napid->napi_cpu[napid->napis[napi_ce]->cpu].napis &= ~(0x01 << napi_ce);
	/* mark the napis bitmap for the new designated cpu */
	napid->napi_cpu[didx].napis |= (0x01 << napi_ce);
	napid->napis[napi_ce]->cpu = didx;

	NAPI_DEBUG("<--%s[%d]", __func__, rc);
	return rc;
}
/**
 * hncm_dest_cpu() - finds a destination CPU for NAPI
 * @napid: pointer to NAPI block
 * @act  : RELOCATE | COLLAPSE | DISPERSE
 *
 * Finds the designated destionation for the next IRQ.
 * RELOCATE: translated to either COLLAPSE or DISPERSE based
 *           on napid->napi_mode (throughput state)
 * COLLAPSE: All have the same destination: the first online CPU in lilcl
 * DISPERSE: One of the CPU in bigcl, which has the smallest number of
 *           NAPIs on it
 *
 * Return: >=0 : index in the cpu topology table
 *       : < 0 : error
 */
static int hncm_dest_cpu(struct qca_napi_data *napid, int act)
{
	int destidx = -1;
	int head, i;

	NAPI_DEBUG("-->%s(act=%d)", __func__, act);
	if (act == HNC_ACT_RELOCATE) {
		if (napid->napi_mode == QCA_NAPI_TPUT_LO)
			act = HNC_ACT_COLLAPSE;
		else
			act = HNC_ACT_DISPERSE;
		NAPI_DEBUG("%s: act changed from HNC_ACT_RELOCATE to %d",
			   __func__, act);
	}
	if (act == HNC_ACT_COLLAPSE) {
		head = i = napid->lilcl_head;
retry_collapse:
		while (i >= 0) {
			if (napid->napi_cpu[i].state == QCA_NAPI_CPU_UP) {
				destidx = i;
				break;
			}
			i = napid->napi_cpu[i].cluster_nxt;
		}
		if ((destidx < 0) && (head == napid->lilcl_head)) {
			NAPI_DEBUG("%s: COLLAPSE: no lilcl dest, try bigcl",
				__func__);
			head = i = napid->bigcl_head;
			goto retry_collapse;
		}
	} else { /* HNC_ACT_DISPERSE */
		int smallest = 99; /* all 32 bits full */
		int smallidx = -1;

		head = i = napid->bigcl_head;
retry_disperse:
		while (i >= 0) {
			if ((napid->napi_cpu[i].state == QCA_NAPI_CPU_UP) &&
			    (hweight32(napid->napi_cpu[i].napis) <= smallest)) {
				smallest = napid->napi_cpu[i].napis;
				smallidx = i;
			}
			i = napid->napi_cpu[i].cluster_nxt;
		}
		/* Check if matches with user sepecified CPU mask */
		smallidx = ((1 << smallidx) & napid->user_cpu_affin_mask) ?
								smallidx : -1;

		if ((smallidx < 0) && (head == napid->bigcl_head)) {
			NAPI_DEBUG("%s: DISPERSE: no bigcl dest, try lilcl",
				__func__);
			head = i = napid->lilcl_head;
			goto retry_disperse;
		}
		destidx = smallidx;
	}
	NAPI_DEBUG("<--%s[dest=%d]", __func__, destidx);
	return destidx;
}
/**
 * hif_napi_cpu_migrate() - migrate IRQs away
 * @cpu: -1: all CPUs <n> specific CPU
 * @act: COLLAPSE | DISPERSE
 *
 * Moves IRQs/NAPIs from specific or all CPUs (specified by @cpu) to eligible
 * cores. Eligible cores are:
 * act=COLLAPSE -> the first online core of the little cluster
 * act=DISPERSE -> separate cores of the big cluster, so that each core will
 *                 host minimum number of NAPIs/IRQs (napid->cpus[cpu].napis)
 *
 * Note that this function is called with a spinlock acquired already.
 *
 * Return: =0: success
 *         <0: error
 */

int hif_napi_cpu_migrate(struct qca_napi_data *napid, int cpu, int action)
{
	int      rc = 0;
	struct qca_napi_cpu *cpup;
	int      i, dind;
	uint32_t napis;

	NAPI_DEBUG("-->%s(.., cpu=%d, act=%d)",
		   __func__, cpu, action);
	/* the following is really: hif_napi_enabled() with less overhead */
	if (napid->ce_map == 0) {
		NAPI_DEBUG("%s: NAPI disabled. Not migrating.", __func__);
		goto hncm_return;
	}

	cpup = napid->napi_cpu;

	switch (action) {
	case HNC_ACT_RELOCATE:
	case HNC_ACT_DISPERSE:
	case HNC_ACT_COLLAPSE: {
		/* first find the src napi set */
		if (cpu == HNC_ANY_CPU)
			napis = napid->ce_map;
		else
			napis = cpup[cpu].napis;
		/* then clear the napi bitmap on each CPU */
		for (i = 0; i < NR_CPUS; i++)
			cpup[i].napis = 0;
		/* then for each of the NAPIs to disperse: */
		for (i = 0; i < CE_COUNT_MAX; i++)
			if (napis & (1 << i)) {
				/* find a destination CPU */
				dind = hncm_dest_cpu(napid, action);
				if (dind >= 0) {
					NAPI_DEBUG("Migrating NAPI ce%d to %d",
						   i, dind);
					rc = hncm_migrate_to(napid, i, dind);
				} else {
					NAPI_DEBUG("No dest for NAPI ce%d", i);
					hnc_dump_cpus(napid);
					rc = -1;
				}
			}
		break;
	}
	default: {
		NAPI_DEBUG("%s: bad action: %d\n", __func__, action);
		QDF_BUG(0);
		break;
	}
	} /* switch action */

hncm_return:
	hnc_dump_cpus(napid);
	return rc;
}


/**
 * hif_napi_bl_irq() - calls irq_modify_status to enable/disable blacklisting
 * @napid: pointer to qca_napi_data structure
 * @bl_flag: blacklist flag to enable/disable blacklisting
 *
 * The function enables/disables blacklisting for all the copy engine
 * interrupts on which NAPI is enabled.
 *
 * Return: None
 */
static inline void hif_napi_bl_irq(struct qca_napi_data *napid, bool bl_flag)
{
	int i;
	struct qca_napi_info *napii;

	for (i = 0; i < CE_COUNT_MAX; i++) {
		/* check if NAPI is enabled on the CE */
		if (!(napid->ce_map & (0x01 << i)))
			continue;

		/*double check that NAPI is allocated for the CE */
		napii = napid->napis[i];
		if (!(napii))
			continue;

		if (bl_flag == true)
			qdf_dev_modify_irq_status(napii->irq,
						  0, QDF_IRQ_NO_BALANCING);
		else
			qdf_dev_modify_irq_status(napii->irq,
						  QDF_IRQ_NO_BALANCING, 0);
		hif_debug("bl_flag %d CE %d", bl_flag, i);
	}
}

/**
 * hif_napi_cpu_blacklist() - en(dis)ables blacklisting for NAPI RX interrupts.
 * @napid: pointer to qca_napi_data structure
 * @op: blacklist operation to perform
 *
 * The function enables/disables/queries blacklisting for all CE RX
 * interrupts with NAPI enabled. Besides blacklisting, it also enables/disables
 * core_ctl_set_boost.
 * Once blacklisting is enabled, the interrupts will not be managed by the IRQ
 * balancer.
 *
 * Return: -EINVAL, in case IRQ_BLACKLISTING and CORE_CTL_BOOST is not enabled
 *         for BLACKLIST_QUERY op - blacklist refcount
 *         for BLACKLIST_ON op    - return value from core_ctl_set_boost API
 *         for BLACKLIST_OFF op   - return value from core_ctl_set_boost API
 */
int hif_napi_cpu_blacklist(struct qca_napi_data *napid,
			   enum qca_blacklist_op op)
{
	int rc = 0;
	static int ref_count; /* = 0 by the compiler */
	uint8_t flags = napid->flags;
	bool bl_en = flags & QCA_NAPI_FEATURE_IRQ_BLACKLISTING;
	bool ccb_en = flags & QCA_NAPI_FEATURE_CORE_CTL_BOOST;

	NAPI_DEBUG("-->%s(%d %d)", __func__, flags, op);

	if (!(bl_en && ccb_en)) {
		rc = -EINVAL;
		goto out;
	}

	switch (op) {
	case BLACKLIST_QUERY:
		rc = ref_count;
		break;
	case BLACKLIST_ON:
		ref_count++;
		rc = 0;
		if (ref_count == 1) {
			rc = hif_napi_core_ctl_set_boost(true);
			NAPI_DEBUG("boost_on() returns %d - refcnt=%d",
				rc, ref_count);
			hif_napi_bl_irq(napid, true);
		}
		break;
	case BLACKLIST_OFF:
		if (ref_count) {
			ref_count--;
			rc = 0;
			if (ref_count == 0) {
				rc = hif_napi_core_ctl_set_boost(false);
				NAPI_DEBUG("boost_off() returns %d - refcnt=%d",
					   rc, ref_count);
				hif_napi_bl_irq(napid, false);
			}
		}
		break;
	default:
		NAPI_DEBUG("Invalid blacklist op: %d", op);
		rc = -EINVAL;
	} /* switch */
out:
	NAPI_DEBUG("<--%s[%d]", __func__, rc);
	return rc;
}

/**
 * hif_napi_serialize() - [de-]serialize NAPI operations
 * @hif:   context
 * @is_on: 1: serialize, 0: deserialize
 *
 * hif_napi_serialize(hif, 1) can be called multiple times. It will perform the
 * following steps (see hif_napi_event for code):
 * - put irqs of all NAPI instances on the same CPU
 * - only for the first serialize call: blacklist
 *
 * hif_napi_serialize(hif, 0):
 * - start a timer (multiple of BusBandwidthTimer -- default: 100 msec)
 * - at the end of the timer, check the current throughput state and
 *   implement it.
 */
static unsigned long napi_serialize_reqs;
int hif_napi_serialize(struct hif_opaque_softc *hif, int is_on)
{
	int rc = -EINVAL;

	if (hif)
		switch (is_on) {
		case 0: { /* de-serialize */
			rc = hif_napi_event(hif, NAPI_EVT_USR_NORMAL,
					    (void *) 0);
			napi_serialize_reqs = 0;
			break;
		} /* end de-serialize */
		case 1: { /* serialize */
			rc = hif_napi_event(hif, NAPI_EVT_USR_SERIAL,
					    (void *)napi_serialize_reqs++);
			break;
		} /* end serialize */
		default:
			break; /* no-op */
		} /* switch */
	return rc;
}

#endif /* ifdef HIF_IRQ_AFFINITY */
