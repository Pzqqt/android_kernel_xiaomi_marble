/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
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
#include "dp_types.h"
#include "qdf_nbuf.h"
#include "qdf_atomic.h"
#include "qdf_types.h"
#include "dp_tx.h"
#include "dp_tx_desc.h"
#include "dp_internal.h"
#include "dp_txrx_me.h"
#define MAX_ME_BUF_CHUNK 1424
#define ME_US_TO_SEC(_x) ((_x) / (1000 * 1000))
#define ME_CLEAN_WAIT_TIMEOUT (200000) /*200ms*/
#define ME_CLEAN_WAIT_COUNT 400

/**
 * dp_tx_me_init():Initialize ME buffer ppol
 * @pdev: DP PDEV handle
 *
 * Return:0 on Succes 1 on failure
 */
static inline uint16_t
dp_tx_me_init(struct dp_pdev *pdev)
{
	uint16_t i, mc_uc_buf_len, num_pool_elems;
	uint32_t pool_size;

	struct dp_tx_me_buf_t *p;

	mc_uc_buf_len = sizeof(struct dp_tx_me_buf_t);

	num_pool_elems = MAX_ME_BUF_CHUNK;
	/* Add flow control buffer count */
	pool_size = (mc_uc_buf_len) * num_pool_elems;
	pdev->me_buf.size = mc_uc_buf_len;
	if (!(pdev->me_buf.vaddr)) {
		qdf_spin_lock_bh(&pdev->tx_mutex);
		pdev->me_buf.vaddr = qdf_mem_malloc(pool_size);
		if (!(pdev->me_buf.vaddr)) {
			qdf_spin_unlock_bh(&pdev->tx_mutex);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
			  "Error allocating memory pool");
			return 1;
		}
		pdev->me_buf.buf_in_use = 0;
		pdev->me_buf.freelist =
			(struct dp_tx_me_buf_t *)pdev->me_buf.vaddr;
		/*
		 * me_buf looks like this
		 * |=======+==========================|
		 * | ptr   |         Dst MAC          |
		 * |=======+==========================|
		 */
		p = pdev->me_buf.freelist;
		for (i = 0; i < num_pool_elems - 1; i++) {
			p->next = (struct dp_tx_me_buf_t *)
				((char *)p + pdev->me_buf.size);
			p = p->next;
		}
		p->next = NULL;
		qdf_spin_unlock_bh(&pdev->tx_mutex);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
			  "ME Pool successfully initialized vaddr - %x",
			  pdev->me_buf.vaddr);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
			  "paddr - %x\n", (unsigned int)pdev->me_buf.paddr);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
			  "num_elems = %d", (unsigned int)num_pool_elems);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
			  "buf_size - %d", (unsigned int)pdev->me_buf.size);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
			  "pool_size = %d", (unsigned int)pool_size);
	} else {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
			  "ME Already Enabled!!");
	}
	return 0;
}

/**
 * dp_tx_me_alloc_descriptor():Allocate ME descriptor
 * @pdev_handle: DP PDEV handle
 *
 * Return:void
 */
void
dp_tx_me_alloc_descriptor(struct cdp_pdev *pdev_handle)
{
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;

	if (qdf_atomic_read(&pdev->mc_num_vap_attached) == 0) {
		dp_tx_me_init(pdev);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
			  FL("Enable MCAST_TO_UCAST "));
	}
	qdf_atomic_inc(&pdev->mc_num_vap_attached);
}

/**
 * dp_tx_me_exit():Free memory and other cleanup required for
 * multicast unicast conversion
 * @pdev - DP_PDEV handle
 *
 * Return:void
 */
void
dp_tx_me_exit(struct dp_pdev *pdev)
{
	/* Add flow control buffer count */
	uint32_t wait_time = ME_US_TO_SEC(ME_CLEAN_WAIT_TIMEOUT *
			ME_CLEAN_WAIT_COUNT);

	if (pdev->me_buf.vaddr) {
		uint16_t wait_cnt = 0;

		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
			  "Disabling Mcastenhance,This may take some time");
		qdf_spin_lock_bh(&pdev->tx_mutex);
		while ((pdev->me_buf.buf_in_use > 0) &&
		       (wait_cnt < ME_CLEAN_WAIT_COUNT)) {
			qdf_spin_unlock_bh(&pdev->tx_mutex);
			OS_SLEEP(ME_CLEAN_WAIT_TIMEOUT);
			wait_cnt++;
			qdf_spin_lock_bh(&pdev->tx_mutex);
		}
		if (pdev->me_buf.buf_in_use > 0) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
				  "Tx-comp pending for %d",
				  pdev->me_buf.buf_in_use);
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
				  "ME frames after waiting %ds!!",
				  wait_time);
			qdf_assert_always(0);
		}

		qdf_mem_free(pdev->me_buf.vaddr);
		pdev->me_buf.vaddr = NULL;
		pdev->me_buf.freelist = NULL;
		qdf_spin_unlock_bh(&pdev->tx_mutex);
	} else {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
			  "ME Already Disabled !!!");
	}
}

/**
 * dp_tx_me_free_descriptor():free ME descriptor
 * @pdev_handle:DP_PDEV handle
 *
 * Return:void
 */
void
dp_tx_me_free_descriptor(struct cdp_pdev *pdev_handle)
{
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;

	qdf_atomic_dec(&pdev->mc_num_vap_attached);
	if (atomic_read(&pdev->mc_num_vap_attached) == 0) {
		dp_tx_me_exit(pdev);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
			  "Disable MCAST_TO_UCAST");
	}
}

/**
 * dp_tx_prepare_send_me(): Call to the umac to get the list of clients
 * @vdev: DP VDEV handle
 * @nbuf: Multicast buffer
 *
 * Return: no of packets transmitted
 */
QDF_STATUS
dp_tx_prepare_send_me(struct dp_vdev *vdev, qdf_nbuf_t nbuf)
{
	if (vdev->me_convert) {
		if (vdev->me_convert(vdev->osif_vdev, nbuf) > 0)
			return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_E_FAILURE;
}

/*
 * dp_tx_me_mem_free(): Function to free allocated memory in mcast enahncement
 * pdev: pointer to DP PDEV structure
 * seg_info_head: Pointer to the head of list
 *
 * return: void
 */
static void dp_tx_me_mem_free(struct dp_pdev *pdev,
			      struct dp_tx_seg_info_s *seg_info_head)
{
	struct dp_tx_me_buf_t *mc_uc_buf;
	struct dp_tx_seg_info_s *seg_info_new = NULL;
	qdf_nbuf_t nbuf = NULL;
	uint64_t phy_addr;

	while (seg_info_head) {
		nbuf = seg_info_head->nbuf;
		mc_uc_buf = (struct dp_tx_me_buf_t *)
			seg_info_head->frags[0].vaddr;
		phy_addr = seg_info_head->frags[0].paddr_hi;
		phy_addr =  (phy_addr << 32) | seg_info_head->frags[0].paddr_lo;
		qdf_mem_unmap_nbytes_single(pdev->soc->osdev,
					    phy_addr,
					    QDF_DMA_TO_DEVICE, QDF_MAC_ADDR_SIZE);
		dp_tx_me_free_buf(pdev, mc_uc_buf);
		qdf_nbuf_free(nbuf);
		seg_info_new = seg_info_head;
		seg_info_head = seg_info_head->next;
		qdf_mem_free(seg_info_new);
	}
}

/**
 * dp_tx_me_send_convert_ucast(): function to convert multicast to unicast
 * @vdev: DP VDEV handle
 * @nbuf: Multicast nbuf
 * @newmac: Table of the clients to which packets have to be sent
 * @new_mac_cnt: No of clients
 *
 * return: no of converted packets
 */
uint16_t
dp_tx_me_send_convert_ucast(struct cdp_vdev *vdev_handle, qdf_nbuf_t nbuf,
		uint8_t newmac[][QDF_MAC_ADDR_SIZE], uint8_t new_mac_cnt)
{
	struct dp_vdev *vdev = (struct dp_vdev *) vdev_handle;
	struct dp_pdev *pdev = vdev->pdev;
	qdf_ether_header_t *eh;
	uint8_t *data;
	uint16_t len;

	/* reference to frame dst addr */
	uint8_t *dstmac;
	/* copy of original frame src addr */
	uint8_t srcmac[QDF_MAC_ADDR_SIZE];

	/* local index into newmac */
	uint8_t new_mac_idx = 0;
	struct dp_tx_me_buf_t *mc_uc_buf;
	qdf_nbuf_t  nbuf_clone;
	struct dp_tx_msdu_info_s msdu_info;
	struct dp_tx_seg_info_s *seg_info_head = NULL;
	struct dp_tx_seg_info_s *seg_info_tail = NULL;
	struct dp_tx_seg_info_s *seg_info_new;
	qdf_dma_addr_t paddr_data;
	qdf_dma_addr_t paddr_mcbuf = 0;
	uint8_t empty_entry_mac[QDF_MAC_ADDR_SIZE] = {0};
	QDF_STATUS status;

	qdf_mem_zero(&msdu_info, sizeof(msdu_info));

	dp_tx_get_queue(vdev, nbuf, &msdu_info.tx_queue);

	eh = (qdf_ether_header_t *)nbuf;
	qdf_mem_copy(srcmac, eh->ether_shost, QDF_MAC_ADDR_SIZE);

	len = qdf_nbuf_len(nbuf);

	data = qdf_nbuf_data(nbuf);

	status = qdf_nbuf_map(vdev->osdev, nbuf,
			QDF_DMA_TO_DEVICE);

	if (status) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"Mapping failure Error:%d", status);
		DP_STATS_INC(vdev, tx_i.mcast_en.dropped_map_error, 1);
		qdf_nbuf_free(nbuf);
		return 1;
	}

	paddr_data = qdf_nbuf_mapped_paddr_get(nbuf) + QDF_MAC_ADDR_SIZE;

	for (new_mac_idx = 0; new_mac_idx < new_mac_cnt; new_mac_idx++) {
		dstmac = newmac[new_mac_idx];
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
				"added mac addr (%pM)", dstmac);

		/* Check for NULL Mac Address */
		if (!qdf_mem_cmp(dstmac, empty_entry_mac, QDF_MAC_ADDR_SIZE))
			continue;

		/* frame to self mac. skip */
		if (!qdf_mem_cmp(dstmac, srcmac, QDF_MAC_ADDR_SIZE))
			continue;

		/*
		 * optimize to avoid malloc in per-packet path
		 * For eg. seg_pool can be made part of vdev structure
		 */
		seg_info_new = qdf_mem_malloc(sizeof(*seg_info_new));

		if (!seg_info_new) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
					"alloc failed");
			DP_STATS_INC(vdev, tx_i.mcast_en.fail_seg_alloc, 1);
			goto fail_seg_alloc;
		}

		mc_uc_buf = dp_tx_me_alloc_buf(pdev);
		if (!mc_uc_buf)
			goto fail_buf_alloc;

		/*
		 * Check if we need to clone the nbuf
		 * Or can we just use the reference for all cases
		 */
		if (new_mac_idx < (new_mac_cnt - 1)) {
			nbuf_clone = qdf_nbuf_clone((qdf_nbuf_t)nbuf);
			if (!nbuf_clone) {
				DP_STATS_INC(vdev, tx_i.mcast_en.clone_fail, 1);
				goto fail_clone;
			}
		} else {
			/*
			 * Update the ref
			 * to account for frame sent without cloning
			 */
			qdf_nbuf_ref(nbuf);
			nbuf_clone = nbuf;
		}

		qdf_mem_copy(mc_uc_buf->data, dstmac, QDF_MAC_ADDR_SIZE);

		status = qdf_mem_map_nbytes_single(vdev->osdev, mc_uc_buf->data,
				QDF_DMA_TO_DEVICE, QDF_MAC_ADDR_SIZE,
				&paddr_mcbuf);

		if (status) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
					"Mapping failure Error:%d", status);
			DP_STATS_INC(vdev, tx_i.mcast_en.dropped_map_error, 1);
			goto fail_map;
		}

		seg_info_new->frags[0].vaddr =  (uint8_t *)mc_uc_buf;
		seg_info_new->frags[0].paddr_lo = (uint32_t) paddr_mcbuf;
		seg_info_new->frags[0].paddr_hi =
			(uint16_t)((uint64_t)paddr_mcbuf >> 32);
		seg_info_new->frags[0].len = QDF_MAC_ADDR_SIZE;

		/*preparing data fragment*/
		seg_info_new->frags[1].vaddr =
			qdf_nbuf_data(nbuf) + QDF_MAC_ADDR_SIZE;
		seg_info_new->frags[1].paddr_lo = (uint32_t)paddr_data;
		seg_info_new->frags[1].paddr_hi =
			(uint16_t)(((uint64_t)paddr_data) >> 32);
		seg_info_new->frags[1].len = len - QDF_MAC_ADDR_SIZE;

		seg_info_new->nbuf = nbuf_clone;
		seg_info_new->frag_cnt = 2;
		seg_info_new->total_len = len;

		seg_info_new->next = NULL;

		if (!seg_info_head)
			seg_info_head = seg_info_new;
		else
			seg_info_tail->next = seg_info_new;

		seg_info_tail = seg_info_new;
	}

	if (!seg_info_head) {
		goto free_return;
	}

	msdu_info.u.sg_info.curr_seg = seg_info_head;
	msdu_info.num_seg = new_mac_cnt;
	msdu_info.frm_type = dp_tx_frm_me;

	msdu_info.tid = HTT_INVALID_TID;
	if (qdf_unlikely(vdev->mcast_enhancement_en > 0) &&
	    qdf_unlikely(pdev->hmmc_tid_override_en))
		msdu_info.tid = pdev->hmmc_tid;

	DP_STATS_INC(vdev, tx_i.mcast_en.ucast, new_mac_cnt);
	dp_tx_send_msdu_multiple(vdev, nbuf, &msdu_info);

	while (seg_info_head->next) {
		seg_info_new = seg_info_head;
		seg_info_head = seg_info_head->next;
		qdf_mem_free(seg_info_new);
	}
	qdf_mem_free(seg_info_head);

	qdf_nbuf_unmap(pdev->soc->osdev, nbuf, QDF_DMA_TO_DEVICE);
	qdf_nbuf_free(nbuf);
	return new_mac_cnt;

fail_map:
	qdf_nbuf_free(nbuf_clone);

fail_clone:
	dp_tx_me_free_buf(pdev, mc_uc_buf);

fail_buf_alloc:
	qdf_mem_free(seg_info_new);

fail_seg_alloc:
	dp_tx_me_mem_free(pdev, seg_info_head);

free_return:
	qdf_nbuf_unmap(pdev->soc->osdev, nbuf, QDF_DMA_TO_DEVICE);
	qdf_nbuf_free(nbuf);
	return 1;
}
