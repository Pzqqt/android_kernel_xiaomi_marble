/*
 * Copyright (c) 2015-2020 The Linux Foundation. All rights reserved.
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
 * DOC: hif_irq_afinity.c
 *
 * This irq afinity implementation is os dependent, so this can be treated as
 * an abstraction layer...  Should this be moved into a /linux folder?
 */

#include <linux/string.h> /* memset */

/* Linux headers */
#include <linux/cpumask.h>
#include <linux/cpufreq.h>
#include <linux/cpu.h>
#include <linux/topology.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/pm.h>
#include <hif_napi.h>
#include <hif_irq_affinity.h>
#include <hif_exec.h>
#include <hif_main.h>

#if defined(FEATURE_NAPI_DEBUG) && defined(HIF_IRQ_AFFINITY)
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

#ifdef HIF_IRQ_AFFINITY
/**
 *
 * hif_exec_event() - reacts to events that impact irq affinity
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
int hif_exec_event(struct hif_opaque_softc *hif_ctx, enum qca_napi_event event,
		   void *data)
{
	int      rc = 0;
	uint32_t prev_state;
	struct hif_softc *hif = HIF_GET_SOFTC(hif_ctx);
	struct qca_napi_data *napid = &(hif->napi_data);
	enum qca_napi_tput_state tput_mode = QCA_NAPI_TPUT_UNINITIALIZED;
	enum {
		BLACKLIST_NOT_PENDING,
		BLACKLIST_ON_PENDING,
		BLACKLIST_OFF_PENDING
	     } blacklist_pending = BLACKLIST_NOT_PENDING;

	NAPI_DEBUG("%s: -->(event=%d, aux=%pK)", __func__, event, data);

	qdf_spin_lock_bh(&(napid->lock));
	prev_state = napid->state;
	switch (event) {
	case NAPI_EVT_INI_FILE:
	case NAPI_EVT_CMD_STATE:
	case NAPI_EVT_INT_STATE:
		/* deprecated */
		break;

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
			rc = hif_exec_cpu_migrate(napid,
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
			rc = hif_exec_cpu_migrate(napid,
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

		rc = hif_exec_cpu_migrate(napid,
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

	qdf_spin_unlock_bh(&(napid->lock));

	NAPI_DEBUG("<--[rc=%d]", rc);
	return rc;
}

#endif

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
static int hncm_exec_migrate_to(struct qca_napi_data *napid, uint8_t ctx_id,
				int didx)
{
	struct hif_exec_context *exec_ctx;
	int rc = 0;
	int status = 0;
	int ind;

	NAPI_DEBUG("-->%s(napi_cd=%d, didx=%d)", __func__, ctx_id, didx);

	exec_ctx = hif_exec_get_ctx(&napid->hif_softc->osc, ctx_id);
	if (!exec_ctx)
		return -EINVAL;

	exec_ctx->cpumask.bits[0] = (1 << didx);

	for (ind = 0; ind < exec_ctx->numirq; ind++) {
		if (exec_ctx->os_irq[ind]) {
			irq_modify_status(exec_ctx->os_irq[ind],
					  IRQ_NO_BALANCING, 0);
			rc = irq_set_affinity_hint(exec_ctx->os_irq[ind],
						   &exec_ctx->cpumask);
			if (rc)
				status = rc;
		}
	}

	/* unmark the napis bitmap in the cpu table */
	napid->napi_cpu[exec_ctx->cpu].napis &= ~(0x01 << ctx_id);
	/* mark the napis bitmap for the new designated cpu */
	napid->napi_cpu[didx].napis |= (0x01 << ctx_id);
	exec_ctx->cpu = didx;

	NAPI_DEBUG("<--%s[%d]", __func__, rc);
	return status;
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
int hif_exec_cpu_migrate(struct qca_napi_data *napid, int cpu, int action)
{
	int      rc = 0;
	struct qca_napi_cpu *cpup;
	int      i, dind;
	uint32_t napis;


	NAPI_DEBUG("-->%s(.., cpu=%d, act=%d)",
		   __func__, cpu, action);

	if (napid->exec_map == 0) {
		NAPI_DEBUG("%s: datapath contexts to disperse", __func__);
		goto hncm_return;
	}
	cpup = napid->napi_cpu;

	switch (action) {
	case HNC_ACT_RELOCATE:
	case HNC_ACT_DISPERSE:
	case HNC_ACT_COLLAPSE: {
		/* first find the src napi set */
		if (cpu == HNC_ANY_CPU)
			napis = napid->exec_map;
		else
			napis = cpup[cpu].napis;
		/* then clear the napi bitmap on each CPU */
		for (i = 0; i < NR_CPUS; i++)
			cpup[i].napis = 0;
		/* then for each of the NAPIs to disperse: */
		for (i = 0; i < HIF_MAX_GROUP; i++)
			if (napis & (1 << i)) {
				/* find a destination CPU */
				dind = hncm_dest_cpu(napid, action);
				if (dind >= 0) {
					rc = hncm_exec_migrate_to(napid, i,
								  dind);
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
 * hif_exec_bl_irq() - calls irq_modify_status to enable/disable blacklisting
 * @napid: pointer to qca_napi_data structure
 * @bl_flag: blacklist flag to enable/disable blacklisting
 *
 * The function enables/disables blacklisting for all the copy engine
 * interrupts on which NAPI is enabled.
 *
 * Return: None
 */
static inline void hif_exec_bl_irq(struct qca_napi_data *napid, bool bl_flag)
{
	int i, j;
	struct hif_exec_context *exec_ctx;

	for (i = 0; i < HIF_MAX_GROUP; i++) {
		/* check if NAPI is enabled on the CE */
		if (!(napid->exec_map & (0x01 << i)))
			continue;

		/*double check that NAPI is allocated for the CE */
		exec_ctx = hif_exec_get_ctx(&napid->hif_softc->osc, i);
		if (!(exec_ctx))
			continue;

		if (bl_flag == true)
			for (j = 0; j < exec_ctx->numirq; j++)
				irq_modify_status(exec_ctx->os_irq[j],
						  0, IRQ_NO_BALANCING);
		else
			for (j = 0; j < exec_ctx->numirq; j++)
				irq_modify_status(exec_ctx->os_irq[j],
						  IRQ_NO_BALANCING, 0);
		HIF_DBG("%s: bl_flag %d CE %d", __func__, bl_flag, i);
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
int hif_exec_cpu_blacklist(struct qca_napi_data *napid,
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
			hif_exec_bl_irq(napid, true);
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
			hif_exec_bl_irq(napid, false);
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

