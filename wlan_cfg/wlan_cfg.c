/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#if defined(CONFIG_HL_SUPPORT)
#include "wlan_tgt_def_config_hl.h"
#else
#include "wlan_tgt_def_config.h"
#endif

#include "qdf_trace.h"
#include "qdf_mem.h"
#include <cdp_txrx_ops.h>
#include "wlan_cfg.h"
#include "cfg_ucfg_api.h"
#include "hal_api.h"
#include "dp_types.h"
#include <qdf_module.h>

/*
 * The max allowed size for tx comp ring is 8191.
 * This is limited by h/w ring max size.
 * As this is not a power of 2 it does not work with nss offload so the
 * nearest available size which is power of 2 is 4096 chosen for nss
 */

#define WLAN_CFG_TX_RING_MASK_0 BIT(0)
#define WLAN_CFG_TX_RING_MASK_1 BIT(1)
#define WLAN_CFG_TX_RING_MASK_2 BIT(2)
#define WLAN_CFG_TX_RING_MASK_3 BIT(3)
#define WLAN_CFG_TX_RING_MASK_4 BIT(4)
#define WLAN_CFG_TX_RING_MASK_5 BIT(5)
#define WLAN_CFG_TX_RING_MASK_6 BIT(6)
#define WLAN_CFG_TX_RING_MASK_7 BIT(7)


#define WLAN_CFG_RX_MON_RING_MASK_0 0x1
#define WLAN_CFG_RX_MON_RING_MASK_1 0x2
#define WLAN_CFG_RX_MON_RING_MASK_2 0x4
#define WLAN_CFG_RX_MON_RING_MASK_3 0x0

#define WLAN_CFG_TX_MON_RING_MASK_0 BIT(0)
#define WLAN_CFG_TX_MON_RING_MASK_1 BIT(1)

#define WLAN_CFG_HOST2RXDMA_MON_RING_MASK_0 0x1
#define WLAN_CFG_HOST2RXDMA_MON_RING_MASK_1 0x2
#define WLAN_CFG_HOST2RXDMA_MON_RING_MASK_2 0x4

#define WLAN_CFG_RXDMA2HOST_MON_RING_MASK_0 0x1
#define WLAN_CFG_RXDMA2HOST_MON_RING_MASK_1 0x2
#define WLAN_CFG_RXDMA2HOST_MON_RING_MASK_2 0x4

#define WLAN_CFG_RX_ERR_RING_MASK_0 0x1
#define WLAN_CFG_RX_ERR_RING_MASK_1 0x0
#define WLAN_CFG_RX_ERR_RING_MASK_2 0x0
#define WLAN_CFG_RX_ERR_RING_MASK_3 0x0

#define WLAN_CFG_RX_WBM_REL_RING_MASK_0 0x1
#define WLAN_CFG_RX_WBM_REL_RING_MASK_1 0x0
#define WLAN_CFG_RX_WBM_REL_RING_MASK_2 0x0
#define WLAN_CFG_RX_WBM_REL_RING_MASK_3 0x0

#define WLAN_CFG_REO_STATUS_RING_MASK_0 0x1
#define WLAN_CFG_REO_STATUS_RING_MASK_1 0x0
#define WLAN_CFG_REO_STATUS_RING_MASK_2 0x0
#define WLAN_CFG_REO_STATUS_RING_MASK_3 0x0

#define WLAN_CFG_RXDMA2HOST_RING_MASK_0 0x1
#define WLAN_CFG_RXDMA2HOST_RING_MASK_1 0x2
#define WLAN_CFG_RXDMA2HOST_RING_MASK_2 0x4
#define WLAN_CFG_RXDMA2HOST_RING_MASK_3 0x0

#define WLAN_CFG_HOST2RXDMA_RING_MASK_0 0x1
#define WLAN_CFG_HOST2RXDMA_RING_MASK_1 0x2
#define WLAN_CFG_HOST2RXDMA_RING_MASK_2 0x4
#define WLAN_CFG_HOST2RXDMA_RING_MASK_3 0x0

struct dp_int_mask_assignment {
	uint8_t tx_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS];
	uint8_t rx_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS];
	uint8_t rx_mon_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS];
	uint8_t tx_mon_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS];
	uint8_t host2rxdma_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS];
	uint8_t rxdma2host_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS];
	uint8_t host2rxdma_mon_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS];
	uint8_t rxdma2host_mon_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS];
	uint8_t rx_err_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS];
	uint8_t rx_wbm_rel_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS];
	uint8_t reo_status_ring_mask[WLAN_CFG_INT_NUM_CONTEXTS];
	uint8_t rx_ring_near_full_irq_1_mask[WLAN_CFG_INT_NUM_CONTEXTS];
	uint8_t rx_ring_near_full_irq_2_mask[WLAN_CFG_INT_NUM_CONTEXTS];
	uint8_t tx_ring_near_full_irq_mask[WLAN_CFG_INT_NUM_CONTEXTS];
};

#if defined(WLAN_MAX_PDEVS) && (WLAN_MAX_PDEVS == 1)
/*
 * For BE, there are 18 available MSI interrupts, assigned in the manner
 * below.
 * TX(5) + RX(8) + (REO ERR + WBM ERR)(1) +
 * (REO status + RXDMA[0] + RXDMA[1])(1) + NEAR_Full_RX(2) +  NEAR_Full_TX(1)
 * For IPA_OFFLOAD enabled case, 2 TX/RX rings would be assigned to IPA.
 */

#ifdef CONFIG_BERYLLIUM
#ifdef IPA_OFFLOAD
/*
 * NEAR-FULL IRQ mask should be updated, if any change is made to
 * the below TX mask.
 */
static const uint8_t tx_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	[0] = WLAN_CFG_TX_RING_MASK_0, [1] = WLAN_CFG_TX_RING_MASK_6,
	[2] = WLAN_CFG_TX_RING_MASK_7};
#else
static const uint8_t tx_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	[0] = WLAN_CFG_TX_RING_MASK_0, [1] = WLAN_CFG_TX_RING_MASK_4,
	[2] = WLAN_CFG_TX_RING_MASK_2, [3] = WLAN_CFG_TX_RING_MASK_6,
	[4] = WLAN_CFG_TX_RING_MASK_7};
#endif /* IPA_OFFLOAD */

static inline const
uint8_t *wlan_cfg_get_tx_ring_int_mask(struct wlan_cfg_dp_soc_ctxt *cfg_ctx)
{
	return &tx_ring_mask_msi[0];
}
#else
static const uint8_t tx_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	[0] = WLAN_CFG_TX_RING_MASK_0};

#ifdef TX_MULTI_TCL
static const uint8_t multi_tx_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	[0] = WLAN_CFG_TX_RING_MASK_0, [4] = WLAN_CFG_TX_RING_MASK_2};

#ifdef IPA_OFFLOAD
static inline const
uint8_t *wlan_cfg_get_tx_ring_int_mask(struct wlan_cfg_dp_soc_ctxt *cfg_ctx)
{
	if (cfg_ctx->ipa_enabled)
		return &tx_ring_mask_msi[0];

	return &multi_tx_ring_mask_msi[0];
}
#else
static inline const
uint8_t *wlan_cfg_get_tx_ring_int_mask(struct wlan_cfg_dp_soc_ctxt *cfg_ctx)
{
	return &multi_tx_ring_mask_msi[0];
}
#endif /* IPA_OFFLOAD */
#else
static inline const
uint8_t *wlan_cfg_get_tx_ring_int_mask(struct wlan_cfg_dp_soc_ctxt *cfg_ctx)
{
	return &tx_ring_mask_msi[0];
}
#endif /* TX_MULTI_TCL */
#endif /* CONFIG_BERYLLIUM */

#ifdef CONFIG_BERYLLIUM
#ifdef IPA_OFFLOAD
static const uint8_t rx_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	[5] = WLAN_CFG_RX_RING_MASK_0, [6] = WLAN_CFG_RX_RING_MASK_1,
	[7] = WLAN_CFG_RX_RING_MASK_2, [9] = WLAN_CFG_RX_RING_MASK_4,
	[10] = WLAN_CFG_RX_RING_MASK_5, [11] = WLAN_CFG_RX_RING_MASK_6};
#else
static const uint8_t rx_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	[5] = WLAN_CFG_RX_RING_MASK_0, [6] = WLAN_CFG_RX_RING_MASK_1,
	[7] = WLAN_CFG_RX_RING_MASK_2, [8] = WLAN_CFG_RX_RING_MASK_3,
	[9] = WLAN_CFG_RX_RING_MASK_4, [10] = WLAN_CFG_RX_RING_MASK_5,
	[11] = WLAN_CFG_RX_RING_MASK_6, [12] = WLAN_CFG_RX_RING_MASK_7};
#endif /* IPA_OFFLOAD */
#else /* !defined(CONFIG_BERYLLIUM) */
#ifdef IPA_OFFLOAD
static const uint8_t rx_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	[1] = WLAN_CFG_RX_RING_MASK_0, [2] = WLAN_CFG_RX_RING_MASK_1,
	[3] = WLAN_CFG_RX_RING_MASK_2};
#else
static const uint8_t rx_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	[1] = WLAN_CFG_RX_RING_MASK_0, [2] = WLAN_CFG_RX_RING_MASK_1,
	[3] = WLAN_CFG_RX_RING_MASK_2 | WLAN_CFG_RX_RING_MASK_3};
#endif
#endif /* CONFIG_BERYLLIUM */

#ifdef CONFIG_BERYLLIUM
static const  uint8_t rxdma2host_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	[13] = WLAN_CFG_RXDMA2HOST_RING_MASK_0};
#else
static const  uint8_t rxdma2host_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	[6] = WLAN_CFG_RXDMA2HOST_RING_MASK_0 |
	      WLAN_CFG_RXDMA2HOST_RING_MASK_1};
#endif /* CONFIG_BERYLLIUM */

#ifdef CONFIG_BERYLLIUM
static const  uint8_t rx_mon_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	[5] = WLAN_CFG_RX_MON_RING_MASK_0};
#else
static const  uint8_t rx_mon_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	[1] = WLAN_CFG_RX_MON_RING_MASK_0, [2] = WLAN_CFG_RX_MON_RING_MASK_1};
#endif

static const  uint8_t host2rxdma_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {0};

static const  uint8_t host2rxdma_mon_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {0};

static const  uint8_t rxdma2host_mon_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {0};

#ifdef CONFIG_BERYLLIUM
static const  uint8_t rx_err_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	[14] = WLAN_CFG_RX_ERR_RING_MASK_0};

static const  uint8_t rx_wbm_rel_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	[14] = WLAN_CFG_RX_WBM_REL_RING_MASK_0};

static const  uint8_t reo_status_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	[13] = WLAN_CFG_REO_STATUS_RING_MASK_0};
#else
static const  uint8_t rx_err_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	[6] = WLAN_CFG_RX_ERR_RING_MASK_0};
static const  uint8_t rx_wbm_rel_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	[6] = WLAN_CFG_RX_WBM_REL_RING_MASK_0};
static const  uint8_t reo_status_ring_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	[6] = WLAN_CFG_REO_STATUS_RING_MASK_0};
#endif

#ifdef CONFIG_BERYLLIUM
#ifdef WLAN_FEATURE_NEAR_FULL_IRQ
static const uint8_t rx_ring_near_full_irq_1_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	[15] = WLAN_CFG_RX_NEAR_FULL_IRQ_MASK_1};
static const uint8_t rx_ring_near_full_irq_2_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	[16] = WLAN_CFG_RX_NEAR_FULL_IRQ_MASK_1};
static const uint8_t tx_ring_near_full_irq_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	[17] = WLAN_CFG_TX_RING_NEAR_FULL_IRQ_MASK};
#else
static const uint8_t rx_ring_near_full_irq_1_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0 };
static const uint8_t rx_ring_near_full_irq_2_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0 };
static const uint8_t tx_ring_near_full_irq_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0 };
#endif
#else
static const uint8_t rx_ring_near_full_irq_1_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0 };
static const uint8_t rx_ring_near_full_irq_2_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0 };
static const uint8_t tx_ring_near_full_irq_mask_msi[WLAN_CFG_INT_NUM_CONTEXTS] = {
	0 };
#endif

#else
/* Integrated configuration + 8 possible MSI configurations */
#define NUM_INTERRUPT_COMBINATIONS 9
/*
 * This structure contains the best possible mask assignment for a given
 * number of MSIs available in the system.
 */
#ifdef IPA_OFFLOAD
static struct dp_int_mask_assignment dp_mask_assignment[NUM_INTERRUPT_COMBINATIONS] = {
	/* Interrupt assignment for integrated configuration */
	{
		/* tx ring masks */
		{ WLAN_CFG_TX_RING_MASK_0,
		  0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* rx ring masks */
		{ 0, 0, 0, 0, 0, 0, 0,
		  WLAN_CFG_RX_RING_MASK_0,
		  WLAN_CFG_RX_RING_MASK_1,
		  WLAN_CFG_RX_RING_MASK_2,
		  0},
		/* rx mon ring masks */
		{ 0, 0, 0, 0,
		  WLAN_CFG_RX_MON_RING_MASK_0,
		  WLAN_CFG_RX_MON_RING_MASK_1,
		  WLAN_CFG_RX_MON_RING_MASK_2,
		  0, 0, 0, 0},
		/* host2rxdma ring masks */
		{ WLAN_CFG_HOST2RXDMA_RING_MASK_0,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_1,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_2,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
		/* rxdma2host ring masks */
		{ WLAN_CFG_RXDMA2HOST_RING_MASK_0,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_1,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_2,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
		/* host2rxdma mon ring masks */
		{ 0, 0, 0, 0,
		  WLAN_CFG_HOST2RXDMA_MON_RING_MASK_0,
		  WLAN_CFG_HOST2RXDMA_MON_RING_MASK_1,
		  WLAN_CFG_HOST2RXDMA_MON_RING_MASK_2,
		  0, 0, 0, 0},
		/* rxdma2host mon ring masks */
		{ 0, 0,	0, 0,
		  WLAN_CFG_RXDMA2HOST_MON_RING_MASK_0,
		  WLAN_CFG_RXDMA2HOST_MON_RING_MASK_1,
		  WLAN_CFG_RXDMA2HOST_MON_RING_MASK_2,
		  0, 0, 0, 0},
		/* rx err ring masks */
		{ WLAN_CFG_RX_ERR_RING_MASK_0,
		  WLAN_CFG_RX_ERR_RING_MASK_1,
		  WLAN_CFG_RX_ERR_RING_MASK_2,
		  WLAN_CFG_RX_ERR_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
		/* rx wbm rel ring masks */
		{ WLAN_CFG_RX_WBM_REL_RING_MASK_0,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_1,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_2,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
		/* reo status ring masks */
		{ WLAN_CFG_REO_STATUS_RING_MASK_0,
		  WLAN_CFG_REO_STATUS_RING_MASK_1,
		  WLAN_CFG_REO_STATUS_RING_MASK_2,
		  WLAN_CFG_REO_STATUS_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
	},
	/* Interrupt assignment for 1 MSI combination */
	{
		/* tx ring masks */
		{ WLAN_CFG_TX_RING_MASK_0,
		  0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* rx ring masks */
		{ WLAN_CFG_RX_RING_MASK_0 |
		    WLAN_CFG_RX_RING_MASK_1 |
		    WLAN_CFG_RX_RING_MASK_2,
		  0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* rx mon ring masks */
		{ WLAN_CFG_RX_MON_RING_MASK_0 |
		    WLAN_CFG_RX_MON_RING_MASK_1 |
		    WLAN_CFG_RX_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* host2rxdma ring masks */
		{ WLAN_CFG_HOST2RXDMA_RING_MASK_0 |
		    WLAN_CFG_HOST2RXDMA_RING_MASK_1 |
		    WLAN_CFG_HOST2RXDMA_RING_MASK_2 |
		    WLAN_CFG_HOST2RXDMA_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* rxdma2host ring masks */
		{ WLAN_CFG_RXDMA2HOST_RING_MASK_0 |
		    WLAN_CFG_RXDMA2HOST_RING_MASK_1 |
		    WLAN_CFG_RXDMA2HOST_RING_MASK_2 |
		  WLAN_CFG_RXDMA2HOST_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* host2rxdma mon ring masks */
		{ WLAN_CFG_HOST2RXDMA_MON_RING_MASK_0 |
		    WLAN_CFG_HOST2RXDMA_MON_RING_MASK_1 |
		    WLAN_CFG_HOST2RXDMA_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* rxdma2host mon ring masks */
		{ WLAN_CFG_RXDMA2HOST_MON_RING_MASK_0 |
		    WLAN_CFG_RXDMA2HOST_MON_RING_MASK_1 |
		    WLAN_CFG_RXDMA2HOST_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* rx err ring masks */
		{ WLAN_CFG_RX_ERR_RING_MASK_0 |
		    WLAN_CFG_RX_ERR_RING_MASK_1 |
		    WLAN_CFG_RX_ERR_RING_MASK_2 |
		    WLAN_CFG_RX_ERR_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* rx wbm rel ring masks */
		{ WLAN_CFG_RX_WBM_REL_RING_MASK_0 |
		    WLAN_CFG_RX_WBM_REL_RING_MASK_1 |
		    WLAN_CFG_RX_WBM_REL_RING_MASK_2 |
		    WLAN_CFG_RX_WBM_REL_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* reo status ring masks */
		{ WLAN_CFG_REO_STATUS_RING_MASK_0 |
		    WLAN_CFG_REO_STATUS_RING_MASK_1 |
		    WLAN_CFG_REO_STATUS_RING_MASK_2 |
		    WLAN_CFG_REO_STATUS_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	},
	/* Interrupt assignment for 2 MSI combination */
	{
		/* tx ring masks */
		{ WLAN_CFG_TX_RING_MASK_0,
		  0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* rx ring masks */
		{ WLAN_CFG_RX_RING_MASK_0 |
		    WLAN_CFG_RX_RING_MASK_1,
		  WLAN_CFG_RX_RING_MASK_2,
		  0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* rx mon ring masks */
		{ WLAN_CFG_RX_MON_RING_MASK_0 |
		    WLAN_CFG_RX_MON_RING_MASK_1,
		  WLAN_CFG_RX_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* host2rxdma ring masks */
		{ WLAN_CFG_HOST2RXDMA_RING_MASK_0 |
		    WLAN_CFG_HOST2RXDMA_RING_MASK_1,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_2 |
		    WLAN_CFG_HOST2RXDMA_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* rxdma2host ring masks */
		{ WLAN_CFG_RXDMA2HOST_RING_MASK_0 |
		    WLAN_CFG_RXDMA2HOST_RING_MASK_1,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_2 |
		    WLAN_CFG_RXDMA2HOST_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* host2rxdma mon ring masks */
		{ WLAN_CFG_HOST2RXDMA_MON_RING_MASK_0 |
		    WLAN_CFG_HOST2RXDMA_MON_RING_MASK_1,
		  WLAN_CFG_HOST2RXDMA_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* rxdma2host mon ring masks */
		{ WLAN_CFG_RXDMA2HOST_MON_RING_MASK_0 |
		    WLAN_CFG_RXDMA2HOST_MON_RING_MASK_1,
		  WLAN_CFG_RXDMA2HOST_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* rx err ring masks */
		{ WLAN_CFG_RX_ERR_RING_MASK_0 |
		    WLAN_CFG_RX_ERR_RING_MASK_1,
		  WLAN_CFG_RX_ERR_RING_MASK_2 |
		    WLAN_CFG_RX_ERR_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* rx wbm rel ring masks */
		{ WLAN_CFG_RX_WBM_REL_RING_MASK_0 |
		    WLAN_CFG_RX_WBM_REL_RING_MASK_1,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_2 |
		    WLAN_CFG_RX_WBM_REL_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* reo status ring masks */
		{ WLAN_CFG_REO_STATUS_RING_MASK_0 |
		    WLAN_CFG_REO_STATUS_RING_MASK_1,
		  WLAN_CFG_REO_STATUS_RING_MASK_2 |
		    WLAN_CFG_REO_STATUS_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0, 0},
	},
	/* Interrupt assignment for 3 MSI combination */
	{
		/* tx ring masks */
		{ WLAN_CFG_TX_RING_MASK_0,
		  0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* rx ring masks */
		{ WLAN_CFG_RX_RING_MASK_0,
		    WLAN_CFG_RX_RING_MASK_1,
		  WLAN_CFG_RX_RING_MASK_2,
		  0, 0, 0, 0, 0, 0, 0, 0},
		/* rx mon ring masks */
		{ 0, 0,
		  WLAN_CFG_RX_MON_RING_MASK_0 |
		    WLAN_CFG_RX_MON_RING_MASK_1 |
		    WLAN_CFG_RX_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0, 0, 0},
		/* host2rxdma ring masks */
		{ 0, 0,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_0 |
		    WLAN_CFG_HOST2RXDMA_RING_MASK_1 |
		    WLAN_CFG_HOST2RXDMA_RING_MASK_2 |
		    WLAN_CFG_HOST2RXDMA_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0},
		/* rxdma2host ring masks */
		{ 0, 0,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_0 |
		    WLAN_CFG_RXDMA2HOST_RING_MASK_1 |
		    WLAN_CFG_RXDMA2HOST_RING_MASK_2 |
		    WLAN_CFG_RXDMA2HOST_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0},
		/* host2rxdma mon ring masks */
		{ 0, 0,
		  WLAN_CFG_HOST2RXDMA_MON_RING_MASK_0 |
		    WLAN_CFG_HOST2RXDMA_MON_RING_MASK_1 |
		    WLAN_CFG_HOST2RXDMA_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0, 0, 0},
		/* rxdma2host mon ring masks */
		{ 0, 0,
		  WLAN_CFG_RXDMA2HOST_MON_RING_MASK_0 |
		    WLAN_CFG_RXDMA2HOST_MON_RING_MASK_1 |
		    WLAN_CFG_RXDMA2HOST_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0, 0, 0},
		/* rx err ring masks */
		{ 0, 0,
		  WLAN_CFG_RX_ERR_RING_MASK_0 |
		    WLAN_CFG_RX_ERR_RING_MASK_1 |
		    WLAN_CFG_RX_ERR_RING_MASK_2 |
		    WLAN_CFG_RX_ERR_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0},
		/* rx wbm rel ring masks */
		{ 0, 0,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_0 |
		    WLAN_CFG_RX_WBM_REL_RING_MASK_1 |
		    WLAN_CFG_RX_WBM_REL_RING_MASK_2 |
		    WLAN_CFG_RX_WBM_REL_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0},
		/* reo status ring masks */
		{ 0, 0,
		  WLAN_CFG_REO_STATUS_RING_MASK_0 |
		    WLAN_CFG_REO_STATUS_RING_MASK_1 |
		    WLAN_CFG_REO_STATUS_RING_MASK_2 |
		    WLAN_CFG_REO_STATUS_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0},
	},
	/* Interrupt assignment for 4 MSI combination */
	{
		/* tx ring masks */
		{ WLAN_CFG_TX_RING_MASK_0,
		  0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* rx ring masks */
		{ WLAN_CFG_RX_RING_MASK_0,
		  WLAN_CFG_RX_RING_MASK_1,
		  WLAN_CFG_RX_RING_MASK_2,
		  0, 0, 0, 0, 0, 0, 0, 0},
		/* rx mon ring masks */
		{ WLAN_CFG_RX_MON_RING_MASK_0,
		  WLAN_CFG_RX_MON_RING_MASK_1,
		  WLAN_CFG_RX_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0, 0, 0},
		/* host2rxdma ring masks */
		{ WLAN_CFG_HOST2RXDMA_RING_MASK_0,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_1,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_2,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
		/* rxdma2host ring masks */
		{ WLAN_CFG_RXDMA2HOST_RING_MASK_0,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_1,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_2,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
		/* host2rxdma mon ring masks */
		{ WLAN_CFG_HOST2RXDMA_MON_RING_MASK_0,
		  WLAN_CFG_HOST2RXDMA_MON_RING_MASK_1,
		  WLAN_CFG_HOST2RXDMA_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0, 0, 0},
		/* rxdma2host mon ring masks */
		{ WLAN_CFG_RXDMA2HOST_MON_RING_MASK_0,
		  WLAN_CFG_RXDMA2HOST_MON_RING_MASK_1,
		  WLAN_CFG_RXDMA2HOST_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0, 0, 0},
		/* rx err ring masks */
		{ WLAN_CFG_RX_ERR_RING_MASK_0,
		  WLAN_CFG_RX_ERR_RING_MASK_1,
		  WLAN_CFG_RX_ERR_RING_MASK_2,
		  WLAN_CFG_RX_ERR_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
		/* rx wbm rel ring masks */
		{ WLAN_CFG_RX_WBM_REL_RING_MASK_0,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_1,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_2,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
		/* reo status ring masks */
		{ WLAN_CFG_REO_STATUS_RING_MASK_0,
		  WLAN_CFG_REO_STATUS_RING_MASK_1,
		  WLAN_CFG_REO_STATUS_RING_MASK_2,
		  WLAN_CFG_REO_STATUS_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
	},
	/* Interrupt assignment for 5 MSI combination */
	{
		/* tx ring masks */
		{ WLAN_CFG_TX_RING_MASK_0,
		  0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* rx ring masks */
		{ WLAN_CFG_RX_RING_MASK_0,
		  WLAN_CFG_RX_RING_MASK_1,
		  WLAN_CFG_RX_RING_MASK_2,
		  0, 0, 0, 0, 0, 0, 0, 0},
		/* rx mon ring masks */
		{ 0, 0, 0, 0,
		  WLAN_CFG_RX_MON_RING_MASK_0 |
		    WLAN_CFG_RX_MON_RING_MASK_1 |
		    WLAN_CFG_RX_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0},
		/* host2rxdma ring masks */
		{ 0, 0, 0, 0,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_0 |
		    WLAN_CFG_HOST2RXDMA_RING_MASK_1 |
		    WLAN_CFG_HOST2RXDMA_RING_MASK_2 |
		    WLAN_CFG_HOST2RXDMA_RING_MASK_3,
		  0, 0, 0, 0, 0, 0},
		/* rxdma2host ring masks */
		{ 0, 0, 0, 0,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_0 |
		    WLAN_CFG_RXDMA2HOST_RING_MASK_1 |
		    WLAN_CFG_RXDMA2HOST_RING_MASK_2 |
		    WLAN_CFG_RXDMA2HOST_RING_MASK_3,
		  0, 0, 0, 0, 0, 0},
		/* host2rxdma mon ring masks */
		{ 0, 0, 0, 0,
		  WLAN_CFG_HOST2RXDMA_MON_RING_MASK_0 |
		    WLAN_CFG_HOST2RXDMA_MON_RING_MASK_1 |
		    WLAN_CFG_HOST2RXDMA_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0},
		/* rxdma2host mon ring masks */
		{ 0, 0, 0, 0,
		  WLAN_CFG_RXDMA2HOST_MON_RING_MASK_0 |
		    WLAN_CFG_RXDMA2HOST_MON_RING_MASK_1 |
		    WLAN_CFG_RXDMA2HOST_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0},
		/* rx err ring masks */
		{ 0, 0, 0, 0,
		  WLAN_CFG_RX_ERR_RING_MASK_0 |
		    WLAN_CFG_RX_ERR_RING_MASK_1 |
		    WLAN_CFG_RX_ERR_RING_MASK_2 |
		    WLAN_CFG_RX_ERR_RING_MASK_3,
		  0, 0, 0, 0, 0, 0},
		/* rx wbm rel ring masks */
		{ 0, 0, 0, 0,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_0 |
		    WLAN_CFG_RX_WBM_REL_RING_MASK_1 |
		    WLAN_CFG_RX_WBM_REL_RING_MASK_2 |
		    WLAN_CFG_RX_WBM_REL_RING_MASK_3,
		  0, 0, 0, 0, 0, 0},
		/* reo status ring masks */
		{ 0, 0, 0, 0,
		  WLAN_CFG_REO_STATUS_RING_MASK_0 |
		    WLAN_CFG_REO_STATUS_RING_MASK_1 |
		    WLAN_CFG_REO_STATUS_RING_MASK_2 |
		    WLAN_CFG_REO_STATUS_RING_MASK_3,
		  0, 0, 0, 0, 0, 0},
	},
	/* Interrupt assignment for 6 MSI combination */
	{
		/* tx ring masks */
		{ WLAN_CFG_TX_RING_MASK_0,
		  0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* rx ring masks */
		{ 0, 0,
		  WLAN_CFG_RX_RING_MASK_0,
		  WLAN_CFG_RX_RING_MASK_1,
		  WLAN_CFG_RX_RING_MASK_2,
		  0, 0, 0, 0, 0, 0},
		/* rx mon ring masks */
		{ WLAN_CFG_RX_MON_RING_MASK_0,
		  WLAN_CFG_RX_MON_RING_MASK_1,
		  WLAN_CFG_RX_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0, 0, 0},
		/* host2rxdma ring masks */
		{ WLAN_CFG_HOST2RXDMA_RING_MASK_0,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_1,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_2,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
		/* rxdma2host ring masks */
		{ WLAN_CFG_RXDMA2HOST_RING_MASK_0,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_1,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_2,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
		/* host2rxdma mon ring masks */
		{ WLAN_CFG_HOST2RXDMA_MON_RING_MASK_0,
		  WLAN_CFG_HOST2RXDMA_MON_RING_MASK_1,
		  WLAN_CFG_HOST2RXDMA_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0, 0, 0},
		/* rxdma2host mon ring masks */
		{ WLAN_CFG_RXDMA2HOST_MON_RING_MASK_0,
		  WLAN_CFG_RXDMA2HOST_MON_RING_MASK_1,
		  WLAN_CFG_RXDMA2HOST_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0, 0, 0},
		/* rx err ring masks */
		{ WLAN_CFG_RX_ERR_RING_MASK_0,
		  WLAN_CFG_RX_ERR_RING_MASK_1,
		  WLAN_CFG_RX_ERR_RING_MASK_2,
		  WLAN_CFG_RX_ERR_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
		/* rx wbm rel ring masks */
		{ WLAN_CFG_RX_WBM_REL_RING_MASK_0,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_1,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_2,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
		/* reo status ring masks */
		{ WLAN_CFG_REO_STATUS_RING_MASK_0,
		  WLAN_CFG_REO_STATUS_RING_MASK_1,
		  WLAN_CFG_REO_STATUS_RING_MASK_2,
		  WLAN_CFG_REO_STATUS_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
	},
	/* Interrupt assignment for 7 MSI combination */
	{
		/* tx ring masks */
		{ WLAN_CFG_TX_RING_MASK_0,
		  0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* rx ring masks */
		{ 0, 0, 0,
		  WLAN_CFG_RX_RING_MASK_0,
		  WLAN_CFG_RX_RING_MASK_1,
		  WLAN_CFG_RX_RING_MASK_2,
		  0, 0, 0, 0},
		/* rx mon ring masks */
		{ 0, 0, 0,
		  WLAN_CFG_RX_MON_RING_MASK_0,
		  WLAN_CFG_RX_MON_RING_MASK_1,
		  WLAN_CFG_RX_MON_RING_MASK_2,
		  0, 0, 0, 0, 0},
		/* host2rxdma ring masks */
		{ 0, 0, 0,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_0,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_1,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_2,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_3,
		  0, 0, 0, 0},
		/* rxdma2host ring masks */
		{ 0, 0, 0,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_0,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_1,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_2,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_3,
		  0, 0, 0, 0},
		/* host2rxdma mon ring masks */
		{ 0, 0, 0,
		  WLAN_CFG_HOST2RXDMA_MON_RING_MASK_0,
		  WLAN_CFG_HOST2RXDMA_MON_RING_MASK_1,
		  WLAN_CFG_HOST2RXDMA_MON_RING_MASK_2,
		  0, 0, 0, 0, 0},
		/* rxdma2host mon ring masks */
		{ 0, 0,	0,
		  WLAN_CFG_RXDMA2HOST_MON_RING_MASK_0,
		  WLAN_CFG_RXDMA2HOST_MON_RING_MASK_1,
		  WLAN_CFG_RXDMA2HOST_MON_RING_MASK_2,
		  0, 0, 0, 0, 0},
		/* rx err ring masks */
		{ 0, 0, 0,
		  WLAN_CFG_RX_ERR_RING_MASK_0,
		  WLAN_CFG_RX_ERR_RING_MASK_1,
		  WLAN_CFG_RX_ERR_RING_MASK_2,
		  WLAN_CFG_RX_ERR_RING_MASK_3,
		  0, 0, 0, 0},
		/* rx wbm rel ring masks */
		{ 0, 0, 0,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_0,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_1,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_2,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_3,
		  0, 0, 0, 0},
		/* reo status ring masks */
		{ 0, 0, 0,
		  WLAN_CFG_REO_STATUS_RING_MASK_0,
		  WLAN_CFG_REO_STATUS_RING_MASK_1,
		  WLAN_CFG_REO_STATUS_RING_MASK_2,
		  WLAN_CFG_REO_STATUS_RING_MASK_3,
		  0, 0, 0, 0},
	},
	/* Interrupt assignment for 8 MSI combination */
	{
		/* tx ring masks */
		{ WLAN_CFG_TX_RING_MASK_0,
		  0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* rx ring masks */
		{ 0, 0, 0, 0,
		  WLAN_CFG_RX_RING_MASK_0,
		  WLAN_CFG_RX_RING_MASK_1,
		  WLAN_CFG_RX_RING_MASK_2,
		  0, 0, 0, 0},
		/* rx mon ring masks */
		{ 0, 0, 0,
		  WLAN_CFG_RX_MON_RING_MASK_0,
		  WLAN_CFG_RX_MON_RING_MASK_1,
		  WLAN_CFG_RX_MON_RING_MASK_2,
		  0, 0, 0, 0, 0},
		/* host2rxdma ring masks */
		{ 0, 0, 0,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_0,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_1,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_2,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_3,
		  0, 0, 0, 0},
		/* rxdma2host ring masks */
		{ 0, 0, 0,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_0,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_1,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_2,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_3,
		  0, 0, 0, 0},
		/* host2rxdma mon ring masks */
		{ 0, 0, 0,
		  WLAN_CFG_HOST2RXDMA_MON_RING_MASK_0,
		  WLAN_CFG_HOST2RXDMA_MON_RING_MASK_1,
		  WLAN_CFG_HOST2RXDMA_MON_RING_MASK_2,
		  0, 0, 0, 0, 0},
		/* rxdma2host mon ring masks */
		{ 0, 0, 0,
		  WLAN_CFG_RXDMA2HOST_MON_RING_MASK_0,
		  WLAN_CFG_RXDMA2HOST_MON_RING_MASK_1,
		  WLAN_CFG_RXDMA2HOST_MON_RING_MASK_2,
		  0, 0, 0, 0, 0},
		/* rx err ring masks */
		{ 0, 0, 0,
		  WLAN_CFG_RX_ERR_RING_MASK_0,
		  WLAN_CFG_RX_ERR_RING_MASK_1,
		  WLAN_CFG_RX_ERR_RING_MASK_2,
		  WLAN_CFG_RX_ERR_RING_MASK_3,
		  0, 0, 0, 0},
		/* rx wbm rel ring masks */
		{ 0, 0, 0,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_0,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_1,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_2,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_3,
		  0, 0, 0, 0},
		/* reo status ring masks */
		{ 0, 0, 0,
		  WLAN_CFG_REO_STATUS_RING_MASK_0,
		  WLAN_CFG_REO_STATUS_RING_MASK_1,
		  WLAN_CFG_REO_STATUS_RING_MASK_2,
		  WLAN_CFG_REO_STATUS_RING_MASK_3,
		  0, 0, 0, 0},
	},
};
#else
static struct dp_int_mask_assignment dp_mask_assignment[NUM_INTERRUPT_COMBINATIONS] = {
	/* Interrupt assignment for integrated configuration */
	{
		/* tx ring masks */
		{ WLAN_CFG_TX_RING_MASK_0,
		  WLAN_CFG_TX_RING_MASK_1,
		  WLAN_CFG_TX_RING_MASK_2,
		  WLAN_CFG_TX_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
		/* rx ring masks */
		{ 0, 0, 0, 0, 0, 0, 0,
		  WLAN_CFG_RX_RING_MASK_0,
		  WLAN_CFG_RX_RING_MASK_1,
		  WLAN_CFG_RX_RING_MASK_2,
		  WLAN_CFG_RX_RING_MASK_3},
		/* rx mon ring masks */
		{ 0, 0, 0, 0,
		  WLAN_CFG_RX_MON_RING_MASK_0,
		  WLAN_CFG_RX_MON_RING_MASK_1,
		  WLAN_CFG_RX_MON_RING_MASK_2,
		  0, 0, 0, 0},
		/* host2rxdma ring masks */
		{ WLAN_CFG_HOST2RXDMA_RING_MASK_0,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_1,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_2,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
		/* rxdma2host ring masks */
		{ WLAN_CFG_RXDMA2HOST_RING_MASK_0,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_1,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_2,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
		/* host2rxdma mon ring masks */
		{ 0, 0, 0, 0,
		  WLAN_CFG_HOST2RXDMA_MON_RING_MASK_0,
		  WLAN_CFG_HOST2RXDMA_MON_RING_MASK_1,
		  WLAN_CFG_HOST2RXDMA_MON_RING_MASK_2,
		  0, 0, 0, 0},
		/* rxdma2host mon ring masks */
		{ 0, 0,	0, 0,
		  WLAN_CFG_RXDMA2HOST_MON_RING_MASK_0,
		  WLAN_CFG_RXDMA2HOST_MON_RING_MASK_1,
		  WLAN_CFG_RXDMA2HOST_MON_RING_MASK_2,
		  0, 0, 0, 0},
		/* rx err ring masks */
		{ WLAN_CFG_RX_ERR_RING_MASK_0,
		  WLAN_CFG_RX_ERR_RING_MASK_1,
		  WLAN_CFG_RX_ERR_RING_MASK_2,
		  WLAN_CFG_RX_ERR_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
		/* rx wbm rel ring masks */
		{ WLAN_CFG_RX_WBM_REL_RING_MASK_0,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_1,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_2,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
		/* reo status ring masks */
		{ WLAN_CFG_REO_STATUS_RING_MASK_0,
		  WLAN_CFG_REO_STATUS_RING_MASK_1,
		  WLAN_CFG_REO_STATUS_RING_MASK_2,
		  WLAN_CFG_REO_STATUS_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
	},
	/* Interrupt assignment for 1 MSI combination */
	{
		/* tx ring masks */
		{ WLAN_CFG_TX_RING_MASK_0 |
		    WLAN_CFG_TX_RING_MASK_1 |
		    WLAN_CFG_TX_RING_MASK_2 |
		    WLAN_CFG_TX_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* rx ring masks */
		{ WLAN_CFG_RX_RING_MASK_0 |
		    WLAN_CFG_RX_RING_MASK_1 |
		    WLAN_CFG_RX_RING_MASK_2 |
		    WLAN_CFG_RX_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* rx mon ring masks */
		{ WLAN_CFG_RX_MON_RING_MASK_0 |
		    WLAN_CFG_RX_MON_RING_MASK_1 |
		    WLAN_CFG_RX_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* host2rxdma ring masks */
		{ WLAN_CFG_HOST2RXDMA_RING_MASK_0 |
		    WLAN_CFG_HOST2RXDMA_RING_MASK_1 |
		    WLAN_CFG_HOST2RXDMA_RING_MASK_2 |
		    WLAN_CFG_HOST2RXDMA_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* rxdma2host ring masks */
		{ WLAN_CFG_RXDMA2HOST_RING_MASK_0 |
		    WLAN_CFG_RXDMA2HOST_RING_MASK_1 |
		    WLAN_CFG_RXDMA2HOST_RING_MASK_2 |
		  WLAN_CFG_RXDMA2HOST_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* host2rxdma mon ring masks */
		{ WLAN_CFG_HOST2RXDMA_MON_RING_MASK_0 |
		    WLAN_CFG_HOST2RXDMA_MON_RING_MASK_1 |
		    WLAN_CFG_HOST2RXDMA_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* rxdma2host mon ring masks */
		{ WLAN_CFG_RXDMA2HOST_MON_RING_MASK_0 |
		    WLAN_CFG_RXDMA2HOST_MON_RING_MASK_1 |
		    WLAN_CFG_RXDMA2HOST_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* rx err ring masks */
		{ WLAN_CFG_RX_ERR_RING_MASK_0 |
		    WLAN_CFG_RX_ERR_RING_MASK_1 |
		    WLAN_CFG_RX_ERR_RING_MASK_2 |
		    WLAN_CFG_RX_ERR_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* rx wbm rel ring masks */
		{ WLAN_CFG_RX_WBM_REL_RING_MASK_0 |
		    WLAN_CFG_RX_WBM_REL_RING_MASK_1 |
		    WLAN_CFG_RX_WBM_REL_RING_MASK_2 |
		    WLAN_CFG_RX_WBM_REL_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* reo status ring masks */
		{ WLAN_CFG_REO_STATUS_RING_MASK_0 |
		    WLAN_CFG_REO_STATUS_RING_MASK_1 |
		    WLAN_CFG_REO_STATUS_RING_MASK_2 |
		    WLAN_CFG_REO_STATUS_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	},
	/* Interrupt assignment for 2 MSI combination */
	{
		/* tx ring masks */
		{ WLAN_CFG_TX_RING_MASK_0 |
		    WLAN_CFG_TX_RING_MASK_1,
		  WLAN_CFG_TX_RING_MASK_2 |
		    WLAN_CFG_TX_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* rx ring masks */
		{ WLAN_CFG_RX_RING_MASK_0 |
		    WLAN_CFG_RX_RING_MASK_1,
		  WLAN_CFG_RX_RING_MASK_2 |
		    WLAN_CFG_RX_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* rx mon ring masks */
		{ WLAN_CFG_RX_MON_RING_MASK_0 |
		    WLAN_CFG_RX_MON_RING_MASK_1,
		  WLAN_CFG_RX_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* host2rxdma ring masks */
		{ WLAN_CFG_HOST2RXDMA_RING_MASK_0 |
		    WLAN_CFG_HOST2RXDMA_RING_MASK_1,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_2 |
		    WLAN_CFG_HOST2RXDMA_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* rxdma2host ring masks */
		{ WLAN_CFG_RXDMA2HOST_RING_MASK_0 |
		    WLAN_CFG_RXDMA2HOST_RING_MASK_1,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_2 |
		    WLAN_CFG_RXDMA2HOST_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* host2rxdma mon ring masks */
		{ WLAN_CFG_HOST2RXDMA_MON_RING_MASK_0 |
		    WLAN_CFG_HOST2RXDMA_MON_RING_MASK_1,
		  WLAN_CFG_HOST2RXDMA_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* rxdma2host mon ring masks */
		{ WLAN_CFG_RXDMA2HOST_MON_RING_MASK_0 |
		    WLAN_CFG_RXDMA2HOST_MON_RING_MASK_1,
		  WLAN_CFG_RXDMA2HOST_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* rx err ring masks */
		{ WLAN_CFG_RX_ERR_RING_MASK_0 |
		    WLAN_CFG_RX_ERR_RING_MASK_1,
		  WLAN_CFG_RX_ERR_RING_MASK_2 |
		    WLAN_CFG_RX_ERR_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* rx wbm rel ring masks */
		{ WLAN_CFG_RX_WBM_REL_RING_MASK_0 |
		    WLAN_CFG_RX_WBM_REL_RING_MASK_1,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_2 |
		    WLAN_CFG_RX_WBM_REL_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* reo status ring masks */
		{ WLAN_CFG_REO_STATUS_RING_MASK_0 |
		    WLAN_CFG_REO_STATUS_RING_MASK_1,
		  WLAN_CFG_REO_STATUS_RING_MASK_2 |
		    WLAN_CFG_REO_STATUS_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0, 0},
	},
	/* Interrupt assignment for 3 MSI combination */
	{
		/* tx ring masks */
		{ WLAN_CFG_TX_RING_MASK_0 |
		    WLAN_CFG_TX_RING_MASK_1,
		  WLAN_CFG_TX_RING_MASK_2 |
		    WLAN_CFG_TX_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* rx ring masks */
		{ WLAN_CFG_RX_RING_MASK_0 |
		    WLAN_CFG_RX_RING_MASK_1,
		  WLAN_CFG_RX_RING_MASK_2 |
		    WLAN_CFG_RX_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0, 0},
		/* rx mon ring masks */
		{ 0, 0,
		  WLAN_CFG_RX_MON_RING_MASK_0 |
		    WLAN_CFG_RX_MON_RING_MASK_1 |
		    WLAN_CFG_RX_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0, 0, 0},
		/* host2rxdma ring masks */
		{ 0, 0,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_0 |
		    WLAN_CFG_HOST2RXDMA_RING_MASK_1 |
		    WLAN_CFG_HOST2RXDMA_RING_MASK_2 |
		    WLAN_CFG_HOST2RXDMA_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0},
		/* rxdma2host ring masks */
		{ 0, 0,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_0 |
		    WLAN_CFG_RXDMA2HOST_RING_MASK_1 |
		    WLAN_CFG_RXDMA2HOST_RING_MASK_2 |
		    WLAN_CFG_RXDMA2HOST_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0},
		/* host2rxdma mon ring masks */
		{ 0, 0,
		  WLAN_CFG_HOST2RXDMA_MON_RING_MASK_0 |
		    WLAN_CFG_HOST2RXDMA_MON_RING_MASK_1 |
		    WLAN_CFG_HOST2RXDMA_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0, 0, 0},
		/* rxdma2host mon ring masks */
		{ 0, 0,
		  WLAN_CFG_RXDMA2HOST_MON_RING_MASK_0 |
		    WLAN_CFG_RXDMA2HOST_MON_RING_MASK_1 |
		    WLAN_CFG_RXDMA2HOST_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0, 0, 0},
		/* rx err ring masks */
		{ 0, 0,
		  WLAN_CFG_RX_ERR_RING_MASK_0 |
		    WLAN_CFG_RX_ERR_RING_MASK_1 |
		    WLAN_CFG_RX_ERR_RING_MASK_2 |
		    WLAN_CFG_RX_ERR_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0},
		/* rx wbm rel ring masks */
		{ 0, 0,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_0 |
		    WLAN_CFG_RX_WBM_REL_RING_MASK_1 |
		    WLAN_CFG_RX_WBM_REL_RING_MASK_2 |
		    WLAN_CFG_RX_WBM_REL_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0},
		/* reo status ring masks */
		{ 0, 0,
		  WLAN_CFG_REO_STATUS_RING_MASK_0 |
		    WLAN_CFG_REO_STATUS_RING_MASK_1 |
		    WLAN_CFG_REO_STATUS_RING_MASK_2 |
		    WLAN_CFG_REO_STATUS_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0, 0},
	},
	/* Interrupt assignment for 4 MSI combination */
	{
		/* tx ring masks */
		{ WLAN_CFG_TX_RING_MASK_0,
		  WLAN_CFG_TX_RING_MASK_1,
		  WLAN_CFG_TX_RING_MASK_2,
		  WLAN_CFG_TX_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
		/* rx ring masks */
		{ WLAN_CFG_RX_RING_MASK_0,
		  WLAN_CFG_RX_RING_MASK_1,
		  WLAN_CFG_RX_RING_MASK_2,
		  WLAN_CFG_RX_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
		/* rx mon ring masks */
		{ WLAN_CFG_RX_MON_RING_MASK_0,
		  WLAN_CFG_RX_MON_RING_MASK_1,
		  WLAN_CFG_RX_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0, 0, 0},
		/* host2rxdma ring masks */
		{ WLAN_CFG_HOST2RXDMA_RING_MASK_0,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_1,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_2,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
		/* rxdma2host ring masks */
		{ WLAN_CFG_RXDMA2HOST_RING_MASK_0,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_1,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_2,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
		/* host2rxdma mon ring masks */
		{ WLAN_CFG_HOST2RXDMA_MON_RING_MASK_0,
		  WLAN_CFG_HOST2RXDMA_MON_RING_MASK_1,
		  WLAN_CFG_HOST2RXDMA_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0, 0, 0},
		/* rxdma2host mon ring masks */
		{ WLAN_CFG_RXDMA2HOST_MON_RING_MASK_0,
		  WLAN_CFG_RXDMA2HOST_MON_RING_MASK_1,
		  WLAN_CFG_RXDMA2HOST_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0, 0, 0},
		/* rx err ring masks */
		{ WLAN_CFG_RX_ERR_RING_MASK_0,
		  WLAN_CFG_RX_ERR_RING_MASK_1,
		  WLAN_CFG_RX_ERR_RING_MASK_2,
		  WLAN_CFG_RX_ERR_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
		/* rx wbm rel ring masks */
		{ WLAN_CFG_RX_WBM_REL_RING_MASK_0,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_1,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_2,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
		/* reo status ring masks */
		{ WLAN_CFG_REO_STATUS_RING_MASK_0,
		  WLAN_CFG_REO_STATUS_RING_MASK_1,
		  WLAN_CFG_REO_STATUS_RING_MASK_2,
		  WLAN_CFG_REO_STATUS_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
	},
	/* Interrupt assignment for 5 MSI combination */
	{
		/* tx ring masks */
		{ WLAN_CFG_TX_RING_MASK_0,
		  WLAN_CFG_TX_RING_MASK_1,
		  WLAN_CFG_TX_RING_MASK_2,
		  WLAN_CFG_TX_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
		/* rx ring masks */
		{ WLAN_CFG_RX_RING_MASK_0,
		  WLAN_CFG_RX_RING_MASK_1,
		  WLAN_CFG_RX_RING_MASK_2,
		  WLAN_CFG_RX_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
		/* rx mon ring masks */
		{ 0, 0, 0, 0,
		  WLAN_CFG_RX_MON_RING_MASK_0 |
		    WLAN_CFG_RX_MON_RING_MASK_1 |
		    WLAN_CFG_RX_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0},
		/* host2rxdma ring masks */
		{ 0, 0, 0, 0,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_0 |
		    WLAN_CFG_HOST2RXDMA_RING_MASK_1 |
		    WLAN_CFG_HOST2RXDMA_RING_MASK_2 |
		    WLAN_CFG_HOST2RXDMA_RING_MASK_3,
		  0, 0, 0, 0, 0, 0},
		/* rxdma2host ring masks */
		{ 0, 0, 0, 0,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_0 |
		    WLAN_CFG_RXDMA2HOST_RING_MASK_1 |
		    WLAN_CFG_RXDMA2HOST_RING_MASK_2 |
		    WLAN_CFG_RXDMA2HOST_RING_MASK_3,
		  0, 0, 0, 0, 0, 0},
		/* host2rxdma mon ring masks */
		{ 0, 0, 0, 0,
		  WLAN_CFG_HOST2RXDMA_MON_RING_MASK_0 |
		    WLAN_CFG_HOST2RXDMA_MON_RING_MASK_1 |
		    WLAN_CFG_HOST2RXDMA_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0},
		/* rxdma2host mon ring masks */
		{ 0, 0, 0, 0,
		  WLAN_CFG_RXDMA2HOST_MON_RING_MASK_0 |
		    WLAN_CFG_RXDMA2HOST_MON_RING_MASK_1 |
		    WLAN_CFG_RXDMA2HOST_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0},
		/* rx err ring masks */
		{ 0, 0, 0, 0,
		  WLAN_CFG_RX_ERR_RING_MASK_0 |
		    WLAN_CFG_RX_ERR_RING_MASK_1 |
		    WLAN_CFG_RX_ERR_RING_MASK_2 |
		    WLAN_CFG_RX_ERR_RING_MASK_3,
		  0, 0, 0, 0, 0, 0},
		/* rx wbm rel ring masks */
		{ 0, 0, 0, 0,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_0 |
		    WLAN_CFG_RX_WBM_REL_RING_MASK_1 |
		    WLAN_CFG_RX_WBM_REL_RING_MASK_2 |
		    WLAN_CFG_RX_WBM_REL_RING_MASK_3,
		  0, 0, 0, 0, 0, 0},
		/* reo status ring masks */
		{ 0, 0, 0, 0,
		  WLAN_CFG_REO_STATUS_RING_MASK_0 |
		    WLAN_CFG_REO_STATUS_RING_MASK_1 |
		    WLAN_CFG_REO_STATUS_RING_MASK_2 |
		    WLAN_CFG_REO_STATUS_RING_MASK_3,
		  0, 0, 0, 0, 0, 0},
	},
	/* Interrupt assignment for 6 MSI combination */
	{
		/* tx ring masks */
		{ WLAN_CFG_TX_RING_MASK_0,
		  WLAN_CFG_TX_RING_MASK_1,
		  WLAN_CFG_TX_RING_MASK_2,
		  WLAN_CFG_TX_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
		/* rx ring masks */
		{ 0, 0,
		  WLAN_CFG_RX_RING_MASK_0,
		  WLAN_CFG_RX_RING_MASK_1,
		  WLAN_CFG_RX_RING_MASK_2,
		  WLAN_CFG_RX_RING_MASK_3,
		  0, 0, 0, 0, 0},
		/* rx mon ring masks */
		{ WLAN_CFG_RX_MON_RING_MASK_0,
		  WLAN_CFG_RX_MON_RING_MASK_1,
		  WLAN_CFG_RX_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0, 0, 0},
		/* host2rxdma ring masks */
		{ WLAN_CFG_HOST2RXDMA_RING_MASK_0,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_1,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_2,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
		/* rxdma2host ring masks */
		{ WLAN_CFG_RXDMA2HOST_RING_MASK_0,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_1,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_2,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
		/* host2rxdma mon ring masks */
		{ WLAN_CFG_HOST2RXDMA_MON_RING_MASK_0,
		  WLAN_CFG_HOST2RXDMA_MON_RING_MASK_1,
		  WLAN_CFG_HOST2RXDMA_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0, 0, 0},
		/* rxdma2host mon ring masks */
		{ WLAN_CFG_RXDMA2HOST_MON_RING_MASK_0,
		  WLAN_CFG_RXDMA2HOST_MON_RING_MASK_1,
		  WLAN_CFG_RXDMA2HOST_MON_RING_MASK_2,
		  0, 0, 0, 0, 0, 0, 0, 0},
		/* rx err ring masks */
		{ WLAN_CFG_RX_ERR_RING_MASK_0,
		  WLAN_CFG_RX_ERR_RING_MASK_1,
		  WLAN_CFG_RX_ERR_RING_MASK_2,
		  WLAN_CFG_RX_ERR_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
		/* rx wbm rel ring masks */
		{ WLAN_CFG_RX_WBM_REL_RING_MASK_0,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_1,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_2,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
		/* reo status ring masks */
		{ WLAN_CFG_REO_STATUS_RING_MASK_0,
		  WLAN_CFG_REO_STATUS_RING_MASK_1,
		  WLAN_CFG_REO_STATUS_RING_MASK_2,
		  WLAN_CFG_REO_STATUS_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
	},
	/* Interrupt assignment for 7 MSI combination */
	{
		/* tx ring masks */
		{ WLAN_CFG_TX_RING_MASK_0,
		  WLAN_CFG_TX_RING_MASK_1,
		  WLAN_CFG_TX_RING_MASK_2,
		  WLAN_CFG_TX_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
		/* rx ring masks */
		{ 0, 0, 0,
		  WLAN_CFG_RX_RING_MASK_0,
		  WLAN_CFG_RX_RING_MASK_1,
		  WLAN_CFG_RX_RING_MASK_2,
		  WLAN_CFG_RX_RING_MASK_3,
		  0, 0, 0},
		/* rx mon ring masks */
		{ 0, 0, 0,
		  WLAN_CFG_RX_MON_RING_MASK_0,
		  WLAN_CFG_RX_MON_RING_MASK_1,
		  WLAN_CFG_RX_MON_RING_MASK_2,
		  0, 0, 0, 0, 0},
		/* host2rxdma ring masks */
		{ 0, 0, 0,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_0,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_1,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_2,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_3,
		  0, 0, 0, 0},
		/* rxdma2host ring masks */
		{ 0, 0, 0,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_0,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_1,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_2,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_3,
		  0, 0, 0, 0},
		/* host2rxdma mon ring masks */
		{ 0, 0, 0,
		  WLAN_CFG_HOST2RXDMA_MON_RING_MASK_0,
		  WLAN_CFG_HOST2RXDMA_MON_RING_MASK_1,
		  WLAN_CFG_HOST2RXDMA_MON_RING_MASK_2,
		  0, 0, 0, 0, 0},
		/* rxdma2host mon ring masks */
		{ 0, 0,	0,
		  WLAN_CFG_RXDMA2HOST_MON_RING_MASK_0,
		  WLAN_CFG_RXDMA2HOST_MON_RING_MASK_1,
		  WLAN_CFG_RXDMA2HOST_MON_RING_MASK_2,
		  0, 0, 0, 0, 0},
		/* rx err ring masks */
		{ 0, 0, 0,
		  WLAN_CFG_RX_ERR_RING_MASK_0,
		  WLAN_CFG_RX_ERR_RING_MASK_1,
		  WLAN_CFG_RX_ERR_RING_MASK_2,
		  WLAN_CFG_RX_ERR_RING_MASK_3,
		  0, 0, 0, 0},
		/* rx wbm rel ring masks */
		{ 0, 0, 0,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_0,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_1,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_2,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_3,
		  0, 0, 0, 0},
		/* reo status ring masks */
		{ 0, 0, 0,
		  WLAN_CFG_REO_STATUS_RING_MASK_0,
		  WLAN_CFG_REO_STATUS_RING_MASK_1,
		  WLAN_CFG_REO_STATUS_RING_MASK_2,
		  WLAN_CFG_REO_STATUS_RING_MASK_3,
		  0, 0, 0, 0},
	},
	/* Interrupt assignment for 8 MSI combination */
	{
		/* tx ring masks */
		{ WLAN_CFG_TX_RING_MASK_0,
		  WLAN_CFG_TX_RING_MASK_1,
		  WLAN_CFG_TX_RING_MASK_2,
		  WLAN_CFG_TX_RING_MASK_3,
		  0, 0, 0, 0, 0, 0, 0},
		/* rx ring masks */
		{ 0, 0, 0, 0,
		  WLAN_CFG_RX_RING_MASK_0,
		  WLAN_CFG_RX_RING_MASK_1,
		  WLAN_CFG_RX_RING_MASK_2,
		  WLAN_CFG_RX_RING_MASK_3,
		  0, 0, 0},
		/* rx mon ring masks */
		{ 0, 0, 0,
		  WLAN_CFG_RX_MON_RING_MASK_0,
		  WLAN_CFG_RX_MON_RING_MASK_1,
		  WLAN_CFG_RX_MON_RING_MASK_2,
		  0, 0, 0, 0, 0},
		/* tx mon ring masks */
		{ WLAN_CFG_TX_MON_RING_MASK_0,
		  WLAN_CFG_TX_MON_RING_MASK_1,
		  0, 0, 0, 0, 0, 0},
		/* host2rxdma ring masks */
		{ 0, 0, 0,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_0,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_1,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_2,
		  WLAN_CFG_HOST2RXDMA_RING_MASK_3,
		  0, 0, 0, 0},
		/* rxdma2host ring masks */
		{ 0, 0, 0,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_0,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_1,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_2,
		  WLAN_CFG_RXDMA2HOST_RING_MASK_3,
		  0, 0, 0, 0},
		/* host2rxdma mon ring masks */
		{ 0, 0, 0,
		  WLAN_CFG_HOST2RXDMA_MON_RING_MASK_0,
		  WLAN_CFG_HOST2RXDMA_MON_RING_MASK_1,
		  WLAN_CFG_HOST2RXDMA_MON_RING_MASK_2,
		  0, 0, 0, 0, 0},
		/* rxdma2host mon ring masks */
		{ 0, 0, 0,
		  WLAN_CFG_RXDMA2HOST_MON_RING_MASK_0,
		  WLAN_CFG_RXDMA2HOST_MON_RING_MASK_1,
		  WLAN_CFG_RXDMA2HOST_MON_RING_MASK_2,
		  0, 0, 0, 0, 0},
		/* rx err ring masks */
		{ 0, 0, 0,
		  WLAN_CFG_RX_ERR_RING_MASK_0,
		  WLAN_CFG_RX_ERR_RING_MASK_1,
		  WLAN_CFG_RX_ERR_RING_MASK_2,
		  WLAN_CFG_RX_ERR_RING_MASK_3,
		  0, 0, 0, 0},
		/* rx wbm rel ring masks */
		{ 0, 0, 0,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_0,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_1,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_2,
		  WLAN_CFG_RX_WBM_REL_RING_MASK_3,
		  0, 0, 0, 0},
		/* reo status ring masks */
		{ 0, 0, 0,
		  WLAN_CFG_REO_STATUS_RING_MASK_0,
		  WLAN_CFG_REO_STATUS_RING_MASK_1,
		  WLAN_CFG_REO_STATUS_RING_MASK_2,
		  WLAN_CFG_REO_STATUS_RING_MASK_3,
		  0, 0, 0, 0},
	},
};
#endif
#endif

/**
 * g_wlan_srng_cfg[] - Per ring_type specific configuration
 *
 */
struct wlan_srng_cfg g_wlan_srng_cfg[MAX_RING_TYPES];

/* REO_DST ring configuration */
struct wlan_srng_cfg wlan_srng_reo_cfg = {
	.timer_threshold = WLAN_CFG_INT_TIMER_THRESHOLD_REO_RING,
	.batch_count_threshold = WLAN_CFG_INT_BATCH_THRESHOLD_REO_RING,
	.low_threshold = 0,
};

/* WBM2SW_RELEASE ring configuration */
struct wlan_srng_cfg wlan_srng_wbm_release_cfg = {
	.timer_threshold = WLAN_CFG_INT_TIMER_THRESHOLD_WBM_RELEASE_RING,
	.batch_count_threshold = 0,
	.low_threshold = 0,
};

/* RXDMA_BUF ring configuration */
struct wlan_srng_cfg wlan_srng_rxdma_buf_cfg = {
	.timer_threshold = WLAN_CFG_INT_TIMER_THRESHOLD_RX,
	.batch_count_threshold = 0,
	.low_threshold = WLAN_CFG_RXDMA_REFILL_RING_SIZE >> 3,
};

/* RXDMA_MONITOR_BUF ring configuration */
struct wlan_srng_cfg wlan_srng_rxdma_monitor_buf_cfg = {
	.timer_threshold = WLAN_CFG_INT_TIMER_THRESHOLD_RX,
	.batch_count_threshold = 0,
	.low_threshold = WLAN_CFG_RXDMA_MONITOR_BUF_RING_SIZE >> 3,
};

/* RXDMA_MONITOR_STATUS ring configuration */
struct wlan_srng_cfg wlan_srng_rxdma_monitor_status_cfg = {
	.timer_threshold = WLAN_CFG_INT_TIMER_THRESHOLD_RX,
	.batch_count_threshold = 0,
	.low_threshold = WLAN_CFG_RXDMA_MONITOR_STATUS_RING_SIZE >> 3,
};

/* TX_MONITOR_BUF ring configuration */
struct wlan_srng_cfg wlan_srng_tx_monitor_buf_cfg = {
	.timer_threshold = WLAN_CFG_INT_TIMER_THRESHOLD_TX,
	.batch_count_threshold = 0,
	.low_threshold = WLAN_CFG_TX_MONITOR_BUF_SIZE_MAX >> 3,
};

/* DEFAULT_CONFIG ring configuration */
struct wlan_srng_cfg wlan_srng_default_cfg = {
	.timer_threshold = WLAN_CFG_INT_TIMER_THRESHOLD_OTHER,
	.batch_count_threshold = WLAN_CFG_INT_BATCH_THRESHOLD_OTHER,
	.low_threshold = 0,
};

/* DEFAULT_CONFIG source ring configuration */
struct wlan_srng_cfg wlan_src_srng_default_cfg = {
	.timer_threshold = 0,
	.batch_count_threshold = 0,
	.low_threshold = 0,
};

void wlan_set_srng_cfg(struct wlan_srng_cfg **wlan_cfg)
{
	g_wlan_srng_cfg[REO_DST] = wlan_srng_reo_cfg;
	g_wlan_srng_cfg[WBM2SW_RELEASE] = wlan_srng_wbm_release_cfg;
	g_wlan_srng_cfg[REO_EXCEPTION] = wlan_srng_default_cfg;
	g_wlan_srng_cfg[REO_REINJECT] = wlan_src_srng_default_cfg;
	g_wlan_srng_cfg[REO_CMD] = wlan_src_srng_default_cfg;
	g_wlan_srng_cfg[REO_STATUS] = wlan_srng_default_cfg;
	g_wlan_srng_cfg[TCL_DATA] = wlan_src_srng_default_cfg;
	g_wlan_srng_cfg[TCL_CMD_CREDIT] = wlan_src_srng_default_cfg;
	g_wlan_srng_cfg[TCL_STATUS] = wlan_srng_default_cfg;
	g_wlan_srng_cfg[WBM_IDLE_LINK] = wlan_src_srng_default_cfg;
	g_wlan_srng_cfg[SW2WBM_RELEASE] = wlan_src_srng_default_cfg;
	g_wlan_srng_cfg[RXDMA_BUF] = wlan_srng_rxdma_buf_cfg;
	g_wlan_srng_cfg[RXDMA_DST] = wlan_srng_default_cfg;
	g_wlan_srng_cfg[RXDMA_MONITOR_BUF] =
			wlan_srng_rxdma_monitor_buf_cfg;
	g_wlan_srng_cfg[RXDMA_MONITOR_STATUS] =
			wlan_srng_rxdma_monitor_status_cfg;
	g_wlan_srng_cfg[RXDMA_MONITOR_DST] = wlan_srng_default_cfg;
	g_wlan_srng_cfg[RXDMA_MONITOR_DESC] = wlan_srng_default_cfg;
	g_wlan_srng_cfg[DIR_BUF_RX_DMA_SRC] = wlan_srng_default_cfg;
#ifdef WLAN_FEATURE_CIF_CFR
	g_wlan_srng_cfg[WIFI_POS_SRC] = wlan_srng_default_cfg;
#endif
	g_wlan_srng_cfg[TX_MONITOR_BUF] = wlan_srng_tx_monitor_buf_cfg;
	*wlan_cfg = g_wlan_srng_cfg;
}

static const uint8_t rx_fst_toeplitz_key[WLAN_CFG_RX_FST_TOEPLITZ_KEYLEN] = {
	0x6d, 0x5a, 0x56, 0xda, 0x25, 0x5b, 0x0e, 0xc2,
	0x41, 0x67, 0x25, 0x3d, 0x43, 0xa3, 0x8f, 0xb0,
	0xd0, 0xca, 0x2b, 0xcb, 0xae, 0x7b, 0x30, 0xb4,
	0x77, 0xcb, 0x2d, 0xa3, 0x80, 0x30, 0xf2, 0x0c,
	0x6a, 0x42, 0xb7, 0x3b, 0xbe, 0xac, 0x01, 0xfa
};

#if defined(WLAN_MAX_PDEVS) && (WLAN_MAX_PDEVS == 1)
void wlan_cfg_fill_interrupt_mask(struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx,
				  int num_dp_msi,
				  int interrupt_mode,
				  bool is_monitor_mode)
{	int i = 0;
	const uint8_t *tx_ring_intr_mask =
				wlan_cfg_get_tx_ring_int_mask(wlan_cfg_ctx);

	for (i = 0; i < WLAN_CFG_INT_NUM_CONTEXTS; i++) {
		wlan_cfg_ctx->int_tx_ring_mask[i] = tx_ring_intr_mask[i];
		if (wlan_cfg_ctx->int_tx_ring_mask[i])
			wlan_cfg_ctx->tx_rings_grp_bitmap |= BIT(i);

		wlan_cfg_ctx->int_rx_mon_ring_mask[i] =
							rx_mon_ring_mask_msi[i];
		wlan_cfg_ctx->int_tx_mon_ring_mask[i] = 0;
		wlan_cfg_ctx->int_rx_err_ring_mask[i] =
							rx_err_ring_mask_msi[i];
		wlan_cfg_ctx->int_rx_wbm_rel_ring_mask[i] =
						rx_wbm_rel_ring_mask_msi[i];
		wlan_cfg_ctx->int_reo_status_ring_mask[i] =
							reo_status_ring_mask_msi[i];
		if (is_monitor_mode) {
			wlan_cfg_ctx->int_rx_ring_mask[i] = 0;
			if (interrupt_mode == DP_INTR_POLL)
				wlan_cfg_ctx->int_rxdma2host_ring_mask[i] = 0;
			else
				wlan_cfg_ctx->int_rxdma2host_ring_mask[i] =
						rxdma2host_ring_mask_msi[i];
		} else {
			wlan_cfg_ctx->int_rx_ring_mask[i] =
							rx_ring_mask_msi[i];
			wlan_cfg_ctx->int_rxdma2host_ring_mask[i] =
						rxdma2host_ring_mask_msi[i];
		}
		wlan_cfg_ctx->int_host2rxdma_ring_mask[i] =
						host2rxdma_ring_mask_msi[i];
		wlan_cfg_ctx->int_host2rxdma_mon_ring_mask[i] =
						host2rxdma_mon_ring_mask_msi[i];
		wlan_cfg_ctx->int_rxdma2host_mon_ring_mask[i] =
						rxdma2host_mon_ring_mask_msi[i];
		wlan_cfg_ctx->int_rx_ring_near_full_irq_1_mask[i] =
					rx_ring_near_full_irq_1_mask_msi[i];
		wlan_cfg_ctx->int_rx_ring_near_full_irq_2_mask[i] =
					rx_ring_near_full_irq_2_mask_msi[i];
		wlan_cfg_ctx->int_tx_ring_near_full_irq_mask[i] =
					tx_ring_near_full_irq_mask_msi[i];
	}
}

#else

void wlan_cfg_fill_interrupt_mask(struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx,
				     int num_dp_msi,
				     int interrupt_mode,
				     bool is_monitor_mode)
{
	int i = 0;
	int interrupt_index = 0;

	if(interrupt_mode == DP_INTR_INTEGRATED) {
		interrupt_index = 0;
	} else if (interrupt_mode == DP_INTR_MSI || interrupt_mode ==
		   DP_INTR_POLL) {
		interrupt_index = num_dp_msi;
	} else {
		qdf_err("Interrupt mode %d", interrupt_mode);
	}

	for (i = 0; i < WLAN_CFG_INT_NUM_CONTEXTS; i++) {
		wlan_cfg_ctx->int_tx_ring_mask[i] =
			dp_mask_assignment[interrupt_index].tx_ring_mask[i];
		wlan_cfg_ctx->int_rx_mon_ring_mask[i] =
			dp_mask_assignment[interrupt_index].rx_mon_ring_mask[i];
		wlan_cfg_ctx->int_tx_mon_ring_mask[i] =
			dp_mask_assignment[interrupt_index].tx_mon_ring_mask[i];
		wlan_cfg_ctx->int_rx_err_ring_mask[i] =
			dp_mask_assignment[interrupt_index].rx_err_ring_mask[i];
		wlan_cfg_ctx->int_rx_wbm_rel_ring_mask[i] =
			dp_mask_assignment[interrupt_index].rx_wbm_rel_ring_mask[i];
		wlan_cfg_ctx->int_reo_status_ring_mask[i] =
			dp_mask_assignment[interrupt_index].reo_status_ring_mask[i];
		if (is_monitor_mode) {
			wlan_cfg_ctx->int_rx_ring_mask[i] = 0;
			wlan_cfg_ctx->int_rxdma2host_ring_mask[i] = 0;
		} else {
			wlan_cfg_ctx->int_rx_ring_mask[i] =
				dp_mask_assignment[interrupt_index].rx_ring_mask[i];
			wlan_cfg_ctx->int_rxdma2host_ring_mask[i] =
				dp_mask_assignment[interrupt_index].rxdma2host_ring_mask[i];
		}
		wlan_cfg_ctx->int_host2rxdma_ring_mask[i] =
			dp_mask_assignment[interrupt_index].host2rxdma_ring_mask[i];
		wlan_cfg_ctx->int_host2rxdma_mon_ring_mask[i] =
			dp_mask_assignment[interrupt_index].host2rxdma_mon_ring_mask[i];
		wlan_cfg_ctx->int_rxdma2host_mon_ring_mask[i] =
			dp_mask_assignment[interrupt_index].rxdma2host_mon_ring_mask[i];
		wlan_cfg_ctx->int_rx_ring_near_full_irq_1_mask[i] =
			dp_mask_assignment[interrupt_index].rx_ring_near_full_irq_1_mask[i];
		wlan_cfg_ctx->int_rx_ring_near_full_irq_2_mask[i] =
			dp_mask_assignment[interrupt_index].rx_ring_near_full_irq_2_mask[i];
		wlan_cfg_ctx->int_tx_ring_near_full_irq_mask[i] =
			dp_mask_assignment[interrupt_index].tx_ring_near_full_irq_mask[i];
	}
}
#endif

#ifdef IPA_OFFLOAD

#define WLAN_CFG_IPA_ENABLE_MASK BIT(0)
#ifdef IPA_WDI3_TX_TWO_PIPES
/**
 * wlan_soc_ipa_cfg_attach() - Update ipa tx and tx alt config
 *  in dp soc cfg context
 * @psoc: Object manager psoc
 * @wlan_cfg_ctx: dp soc cfg ctx
 *
 * Return: None
 */
static void
wlan_soc_ipa_cfg_attach(struct cdp_ctrl_objmgr_psoc *psoc,
			struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx)
{
	wlan_cfg_ctx->ipa_enabled = (cfg_get(psoc, CFG_DP_IPA_OFFLOAD_CONFIG) &
				     WLAN_CFG_IPA_ENABLE_MASK);
	wlan_cfg_ctx->ipa_tx_ring_size =
			cfg_get(psoc, CFG_DP_IPA_TX_RING_SIZE);
	wlan_cfg_ctx->ipa_tx_comp_ring_size =
			cfg_get(psoc, CFG_DP_IPA_TX_COMP_RING_SIZE);
	wlan_cfg_ctx->ipa_tx_alt_ring_size =
			cfg_get(psoc, CFG_DP_IPA_TX_ALT_RING_SIZE);
	wlan_cfg_ctx->ipa_tx_alt_comp_ring_size =
			cfg_get(psoc, CFG_DP_IPA_TX_ALT_COMP_RING_SIZE);
}
#else /* !IPA_WDI3_TX_TWO_PIPES */
/**
 * wlan_soc_ipa_cfg_attach() - Update ipa config in dp soc
 *  cfg context
 * @psoc - Object manager psoc
 * @wlan_cfg_ctx - dp soc cfg ctx
 *
 * Return: None
 */
static void
wlan_soc_ipa_cfg_attach(struct cdp_ctrl_objmgr_psoc *psoc,
			struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx)
{
	wlan_cfg_ctx->ipa_enabled = (cfg_get(psoc, CFG_DP_IPA_OFFLOAD_CONFIG) &
				     WLAN_CFG_IPA_ENABLE_MASK);
	wlan_cfg_ctx->ipa_tx_ring_size =
			cfg_get(psoc, CFG_DP_IPA_TX_RING_SIZE);
	wlan_cfg_ctx->ipa_tx_comp_ring_size =
			cfg_get(psoc, CFG_DP_IPA_TX_COMP_RING_SIZE);
}
#endif /* IPA_WDI3_TX_TWO_PIPES */
#else /* !IPA_OFFLOAD */
static inline void
wlan_soc_ipa_cfg_attach(struct cdp_ctrl_objmgr_psoc *psoc,
			struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx)
{
}
#endif

#ifdef DP_HW_COOKIE_CONVERT_EXCEPTION
static void
wlan_soc_hw_cc_cfg_attach(struct cdp_ctrl_objmgr_psoc *psoc,
			  struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx)
{
	wlan_cfg_ctx->hw_cc_enabled =
			cfg_get(psoc, CFG_DP_HW_CC_ENABLE);
}
#else
static void
wlan_soc_hw_cc_cfg_attach(struct cdp_ctrl_objmgr_psoc *psoc,
			  struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx)
{
	wlan_cfg_ctx->hw_cc_enabled = true;
}
#endif

#ifdef WLAN_SUPPORT_PPEDS
/**
 * wlan_soc_ppe_cfg_attach() - Update ppe config in dp soc
 *  cfg context
 * @psoc - Object manager psoc
 * @wlan_cfg_ctx - dp soc cfg ctx
 *
 * Return: None
 */
static void
wlan_soc_ppe_cfg_attach(struct cdp_ctrl_objmgr_psoc *psoc,
			struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx)
{
	wlan_cfg_ctx->ppe_enable = cfg_get(psoc, CFG_DP_PPE_ENABLE);
	wlan_cfg_ctx->reo2ppe_ring = cfg_get(psoc, CFG_DP_REO2PPE_RING);
	wlan_cfg_ctx->ppe2tcl_ring = cfg_get(psoc, CFG_DP_PPE2TCL_RING);
	wlan_cfg_ctx->ppe_release_ring = cfg_get(psoc,
						 CFG_DP_PPE_RELEASE_RING);
}
#else
static inline void
wlan_soc_ppe_cfg_attach(struct cdp_ctrl_objmgr_psoc *psoc,
			struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx)
{
}
#endif

#if defined(WLAN_FEATURE_11BE_MLO) && defined(WLAN_MLO_MULTI_CHIP)
/**
 * wlan_cfg_get_lsb_set_pos() - returns position of LSB which is set
 *
 * Return: position of LSB which is set
 */
static uint8_t wlan_cfg_get_lsb_set_pos(uint8_t val)
{
	uint8_t pos = 0;

	while (pos < 8) {
		if (val & (1 << pos))
			return pos;

		pos++;
	}

	return 0;
}

/**
 * wlan_multi_soc_mlo_cfg_attach() - Update multi soc mlo config in dp soc
 *  cfg context
 * @psoc - Object manager psoc
 * @wlan_cfg_ctx - dp soc cfg ctx
 *
 * Return: None
 */
static void
wlan_multi_soc_mlo_cfg_attach(struct cdp_ctrl_objmgr_psoc *psoc,
			      struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx)
{
	uint8_t rx_ring_map;

	rx_ring_map =
		cfg_get(psoc, CFG_DP_MLO_CHIP0_RX_RING_MAP);
	wlan_cfg_ctx->mlo_chip_rx_ring_map[0] = rx_ring_map;
	wlan_cfg_ctx->mlo_chip_default_rx_ring_id[0] =
			wlan_cfg_get_lsb_set_pos(rx_ring_map);
	wlan_cfg_ctx->lmac_peer_id_msb[0] = 1;

	rx_ring_map =
		cfg_get(psoc, CFG_DP_MLO_CHIP1_RX_RING_MAP);
	wlan_cfg_ctx->mlo_chip_rx_ring_map[1] = rx_ring_map;
	wlan_cfg_ctx->mlo_chip_default_rx_ring_id[1] =
			wlan_cfg_get_lsb_set_pos(rx_ring_map);
	wlan_cfg_ctx->lmac_peer_id_msb[1] = 2;

	rx_ring_map =
		cfg_get(psoc, CFG_DP_MLO_CHIP2_RX_RING_MAP);
	wlan_cfg_ctx->mlo_chip_rx_ring_map[2] = rx_ring_map;
	wlan_cfg_ctx->mlo_chip_default_rx_ring_id[2] =
			wlan_cfg_get_lsb_set_pos(rx_ring_map);
	wlan_cfg_ctx->lmac_peer_id_msb[2] = 3;
}
#else
static inline void
wlan_multi_soc_mlo_cfg_attach(struct cdp_ctrl_objmgr_psoc *psoc,
			      struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx)
{
}
#endif

#ifdef WLAN_FEATURE_11BE_MLO
/**
 * wlan_soc_mlo_cfg_attach() - Update mlo config in dp soc
 *  cfg context
 * @psoc - Object manager psoc
 * @wlan_cfg_ctx - dp soc cfg ctx
 *
 * Return: None
 */
static void
wlan_soc_mlo_cfg_attach(struct cdp_ctrl_objmgr_psoc *psoc,
			struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx)
{
	wlan_multi_soc_mlo_cfg_attach(psoc, wlan_cfg_ctx);
}
#else
static inline void
wlan_soc_mlo_cfg_attach(struct cdp_ctrl_objmgr_psoc *psoc,
			struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx)
{
}
#endif

#ifdef QCA_VDEV_STATS_HW_OFFLOAD_SUPPORT
/**
 * wlan_soc_vdev_hw_stats_cfg_attach() - Update hw vdev stats config in dp soc
 *  cfg context
 * @psoc - Object manager psoc
 * @wlan_cfg_ctx - dp soc cfg ctx
 *
 * Return: None
 */
static void
wlan_soc_vdev_hw_stats_cfg_attach(struct cdp_ctrl_objmgr_psoc *psoc,
				  struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx)
{
	wlan_cfg_ctx->vdev_stats_hw_offload_config = cfg_get(psoc,
					CFG_DP_VDEV_STATS_HW_OFFLOAD_CONFIG);
	wlan_cfg_ctx->vdev_stats_hw_offload_timer = cfg_get(psoc,
					CFG_DP_VDEV_STATS_HW_OFFLOAD_TIMER);
}
#else
static void
wlan_soc_vdev_hw_stats_cfg_attach(struct cdp_ctrl_objmgr_psoc *psoc,
				  struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx)
{
}
#endif

/**
 * wlan_cfg_soc_attach() - Allocate and prepare SoC configuration
 * @psoc - Object manager psoc
 * Return: wlan_cfg_ctx - Handle to Configuration context
 */
struct wlan_cfg_dp_soc_ctxt *
wlan_cfg_soc_attach(struct cdp_ctrl_objmgr_psoc *psoc)
{
	struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx =
		qdf_mem_malloc(sizeof(struct wlan_cfg_dp_soc_ctxt));
	uint32_t gro_bit_set;

	if (!wlan_cfg_ctx)
		return NULL;

	wlan_cfg_ctx->rxdma1_enable = WLAN_CFG_RXDMA1_ENABLE;
	wlan_cfg_ctx->num_int_ctxts = WLAN_CFG_INT_NUM_CONTEXTS;
	wlan_cfg_ctx->max_clients = cfg_get(psoc, CFG_DP_MAX_CLIENTS);
	wlan_cfg_ctx->max_alloc_size = cfg_get(psoc, CFG_DP_MAX_ALLOC_SIZE);
	wlan_cfg_ctx->per_pdev_tx_ring = cfg_get(psoc, CFG_DP_PDEV_TX_RING);
	wlan_cfg_ctx->num_reo_dest_rings = cfg_get(psoc, CFG_DP_REO_DEST_RINGS);
	wlan_cfg_ctx->num_tcl_data_rings = cfg_get(psoc, CFG_DP_TCL_DATA_RINGS);
	wlan_cfg_ctx->num_nss_reo_dest_rings =
				cfg_get(psoc, CFG_DP_NSS_REO_DEST_RINGS);
	wlan_cfg_ctx->num_nss_tcl_data_rings =
				cfg_get(psoc, CFG_DP_NSS_TCL_DATA_RINGS);
	wlan_cfg_ctx->per_pdev_rx_ring = cfg_get(psoc, CFG_DP_PDEV_RX_RING);
	wlan_cfg_ctx->per_pdev_lmac_ring = cfg_get(psoc, CFG_DP_PDEV_LMAC_RING);
	wlan_cfg_ctx->num_tx_desc_pool = MAX_TXDESC_POOLS;
	wlan_cfg_ctx->num_tx_ext_desc_pool = cfg_get(psoc,
						     CFG_DP_TX_EXT_DESC_POOLS);
	wlan_cfg_ctx->num_tx_desc = cfg_get(psoc, CFG_DP_TX_DESC);
	wlan_cfg_ctx->min_tx_desc = WLAN_CFG_NUM_TX_DESC_MIN;
	wlan_cfg_ctx->num_tx_ext_desc = cfg_get(psoc, CFG_DP_TX_EXT_DESC);
	wlan_cfg_ctx->htt_packet_type = cfg_get(psoc, CFG_DP_HTT_PACKET_TYPE);
	wlan_cfg_ctx->max_peer_id = cfg_get(psoc, CFG_DP_MAX_PEER_ID);

	wlan_cfg_ctx->tx_ring_size = cfg_get(psoc, CFG_DP_TX_RING_SIZE);
	wlan_cfg_ctx->tx_comp_ring_size = cfg_get(psoc,
						  CFG_DP_TX_COMPL_RING_SIZE);

	wlan_cfg_ctx->tx_comp_ring_size_nss =
		cfg_get(psoc, CFG_DP_NSS_COMP_RING_SIZE);

	wlan_cfg_ctx->int_batch_threshold_tx =
			cfg_get(psoc, CFG_DP_INT_BATCH_THRESHOLD_TX);
	wlan_cfg_ctx->int_timer_threshold_tx =
			cfg_get(psoc, CFG_DP_INT_TIMER_THRESHOLD_TX);
	wlan_cfg_ctx->int_batch_threshold_rx =
			cfg_get(psoc, CFG_DP_INT_BATCH_THRESHOLD_RX);
	wlan_cfg_ctx->int_timer_threshold_rx =
			cfg_get(psoc, CFG_DP_INT_TIMER_THRESHOLD_RX);
	wlan_cfg_ctx->int_batch_threshold_other =
		cfg_get(psoc, CFG_DP_INT_BATCH_THRESHOLD_OTHER);
	wlan_cfg_ctx->int_timer_threshold_other =
		cfg_get(psoc, CFG_DP_INT_TIMER_THRESHOLD_OTHER);
	wlan_cfg_ctx->pktlog_buffer_size =
		cfg_get(psoc, CFG_DP_PKTLOG_BUFFER_SIZE);

	/* This is default mapping and can be overridden by HW config
	 * received from FW */
	wlan_cfg_set_hw_mac_idx(wlan_cfg_ctx, 0, 0);
	if (MAX_PDEV_CNT > 1)
		wlan_cfg_set_hw_mac_idx(wlan_cfg_ctx, 1, 2);
	if (MAX_PDEV_CNT > 2)
		wlan_cfg_set_hw_mac_idx(wlan_cfg_ctx, 2, 1);

	wlan_cfg_ctx->base_hw_macid = cfg_get(psoc, CFG_DP_BASE_HW_MAC_ID);

	wlan_cfg_ctx->rx_hash = cfg_get(psoc, CFG_DP_RX_HASH);
	wlan_cfg_ctx->tso_enabled = cfg_get(psoc, CFG_DP_TSO);
	wlan_cfg_ctx->lro_enabled = cfg_get(psoc, CFG_DP_LRO);
	wlan_cfg_ctx->sg_enabled = cfg_get(psoc, CFG_DP_SG);
	gro_bit_set = cfg_get(psoc, CFG_DP_GRO);
	if (gro_bit_set & DP_GRO_ENABLE_BIT_SET) {
		wlan_cfg_ctx->gro_enabled = true;
		if (gro_bit_set & DP_FORCE_USE_GRO_BIT_SET)
			wlan_cfg_ctx->force_gro_enabled = true;
	}
	wlan_cfg_ctx->ol_tx_csum_enabled = cfg_get(psoc, CFG_DP_OL_TX_CSUM);
	wlan_cfg_ctx->ol_rx_csum_enabled = cfg_get(psoc, CFG_DP_OL_RX_CSUM);
	wlan_cfg_ctx->rawmode_enabled = cfg_get(psoc, CFG_DP_RAWMODE);
	wlan_cfg_ctx->peer_flow_ctrl_enabled =
			cfg_get(psoc, CFG_DP_PEER_FLOW_CTRL);
	wlan_cfg_ctx->napi_enabled = cfg_get(psoc, CFG_DP_NAPI);
	wlan_cfg_ctx->p2p_tcp_udp_checksumoffload =
			cfg_get(psoc, CFG_DP_P2P_TCP_UDP_CKSUM_OFFLOAD);
	wlan_cfg_ctx->nan_tcp_udp_checksumoffload =
			cfg_get(psoc, CFG_DP_NAN_TCP_UDP_CKSUM_OFFLOAD);
	wlan_cfg_ctx->tcp_udp_checksumoffload =
			cfg_get(psoc, CFG_DP_TCP_UDP_CKSUM_OFFLOAD);
	wlan_cfg_ctx->legacy_mode_checksumoffload_disable =
			cfg_get(psoc, CFG_DP_LEGACY_MODE_CSUM_DISABLE);
	wlan_cfg_ctx->per_pkt_trace = cfg_get(psoc, CFG_DP_PER_PKT_LOGGING);
	wlan_cfg_ctx->defrag_timeout_check =
			cfg_get(psoc, CFG_DP_DEFRAG_TIMEOUT_CHECK);
	wlan_cfg_ctx->rx_defrag_min_timeout =
			cfg_get(psoc, CFG_DP_RX_DEFRAG_TIMEOUT);

	wlan_cfg_ctx->wbm_release_ring = cfg_get(psoc,
						 CFG_DP_WBM_RELEASE_RING);
	wlan_cfg_ctx->tcl_cmd_credit_ring = cfg_get(psoc,
					     CFG_DP_TCL_CMD_CREDIT_RING);
	wlan_cfg_ctx->tcl_status_ring = cfg_get(psoc,
						CFG_DP_TCL_STATUS_RING);
	wlan_cfg_ctx->reo_dst_ring_size = cfg_get(psoc,
						  CFG_DP_RX_DESTINATION_RING);
	wlan_cfg_ctx->reo_reinject_ring = cfg_get(psoc,
						  CFG_DP_REO_REINJECT_RING);
	wlan_cfg_ctx->rx_release_ring = cfg_get(psoc,
						CFG_DP_RX_RELEASE_RING);
	wlan_cfg_ctx->reo_exception_ring = cfg_get(psoc,
						   CFG_DP_REO_EXCEPTION_RING);
	wlan_cfg_ctx->reo_cmd_ring = cfg_get(psoc,
					     CFG_DP_REO_CMD_RING);
	wlan_cfg_ctx->reo_status_ring = cfg_get(psoc,
						CFG_DP_REO_STATUS_RING);
	wlan_cfg_ctx->rxdma_refill_ring = cfg_get(psoc,
						  CFG_DP_RXDMA_REFILL_RING);
	wlan_cfg_ctx->tx_desc_limit_0 = cfg_get(psoc,
						CFG_DP_TX_DESC_LIMIT_0);
	wlan_cfg_ctx->tx_desc_limit_1 = cfg_get(psoc,
						CFG_DP_TX_DESC_LIMIT_1);
	wlan_cfg_ctx->tx_desc_limit_2 = cfg_get(psoc,
						CFG_DP_TX_DESC_LIMIT_2);
	wlan_cfg_ctx->tx_device_limit = cfg_get(psoc,
						CFG_DP_TX_DEVICE_LIMIT);
	wlan_cfg_ctx->tx_sw_internode_queue = cfg_get(psoc,
						CFG_DP_TX_SW_INTERNODE_QUEUE);
	wlan_cfg_ctx->rxdma_err_dst_ring = cfg_get(psoc,
						   CFG_DP_RXDMA_ERR_DST_RING);
	wlan_cfg_ctx->enable_data_stall_detection =
		cfg_get(psoc, CFG_DP_ENABLE_DATA_STALL_DETECTION);
	wlan_cfg_ctx->tx_flow_start_queue_offset =
		cfg_get(psoc, CFG_DP_TX_FLOW_START_QUEUE_OFFSET);
	wlan_cfg_ctx->tx_flow_stop_queue_threshold =
		cfg_get(psoc, CFG_DP_TX_FLOW_STOP_QUEUE_TH);
	wlan_cfg_ctx->disable_intra_bss_fwd =
		cfg_get(psoc, CFG_DP_AP_STA_SECURITY_SEPERATION);
	wlan_cfg_ctx->rx_sw_desc_weight = cfg_get(psoc,
						   CFG_DP_RX_SW_DESC_WEIGHT);
	wlan_cfg_ctx->rx_sw_desc_num = cfg_get(psoc,
						   CFG_DP_RX_SW_DESC_NUM);
	wlan_cfg_ctx->rx_toeplitz_hash_key = (uint8_t *)rx_fst_toeplitz_key;
	wlan_cfg_ctx->rx_flow_max_search = WLAN_CFG_RX_FST_MAX_SEARCH;
	wlan_cfg_ctx->is_rx_flow_tag_enabled =
			cfg_get(psoc, CFG_DP_RX_FLOW_TAG_ENABLE);
	wlan_cfg_ctx->is_rx_flow_search_table_per_pdev =
			cfg_get(psoc, CFG_DP_RX_FLOW_SEARCH_TABLE_PER_PDEV);
	wlan_cfg_ctx->rx_flow_search_table_size =
			cfg_get(psoc, CFG_DP_RX_FLOW_SEARCH_TABLE_SIZE);
	wlan_cfg_ctx->is_rx_mon_protocol_flow_tag_enabled =
			cfg_get(psoc, CFG_DP_RX_MON_PROTOCOL_FLOW_TAG_ENABLE);
	wlan_cfg_ctx->mon_drop_thresh =
		cfg_get(psoc, CFG_DP_RXDMA_MONITOR_RX_DROP_THRESHOLD);
	wlan_cfg_ctx->is_rx_fisa_enabled = cfg_get(psoc, CFG_DP_RX_FISA_ENABLE);
	wlan_cfg_ctx->reo_rings_mapping = cfg_get(psoc, CFG_DP_REO_RINGS_MAP);
	wlan_cfg_ctx->pext_stats_enabled = cfg_get(psoc, CFG_DP_PEER_EXT_STATS);
	wlan_cfg_ctx->is_rx_buff_pool_enabled =
			cfg_get(psoc, CFG_DP_RX_BUFF_POOL_ENABLE);
	wlan_cfg_ctx->is_rx_refill_buff_pool_enabled =
			cfg_get(psoc, CFG_DP_RX_REFILL_BUFF_POOL_ENABLE);
	wlan_cfg_ctx->rx_pending_high_threshold =
			cfg_get(psoc, CFG_DP_RX_PENDING_HL_THRESHOLD);
	wlan_cfg_ctx->rx_pending_low_threshold =
			cfg_get(psoc, CFG_DP_RX_PENDING_LO_THRESHOLD);
	wlan_cfg_ctx->is_poll_mode_enabled =
			cfg_get(psoc, CFG_DP_POLL_MODE_ENABLE);
	wlan_cfg_ctx->is_swlm_enabled = cfg_get(psoc, CFG_DP_SWLM_ENABLE);
	wlan_cfg_ctx->fst_in_cmem = cfg_get(psoc, CFG_DP_RX_FST_IN_CMEM);
	wlan_cfg_ctx->tx_per_pkt_vdev_id_check =
			cfg_get(psoc, CFG_DP_TX_PER_PKT_VDEV_ID_CHECK);
	wlan_cfg_ctx->radio0_rx_default_reo =
			cfg_get(psoc, CFG_DP_RX_RADIO_0_DEFAULT_REO);
	wlan_cfg_ctx->radio1_rx_default_reo =
			cfg_get(psoc, CFG_DP_RX_RADIO_1_DEFAULT_REO);
	wlan_cfg_ctx->radio2_rx_default_reo =
			cfg_get(psoc, CFG_DP_RX_RADIO_2_DEFAULT_REO);
	wlan_cfg_ctx->wow_check_rx_pending_enable =
			cfg_get(psoc, CFG_DP_WOW_CHECK_RX_PENDING);
	wlan_cfg_ctx->delay_mon_replenish = cfg_get(psoc,
			CFG_DP_DELAY_MON_REPLENISH);
	wlan_cfg_ctx->rx_mon_buf_ring_size = cfg_get(psoc,
					CFG_DP_RXDMA_MONITOR_BUF_RING);
	wlan_cfg_ctx->tx_mon_buf_ring_size = cfg_get(psoc,
					CFG_DP_TX_MONITOR_BUF_RING);
	wlan_soc_ipa_cfg_attach(psoc, wlan_cfg_ctx);
	wlan_soc_hw_cc_cfg_attach(psoc, wlan_cfg_ctx);
	wlan_soc_ppe_cfg_attach(psoc, wlan_cfg_ctx);
	wlan_soc_mlo_cfg_attach(psoc, wlan_cfg_ctx);
	wlan_soc_vdev_hw_stats_cfg_attach(psoc, wlan_cfg_ctx);
#ifdef WLAN_FEATURE_PKT_CAPTURE_V2
	wlan_cfg_ctx->pkt_capture_mode = cfg_get(psoc, CFG_PKT_CAPTURE_MODE) &
						 PKT_CAPTURE_MODE_DATA_ONLY;
#endif
	wlan_cfg_ctx->num_rxdma_dst_rings_per_pdev = NUM_RXDMA_RINGS_PER_PDEV;
	wlan_cfg_ctx->num_rxdma_status_rings_per_pdev =
					NUM_RXDMA_RINGS_PER_PDEV;

	return wlan_cfg_ctx;
}

void wlan_cfg_soc_detach(struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx)
{
	qdf_mem_free(wlan_cfg_ctx);
}

struct wlan_cfg_dp_pdev_ctxt *
wlan_cfg_pdev_attach(struct cdp_ctrl_objmgr_psoc *psoc)
{
	struct wlan_cfg_dp_pdev_ctxt *wlan_cfg_ctx =
		qdf_mem_malloc(sizeof(struct wlan_cfg_dp_pdev_ctxt));

	if (!wlan_cfg_ctx)
		return NULL;

	wlan_cfg_ctx->rx_dma_buf_ring_size = cfg_get(psoc,
					CFG_DP_RXDMA_BUF_RING);
	wlan_cfg_ctx->dma_mon_buf_ring_size = cfg_get(psoc,
					CFG_DP_RXDMA_MONITOR_BUF_RING);
	wlan_cfg_ctx->dma_rx_mon_dest_ring_size = cfg_get(psoc,
					CFG_DP_RXDMA_MONITOR_DST_RING);
	wlan_cfg_ctx->dma_tx_mon_dest_ring_size = cfg_get(psoc,
					CFG_DP_TX_MONITOR_DST_RING);
	wlan_cfg_ctx->dma_mon_status_ring_size = cfg_get(psoc,
					CFG_DP_RXDMA_MONITOR_STATUS_RING);
	wlan_cfg_ctx->rxdma_monitor_desc_ring = cfg_get(psoc,
					CFG_DP_RXDMA_MONITOR_DESC_RING);
	wlan_cfg_ctx->num_mac_rings = NUM_RXDMA_RINGS_PER_PDEV;

	return wlan_cfg_ctx;
}

void wlan_cfg_set_mon_delayed_replenish_entries(
					struct wlan_cfg_dp_soc_ctxt *cfg,
					uint32_t val)
{
	cfg->delayed_replenish_entries = val;
}

qdf_export_symbol(wlan_cfg_set_mon_delayed_replenish_entries);

int wlan_cfg_get_mon_delayed_replenish_entries(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->delayed_replenish_entries;
}

void wlan_cfg_pdev_detach(struct wlan_cfg_dp_pdev_ctxt *wlan_cfg_ctx)
{
	if (wlan_cfg_ctx)
		qdf_mem_free(wlan_cfg_ctx);
}

int wlan_cfg_get_mon_drop_thresh(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->mon_drop_thresh;
}

void wlan_cfg_set_num_contexts(struct wlan_cfg_dp_soc_ctxt *cfg, int num)
{
	cfg->num_int_ctxts = num;
}

void wlan_cfg_set_max_peer_id(struct wlan_cfg_dp_soc_ctxt *cfg, uint32_t val)
{
	cfg->max_peer_id = val;
}

void wlan_cfg_set_max_ast_idx(struct wlan_cfg_dp_soc_ctxt *cfg, uint32_t val)
{
	cfg->max_ast_idx = val;
}

int wlan_cfg_get_max_ast_idx(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->max_ast_idx;
}

qdf_export_symbol(wlan_cfg_get_max_ast_idx);

void wlan_cfg_set_tx_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
		int context, int mask)
{
	cfg->int_tx_ring_mask[context] = mask;
}

void wlan_cfg_set_rx_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
			       int context, int mask)
{
	cfg->int_rx_ring_mask[context] = mask;
}

void wlan_cfg_set_rx_mon_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
		int context, int mask)
{
	cfg->int_rx_mon_ring_mask[context] = mask;
}

void wlan_cfg_set_tx_mon_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
				   int context, int mask)
{
	cfg->int_tx_mon_ring_mask[context] = mask;
}

int wlan_cfg_get_host2rxdma_mon_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
					  int context)
{
	return cfg->int_host2rxdma_mon_ring_mask[context];
}

void wlan_cfg_set_host2rxdma_mon_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
					   int context, int mask)
{
	cfg->int_host2rxdma_mon_ring_mask[context] = mask;
}

int wlan_cfg_get_rxdma2host_mon_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
					  int context)
{
	return cfg->int_rxdma2host_mon_ring_mask[context];
}

void wlan_cfg_set_rxdma2host_mon_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
					   int context, int mask)
{
	cfg->int_rxdma2host_mon_ring_mask[context] = mask;
}

void wlan_cfg_set_rxdma2host_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
	int context, int mask)
{
	cfg->int_rxdma2host_ring_mask[context] = mask;
}

int wlan_cfg_get_rxdma2host_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
	int context)
{
	return cfg->int_rxdma2host_ring_mask[context];
}

void wlan_cfg_set_host2rxdma_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
	int context, int mask)
{
	cfg->int_host2rxdma_ring_mask[context] = mask;
}

int wlan_cfg_get_host2rxdma_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
	int context)
{
	return cfg->int_host2rxdma_ring_mask[context];
}

int wlan_cfg_get_rx_near_full_grp_1_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
					 int context)
{
	return cfg->int_rx_ring_near_full_irq_1_mask[context];
}

int wlan_cfg_get_rx_near_full_grp_2_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
					 int context)
{
	return cfg->int_rx_ring_near_full_irq_2_mask[context];
}

int wlan_cfg_get_tx_ring_near_full_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
					int context)
{
	return cfg->int_tx_ring_near_full_irq_mask[context];
}

void wlan_cfg_set_hw_mac_idx(struct wlan_cfg_dp_soc_ctxt *cfg, int pdev_idx,
			     int hw_macid)
{
	qdf_assert_always(pdev_idx < MAX_PDEV_CNT);
	cfg->hw_macid[pdev_idx] = hw_macid;
}

int wlan_cfg_get_hw_mac_idx(struct wlan_cfg_dp_soc_ctxt *cfg, int pdev_idx)
{
	qdf_assert_always(pdev_idx < MAX_PDEV_CNT);
	return cfg->hw_macid[pdev_idx];
}

qdf_export_symbol(wlan_cfg_get_hw_mac_idx);

int wlan_cfg_get_target_pdev_id(struct wlan_cfg_dp_soc_ctxt *cfg,
				int hw_macid)
{
	int idx;

	for (idx = 0; idx < MAX_PDEV_CNT; idx++) {
		if (cfg->hw_macid[idx] == hw_macid)
			return (idx + 1);
	}
	qdf_assert_always(idx < MAX_PDEV_CNT);
	return WLAN_INVALID_PDEV_ID;
}

void wlan_cfg_set_pdev_idx(struct wlan_cfg_dp_soc_ctxt *cfg, int pdev_idx,
			   int hw_macid)
{
	qdf_assert_always((pdev_idx < MAX_PDEV_CNT) ||
			  (pdev_idx == INVALID_PDEV_ID));
	qdf_assert_always(hw_macid < MAX_NUM_LMAC_HW);
	cfg->hw_macid_pdev_id_map[hw_macid] = pdev_idx;
}

int wlan_cfg_get_pdev_idx(struct wlan_cfg_dp_soc_ctxt *cfg, int hw_macid)
{
	qdf_assert_always(hw_macid < MAX_NUM_LMAC_HW);
	return cfg->hw_macid_pdev_id_map[hw_macid];
}

qdf_export_symbol(wlan_cfg_get_pdev_idx);

void wlan_cfg_set_ce_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
		int context, int mask)
{
	cfg->int_ce_ring_mask[context] = mask;
}

void wlan_cfg_set_rxbuf_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg, int context,
		int mask)
{
	cfg->int_rx_ring_mask[context] = mask;
}

int wlan_cfg_set_rx_err_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
		int context, int mask)
{
	return cfg->int_rx_err_ring_mask[context] = mask;
}

int wlan_cfg_set_rx_wbm_rel_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
		int context, int mask)
{
	return cfg->int_rx_wbm_rel_ring_mask[context] = mask;
}

int wlan_cfg_set_reo_status_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
		int context, int mask)
{
	return cfg->int_reo_status_ring_mask[context] = mask;
}

int wlan_cfg_get_num_contexts(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->num_int_ctxts;
}

int wlan_cfg_get_tx_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg, int context)
{
	return cfg->int_tx_ring_mask[context];
}

int wlan_cfg_get_rx_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg, int context)
{
	return cfg->int_rx_ring_mask[context];
}

int wlan_cfg_get_rx_err_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
						int context)
{
	return cfg->int_rx_err_ring_mask[context];
}

int wlan_cfg_get_rx_wbm_rel_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
					int context)
{
	return cfg->int_rx_wbm_rel_ring_mask[context];
}

int wlan_cfg_get_reo_status_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg,
					int context)
{
	return cfg->int_reo_status_ring_mask[context];
}

int wlan_cfg_get_rx_mon_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg, int context)
{
	return cfg->int_rx_mon_ring_mask[context];
}

int wlan_cfg_get_tx_mon_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg, int context)
{
	return cfg->int_tx_mon_ring_mask[context];
}

int wlan_cfg_get_ce_ring_mask(struct wlan_cfg_dp_soc_ctxt *cfg, int context)
{
	return cfg->int_ce_ring_mask[context];
}

uint32_t wlan_cfg_get_max_clients(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->max_clients;
}

uint32_t wlan_cfg_max_alloc_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->max_alloc_size;
}

int wlan_cfg_per_pdev_tx_ring(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->per_pdev_tx_ring;
}

uint32_t
wlan_cfg_rx_pending_hl_threshold(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->rx_pending_high_threshold;
}

uint32_t
wlan_cfg_rx_pending_lo_threshold(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->rx_pending_low_threshold;
}

int wlan_cfg_per_pdev_lmac_ring(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->per_pdev_lmac_ring;
}

qdf_export_symbol(wlan_cfg_per_pdev_lmac_ring);

#if defined(DP_USE_SINGLE_TCL) && !defined(TX_MULTI_TCL)
int wlan_cfg_num_tcl_data_rings(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return 1;
}

int wlan_cfg_num_nss_tcl_data_rings(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return 1;
}

#else

#if defined(IPA_OFFLOAD) && defined(TX_MULTI_TCL)
int wlan_cfg_num_tcl_data_rings(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	if (!cfg->ipa_enabled)
		return cfg->num_tcl_data_rings;

	return 1;
}

int wlan_cfg_num_nss_tcl_data_rings(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	if (!cfg->ipa_enabled)
		return cfg->num_nss_tcl_data_rings;

	return 1;
}
#else
int wlan_cfg_num_tcl_data_rings(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->num_tcl_data_rings;
}

int wlan_cfg_num_nss_tcl_data_rings(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->num_nss_tcl_data_rings;
}
#endif
#endif

int wlan_cfg_tx_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->tx_ring_size;
}

int wlan_cfg_tx_comp_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->tx_comp_ring_size;
}

int wlan_cfg_per_pdev_rx_ring(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->per_pdev_rx_ring;
}

int wlan_cfg_num_reo_dest_rings(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->num_reo_dest_rings;
}

int wlan_cfg_num_nss_reo_dest_rings(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->num_nss_reo_dest_rings;
}

int wlan_cfg_pkt_type(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->htt_packet_type;            /*htt_pkt_type_ethernet*/
}

int wlan_cfg_get_num_tx_desc_pool(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->num_tx_desc_pool;
}

void wlan_cfg_set_num_tx_desc_pool(struct wlan_cfg_dp_soc_ctxt *cfg, int num_pool)
{
	cfg->num_tx_desc_pool = num_pool;
}

int wlan_cfg_get_num_tx_ext_desc_pool(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->num_tx_ext_desc_pool;
}

void wlan_cfg_set_num_tx_ext_desc_pool(struct wlan_cfg_dp_soc_ctxt *cfg, int num_pool)
{
	cfg->num_tx_ext_desc_pool = num_pool;
}

int wlan_cfg_get_reo_dst_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->reo_dst_ring_size;
}

void wlan_cfg_set_reo_dst_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg,
				    int reo_dst_ring_size)
{
	cfg->reo_dst_ring_size = reo_dst_ring_size;
}

void wlan_cfg_set_raw_mode_war(struct wlan_cfg_dp_soc_ctxt *cfg,
			       bool raw_mode_war)
{
	cfg->raw_mode_war = raw_mode_war;
}

bool wlan_cfg_get_raw_mode_war(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->raw_mode_war;
}

int wlan_cfg_get_num_tx_desc(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->num_tx_desc;
}

void wlan_cfg_set_num_tx_desc(struct wlan_cfg_dp_soc_ctxt *cfg, int num_desc)
{
	cfg->num_tx_desc = num_desc;
}

int wlan_cfg_get_min_tx_desc(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->min_tx_desc;
}

int wlan_cfg_get_num_tx_ext_desc(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->num_tx_ext_desc;
}

void wlan_cfg_set_num_tx_ext_desc(struct wlan_cfg_dp_soc_ctxt *cfg, int num_ext_desc)
{
	cfg->num_tx_ext_desc = num_ext_desc;
}

uint32_t wlan_cfg_max_peer_id(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	/* TODO: This should be calculated based on target capabilities */
	return cfg->max_peer_id;
}

int wlan_cfg_get_dma_mon_buf_ring_size(struct wlan_cfg_dp_pdev_ctxt *cfg)
{
	return  cfg->dma_mon_buf_ring_size;
}

qdf_export_symbol(wlan_cfg_get_dma_mon_buf_ring_size);

int wlan_cfg_get_dma_rx_mon_dest_ring_size(struct wlan_cfg_dp_pdev_ctxt *cfg)
{
	return  cfg->dma_rx_mon_dest_ring_size;
}

qdf_export_symbol(wlan_cfg_get_dma_rx_mon_dest_ring_size);

int wlan_cfg_get_dma_tx_mon_dest_ring_size(struct wlan_cfg_dp_pdev_ctxt *cfg)
{
	return  cfg->dma_tx_mon_dest_ring_size;
}

qdf_export_symbol(wlan_cfg_get_dma_tx_mon_dest_ring_size);

int wlan_cfg_get_dma_mon_stat_ring_size(struct wlan_cfg_dp_pdev_ctxt *cfg)
{
	return  cfg->dma_mon_status_ring_size;
}

qdf_export_symbol(wlan_cfg_get_dma_mon_stat_ring_size);

int
wlan_cfg_get_dma_mon_desc_ring_size(struct wlan_cfg_dp_pdev_ctxt *cfg)
{
	return cfg->rxdma_monitor_desc_ring;
}

qdf_export_symbol(wlan_cfg_get_dma_mon_desc_ring_size);

int wlan_cfg_get_rx_dma_buf_ring_size(struct wlan_cfg_dp_pdev_ctxt *cfg)
{
	return  cfg->rx_dma_buf_ring_size;
}

int wlan_cfg_get_num_mac_rings(struct wlan_cfg_dp_pdev_ctxt *cfg)
{
	return  cfg->num_mac_rings;
}

qdf_export_symbol(wlan_cfg_get_num_mac_rings);

bool wlan_cfg_is_gro_enabled(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return  cfg->gro_enabled;
}

bool wlan_cfg_is_lro_enabled(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return  cfg->lro_enabled;
}

bool wlan_cfg_is_ipa_enabled(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return  cfg->ipa_enabled;
}

void wlan_cfg_set_rx_hash(struct wlan_cfg_dp_soc_ctxt *cfg, bool val)
{
	cfg->rx_hash = val;
}

bool wlan_cfg_is_rx_hash_enabled(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return  cfg->rx_hash;
}

int wlan_cfg_get_dp_pdev_nss_enabled(struct wlan_cfg_dp_pdev_ctxt *cfg)
{
	return  cfg->nss_enabled;
}

void wlan_cfg_set_dp_pdev_nss_enabled(struct wlan_cfg_dp_pdev_ctxt *cfg, int nss_enabled)
{
	cfg->nss_enabled = nss_enabled;
}

int wlan_cfg_get_dp_soc_nss_cfg(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return  cfg->nss_cfg;
}

void wlan_cfg_set_dp_soc_nss_cfg(struct wlan_cfg_dp_soc_ctxt *cfg, int nss_cfg)
{
	cfg->nss_cfg = nss_cfg;
	if (cfg->nss_cfg)
		cfg->tx_comp_ring_size = cfg->tx_comp_ring_size_nss;
}

int wlan_cfg_get_int_batch_threshold_tx(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->int_batch_threshold_tx;
}

int wlan_cfg_get_int_timer_threshold_tx(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->int_timer_threshold_tx;
}

int wlan_cfg_get_int_batch_threshold_rx(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->int_batch_threshold_rx;
}

int wlan_cfg_get_int_timer_threshold_rx(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->int_timer_threshold_rx;
}

int wlan_cfg_get_int_batch_threshold_other(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->int_batch_threshold_other;
}

int wlan_cfg_get_int_timer_threshold_other(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->int_timer_threshold_other;
}

int wlan_cfg_get_int_timer_threshold_mon(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->int_timer_threshold_mon;
}

int wlan_cfg_get_p2p_checksum_offload(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->p2p_tcp_udp_checksumoffload;
}

int wlan_cfg_get_nan_checksum_offload(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->nan_tcp_udp_checksumoffload;
}

int wlan_cfg_get_checksum_offload(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->tcp_udp_checksumoffload;
}

int wlan_cfg_get_rx_defrag_min_timeout(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->rx_defrag_min_timeout;
}

int wlan_cfg_get_defrag_timeout_check(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->defrag_timeout_check;
}

int
wlan_cfg_get_dp_soc_wbm_release_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->wbm_release_ring;
}

int
wlan_cfg_get_dp_soc_tcl_cmd_credit_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->tcl_cmd_credit_ring;
}

int
wlan_cfg_get_dp_soc_tcl_status_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->tcl_status_ring;
}

int
wlan_cfg_get_dp_soc_reo_reinject_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->reo_reinject_ring;
}

int
wlan_cfg_get_dp_soc_rx_release_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->rx_release_ring;
}

int
wlan_cfg_get_dp_soc_reo_exception_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->reo_exception_ring;
}

int
wlan_cfg_get_dp_soc_reo_cmd_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->reo_cmd_ring;
}

int
wlan_cfg_get_dp_soc_reo_status_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->reo_status_ring;
}

int
wlan_cfg_get_dp_soc_rxdma_refill_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->rxdma_refill_ring;
}

int
wlan_cfg_get_dp_soc_tx_desc_limit_0(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->tx_desc_limit_0;
}

int
wlan_cfg_get_dp_soc_tx_desc_limit_1(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->tx_desc_limit_1;
}

int
wlan_cfg_get_dp_soc_tx_desc_limit_2(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->tx_desc_limit_2;
}

int
wlan_cfg_get_dp_soc_tx_device_limit(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->tx_device_limit;
}

int
wlan_cfg_get_dp_soc_tx_sw_internode_queue(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->tx_sw_internode_queue;
}

int
wlan_cfg_get_dp_soc_rxdma_err_dst_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->rxdma_err_dst_ring;
}

int
wlan_cfg_get_dp_soc_rx_sw_desc_weight(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->rx_sw_desc_weight;
}

qdf_export_symbol(wlan_cfg_get_dp_soc_rx_sw_desc_weight);

int
wlan_cfg_get_dp_soc_rx_sw_desc_num(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->rx_sw_desc_num;
}

uint32_t
wlan_cfg_get_reo_rings_mapping(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->reo_rings_mapping;
}

bool
wlan_cfg_get_dp_caps(struct wlan_cfg_dp_soc_ctxt *cfg,
		     enum cdp_capabilities dp_caps)
{
	switch (dp_caps) {
	case CDP_CFG_DP_TSO:
		return cfg->tso_enabled;
	case CDP_CFG_DP_LRO:
		return cfg->lro_enabled;
	case CDP_CFG_DP_SG:
		return cfg->sg_enabled;
	case CDP_CFG_DP_GRO:
		return cfg->gro_enabled;
	case CDP_CFG_DP_OL_TX_CSUM:
		return cfg->ol_tx_csum_enabled;
	case CDP_CFG_DP_OL_RX_CSUM:
		return cfg->ol_rx_csum_enabled;
	case CDP_CFG_DP_RAWMODE:
		return cfg->rawmode_enabled;
	case CDP_CFG_DP_PEER_FLOW_CTRL:
		return cfg->peer_flow_ctrl_enabled;
	default:
		return false;
	}
}

void wlan_cfg_set_tso_desc_attach_defer(struct wlan_cfg_dp_soc_ctxt *cfg,
					bool val)
{
	cfg->is_tso_desc_attach_defer = val;
}

bool wlan_cfg_is_tso_desc_attach_defer(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->is_tso_desc_attach_defer;
}

#ifdef QCA_LL_TX_FLOW_CONTROL_V2
/**
 * wlan_cfg_get_tx_flow_stop_queue_th() - Get flow control stop threshold
 * @cfg: config context
 *
 * Return: stop threshold
 */
int wlan_cfg_get_tx_flow_stop_queue_th(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->tx_flow_stop_queue_threshold;
}

/**
 * wlan_cfg_get_tx_flow_start_queue_offset() - Get flow control start offset
 *					for TX to resume
 * @cfg: config context
 *
 * Return: stop threshold
 */
int wlan_cfg_get_tx_flow_start_queue_offset(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->tx_flow_start_queue_offset;
}
#endif /* QCA_LL_TX_FLOW_CONTROL_V2 */

void wlan_cfg_set_rx_flow_tag_enabled(struct wlan_cfg_dp_soc_ctxt *cfg,
				      bool val)
{
	cfg->is_rx_flow_tag_enabled = val;
}

uint8_t *wlan_cfg_rx_fst_get_hash_key(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->rx_toeplitz_hash_key;
}

uint8_t wlan_cfg_rx_fst_get_max_search(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->rx_flow_max_search;
}

bool wlan_cfg_is_rx_flow_tag_enabled(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->is_rx_flow_tag_enabled;
}

qdf_export_symbol(wlan_cfg_is_rx_flow_tag_enabled);

#ifdef WLAN_SUPPORT_RX_FISA
bool wlan_cfg_is_rx_fisa_enabled(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return (bool)(cfg->is_rx_fisa_enabled);
}
#else
bool wlan_cfg_is_rx_fisa_enabled(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return false;
}
#endif

bool wlan_cfg_is_poll_mode_enabled(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return (bool)(cfg->is_poll_mode_enabled);
}

void
wlan_cfg_set_rx_flow_search_table_per_pdev(struct wlan_cfg_dp_soc_ctxt *cfg,
					   bool val)
{
	cfg->is_rx_flow_search_table_per_pdev = val;
}

bool wlan_cfg_is_rx_flow_search_table_per_pdev(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->is_rx_flow_search_table_per_pdev;
}

void wlan_cfg_set_rx_flow_search_table_size(struct wlan_cfg_dp_soc_ctxt *cfg,
					    uint16_t val)
{
	cfg->rx_flow_search_table_size = val;
}

uint16_t
wlan_cfg_get_rx_flow_search_table_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return  cfg->rx_flow_search_table_size;
}

void
wlan_cfg_set_rx_mon_protocol_flow_tag_enabled(struct wlan_cfg_dp_soc_ctxt *cfg,
					      bool val)
{
	cfg->is_rx_mon_protocol_flow_tag_enabled = val;
}

bool
wlan_cfg_is_rx_mon_protocol_flow_tag_enabled(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->is_rx_mon_protocol_flow_tag_enabled;
}

qdf_export_symbol(wlan_cfg_is_rx_mon_protocol_flow_tag_enabled);

void
wlan_cfg_set_tx_per_pkt_vdev_id_check(struct wlan_cfg_dp_soc_ctxt *cfg,
				      bool val)
{
	cfg->tx_per_pkt_vdev_id_check = val;
}

bool
wlan_cfg_is_tx_per_pkt_vdev_id_check_enabled(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->tx_per_pkt_vdev_id_check;
}

void
wlan_cfg_set_peer_ext_stats(struct wlan_cfg_dp_soc_ctxt *cfg,
			    bool val)
{
	cfg->pext_stats_enabled = val;
}

bool
wlan_cfg_is_peer_ext_stats_enabled(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->pext_stats_enabled;
}

bool wlan_cfg_is_fst_in_cmem_enabled(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->fst_in_cmem;
}

#ifdef WLAN_FEATURE_RX_PREALLOC_BUFFER_POOL
bool wlan_cfg_is_rx_buffer_pool_enabled(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->is_rx_buff_pool_enabled;
}

bool wlan_cfg_is_rx_refill_buffer_pool_enabled(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->is_rx_refill_buff_pool_enabled;
}
#else
bool wlan_cfg_is_rx_buffer_pool_enabled(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return false;
}

bool wlan_cfg_is_rx_refill_buffer_pool_enabled(
					struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return false;
}
#endif /* WLAN_FEATURE_RX_PREALLOC_BUFFER_POOL */

#ifdef WLAN_DP_FEATURE_SW_LATENCY_MGR
bool wlan_cfg_is_swlm_enabled(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return (bool)(cfg->is_swlm_enabled);
}
#else
bool wlan_cfg_is_swlm_enabled(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return false;
}
#endif
uint8_t wlan_cfg_radio0_default_reo_get(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->radio0_rx_default_reo;
}

uint8_t wlan_cfg_radio1_default_reo_get(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->radio1_rx_default_reo;
}

uint8_t wlan_cfg_radio2_default_reo_get(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->radio2_rx_default_reo;
}

void wlan_cfg_set_rxdma1_enable(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	cfg->rxdma1_enable = true;
}

void
wlan_cfg_set_delay_mon_replenish(struct wlan_cfg_dp_soc_ctxt *cfg,
				 bool val)
{
	cfg->delay_mon_replenish = val;
}

bool
wlan_cfg_is_delay_mon_replenish(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->delay_mon_replenish;
}

qdf_export_symbol(wlan_cfg_is_delay_mon_replenish);

void wlan_cfg_dp_soc_ctx_dump(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	dp_info("DP CFG SoC ctx: delay_mon_replenish = %d,",
		cfg->delay_mon_replenish);
	dp_info("reo_dst_ring_size = %d, delayed_replenish_entries = %d",
		cfg->reo_dst_ring_size, cfg->delayed_replenish_entries);
}

#ifdef IPA_OFFLOAD
uint32_t wlan_cfg_ipa_tx_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->ipa_tx_ring_size;
}

uint32_t wlan_cfg_ipa_tx_comp_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->ipa_tx_comp_ring_size;
}

#ifdef IPA_WDI3_TX_TWO_PIPES
int wlan_cfg_ipa_tx_alt_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->ipa_tx_alt_ring_size;
}

int wlan_cfg_ipa_tx_alt_comp_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->ipa_tx_alt_comp_ring_size;
}

#else
int wlan_cfg_ipa_tx_alt_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->ipa_tx_ring_size;
}

int wlan_cfg_ipa_tx_alt_comp_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->ipa_tx_comp_ring_size;
}
#endif
#endif

#ifdef WLAN_SUPPORT_PPEDS
bool
wlan_cfg_get_dp_soc_is_ppe_enabled(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->ppe_enable;
}

int
wlan_cfg_get_dp_soc_reo2ppe_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->reo2ppe_ring;
}

int
wlan_cfg_get_dp_soc_ppe2tcl_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->ppe2tcl_ring;
}

int
wlan_cfg_get_dp_soc_ppe_release_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->ppe_release_ring;
}
#endif

void
wlan_cfg_get_prealloc_cfg(struct cdp_ctrl_objmgr_psoc *ctrl_psoc,
			  struct wlan_dp_prealloc_cfg *cfg)
{
	if (!ctrl_psoc || !cfg)
		return;

	cfg->num_tx_ring_entries = cfg_get(ctrl_psoc, CFG_DP_TX_RING_SIZE);
	cfg->num_tx_comp_ring_entries = cfg_get(ctrl_psoc,
						CFG_DP_TX_COMPL_RING_SIZE);
	cfg->num_wbm_rel_ring_entries = cfg_get(ctrl_psoc,
						CFG_DP_WBM_RELEASE_RING);
	cfg->num_rxdma_err_dst_ring_entries = cfg_get(ctrl_psoc,
						     CFG_DP_RXDMA_ERR_DST_RING);
	cfg->num_reo_exception_ring_entries = cfg_get(ctrl_psoc,
						     CFG_DP_REO_EXCEPTION_RING);
	cfg->num_tx_desc = cfg_get(ctrl_psoc, CFG_DP_TX_DESC);
	cfg->num_tx_ext_desc = cfg_get(ctrl_psoc, CFG_DP_TX_EXT_DESC);
}

#ifdef WLAN_FEATURE_PKT_CAPTURE_V2
uint32_t wlan_cfg_get_pkt_capture_mode(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->pkt_capture_mode;
}
#endif

uint32_t
wlan_cfg_get_dp_soc_rx_mon_buf_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->rx_mon_buf_ring_size;
}

qdf_export_symbol(wlan_cfg_get_dp_soc_rx_mon_buf_ring_size);

uint32_t
wlan_cfg_get_dp_soc_tx_mon_buf_ring_size(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->tx_mon_buf_ring_size;
}

qdf_export_symbol(wlan_cfg_get_dp_soc_tx_mon_buf_ring_size);

uint8_t
wlan_cfg_get_rx_rel_ring_id(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->rx_rel_wbm2sw_ring_id;
}

void
wlan_cfg_set_rx_rel_ring_id(struct wlan_cfg_dp_soc_ctxt *cfg,
			    uint8_t wbm2sw_ring_id)
{
	cfg->rx_rel_wbm2sw_ring_id = wbm2sw_ring_id;
}

#if defined(WLAN_FEATURE_11BE_MLO) && defined(WLAN_MLO_MULTI_CHIP)
uint8_t
wlan_cfg_mlo_rx_ring_map_get_by_chip_id(struct wlan_cfg_dp_soc_ctxt *cfg,
					uint8_t chip_id)
{
	return cfg->mlo_chip_rx_ring_map[chip_id];
}

uint8_t
wlan_cfg_mlo_default_rx_ring_get_by_chip_id(struct wlan_cfg_dp_soc_ctxt *cfg,
					    uint8_t chip_id)
{
	return cfg->mlo_chip_default_rx_ring_id[chip_id];
}

uint8_t
wlan_cfg_mlo_lmac_peer_id_msb_get_by_chip_id(struct wlan_cfg_dp_soc_ctxt *cfg,
					     uint8_t chip_id)
{
	return cfg->lmac_peer_id_msb[chip_id];
}
#endif

#ifdef QCA_VDEV_STATS_HW_OFFLOAD_SUPPORT
bool
wlan_cfg_get_vdev_stats_hw_offload_config(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->vdev_stats_hw_offload_config;
}

int wlan_cfg_get_vdev_stats_hw_offload_timer(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return cfg->vdev_stats_hw_offload_timer;
}

void
wlan_cfg_set_vdev_stats_hw_offload_config(struct wlan_cfg_dp_soc_ctxt *cfg,
					  bool val)
{
	cfg->vdev_stats_hw_offload_config = val;
}
#else
bool
wlan_cfg_get_vdev_stats_hw_offload_config(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return false;
}

int wlan_cfg_get_vdev_stats_hw_offload_timer(struct wlan_cfg_dp_soc_ctxt *cfg)
{
	return 0;
}

void
wlan_cfg_set_vdev_stats_hw_offload_config(struct wlan_cfg_dp_soc_ctxt *cfg,
					  bool val)
{}
#endif
