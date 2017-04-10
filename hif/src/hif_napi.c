/*
 * Copyright (c) 2015-2017 The Linux Foundation. All rights reserved.
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

/**
 * DOC: hif_napi.c
 *
 * HIF NAPI interface implementation
 */

#include <string.h> /* memset */

/* Linux headers */
#include <linux/cpumask.h>
#include <linux/cpufreq.h>
#include <linux/cpu.h>
#include <linux/topology.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#ifdef HELIUMPLUS
#ifdef CONFIG_SCHED_CORE_CTL
#include <linux/sched/core_ctl.h>
#endif
#include <pld_snoc.h>
#endif
#include <linux/pm.h>

/* Driver headers */
#include <hif_napi.h>
#include <hif_debug.h>
#include <hif_io32.h>
#include <ce_api.h>
#include <ce_internal.h>

enum napi_decision_vector {
	HIF_NAPI_NOEVENT = 0,
	HIF_NAPI_INITED  = 1,
	HIF_NAPI_CONF_UP = 2
};
#define ENABLE_NAPI_MASK (HIF_NAPI_INITED | HIF_NAPI_CONF_UP)

#ifdef HELIUMPLUS
static inline int hif_get_irq_for_ce(int ce_id)
{
	return pld_snoc_get_irq(ce_id);
}
#else /* HELIUMPLUS */
static inline int hif_get_irq_for_ce(int ce_id)
{
	return -EINVAL;
}
static int hif_napi_cpu_migrate(struct qca_napi_data *napid, int cpu,
				int action)
{
	return 0;
}

int hif_napi_cpu_blacklist(struct qca_napi_data *napid,
					enum qca_blacklist_op op)
{
	return 0;
}
#endif /* HELIUMPLUS */

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
		if (rc != 0) {
			HIF_ERROR("NAPI_initialization failed,. %d", rc);
			goto hnc_err;
		}

		HIF_INFO("%s: NAPI structures initialized, rc=%d",
			 __func__, rc);
	}
	for (i = 0; i < hif->ce_count; i++) {
		ce_state = hif->ce_id_to_state[i];
		NAPI_DEBUG("ce %d: htt_rx=%d htt_tx=%d",
			   i, ce_state->htt_rx_data,
			   ce_state->htt_tx_data);
		if (!ce_state->htt_rx_data)
			continue;

		/* Now this is a CE where we need NAPI on */
		NAPI_DEBUG("Creating NAPI on pipe %d", i);

		napii = &(napid->napis[i]);
		memset(napii, 0, sizeof(struct qca_napi_info));
		napii->scale = scale;
		napii->id    = NAPI_PIPE2ID(i);
		napii->hif_ctx = hif_ctx;
		napii->irq   = hif_get_irq_for_ce(i);

		if (napii->irq < 0)
			HIF_WARN("%s: bad IRQ value for CE %d: %d",
				 __func__, i, napii->irq);

		qdf_spinlock_create(&napii->lro_unloading_lock);
		init_dummy_netdev(&(napii->netdev));

		NAPI_DEBUG("adding napi=%p to netdev=%p (poll=%p, bdgt=%d)",
			   &(napii->napi), &(napii->netdev), poll, budget);
		netif_napi_add(&(napii->netdev), &(napii->napi), poll, budget);

		NAPI_DEBUG("after napi_add");
		NAPI_DEBUG("napi=0x%p, netdev=0x%p",
			   &(napii->napi), &(napii->netdev));
		NAPI_DEBUG("napi.dev_list.prev=0x%p, next=0x%p",
			   napii->napi.dev_list.prev,
			   napii->napi.dev_list.next);
		NAPI_DEBUG("dev.napi_list.prev=0x%p, next=0x%p",
			   napii->netdev.napi_list.prev,
			   napii->netdev.napi_list.next);

		/* It is OK to change the state variable below without
		 * protection as there should be no-one around yet
		 */
		napid->ce_map |= (0x01 << i);
		HIF_INFO("%s: NAPI id %d created for pipe %d", __func__,
			 napii->id, i);
	}
	NAPI_DEBUG("NAPI ids created for all applicable pipes");
hnc_err:
	NAPI_DEBUG("<--napi_instances_map=%x]", napid->ce_map);
	return napid->ce_map;
}

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
		HIF_ERROR("%s: NAPI not initialized or entry %d not created",
			  __func__, id);
		rc = -EINVAL;
	} else if (0 == (hif->napi_data.ce_map & (0x01 << ce))) {
		HIF_ERROR("%s: NAPI instance %d (pipe %d) not created",
			  __func__, id, ce);
		rc = -EINVAL;
	} else {
		struct qca_napi_data *napid;
		struct qca_napi_info *napii;

		napid = &(hif->napi_data);
		napii = &(napid->napis[ce]);

		if (hif->napi_data.state == HIF_NAPI_CONF_UP) {
			if (force) {
				napi_disable(&(napii->napi));
				HIF_INFO("%s: NAPI entry %d force disabled",
					 __func__, id);
				NAPI_DEBUG("NAPI %d force disabled", id);
			} else {
				HIF_ERROR("%s: Cannot destroy active NAPI %d",
					  __func__, id);
				rc = -EPERM;
			}
		}
		if (0 == rc) {
			NAPI_DEBUG("before napi_del");
			NAPI_DEBUG("napi.dlist.prv=0x%p, next=0x%p",
				  napii->napi.dev_list.prev,
				  napii->napi.dev_list.next);
			NAPI_DEBUG("dev.napi_l.prv=0x%p, next=0x%p",
				   napii->netdev.napi_list.prev,
				   napii->netdev.napi_list.next);

			qdf_spinlock_destroy(&napii->lro_unloading_lock);
			netif_napi_del(&(napii->napi));

			napid->ce_map &= ~(0x01 << ce);
			napii->scale  = 0;
			HIF_INFO("%s: NAPI %d destroyed\n", __func__, id);

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
				HIF_INFO("%s: no NAPI instances. Zapped.",
					 __func__);
			}
		}
	}

	return rc;
}

/**
 * hif_napi_lro_flush_cb_register() - init and register flush callback for LRO
 * @hif_hdl: pointer to hif context
 * @lro_flush_handler: register LRO flush callback
 * @lro_init_handler: Callback for initializing LRO
 *
 * Return: positive value on success and 0 on failure
 */
int hif_napi_lro_flush_cb_register(struct hif_opaque_softc *hif_hdl,
				   void (lro_flush_handler)(void *),
				   void *(lro_init_handler)(void))
{
	int rc = 0;
	int i;
	struct CE_state *ce_state;
	struct hif_softc *scn = HIF_GET_SOFTC(hif_hdl);
	void *data = NULL;
	struct qca_napi_data *napid;
	struct qca_napi_info *napii;

	QDF_ASSERT(scn != NULL);

	napid = hif_napi_get_all(hif_hdl);
	if (scn != NULL) {
		for (i = 0; i < scn->ce_count; i++) {
			ce_state = scn->ce_id_to_state[i];
			if ((ce_state != NULL) && (ce_state->htt_rx_data)) {
				data = lro_init_handler();
				if (data == NULL) {
					HIF_ERROR("%s: Failed to init LRO for CE %d",
						  __func__, i);
					continue;
				}
				napii = &(napid->napis[i]);
				napii->lro_flush_cb = lro_flush_handler;
				napii->lro_ctx = data;
				HIF_ERROR("Registering LRO for ce_id %d NAPI callback for %d flush_cb %p, lro_data %p\n",
					i, napii->id, napii->lro_flush_cb,
					napii->lro_ctx);
				rc++;
			}
		}
	} else {
		HIF_ERROR("%s: hif_state NULL!", __func__);
	}
	return rc;
}

/**
 * hif_napi_lro_flush_cb_deregister() - Degregister and free LRO.
 * @hif: pointer to hif context
 * @lro_deinit_cb: LRO deinit callback
 *
 * Return: NONE
 */
void hif_napi_lro_flush_cb_deregister(struct hif_opaque_softc *hif_hdl,
				     void (lro_deinit_cb)(void *))
{
	int i;
	struct CE_state *ce_state;
	struct hif_softc *scn = HIF_GET_SOFTC(hif_hdl);
	struct qca_napi_data *napid;
	struct qca_napi_info *napii;

	QDF_ASSERT(scn != NULL);

	napid = hif_napi_get_all(hif_hdl);
	if (scn != NULL) {
		for (i = 0; i < scn->ce_count; i++) {
			ce_state = scn->ce_id_to_state[i];
			if ((ce_state != NULL) && (ce_state->htt_rx_data)) {
				napii = &(napid->napis[i]);
				HIF_ERROR("deRegistering LRO for ce_id %d NAPI callback for %d flush_cb %p, lro_data %p\n",
					i, napii->id, napii->lro_flush_cb,
					napii->lro_ctx);
				qdf_spin_lock_bh(&napii->lro_unloading_lock);
				napii->lro_flush_cb = NULL;
				lro_deinit_cb(napii->lro_ctx);
				napii->lro_ctx = NULL;
				qdf_spin_unlock_bh(
					&napii->lro_unloading_lock);
			}
		}
	} else {
		HIF_ERROR("%s: hif_state NULL!", __func__);
	}
}

/**
 * hif_napi_get_lro_info() - returns the address LRO data for napi_id
 * @hif: pointer to hif context
 * @napi_id: napi instance
 *
 * Description:
 *    Returns the address of the LRO structure
 *
 * Return:
 *  <addr>: address of the LRO structure
 */
void *hif_napi_get_lro_info(struct hif_opaque_softc *hif_hdl, int napi_id)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_hdl);
	struct qca_napi_data *napid;
	struct qca_napi_info *napii;

	napid = &(scn->napi_data);
	napii = &(napid->napis[NAPI_ID2PIPE(napi_id)]);

	return napii->lro_ctx;
}

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
	struct napi_struct *napi;
	struct hif_softc *hif = HIF_GET_SOFTC(hif_ctx);
	struct qca_napi_data *napid = &(hif->napi_data);
	enum qca_napi_tput_state tput_mode = QCA_NAPI_TPUT_UNINITIALIZED;
	enum {
		BLACKLIST_NOT_PENDING,
		BLACKLIST_ON_PENDING,
		BLACKLIST_OFF_PENDING
	     } blacklist_pending = BLACKLIST_NOT_PENDING;

	NAPI_DEBUG("%s: -->(event=%d, aux=%p)", __func__, event, data);

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

		HIF_INFO("%s: recved evnt: STATE_CMD %d; v = %d (state=0x%0x)",
			 __func__, event,
			 on, prev_state);
		if (on)
			if (prev_state & HIF_NAPI_CONF_UP) {
				HIF_INFO("%s: duplicate NAPI conf ON msg",
					 __func__);
			} else {
				HIF_INFO("%s: setting state to ON",
					 __func__);
				napid->state |= HIF_NAPI_CONF_UP;
			}
		else /* off request */
			if (prev_state & HIF_NAPI_CONF_UP) {
				HIF_INFO("%s: setting state to OFF",
				 __func__);
				napid->state &= ~HIF_NAPI_CONF_UP;
			} else {
				HIF_INFO("%s: duplicate NAPI conf OFF msg",
					 __func__);
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
		/*
		 * Deserialization timeout is handled at hdd layer;
		 * just mark current mode to uninitialized to ensure
		 * it will be set when the delay is over
		 */
		napid->napi_mode = QCA_NAPI_TPUT_UNINITIALIZED;
		break;
	}
	default: {
		HIF_ERROR("%s: unknown event: %d (data=0x%0lx)",
			  __func__, event, (unsigned long) data);
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

	qdf_spin_unlock_bh(&(napid->lock));

	if (prev_state != napid->state) {
		if (napid->state == ENABLE_NAPI_MASK) {
			rc = 1;
			for (i = 0; i < CE_COUNT_MAX; i++)
				if ((napid->ce_map & (0x01 << i))) {
					napi = &(napid->napis[i].napi);
					NAPI_DEBUG("%s: enabling NAPI %d",
						   __func__, i);
					napi_enable(napi);
				}
		} else {
			rc = 0;
			for (i = 0; i < CE_COUNT_MAX; i++)
				if (napid->ce_map & (0x01 << i)) {
					napi = &(napid->napis[i].napi);
					NAPI_DEBUG("%s: disabling NAPI %d",
						   __func__, i);
					napi_disable(napi);
					/* in case it is affined, remove it */
					irq_set_affinity_hint(
							napid->napis[i].irq,
							NULL);
				}
		}
	} else {
		HIF_INFO("%s: no change in hif napi state (still %d)",
			 __func__, prev_state);
	}

	NAPI_DEBUG("<--[rc=%d]", rc);
	return rc;
}

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
};

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
 * Return: void
 */
int hif_napi_schedule(struct hif_opaque_softc *hif_ctx, int ce_id)
{
	int cpu = smp_processor_id();
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	hif_record_ce_desc_event(scn,  ce_id, NAPI_SCHEDULE,
				 NULL, NULL, 0);

	scn->napi_data.napis[ce_id].stats[cpu].napi_schedules++;
	NAPI_DEBUG("scheduling napi %d (ce:%d)",
		   scn->napi_data.napis[ce_id].id, ce_id);
	napi_schedule(&(scn->napi_data.napis[ce_id].napi));

	return true;
}

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
	cpumask_t cpumask;
	int cpu;
	struct qca_napi_data *napid;

	napid = hif_napi_get_all(GET_HIF_OPAQUE_HDL(napi_info->hif_ctx));

	if (napid->flags & QCA_NAPI_FEATURE_CPU_CORRECTION) {

		cpu = qdf_get_cpu();
		if (unlikely((hif_napi_cpu_blacklist(napid,
						BLACKLIST_QUERY) > 0) &&
						(cpu != napi_info->cpu))) {
			right_cpu = false;

			NAPI_DEBUG("interrupt on wrong CPU, correcting");
			cpumask.bits[0] = (0x01 << napi_info->cpu);

			irq_modify_status(napi_info->irq, IRQ_NO_BALANCING, 0);
			rc = irq_set_affinity_hint(napi_info->irq,
						   &cpumask);
			irq_modify_status(napi_info->irq, 0, IRQ_NO_BALANCING);

			if (rc)
				HIF_ERROR("error setting irq affinity hint: %d",
					  rc);
			else
				napi_info->stats[cpu].cpu_corrected++;
		}
	}
	return right_cpu;
}

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

	NAPI_DEBUG("%s -->(napi(%d, irq=%d), budget=%d)",
		   __func__, napi_info->id, napi_info->irq, budget);

	if (unlikely(NULL == hif)) {
		HIF_ERROR("%s: hif context is NULL", __func__);
		QDF_ASSERT(0);
		goto out;
	}

	napi_info = (struct qca_napi_info *)
		container_of(napi, struct qca_napi_info, napi);

	napi_info->stats[cpu].napi_polls++;

	hif_record_ce_desc_event(hif, NAPI_ID2PIPE(napi_info->id),
				 NAPI_POLL_ENTER, NULL, NULL, cpu);

	qdf_spin_lock_bh(&napi_info->lro_unloading_lock);

	rc = ce_per_engine_service(hif, NAPI_ID2PIPE(napi_info->id));
	NAPI_DEBUG("%s: ce_per_engine_service processed %d msgs",
		    __func__, rc);

	if (napi_info->lro_flush_cb)
		napi_info->lro_flush_cb(napi_info->lro_ctx);
	qdf_spin_unlock_bh(&napi_info->lro_unloading_lock);

	/* do not return 0, if there was some work done,
	 * even if it is below the scale
	 */
	if (rc) {
		napi_info->stats[cpu].napi_workdone += rc;
		normalized = (rc / napi_info->scale);
		if (normalized == 0)
			normalized++;
		bucket = normalized / (QCA_NAPI_BUDGET / QCA_NAPI_NUM_BUCKETS);
		if (bucket >= QCA_NAPI_NUM_BUCKETS) {
			bucket = QCA_NAPI_NUM_BUCKETS - 1;
			HIF_ERROR("Bad bucket#(%d) > QCA_NAPI_NUM_BUCKETS(%d)",
				bucket, QCA_NAPI_NUM_BUCKETS);
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
					 NULL, NULL, 0);
		if (normalized >= budget)
			normalized = budget - 1;

		/* enable interrupts */
		napi_complete(napi);
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
				 NAPI_POLL_EXIT, NULL, NULL, normalized);

	NAPI_DEBUG("%s <--[normalized=%d]", __func__, normalized);
	return normalized;
out:
	return rc;
}

#ifdef HELIUMPLUS
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

	if (unlikely(NULL == ce_state)) {
		QDF_ASSERT(NULL != ce_state);
		return;
	}

	hif = ce_state->scn;

	if (unlikely(NULL == hif)) {
		QDF_ASSERT(NULL != hif);
		return;
	}
	napi_data = &(hif->napi_data);
	if (unlikely(NULL == napi_data)) {
		QDF_ASSERT(NULL != napi_data);
		return;
	}

	ce_id = ce_state->id;
	cpu_id = qdf_get_cpu();

	if (time_limit_reached)
		napi_data->napis[ce_id].stats[cpu_id].time_limit_reached++;
	else
		napi_data->napis[ce_id].stats[cpu_id].rxpkt_thresh_reached++;
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

	if (napid == NULL) {
		qdf_print("%s: napiid struct is null", __func__);
		return;
	}

	cpu = napid->napi_cpu;
	qdf_print("NAPI CPU TABLE");
	qdf_print("lilclhead=%d, bigclhead=%d",
		  napid->lilcl_head, napid->bigcl_head);
	for (i = 0; i < NR_CPUS; i++) {
		qdf_print("CPU[%02d]: state:%d crid=%02d clid=%02d crmk:0x%0lx thmk:0x%0lx frq:%d napi = 0x%08x lnk:%d",
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
#define HNC_MAX_CLUSTER 31
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
				QDF_ASSERT(0);
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
 * hnc_cpu_notify_cb() - handles CPU hotplug events
 *
 * On transitions to online, we onlu handle the ONLINE event,
 * and ignore the PREP events, because we dont want to act too
 * early.
 * On transtion to offline, we act on PREP events, because
 * we may need to move the irqs/NAPIs to another CPU before
 * it is actually off-lined.
 *
 * Return: NOTIFY_OK (dont block action)
 */
static int hnc_cpu_notify_cb(struct notifier_block *nb,
			     unsigned long          action,
			     void                  *hcpu)
{
	int rc = NOTIFY_OK;
	unsigned long cpu = (unsigned long)hcpu;
	struct hif_opaque_softc *hif;
	struct qca_napi_data *napid = NULL;

	NAPI_DEBUG("-->%s(act=%ld, cpu=%ld)", __func__, action, cpu);

	napid = qdf_container_of(nb, struct qca_napi_data, hnc_cpu_notifier);
	hif = &qdf_container_of(napid, struct hif_softc, napi_data)->osc;

	switch (action) {
	case CPU_ONLINE:
		napid->napi_cpu[cpu].state = QCA_NAPI_CPU_UP;
		NAPI_DEBUG("%s: CPU %ld marked %d",
			   __func__, cpu, napid->napi_cpu[cpu].state);
		break;
	case CPU_DEAD: /* already dead; we have marked it before, but ... */
	case CPU_DEAD_FROZEN:
		napid->napi_cpu[cpu].state = QCA_NAPI_CPU_DOWN;
		NAPI_DEBUG("%s: CPU %ld marked %d",
			   __func__, cpu, napid->napi_cpu[cpu].state);
		break;
	case CPU_DOWN_PREPARE:
	case CPU_DOWN_PREPARE_FROZEN:
		napid->napi_cpu[cpu].state = QCA_NAPI_CPU_DOWN;

		NAPI_DEBUG("%s: CPU %ld marked %d; updating affinity",
			   __func__, cpu, napid->napi_cpu[cpu].state);

		/**
		 * we need to move any NAPIs on this CPU out.
		 * if we are in LO throughput mode, then this is valid
		 * if the CPU is the the low designated CPU.
		 */
		hif_napi_event(hif,
			       NAPI_EVT_CPU_STATE,
			       (void *)
			       ((cpu << 16) | napid->napi_cpu[cpu].state));
		break;
	default:
		NAPI_DEBUG("%s: ignored. action: %ld", __func__, action);
		break;
	} /* switch */
	NAPI_DEBUG("<--%s [%d]", __func__, rc);
	return rc;
}

/**
 * hnc_hotplug_hook() - installs a hotplug notifier
 * @hif_sc: hif_sc context
 * @register: !0 => register , =0 => deregister
 *
 * Because the callback relies on the data layout of
 * struct hif_softc & its napi_data member, this callback
 * registration requires that the hif_softc is passed in.
 *
 * Note that this is different from the cpu notifier used by
 * rx_thread (cds_schedule.c).
 * We may consider combining these modifiers in the future.
 *
 * Return: 0: success
 *        <0: error
 */
static int hnc_hotplug_hook(struct hif_softc *hif_sc, int install)
{
	int rc = 0;

	NAPI_DEBUG("-->%s(%d)", __func__, install);

	if (install) {
		hif_sc->napi_data.hnc_cpu_notifier.notifier_call
			= hnc_cpu_notify_cb;
		rc = register_hotcpu_notifier(
			&hif_sc->napi_data.hnc_cpu_notifier);
	} else {
		unregister_hotcpu_notifier(
			&hif_sc->napi_data.hnc_cpu_notifier);
	}

	NAPI_DEBUG("<--%s()[%d]", __func__, rc);
	return rc;
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
		cpumask_copy(&(cpus[i].thread_mask),
			     topology_sibling_cpumask(i));
		cpus[i].max_freq    = cpufreq_quick_get_max(i);
		cpus[i].napis       = 0x0;
		cpus[i].cluster_nxt = -1; /* invalid */
	}

	/* link clusters together */
	rc = hnc_link_clusters(napid);
	if (0 != rc)
		goto lab_err_topology;

	/* install hotplug notifier */
	rc = hnc_hotplug_hook(HIF_GET_SOFTC(hif), 1);
	if (0 != rc)
		goto lab_err_hotplug;

	/* install throughput notifier */
	rc = hnc_tput_hook(1);
	if (0 == rc)
		goto lab_rss_init;

lab_err_hotplug:
	hnc_tput_hook(0);
	hnc_hotplug_hook(HIF_GET_SOFTC(hif), 0);
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
	rc = hnc_hotplug_hook(HIF_GET_SOFTC(hif), 0);

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
	cpumask_t cpumask;

	NAPI_DEBUG("-->%s(napi_cd=%d, didx=%d)", __func__, napi_ce, didx);

	cpumask.bits[0] = (1 << didx);

	irq_modify_status(napid->napis[napi_ce].irq, IRQ_NO_BALANCING, 0);
	rc = irq_set_affinity_hint(napid->napis[napi_ce].irq, &cpumask);

	/* unmark the napis bitmap in the cpu table */
	napid->napi_cpu[napid->napis[napi_ce].cpu].napis &= ~(0x01 << napi_ce);
	/* mark the napis bitmap for the new designated cpu */
	napid->napi_cpu[didx].napis |= (0x01 << napi_ce);
	napid->napis[napi_ce].cpu = didx;

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
		destidx = smallidx;
		if ((destidx < 0) && (head == napid->bigcl_head)) {
			NAPI_DEBUG("%s: DISPERSE: no bigcl dest, try lilcl",
				__func__);
			head = i = napid->lilcl_head;
			goto retry_disperse;
		}
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

	for (i = 0; i < CE_COUNT_MAX; i++) {
		/* check if NAPI is enabled on the CE */
		if (!(napid->ce_map & (0x01 << i)))
			continue;

		if (bl_flag == true)
			irq_modify_status(napid->napis[i].irq,
					  0, IRQ_NO_BALANCING);
		else
			irq_modify_status(napid->napis[i].irq,
					  IRQ_NO_BALANCING, 0);
		HIF_INFO("%s: bl_flag %d CE %d", __func__, bl_flag, i);
	}
}

#ifdef CONFIG_SCHED_CORE_CTL
/* Enable this API only if kernel feature - CONFIG_SCHED_CORE_CTL is defined */
static inline int hif_napi_core_ctl_set_boost(bool boost)
{
	return core_ctl_set_boost(boost);
}
#else
static inline int hif_napi_core_ctl_set_boost(bool boost)
{
	return 0;
}
#endif
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
		if (ref_count)
			ref_count--;
		rc = 0;
		if (ref_count == 0) {
			rc = hif_napi_core_ctl_set_boost(false);
			NAPI_DEBUG("boost_off() returns %d - refcnt=%d",
				   rc, ref_count);
			hif_napi_bl_irq(napid, false);
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

	if (hif != NULL)
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

#endif /* ifdef HELIUMPLUS */
