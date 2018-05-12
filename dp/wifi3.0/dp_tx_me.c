/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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

#include "dp_types.h"
#include "qdf_nbuf.h"
#include "qdf_atomic.h"
#include "qdf_types.h"
#include "dp_tx.h"
#include "dp_tx_desc.h"
#include "dp_internal.h"

#ifdef ATH_SUPPORT_IQUE
#define MAX_ME_BUF_CHUNK 1424
#define ME_US_TO_SEC(_x) ((_x)/(1000 * 1000))
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
	if (pdev->me_buf.vaddr == NULL) {
		qdf_spin_lock_bh(&pdev->tx_mutex);
		pdev->me_buf.vaddr = qdf_mem_malloc(pool_size);
		if (pdev->me_buf.vaddr == NULL) {
			qdf_spin_unlock_bh(&pdev->tx_mutex);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
				"Error allocating memory pool");
			return 1;
		}
		pdev->me_buf.buf_in_use = 0;
		pdev->me_buf.freelist =
			(struct dp_tx_me_buf_t *) pdev->me_buf.vaddr;
		/*
		 * me_buf looks like this
		 * |=======+==========================|
		 * | ptr   |         Dst MAC          |
		 * |=======+==========================|
		 */
		p = pdev->me_buf.freelist;
		for (i = 0; i < num_pool_elems-1; i++) {
			p->next = (struct dp_tx_me_buf_t *)
				((char *)p + pdev->me_buf.size);
			p = p->next;
		}
		p->next = NULL;
		qdf_spin_unlock_bh(&pdev->tx_mutex);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
				"ME Pool successfully initialized vaddr - %x \
				paddr - %x\n num_elems = %d buf_size - %d"
				"pool_size = %d",
				pdev->me_buf.vaddr,
				(unsigned int)pdev->me_buf.paddr,
				(unsigned int)num_pool_elems,
				(unsigned int)pdev->me_buf.size,
				(unsigned int)pool_size);
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
	struct dp_pdev *pdev = (struct dp_pdev *) pdev_handle;
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
				"Disabling Mcastenhance"
				"This may take some time");
		qdf_spin_lock_bh(&pdev->tx_mutex);
		while ((pdev->me_buf.buf_in_use > 0) &&
				(wait_cnt < ME_CLEAN_WAIT_COUNT)) {
			qdf_spin_unlock_bh(&pdev->tx_mutex);
			OS_SLEEP(ME_CLEAN_WAIT_TIMEOUT);
			wait_cnt++;
			qdf_spin_lock_bh(&pdev->tx_mutex);
		}
		if (pdev->me_buf.buf_in_use > 0) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
					"Tx-comp pending for %d "
					"ME frames after waiting %ds!!\n",
					pdev->me_buf.buf_in_use, wait_time);
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
	struct dp_pdev *pdev = (struct dp_pdev *) pdev_handle;
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

#endif
