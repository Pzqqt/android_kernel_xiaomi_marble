/*
 * Copyright (c) 2015-2016 The Linux Foundation. All rights reserved.
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

/**
 * hif_napi_create() - creates the NAPI structures for a given CE
 * @hif    : pointer to hif context
 * @pipe_id: the CE id on which the instance will be created
 * @poll   : poll function to be used for this NAPI instance
 * @budget : budget to be registered with the NAPI instance
 * @scale  : scale factor on the weight (to scaler budget to 1000)
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
		    int                scale)
{
	int i;
	struct qca_napi_data *napid;
	struct qca_napi_info *napii;
	struct CE_state      *ce_state;
	struct hif_softc *hif = HIF_GET_SOFTC(hif_ctx);

	NAPI_DEBUG("-->(budget=%d, scale=%d)",
		   budget, scale);
	NAPI_DEBUG("hif->napi_data.state = 0x%08x",
		   hif->napi_data.state);
	NAPI_DEBUG("hif->napi_data.ce_map = 0x%08x",
		   hif->napi_data.ce_map);

	napid = &(hif->napi_data);
	if (0 == (napid->state &  HIF_NAPI_INITED)) {
		memset(napid, 0, sizeof(struct qca_napi_data));
		mutex_init(&(napid->mutex));

		napid->state |= HIF_NAPI_INITED;
		HIF_INFO("%s: NAPI structures initialized", __func__);

		NAPI_DEBUG("NAPI structures initialized");
	}
	for (i = 0; i < hif->ce_count; i++) {
		ce_state = hif->ce_id_to_state[i];
		NAPI_DEBUG("ce %d: htt_rx=%d htt_rx=%d",
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
	NAPI_DEBUG("NAPI idscreated for pipe all applicable pipes");
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
		HIF_ERROR("%s: NAPI not initialized or entry %d not created\n",
			  __func__, id);
		rc = -EINVAL;
	} else if (0 == (hif->napi_data.ce_map & (0x01 << ce))) {
		HIF_ERROR("%s: NAPI instance %d (pipe %d) not created\n",
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
				HIF_INFO("%s: NAPI entry %d force disabled\n",
					 __func__, id);
				NAPI_DEBUG("NAPI %d force disabled", id);
			} else {
				HIF_ERROR("%s: Cannot destroy active NAPI %d\n",
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

			netif_napi_del(&(napii->napi));

			napid->ce_map &= ~(0x01 << ce);
			napii->scale  = 0;
			HIF_INFO("%s: NAPI %d destroyed\n", __func__, id);

			/* if there are no active instances and
			 * if they are all destroyed,
			 * set the whole structure to uninitialized state
			 */
			if (napid->ce_map == 0) {
				/* hif->napi_data.state = 0; */
				memset(napid,
				       0, sizeof(struct qca_napi_data));
				HIF_INFO("%s: no NAPI instances. Zapped.\n",
					 __func__);
			}
		}
	}

	return rc;
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
 * hif_napi_event() - Decision-maker to enable/disable NAPI.
 * @hif : pointer to hif context
 * @evnt: event that has been detected
 * @data: more data regarding the event
 *
 * Description:
 *   This function decides whether or not NAPI should be enabled.
 *   NAPI will be enabled, if all the following is satisfied.
 *    1- has been enabled administratively:
 *       the .ini file has the enabled setting and it has not been disabled
 *           by an vendor command override later
 *
 * Return:
 *  < 0: some error
 *  = 0: NAPI is now disabled
 *  = 1: NAPI is now enabled
 */
int hif_napi_event(struct hif_opaque_softc *hif_ctx, enum qca_napi_event event,
		   void *data)
{
	int      rc;
	uint32_t prev_state;
	int      i;
	struct napi_struct *napi;
	struct hif_softc *hif = HIF_GET_SOFTC(hif_ctx);

	NAPI_DEBUG("-->(event=%d, aux=%p)", event, data);

	mutex_lock(&(hif->napi_data.mutex));
	prev_state = hif->napi_data.state;
	switch (event) {
	case NAPI_EVT_INI_FILE:
	case NAPI_EVT_CMD_STATE: {
		int on = (data != ((void *)0));

		HIF_INFO("%s: received evnt: CONF %s; v = %d (state=0x%0x)\n",
			 __func__,
			 (event == NAPI_EVT_INI_FILE)?".ini file":"cmd",
			 on, prev_state);
		if (on)
			if (prev_state & HIF_NAPI_CONF_UP) {
				HIF_INFO("%s: duplicate NAPI conf ON msg\n",
					 __func__);
			} else {
				HIF_INFO("%s: setting configuration to ON\n",
					 __func__);
				hif->napi_data.state |= HIF_NAPI_CONF_UP;
			}
		else /* off request */
			if (prev_state & HIF_NAPI_CONF_UP) {
				HIF_INFO("%s: setting configuration to OFF\n",
				 __func__);
				hif->napi_data.state &= ~HIF_NAPI_CONF_UP;
			} else {
				HIF_INFO("%s: duplicate NAPI conf OFF msg\n",
					 __func__);
			}
		break;
	}
	/* case NAPI_INIT_FILE/CMD_STATE */
	default: {
		HIF_ERROR("%s: unknown event: %d (data=0x%0lx)\n",
			  __func__, event, (unsigned long) data);
		break;
	} /* default */
	}; /* switch */


	mutex_unlock(&(hif->napi_data.mutex));

	if (prev_state != hif->napi_data.state) {
		if (hif->napi_data.state == ENABLE_NAPI_MASK) {
			rc = 1;
			for (i = 0; i < CE_COUNT_MAX; i++)
				if ((hif->napi_data.ce_map & (0x01 << i))) {
					napi = &(hif->napi_data.napis[i].napi);
					NAPI_DEBUG("enabling NAPI %d", i);
					napi_enable(napi);
				}
		} else {
			rc = 0;
			for (i = 0; i < CE_COUNT_MAX; i++)
				if (hif->napi_data.ce_map & (0x01 << i)) {
					napi = &(hif->napi_data.napis[i].napi);
					NAPI_DEBUG("disabling NAPI %d", i);
					napi_disable(napi);
				}
		}
	} else {
		HIF_INFO("%s: no change in hif napi state (still %d)\n",
			 __func__, prev_state);
		rc = (hif->napi_data.state == ENABLE_NAPI_MASK);
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
 *  int: the amount of work done in this poll ( <= budget)
 */
int hif_napi_poll(struct hif_opaque_softc *hif_ctx, struct napi_struct *napi,
								int budget)
{
	int    rc = 0; /* default: no work done, also takes care of error */
	int    normalized, bucket;
	int    cpu = smp_processor_id();
	struct hif_softc      *hif = HIF_GET_SOFTC(hif_ctx);
	struct qca_napi_info *napi_info;
	struct CE_state *ce_state = NULL;

	NAPI_DEBUG("%s -->(.., budget=%d)", budget);

	napi_info = (struct qca_napi_info *)
		container_of(napi, struct qca_napi_info, napi);
	napi_info->stats[cpu].napi_polls++;

	hif_record_ce_desc_event(hif, NAPI_ID2PIPE(napi_info->id),
				 NAPI_POLL_ENTER, NULL, NULL, cpu);

	if (unlikely(NULL == hif))
		QDF_ASSERT(hif != NULL); /* emit a warning if hif NULL */
	else {
		rc = ce_per_engine_service(hif, NAPI_ID2PIPE(napi_info->id));
		NAPI_DEBUG("%s: ce_per_engine_service processed %d msgs",
			    __func__, rc);
	}
	napi_info->stats[cpu].napi_workdone += rc;
	normalized = (rc / napi_info->scale);

	if (NULL != hif) {
		ce_state = hif->ce_id_to_state[NAPI_ID2PIPE(napi_info->id)];
		if (ce_state->lro_flush_cb != NULL) {
			ce_state->lro_flush_cb(ce_state->lro_data);
		}
	}

	/* do not return 0, if there was some work done,
	 * even if it is below the scale
	 */
	if (rc)
		normalized++;
	bucket   = (normalized / QCA_NAPI_DEF_SCALE);
	napi_info->stats[cpu].napi_budget_uses[bucket]++;

	/* if ce_per engine reports 0, then poll should be terminated */
	if (0 == rc)
		NAPI_DEBUG("%s:%d: nothing processed by CE. Completing NAPI",
			   __func__, __LINE__);

	if ((ce_state != NULL && !ce_check_rx_pending(ce_state)) || 0 == rc) {
		napi_info->stats[cpu].napi_completes++;

		hif_record_ce_desc_event(hif, ce_state->id, NAPI_COMPLETE,
					 NULL, NULL, 0);
		/* enable interrupts */
		napi_complete(napi);
		if (NULL != hif) {
			hif_napi_enable_irq(hif_ctx, napi_info->id);

			/* support suspend/resume */
			qdf_atomic_dec(&(hif->active_tasklet_cnt));
		}

		NAPI_DEBUG("%s:%d: napi_complete + enabling the interrupts",
			   __func__, __LINE__);
	}

	hif_record_ce_desc_event(hif, NAPI_ID2PIPE(napi_info->id),
				 NAPI_POLL_EXIT, NULL, NULL, normalized);

	NAPI_DEBUG("%s <--[normalized=%d]", _func__, normalized);
	return normalized;
}
