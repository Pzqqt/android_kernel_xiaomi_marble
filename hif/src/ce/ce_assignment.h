/*
 * Copyright (c) 2014-2019 The Linux Foundation. All rights reserved.
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
 * Implementation of the Host-side Host InterFace (HIF) API
 * for a Host/Target interconnect using Copy Engines over PCIe.
 */

#ifndef __HIF_PCI_INTERNAL_H__
#define __HIF_PCI_INTERNAL_H__

#ifndef PEER_CACHEING_HOST_ENABLE
#define PEER_CACHEING_HOST_ENABLE 0
#endif

#define HIF_PCI_DEBUG   ATH_DEBUG_MAKE_MODULE_MASK(0)
#define HIF_PCI_IPA_UC_ASSIGNED_CE  5

#if defined(WLAN_DEBUG) || defined(DEBUG)
static ATH_DEBUG_MASK_DESCRIPTION g_hif_debug_description[] = {
	{HIF_PCI_DEBUG, "hif_pci"},
};

ATH_DEBUG_INSTANTIATE_MODULE_VAR(hif, "hif", "PCIe Host Interface",
				ATH_DEBUG_MASK_DEFAULTS | ATH_DEBUG_INFO,
				ATH_DEBUG_DESCRIPTION_COUNT
					 (g_hif_debug_description),
				 g_hif_debug_description);
#endif

#ifdef CONFIG_ATH_PCIE_ACCESS_DEBUG
/* globals are initialized to 0 by the compiler */;
spinlock_t pcie_access_log_lock;
unsigned int pcie_access_log_seqnum;
struct HIF_ACCESS_LOG pcie_access_log[PCIE_ACCESS_LOG_NUM];
static void hif_target_dump_access_log(void);
#endif

/*
 * Host software's Copy Engine configuration.
 * This table is derived from the CE_PCI TABLE, above.
 */
#ifdef BIG_ENDIAN_HOST
#define CE_ATTR_FLAGS CE_ATTR_BYTE_SWAP_DATA
#else
#define CE_ATTR_FLAGS 0
#endif

/* Maximum number of Copy Engine's supported */
#define CE_HTT_H2T_MSG_SRC_NENTRIES 2048
#define CE_HTT_H2T_MSG_SRC_NENTRIES_AR900B 4096

#define EPPING_CE_FLAGS_POLL \
	(CE_ATTR_DISABLE_INTR|CE_ATTR_ENABLE_POLL|CE_ATTR_FLAGS)

#define CE_ATTR_DIAG_FLAGS \
	(CE_ATTR_FLAGS | CE_ATTR_DIAG)

#ifdef QCN7605_SUPPORT

#define QCN7605_CE_COUNT 9
static struct CE_attr host_ce_config_wlan_qcn7605[] = {
	/* host->target HTC control and raw streams & WMI Control*/
	{ /* CE0 */ CE_ATTR_FLAGS, 0, 16, 2048, 0, NULL,},
	/* target->host HTT + HTC control */
	{ /* CE1 */ CE_ATTR_FLAGS, 0, 0,  2048, 512, NULL,},
	/* target->host WMI */
	{ /* CE2 */ CE_ATTR_FLAGS, 0, 0,  2048, 128, NULL,},
	/* target->host HTT */
	{ /* CE3 */ CE_ATTR_FLAGS, 0, 0, 2048, 512, NULL,},
	/* host->target HTT */
	{ /* CE4 */ (CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR), 0,
		CE_HTT_H2T_MSG_SRC_NENTRIES, 256, 0, NULL,},
#ifdef IPA_OFFLOAD
	/* ipa_uc->target HTC control */
	{ /* CE5 */ (CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR), 0,
		CE_HTT_H2T_MSG_SRC_NENTRIES, 512, 0, NULL,},
#else
	/* unused */
	{ /* CE5 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL,},
#endif
	/* Target autonomous HIF_memcpy */
	{ /* CE6 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL,},
	/* target->host PKTLOG */
	{ /* CE7 */ CE_ATTR_FLAGS, 0, 0, 2048, 512, NULL,},
#ifdef IPA_OFFLOAD
	/* Target to uMC */
	{ /* CE8 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL,},
#else
	/* target->host HTT*/
	{ /* CE8 */ CE_ATTR_FLAGS, 0, 0, 2048, 512, NULL,},
#endif
	/* CE 9, 10, 11 belong to CoreBsp & MHI driver */
};

static struct CE_pipe_config target_ce_config_wlan_qcn7605[] = {
	/* host->target HTC control and raw streams */
	{ /* CE0 */ 0, PIPEDIR_OUT, 4, 2048, CE_ATTR_FLAGS, 0,},
	/* target->host HTT */
	{ /* CE1 */ 1, PIPEDIR_IN,  32, 2048, CE_ATTR_FLAGS, 0,},
	/* target->host WMI  + HTC control */
	{ /* CE2 */ 2, PIPEDIR_IN,  32, 2048, CE_ATTR_FLAGS, 0,},
	/* target->host HTT */
	{ /* CE3 */ 3, PIPEDIR_IN, 32, 2048, CE_ATTR_FLAGS, 0,},
	/* host->target HTT */
	{ /* CE4 */ 4, PIPEDIR_OUT, 128, 256,
		(CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR), 0,},
	/* NB: 50% of src nentries, since tx has 2 frags */
#ifdef IPA_OFFLOAD
	/* ipa_uc->target */
	{ /* CE5 */ 5, PIPEDIR_OUT, 512,   64,
		(CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR), 0,},
#else
	/* unused */
	{ /* CE5 */ 5, PIPEDIR_INOUT_H2H, 0, 0, 0, 0,},
#endif
	/* Reserved for target autonomous HIF_memcpy */
	{ /* CE6 */ 6, PIPEDIR_INOUT, 32, 16384, CE_ATTR_FLAGS, 0,},
	/* Target -> host PKTLOG */
	{ /* CE7 */ 7, PIPEDIR_IN,  32, 2048, CE_ATTR_FLAGS, 0,},
#ifdef IPA_OFFLOAD
	/* CE8 used only by IPA */
	{ /* CE8 */ 8, PIPEDIR_IN, 32, 2048, CE_ATTR_FLAGS, 0,},
#else
	/* target->host HTT*/
	{ /* CE8 */ 8, PIPEDIR_IN, 32, 2048, CE_ATTR_FLAGS, 0,},
#endif
	/* CE 9, 10, 11 belong to CoreBsp & MHI driver */
};
#endif

#ifdef QCA_WIFI_3_0
static struct CE_attr host_ce_config_wlan[] = {
	/* host->target HTC control and raw streams */
	{ /* CE0 */ CE_ATTR_FLAGS, 0, 16, 2048, 0, NULL,},
	/* target->host HTT + HTC control */
	{ /* CE1 */ CE_ATTR_FLAGS, 0, 0,  2048, 512, NULL,},
	/* target->host WMI */
	{ /* CE2 */ CE_ATTR_FLAGS, 0, 0,  2048, 128, NULL,},
	/* host->target WMI */
	{ /* CE3 */ CE_ATTR_FLAGS, 0, 32, 2048, 0, NULL,},
	/* host->target HTT */
	{ /* CE4 */ (CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR), 0,
		CE_HTT_H2T_MSG_SRC_NENTRIES, 256, 0, NULL,},
	/* ipa_uc->target HTC control */
	{ /* CE5 */ (CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR), 0,
		CE_HTT_H2T_MSG_SRC_NENTRIES, 512, 0, NULL,},
	/* Target autonomous HIF_memcpy */
	{ /* CE6 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL,},
	/* ce_diag, the Diagnostic Window */
	{ /* CE7 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL,},
	/* Target to uMC */
	{ /* CE8 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL,},
	/* target->host HTT */
	{ /* CE9 */ CE_ATTR_FLAGS, 0, 0,  2048, 512, NULL,},
	/* target->host HTT */
	{ /* CE10 */ CE_ATTR_FLAGS, 0, 0,  2048, 512, NULL,},
	/* target -> host PKTLOG */
	{ /* CE11 */ CE_ATTR_FLAGS, 0, 0, 2048, 512, NULL,},
};

static struct CE_pipe_config target_ce_config_wlan[] = {
	/* host->target HTC control and raw streams */
	{ /* CE0 */ 0, PIPEDIR_OUT, 32, 2048, CE_ATTR_FLAGS, 0,},
	/* target->host HTT */
	{ /* CE1 */ 1, PIPEDIR_IN,  32, 2048, CE_ATTR_FLAGS, 0,},
	/* target->host WMI  + HTC control */
	{ /* CE2 */ 2, PIPEDIR_IN,  64, 2048, CE_ATTR_FLAGS, 0,},
	/* host->target WMI */
	{ /* CE3 */ 3, PIPEDIR_OUT, 32, 2048, CE_ATTR_FLAGS, 0,},
	/* host->target HTT */
	{ /* CE4 */ 4, PIPEDIR_OUT, 256, 256,
		(CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR), 0,},
	/* NB: 50% of src nentries, since tx has 2 frags */
	/* ipa_uc->target */
	{ /* CE5 */ 5, PIPEDIR_OUT, 1024,   64,
		(CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR), 0,},
	/* Reserved for target autonomous HIF_memcpy */
	{ /* CE6 */ 6, PIPEDIR_INOUT, 32, 16384, CE_ATTR_FLAGS, 0,},
	/* CE7 used only by Host */
	{ /* CE7 */ 7, PIPEDIR_INOUT_H2H, 0, 0,
		(CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR), 0,},
	/* CE8 used only by IPA */
	{ /* CE8 */ 8, PIPEDIR_IN, 32, 2048, CE_ATTR_FLAGS, 0,},
	/* CE9 target->host HTT */
	{ /* CE9 */ 9, PIPEDIR_IN,  32, 2048, CE_ATTR_FLAGS, 0,},
	/* CE10 target->host HTT */
	{ /* CE10 */ 10, PIPEDIR_IN,  32, 2048, CE_ATTR_FLAGS, 0,},
	/* Target -> host PKTLOG */
	{ /* CE11 */ 11, PIPEDIR_IN,  32, 2048, CE_ATTR_FLAGS, 0,},
};

#ifdef WLAN_FEATURE_EPPING
static struct CE_attr host_ce_config_wlan_epping_poll[] = {
	/* host->target HTC control and raw streams */
	{ /* CE0 */ CE_ATTR_FLAGS, 0, 16, 2048, 0, NULL,},
	/* target->host EP-ping */
	{ /* CE1 */ EPPING_CE_FLAGS_POLL, 0, 0, 2048, 128, NULL,},
	/* target->host EP-ping */
	{ /* CE2 */ EPPING_CE_FLAGS_POLL, 0, 0, 2048, 128, NULL,},
	/* host->target EP-ping */
	{ /* CE3 */ CE_ATTR_FLAGS, 0, 128, 2048, 0, NULL,},
	/* host->target EP-ping */
	{ /* CE4 */ CE_ATTR_FLAGS, 0, 128, 2048, 0, NULL,},
	/* EP-ping heartbeat */
	{ /* CE5 */ CE_ATTR_FLAGS, 0, 0,   2048, 128, NULL,},
	/* unused */
	{ /* CE6 */ CE_ATTR_FLAGS, 0, 0,   0, 0, NULL,},
	/* ce_diag, the Diagnostic Window */
	{ /* CE7 */ CE_ATTR_DIAG_FLAGS, 0, 2,   DIAG_TRANSFER_LIMIT, 2, NULL,},
};

#ifdef QCN7605_SUPPORT
static struct CE_attr host_ce_config_wlan_epping_irq[] = {
	/* host->target HTC control and raw streams */
	{ /* CE0 */ CE_ATTR_FLAGS, 0,  16, 2048, 0, NULL,},
	/* target->host EP-ping */
	{ /* CE1 */ CE_ATTR_FLAGS, 0,   0, 2048, 32, NULL,},
	/* target->host EP-ping */
	{ /* CE2 */ CE_ATTR_FLAGS, 0,   0, 2048, 32, NULL,},
	/* host->target EP-ping */
	{ /* CE3 */ CE_ATTR_FLAGS, 0, 32, 2048, 0, NULL,},
	/* host->target EP-ping */
	{ /* CE4 */ CE_ATTR_FLAGS, 0, 32, 2048, 0, NULL,},
	/* EP-ping heartbeat */
	{ /* CE5 */ CE_ATTR_FLAGS, 0,   0, 2048, 16, NULL,},
	/* unused */
	{ /* CE6 */ CE_ATTR_FLAGS, 0,   0, 0, 0, NULL,},
	/* ce_diag, the Diagnostic Window */
	{ /* CE7 */ CE_ATTR_DIAG_FLAGS, 0,   2, DIAG_TRANSFER_LIMIT, 2, NULL,},
};

/*
 * EP-ping firmware's CE configuration
 */
static struct CE_pipe_config target_ce_config_wlan_epping[] = {
	/* host->target HTC control and raw streams */
	{ /* CE0 */ 0, PIPEDIR_OUT,  16, 2048, CE_ATTR_FLAGS, 0,},
	/* target->host EP-ping */
	{ /* CE1 */ 1, PIPEDIR_IN,  32, 2048, CE_ATTR_FLAGS, 0,},
	/* target->host EP-ping */
	{ /* CE2 */ 2, PIPEDIR_IN,  32, 2048, CE_ATTR_FLAGS, 0,},
	/* host->target EP-ping */
	{ /* CE3 */ 3, PIPEDIR_OUT, 32, 2048, CE_ATTR_FLAGS, 0,},
	/* host->target EP-ping */
	{ /* CE4 */ 4, PIPEDIR_OUT, 32, 2048, CE_ATTR_FLAGS, 0,},
	/* EP-ping heartbeat */
	{ /* CE5 */ 5, PIPEDIR_IN,  16, 2048, CE_ATTR_FLAGS, 0,},
	/* unused */
	{ /* CE6 */ 6, PIPEDIR_INOUT, 0, 0, CE_ATTR_FLAGS, 0,},
	/* CE7 used only by Host */
	{ /* CE7 */ 7, PIPEDIR_INOUT_H2H, 0, 0, 0, 0,},
	/* CE8 used only by IPA */
	{ /* CE8 */ 8, PIPEDIR_IN, 4, 2048, CE_ATTR_FLAGS, 0,}
};
#else
static struct CE_attr host_ce_config_wlan_epping_irq[] = {
	/* host->target HTC control and raw streams */
	{ /* CE0 */ CE_ATTR_FLAGS, 0,  16, 2048, 0, NULL,},
	/* target->host EP-ping */
	{ /* CE1 */ CE_ATTR_FLAGS, 0,   0, 2048, 128, NULL,},
	/* target->host EP-ping */
	{ /* CE2 */ CE_ATTR_FLAGS, 0,   0, 2048, 128, NULL,},
	/* host->target EP-ping */
	{ /* CE3 */ CE_ATTR_FLAGS, 0, 128, 2048, 0, NULL,},
	/* host->target EP-ping */
	{ /* CE4 */ CE_ATTR_FLAGS, 0, 128, 2048, 0, NULL,},
	/* EP-ping heartbeat */
	{ /* CE5 */ CE_ATTR_FLAGS, 0,   0, 2048, 128, NULL,},
	/* unused */
	{ /* CE6 */ CE_ATTR_FLAGS, 0,   0, 0, 0, NULL,},
	/* ce_diag, the Diagnostic Window */
	{ /* CE7 */ CE_ATTR_DIAG_FLAGS, 0,   2, DIAG_TRANSFER_LIMIT, 2, NULL,},
};
/*
 * EP-ping firmware's CE configuration
 */
static struct CE_pipe_config target_ce_config_wlan_epping[] = {
	/* host->target HTC control and raw streams */
	{ /* CE0 */ 0, PIPEDIR_OUT,  16, 2048, CE_ATTR_FLAGS, 0,},
	/* target->host EP-ping */
	{ /* CE1 */ 1, PIPEDIR_IN,  128, 2048, CE_ATTR_FLAGS, 0,},
	/* target->host EP-ping */
	{ /* CE2 */ 2, PIPEDIR_IN,  128, 2048, CE_ATTR_FLAGS, 0,},
	/* host->target EP-ping */
	{ /* CE3 */ 3, PIPEDIR_OUT, 128, 2048, CE_ATTR_FLAGS, 0,},
	/* host->target EP-ping */
	{ /* CE4 */ 4, PIPEDIR_OUT, 128, 2048, CE_ATTR_FLAGS, 0,},
	/* EP-ping heartbeat */
	{ /* CE5 */ 5, PIPEDIR_IN,  128, 2048, CE_ATTR_FLAGS, 0,},
	/* unused */
	{ /* CE6 */ 6, PIPEDIR_INOUT, 0, 0, CE_ATTR_FLAGS, 0,},
	/* CE7 used only by Host */
	{ /* CE7 */ 7, PIPEDIR_INOUT_H2H, 0, 0, 0, 0,},
	/* CE8 used only by IPA */
	{ /* CE8 */ 8, PIPEDIR_IN, 32, 2048, CE_ATTR_FLAGS, 0,}
};
#endif
#endif
#else
static struct CE_attr host_ce_config_wlan[] = {
	/* host->target HTC control and raw streams */
	{ /* CE0 */ CE_ATTR_FLAGS, 0, 16,  256, 0, NULL,},
	/* target->host HTT + HTC control */
	{ /* CE1 */ CE_ATTR_FLAGS, 0, 0,  2048, 512, NULL,},
	/* target->host WMI */
	{ /* CE2 */ CE_ATTR_FLAGS, 0, 0,  2048, 32, NULL,},
	/* host->target WMI */
	{ /* CE3 */ CE_ATTR_FLAGS, 0, 32, 2048, 0, NULL,},
	/* host->target HTT */
	{ /* CE4 */ CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR, 0,
		CE_HTT_H2T_MSG_SRC_NENTRIES, 256, 0, NULL,},
	/* ipa_uc->target HTC control */
	{ /* CE5 */ CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR, 0,
		1024, 512, 0, NULL,},
	/* Target autonomous HIF_memcpy */
	{ /* CE6 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL,},
	/* ce_diag, the Diagnostic Window */
	{ /* CE7 */ (CE_ATTR_DIAG_FLAGS | CE_ATTR_DISABLE_INTR),
		0, 2, DIAG_TRANSFER_LIMIT, 2, NULL,},
};

static struct CE_pipe_config target_ce_config_wlan[] = {
	/* host->target HTC control and raw streams */
	{ /* CE0 */ 0, PIPEDIR_OUT, 32,  256, CE_ATTR_FLAGS, 0,},
	/* target->host HTT + HTC control */
	{ /* CE1 */ 1, PIPEDIR_IN, 32,  2048, CE_ATTR_FLAGS, 0,},
	/* target->host WMI */
	{ /* CE2 */ 2, PIPEDIR_IN, 32,  2048, CE_ATTR_FLAGS, 0,},
	/* host->target WMI */
	{ /* CE3 */ 3, PIPEDIR_OUT, 32, 2048, CE_ATTR_FLAGS, 0,},
	/* host->target HTT */
	{ /* CE4 */ 4, PIPEDIR_OUT, 256, 256, CE_ATTR_FLAGS, 0,},
	/* NB: 50% of src nentries, since tx has 2 frags */
	/* ipa_uc->target HTC control */
	{ /* CE5 */ 5, PIPEDIR_OUT, 1024,   64, CE_ATTR_FLAGS, 0,},
	/* Reserved for target autonomous HIF_memcpy */
	{ /* CE6 */ 6, PIPEDIR_INOUT, 32, 4096, CE_ATTR_FLAGS, 0,},
	/* CE7 used only by Host */
	{ /* CE7 */ 7, PIPEDIR_INOUT_H2H, 0, 0, 0, 0,},
	/* CE8 used only by IPA */
	{ /* CE8 */ 8, PIPEDIR_IN, 32, 2048, CE_ATTR_FLAGS, 0,}
};

#ifdef WLAN_FEATURE_EPPING
static struct CE_attr host_ce_config_wlan_epping_poll[] = {
	/* host->target HTC control and raw streams */
	{ /* CE0 */ CE_ATTR_FLAGS, 0, 16, 256, 0, NULL,},
	/* target->host EP-ping */
	{ /* CE1 */ EPPING_CE_FLAGS_POLL, 0, 0, 2048, 128, NULL,},
	/* target->host EP-ping */
	{ /* CE2 */ EPPING_CE_FLAGS_POLL, 0, 0, 2048, 128, NULL,},
	/* host->target EP-ping */
	{ /* CE3 */ CE_ATTR_FLAGS, 0, 128, 2048, 0, NULL,},
	/* host->target EP-ping */
	{ /* CE4 */ CE_ATTR_FLAGS, 0, 128, 2048, 0, NULL,},
	/* EP-ping heartbeat */
	{ /* CE5 */ CE_ATTR_FLAGS, 0, 0,   2048, 128, NULL,},
	/* unused */
	{ /* CE6 */ CE_ATTR_FLAGS, 0, 0,   0, 0, NULL,},
	/* ce_diag, the Diagnostic Window */
	{ /* CE7 */ CE_ATTR_DIAG_FLAGS, 0, 2,   DIAG_TRANSFER_LIMIT, 2, NULL,},
	{ /* CE8 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL,},
	/* The following CEs are not being used yet */
	{ /* CE9 */ CE_ATTR_FLAGS, 0, 0,  0, 0, NULL,},
	{ /* CE10 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL,},
	{ /* CE11 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL,},
};
static struct CE_attr host_ce_config_wlan_epping_irq[] = {
	/* host->target HTC control and raw streams */
	{ /* CE0 */ CE_ATTR_FLAGS, 0, 16, 256, 0, NULL,},
	/* target->host EP-ping */
	{ /* CE1 */ CE_ATTR_FLAGS, 0, 0, 2048, 128, NULL,},
	/* target->host EP-ping */
	{ /* CE2 */ CE_ATTR_FLAGS, 0, 0, 2048, 128, NULL,},
	/* host->target EP-ping */
	{ /* CE3 */ CE_ATTR_FLAGS, 0, 128, 2048, 0, NULL,},
	/* host->target EP-ping */
	{ /* CE4 */ CE_ATTR_FLAGS, 0, 128, 2048, 0, NULL,},
	/* EP-ping heartbeat */
	{ /* CE5 */ CE_ATTR_FLAGS, 0, 0, 2048, 128, NULL,},
	/* unused */
	{ /* CE6 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL,},
	/* ce_diag, the Diagnostic Window */
	{ /* CE7 */ CE_ATTR_DIAG_FLAGS, 0, 2, DIAG_TRANSFER_LIMIT, 2, NULL,},
	{ /* CE8 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL,},
	/* The following CEs are not being used yet */
	{ /* CE9 */ CE_ATTR_FLAGS, 0, 0,  0, 0, NULL,},
	{ /* CE10 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL,},
	{ /* CE11 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL,},
};
/*
 * EP-ping firmware's CE configuration
 */
static struct CE_pipe_config target_ce_config_wlan_epping[] = {
	/* host->target HTC control and raw streams */
	{ /* CE0 */ 0, PIPEDIR_OUT, 16,   256, CE_ATTR_FLAGS, 0,},
	/* target->host EP-ping */
	{ /* CE1 */ 1, PIPEDIR_IN, 128,  2048, CE_ATTR_FLAGS, 0,},
	/* target->host EP-ping */
	{ /* CE2 */ 2, PIPEDIR_IN, 128,  2048, CE_ATTR_FLAGS, 0,},
	/* host->target EP-ping */
	{ /* CE3 */ 3, PIPEDIR_OUT, 128, 2048, CE_ATTR_FLAGS, 0,},
	/* host->target EP-ping */
	{ /* CE4 */ 4, PIPEDIR_OUT, 128, 2048, CE_ATTR_FLAGS, 0,},
	/* EP-ping heartbeat */
	{ /* CE5 */ 5, PIPEDIR_IN, 128,  2048, CE_ATTR_FLAGS, 0,},
	/* unused */
	{ /* CE6 */ 6, PIPEDIR_INOUT, 0, 0, CE_ATTR_FLAGS, 0,},
	/* CE7 used only by Host */
	{ /* CE7 */ 7, PIPEDIR_INOUT_H2H, 0, 0, 0, 0,},
	/* CE8 used only by IPA */
	{ /* CE8 */ 8, PIPEDIR_IN, 32, 2048, CE_ATTR_FLAGS, 0,},
	{ /* CE9 */ 9, PIPEDIR_IN,  0, 0, CE_ATTR_FLAGS, 0,},
	{ /* CE10 */ 10, PIPEDIR_IN,  0, 0, CE_ATTR_FLAGS, 0,},
	{ /* CE11 */ 11, PIPEDIR_IN,  0, 0, CE_ATTR_FLAGS, 0,},
};
#endif
#endif

static struct CE_attr host_ce_config_wlan_ar9888[] = {
	/* host->target HTC control and raw streams */
	{ /* CE0 */ CE_ATTR_FLAGS, 0, 16, 256, 0, NULL, },
	/* target->host BMI + HTC control */
	/* could be moved to share CE3 */
	{ /* CE1 */ CE_ATTR_FLAGS, 0, 0, 512, 512, NULL, },
	/* target->host WMI */
	{ /* CE2 */ CE_ATTR_FLAGS, 0, 0, 2048, 128, NULL, },
	/* host->target WMI */
	{ /* CE3 */ CE_ATTR_FLAGS, 0, 32, 2048, 0, NULL, },
	/* host->target HTT */
	{ /* CE4 */ CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR, 0,
		CE_HTT_H2T_MSG_SRC_NENTRIES_AR900B, 256, 0, NULL, },
#ifdef WLAN_FEATURE_FASTPATH
	/* target->host HTT messages */
	{ /* CE5 */ CE_ATTR_FLAGS, 0, 0, 512, 512, NULL, },
#else   /* WLAN_FEATURE_FASTPATH */
	/* unused */
	{ /* CE5 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL, },
#endif  /* WLAN_FEATURE_FASTPATH */
	/* Target autonomous HIF_memcpy */
	{ /* CE6 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL, },
	/* ce_diag, the Diagnostic Window */
	{ /* CE7 */ CE_ATTR_DIAG_FLAGS, 0, 2, DIAG_TRANSFER_LIMIT, 2, NULL, },
	/* Target autonomous HIF_memcpy */
	{ /* CE8 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL, },
};

static struct CE_attr host_ce_config_wlan_ar900b[] = {
	/* host->target HTC control and raw streams */
	{ /* CE0 */ CE_ATTR_FLAGS, 0, 16, 256, 0, NULL, },
	/* target->host BMI + HTC control */
	/* could be moved to share CE3 */
	{ /* CE1 */ CE_ATTR_FLAGS, 0, 0, 512, 512, NULL, },
	/* target->host WMI */
	{ /* CE2 */ CE_ATTR_FLAGS, 0, 0, 2048, 128, NULL, },
	/* host->target WMI */
	{ /* CE3 */ CE_ATTR_FLAGS, 0, 32, 2048, 0, NULL, },
	/* host->target HTT */
	{ /* CE4 */ CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR, 0,
		CE_HTT_H2T_MSG_SRC_NENTRIES_AR900B, 256, 0, NULL, },
#ifdef WLAN_FEATURE_FASTPATH
	/* target->host HTT messages */
	{ /* CE5 */ CE_ATTR_FLAGS, 0, 0, 512, 512, NULL, },
#else   /* WLAN_FEATURE_FASTPATH */
	/* unused */
	{ /* CE5 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL, },
#endif  /* WLAN_FEATURE_FASTPATH */
	/* Target autonomous HIF_memcpy */
	{ /* CE6 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL, },
	/* ce_diag, the Diagnostic Window */
	{ /* CE7 */ CE_ATTR_DIAG_FLAGS, 0, 2, DIAG_TRANSFER_LIMIT, 2, NULL, },
	/* target->host pktlog */
	{ /* CE8 */ CE_ATTR_FLAGS, 0, 0, 2048, 128, NULL, },
	/* Target autonomous HIF_memcpy */
	{ /* CE9 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL, },
	/* Target autonomous HIF_memcpy */
	{ /* CE10 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL, },
	/* Target autonomous HIF_memcpy */
	{ /* CE11 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL, },
};

static struct CE_attr host_lowdesc_ce_cfg_wlan_ar9888[] = {
	/* host->target HTC control and raw streams */
	{ /* CE0 */ CE_ATTR_FLAGS, 0, 16, 256, 0, NULL, },
	/* could be moved to share CE3 */
#ifdef WLAN_FEATURE_FASTPATH
	/* target->host BMI + HTC control */
	{ /* CE1 */ CE_ATTR_FLAGS, 0, 0, 512, 64, NULL, },
#else
	/* target->host BMI + HTC control */
	{ /* CE1 */ CE_ATTR_FLAGS, 0, 0, 512, 512, NULL, },
#endif
	/* target->host WMI */
	{ /* CE2 */ CE_ATTR_FLAGS, 0, 0, 2048, 128, NULL, },
	/* host->target WMI */
	{ /* CE3 */ CE_ATTR_FLAGS, 0, 32, 2048, 0, NULL, },
	/* host->target HTT */
	{ /* CE4 */ CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR, 0,
		CE_HTT_H2T_MSG_SRC_NENTRIES_AR900B, 256, 0, NULL, },
#ifdef WLAN_FEATURE_FASTPATH
	/* target->host HTT messages */
	{ /* CE5 */ CE_ATTR_FLAGS, 0, 0, 512, 512, NULL, },
#else   /* WLAN_FEATURE_FASTPATH */
	/* unused */
	{ /* CE5 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL, },
#endif  /* WLAN_FEATURE_FASTPATH */
	/* Target autonomous HIF_memcpy */
	{ /* CE6 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL, },
	/* ce_diag, the Diagnostic Window */
	{ /* CE7 */ CE_ATTR_DIAG_FLAGS, 0, 2, DIAG_TRANSFER_LIMIT, 2, NULL, },
	/* Target autonomous HIF_memcpy */
	{ /* CE8 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL, },
};

static struct CE_attr host_lowdesc_ce_cfg_wlan_ar900b[] = {
	/* host->target HTC control and raw streams */
	{ /* CE0 */ CE_ATTR_FLAGS, 0, 16, 256, 0, NULL, },
	/* could be moved to share CE3 */
#ifdef WLAN_FEATURE_FASTPATH
	/* target->host BMI + HTC control */
	{ /* CE1 */ CE_ATTR_FLAGS, 0, 0, 512, 64, NULL, },
#else
	/* target->host BMI + HTC control */
	{ /* CE1 */ CE_ATTR_FLAGS, 0, 0, 512, 512, NULL, },
#endif
	/* target->host WMI */
	{ /* CE2 */ CE_ATTR_FLAGS, 0, 0, 2048, 128, NULL, },
	/* host->target WMI */
	{ /* CE3 */ CE_ATTR_FLAGS, 0, 32, 2048, 0, NULL, },
	/* host->target HTT */
	{ /* CE4 */ CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR, 0,
		CE_HTT_H2T_MSG_SRC_NENTRIES_AR900B, 256, 0, NULL, },
#ifdef WLAN_FEATURE_FASTPATH
	/* target->host HTT messages */
	{ /* CE5 */ CE_ATTR_FLAGS, 0, 0, 512, 512, NULL, },
#else   /* WLAN_FEATURE_FASTPATH */
	/* unused */
	{ /* CE5 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL, },
#endif  /* WLAN_FEATURE_FASTPATH */
	/* Target autonomous HIF_memcpy */
	{ /* CE6 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL, },
	/* ce_diag, the Diagnostic Window */
	{ /* CE7 */ CE_ATTR_DIAG_FLAGS, 0, 2, DIAG_TRANSFER_LIMIT, 2, NULL, },
	/* target->host pktlog */
	{ /* CE8 */ CE_ATTR_FLAGS, 0, 0, 2048, 128, NULL, },
	/* Target autonomous HIF_memcpy */
	{ /* CE9 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL, },
	/* Target autonomous HIF_memcpy */
	{ /* CE10 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL, },
	/* Target autonomous HIF_memcpy */
	{ /* CE11 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL, },
};

static struct CE_attr host_lowdesc_ce_cfg_wlan_ar900b_nopktlog[] = {
	/* host->target HTC control and raw streams */
	{ /* CE0 */ CE_ATTR_FLAGS, 0, 16, 256, 0, NULL, },
	/* could be moved to share CE3 */
#ifdef WLAN_FEATURE_FASTPATH
	/* target->host BMI + HTC control */
	{ /* CE1 */ CE_ATTR_FLAGS, 0, 0, 512, 64, NULL, },
#else
	/* target->host BMI + HTC control */
	{ /* CE1 */ CE_ATTR_FLAGS, 0, 0, 512, 512, NULL, },
#endif
	/* target->host WMI */
	{ /* CE2 */ CE_ATTR_FLAGS, 0, 0, 2048, 128, NULL, },
	/* host->target WMI */
	{ /* CE3 */ CE_ATTR_FLAGS, 0, 32, 2048, 0, NULL, },
	/* host->target HTT */
	{ /* CE4 */ CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR, 0,
		CE_HTT_H2T_MSG_SRC_NENTRIES_AR900B, 256, 0, NULL, },
#ifdef WLAN_FEATURE_FASTPATH
	/* target->host HTT messages */
	{ /* CE5 */ CE_ATTR_FLAGS, 0, 0, 512, 512, NULL, },
#else   /* WLAN_FEATURE_FASTPATH */
	/* unused */
	{ /* CE5 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL, },
#endif  /* WLAN_FEATURE_FASTPATH */
	/* Target autonomous HIF_memcpy */
	{ /* CE6 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL, },
	/* ce_diag, the Diagnostic Window */
	{ /* CE7 */ CE_ATTR_DIAG_FLAGS, 0, 2, DIAG_TRANSFER_LIMIT, 2, NULL, },
	/* target->host pktlog */
	{ /* CE8 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL, },
	/* Target autonomous HIF_memcpy */
	{ /* CE9 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL, },
	/* Target autonomous HIF_memcpy */
	{ /* CE10 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL, },
	/* Target autonomous HIF_memcpy */
	{ /* CE11 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL, },
};

static struct CE_pipe_config target_ce_config_wlan_ar9888[] = {
	/* host->target HTC control and raw streams */
	{ /* CE0 */ 0, PIPEDIR_OUT, 32, 256, CE_ATTR_FLAGS, 0, },
	/* target->host HTC control */
	{ /* CE1 */ 1, PIPEDIR_IN, 32, 512, CE_ATTR_FLAGS, 0, },
	/* target->host WMI */
	{ /* CE2 */ 2, PIPEDIR_IN, 64, 2048, CE_ATTR_FLAGS, 0, },
	/* host->target WMI */
	{ /* CE3 */ 3, PIPEDIR_OUT, 32, 2048, CE_ATTR_FLAGS, 0, },
	/* host->target HTT */
	{ /* CE4 */ 4, PIPEDIR_OUT, 256, 256, CE_ATTR_FLAGS, 0, },
	/* NB: 50% of src nentries, since tx has 2 frags */
#ifdef WLAN_FEATURE_FASTPATH
	/* target->host HTT */
	{ /* CE5 */ 5, PIPEDIR_IN, 32, 512, CE_ATTR_FLAGS, 0, },
#else
	/* unused */
	{ /* CE5 */ 5, PIPEDIR_OUT, 32, 2048, CE_ATTR_FLAGS, 0, },
#endif
	/* Reserved for target autonomous HIF_memcpy */
	{ /* CE6 */ 6, PIPEDIR_INOUT, 32, 4096, CE_ATTR_FLAGS, 0, },
	/* CE7 used only by Host */
};

static struct CE_pipe_config target_ce_config_wlan_ar900b[] = {
	/* host->target HTC control and raw streams */
	{ /* CE0 */ 0, PIPEDIR_OUT, 32, 256, CE_ATTR_FLAGS, 0, },
	/* target->host HTC control */
	{ /* CE1 */ 1, PIPEDIR_IN, 32, 512, CE_ATTR_FLAGS, 0, },
	/* target->host WMI */
	{ /* CE2 */ 2, PIPEDIR_IN, 64, 2048, CE_ATTR_FLAGS, 0, },
	/* host->target WMI */
	{ /* CE3 */ 3, PIPEDIR_OUT, 32, 2048, CE_ATTR_FLAGS, 0, },
	/* host->target HTT */
	{ /* CE4 */ 4, PIPEDIR_OUT, 256, 256, CE_ATTR_FLAGS, 0, },
	/* NB: 50% of src nentries, since tx has 2 frags */
#ifdef WLAN_FEATURE_FASTPATH
	/* target->host HTT */
	{ /* CE5 */ 5, PIPEDIR_IN, 32, 512, CE_ATTR_FLAGS, 0, },
#else
	/* unused */
	{ /* CE5 */ 5, PIPEDIR_OUT, 32, 2048, CE_ATTR_FLAGS, 0, },
#endif
	/* Reserved for target autonomous HIF_memcpy */
	{ /* CE6 */ 6, PIPEDIR_INOUT, 32, 4096, CE_ATTR_FLAGS, 0, },
	/* CE7 used only by Host */
	{ /* CE7 */ 7, PIPEDIR_INOUT, 0, 0, 0, 0, },
	{ /* CE8 */ 8, PIPEDIR_IN, 64, 2048, CE_ATTR_FLAGS
		/* target->host packtlog */
		| CE_ATTR_DISABLE_INTR, 0, },
#if PEER_CACHEING_HOST_ENABLE
	/* target autonomous qcache memcpy */
	{ /* CE9 */ 9, PIPEDIR_INOUT, 32, 2048, CE_ATTR_FLAGS |
		CE_ATTR_DISABLE_INTR, 0, },
#endif
};

static struct CE_attr host_ce_config_wlan_qca8074[] = {
	/* host->target HTC control and raw streams */
	{ /* CE0 */ CE_ATTR_FLAGS, 0, 16, 2048, 0, NULL,},
	/* target->host HTT + HTC control */
	{ /* CE1 */ CE_ATTR_FLAGS, 0, 0,  2048, 512, NULL,},
	/* target->host WMI */
	{ /* CE2 */ CE_ATTR_FLAGS, 0, 0,  2048, 512, NULL,},
	/* host->target WMI (mac0) */
	{ /* CE3 */ CE_ATTR_FLAGS, 0, 32, 2048, 0, NULL,},
	/* host->target HTT */
	{ /* CE4 */ (CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR), 0,
		CE_HTT_H2T_MSG_SRC_NENTRIES, 256, 0, NULL,},
	/* target -> host PKTLOG */
	{ /* CE5 */ CE_ATTR_FLAGS, 0, 0, 2048, 512, NULL,},
	/* Target autonomous HIF_memcpy */
	{ /* CE6 */ CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR, 0, 0,
		0, 0, NULL,},
	/* host->target WMI (mac1) */
	{ /* CE7 */ CE_ATTR_FLAGS, 0, 32, 2048, 0, NULL,},
	/* Target to uMC */
	{ /* CE8 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL,},
	/* host->target WMI (mac2) */
	{ /* CE9 */ CE_ATTR_FLAGS, 0, 32,  2048, 0, NULL,},
	/* target->host HTT */
	{ /* CE10 unused */ 0, 0, 0, 0, 0, NULL,},
	{ /* CE11 unused */ 0, 0, 0, 0, 0, NULL,},
};

#if defined(QCA_LOWMEM_CONFIG) || defined(QCA_512M_CONFIG)
#define T2H_WMI_RING_SIZE 32
#else
#define T2H_WMI_RING_SIZE 512
#endif
static struct CE_pipe_config target_ce_config_wlan_qca8074[] = {
	/* host->target HTC control and raw streams */
	{ /* CE0 */ 0, PIPEDIR_OUT, 32, 2048, CE_ATTR_FLAGS, 0,},
	/* target->host HTT */
	{ /* CE1 */ 1, PIPEDIR_IN,  32, 2048, CE_ATTR_FLAGS, 0,},
	/* target->host WMI  + HTC control */
	{ /* CE2 */ 2, PIPEDIR_IN,  T2H_WMI_RING_SIZE, 2048, CE_ATTR_FLAGS, 0,},
	/* host->target WMI */
	{ /* CE3 */ 3, PIPEDIR_OUT, 32, 2048, CE_ATTR_FLAGS, 0,},
	/* host->target HTT */
	{ /* CE4 */ 4, PIPEDIR_OUT, 256, 256,
		(CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR), 0,},
	/* NB: 50% of src nentries, since tx has 2 frags */
	/* Target -> host PKTLOG */
	{ /* CE5 */ 5, PIPEDIR_IN,  32, 2048, 0, 0,},
	/* Reserved for target autonomous HIF_memcpy */
	{ /* CE6 */ 6, PIPEDIR_INOUT, 32, 65535, 64, 0,},
	/* CE7 used only by Host */
	{ /* CE7 */ 7, PIPEDIR_OUT, 32, 2048,
		8192, 0,},
	/* CE8 used only by IPA */
	{ /* CE8 */ 8, PIPEDIR_INOUT, 32, 65535, 112, 0,},
	/* CE9 target->host HTT */
	{ /* CE9 */ 9, PIPEDIR_OUT,  32, 2048, 8192, 0,},
	/* CE10 target->host HTT */
	{/* CE10 unused */10, PIPEDIR_NONE, 0, 0, 0, 0,},
	{/* CE11 unused */11, PIPEDIR_NONE, 0, 0, 0, 0,},
};

static struct CE_attr host_ce_config_wlan_qca8074_pci[] = {
	/* host->target HTC control and raw streams */
	{ /* CE0 */ EPPING_CE_FLAGS_POLL, 0, 16, 2048, 0, NULL,},
	/* target->host HTT + HTC control */
	{ /* CE1 */ EPPING_CE_FLAGS_POLL, 0, 0,  2048, 512, NULL,},
	/* target->host WMI */
	{ /* CE2 */ EPPING_CE_FLAGS_POLL, 0, 0,  2048, 256, NULL,},
	/* host->target WMI (mac0) */
	{ /* CE3 */ EPPING_CE_FLAGS_POLL, 0, 32, 2048, 0, NULL,},
	/* host->target HTT */
	{ /* CE4 */ (CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR), 0,
		CE_HTT_H2T_MSG_SRC_NENTRIES, 256, 0, NULL,},
	/* target -> host PKTLOG */
	{ /* CE5 */ EPPING_CE_FLAGS_POLL, 0, 0, 2048, 512, NULL,},
	/* Target autonomous HIF_memcpy */
	{ /* CE6 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL,},
	/* host->target WMI (mac1) */
	{ /* CE7 */ EPPING_CE_FLAGS_POLL, 0, 32, 2048, 0, NULL,},
	/* Target to uMC */
	{ /* CE8 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL,},
	/* host->target WMI (mac2) */
	{ /* CE9 */ EPPING_CE_FLAGS_POLL, 0, 32,  2048, 0, NULL,},
	/* target->host HTT */
	{ /* CE10 unused */ 0, 0, 0, 0, 0, NULL,},
	{ /* CE11 unused */ 0, 0, 0, 0, 0, NULL,},
};

static struct CE_pipe_config target_ce_config_wlan_qca8074_pci[] = {
	/* host->target HTC control and raw streams */
	{ /* CE0 */ 0, PIPEDIR_OUT, 32, 2048, CE_ATTR_FLAGS, 0,},
	/* target->host HTT */
	{ /* CE1 */ 1, PIPEDIR_IN,  32, 2048, CE_ATTR_FLAGS, 0,},
	/* target->host WMI  + HTC control */
	{ /* CE2 */ 2, PIPEDIR_IN,  32, 2048, CE_ATTR_FLAGS, 0,},
	/* host->target WMI */
	{ /* CE3 */ 3, PIPEDIR_OUT, 32, 2048, CE_ATTR_FLAGS, 0,},
	/* host->target HTT */
	{ /* CE4 */ 4, PIPEDIR_OUT, 256, 256,
		(CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR), 0,},
	/* NB: 50% of src nentries, since tx has 2 frags */
	/* ipa_uc->target */
	{ /* CE5 */ 5, PIPEDIR_OUT, 1024,   64,
		(CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR), 0,},
	/* Reserved for target autonomous HIF_memcpy */
	{ /* CE6 */ 6, PIPEDIR_INOUT, 32, 16384, CE_ATTR_FLAGS, 0,},
	/* CE7 used only by Host */
	{ /* CE7 */ 7, PIPEDIR_INOUT_H2H, 0, 0,
		(CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR), 0,},
	/* CE8 used only by IPA */
	{ /* CE8 */ 8, PIPEDIR_IN, 32, 2048, CE_ATTR_FLAGS, 0,},
	/* CE9 target->host HTT */
	{ /* CE9 */ 9, PIPEDIR_IN,  32, 2048, CE_ATTR_FLAGS, 0,},
	/* CE10 target->host HTT */
	{ /* CE10 */ 10, PIPEDIR_IN,  32, 2048, CE_ATTR_FLAGS, 0,},
	/* Target -> host PKTLOG */
	{ /* CE11 */ 11, PIPEDIR_IN,  32, 2048, CE_ATTR_FLAGS, 0,},
};

static struct CE_attr host_lowdesc_ce_config_wlan_adrastea_nopktlog[] = {
	/* host->target HTC control and raw streams */
	{ /* CE0 */ CE_ATTR_FLAGS, 0, 16, 2048, 0, NULL,},
	/* target->host HTT + HTC control */
	{ /* CE1 */ CE_ATTR_FLAGS, 0, 0,  2048, 256, NULL,},
	/* target->host WMI */
	{ /* CE2 */ CE_ATTR_FLAGS, 0, 0,  2048, 64, NULL,},
	/* host->target WMI */
	{ /* CE3 */ CE_ATTR_FLAGS, 0, 32, 2048, 0, NULL,},
	/* host->target HTT */
	{ /* CE4 */ (CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR), 0,
		CE_HTT_H2T_MSG_SRC_NENTRIES, 256, 0, NULL,},
	/* ipa_uc->target HTC control */
	{ /* CE5 */ (CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR), 0,
		CE_HTT_H2T_MSG_SRC_NENTRIES, 512, 0, NULL,},
	/* Target autonomous HIF_memcpy */
	{ /* CE6 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL,},
	/* ce_diag, the Diagnostic Window */
	{ /* CE7 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL,},
	/* Target to uMC */
	{ /* CE8 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL,},
	/* target->host HTT */
	{ /* CE9 */ CE_ATTR_FLAGS, 0, 0,  2048, 64, NULL,},
	/* target->host HTT */
	{ /* CE10 */ CE_ATTR_FLAGS, 0, 0,  2048, 64, NULL,},
	/* target -> host PKTLOG */
	{ /* CE11 */ (CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR), 0,
		0, 2048, 0, NULL,},
};

static struct CE_attr host_ce_config_wlan_adrastea[] = {
	/* host->target HTC control and raw streams */
	{ /* CE0 */ CE_ATTR_FLAGS, 0, 16, 2048, 0, NULL,},
	/* target->host HTT + HTC control */
	{ /* CE1 */ CE_ATTR_FLAGS, 0, 0,  2048, 512, NULL,},
	/* target->host WMI */
	{ /* CE2 */ CE_ATTR_FLAGS, 0, 0,  2048, 128, NULL,},
	/* host->target WMI */
	{ /* CE3 */ CE_ATTR_FLAGS, 0, 32, 2048, 0, NULL,},
	/* host->target HTT */
	{ /* CE4 */ (CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR), 0,
		CE_HTT_H2T_MSG_SRC_NENTRIES, 256, 0, NULL,},
	/* ipa_uc->target HTC control */
	{ /* CE5 */ (CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR), 0,
		CE_HTT_H2T_MSG_SRC_NENTRIES, 512, 0, NULL,},
	/* Target autonomous HIF_memcpy */
	{ /* CE6 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL,},
	/* ce_diag, the Diagnostic Window */
	{ /* CE7 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL,},
	/* Target to uMC */
	{ /* CE8 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL,},
	/* target->host HTT */
	{ /* CE9 */ CE_ATTR_FLAGS, 0, 0,  2048, 512, NULL,},
	/* target->host HTT */
	{ /* CE10 */ CE_ATTR_FLAGS, 0, 0,  2048, 512, NULL,},
	/* target -> host PKTLOG */
	{ /* CE11 */ CE_ATTR_FLAGS, 0, 0, 2048, 512, NULL,},
};

static struct CE_pipe_config target_ce_config_wlan_adrastea[] = {
	/* host->target HTC control and raw streams */
	{ /* CE0 */ 0, PIPEDIR_OUT, 32, 2048, CE_ATTR_FLAGS, 0,},
	/* target->host HTT */
	{ /* CE1 */ 1, PIPEDIR_IN,  32, 2048, CE_ATTR_FLAGS, 0,},
	/* target->host WMI  + HTC control */
	{ /* CE2 */ 2, PIPEDIR_IN,  64, 2048, CE_ATTR_FLAGS, 0,},
	/* host->target WMI */
	{ /* CE3 */ 3, PIPEDIR_OUT, 32, 2048, CE_ATTR_FLAGS, 0,},
	/* host->target HTT */
	{ /* CE4 */ 4, PIPEDIR_OUT, 256, 256,
		(CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR), 0,},
	/* NB: 50% of src nentries, since tx has 2 frags */
	/* ipa_uc->target */
	{ /* CE5 */ 5, PIPEDIR_OUT, 1024,   64,
		(CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR), 0,},
	/* Reserved for target autonomous HIF_memcpy */
	{ /* CE6 */ 6, PIPEDIR_INOUT, 32, 16384, CE_ATTR_FLAGS, 0,},
	/* CE7 used only by Host */
	{ /* CE7 */ 7, PIPEDIR_INOUT_H2H, 0, 0,
		(CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR), 0,},
	/* CE8 used only by IPA */
	{ /* CE8 */ 8, PIPEDIR_IN, 32, 2048, CE_ATTR_FLAGS, 0,},
	/* CE9 target->host HTT */
	{ /* CE9 */ 9, PIPEDIR_IN,  32, 2048, CE_ATTR_FLAGS, 0,},
	/* CE10 target->host HTT */
	{ /* CE10 */ 10, PIPEDIR_IN,  32, 2048, CE_ATTR_FLAGS, 0,},
	/* Target -> host PKTLOG */
	{ /* CE11 */ 11, PIPEDIR_IN,  32, 2048, CE_ATTR_FLAGS, 0,},
};

#define QCA_6290_CE_COUNT 9
#ifdef QCA_6290_AP_MODE
static struct CE_attr host_ce_config_wlan_qca6290[] = {
	/* host->target HTC control and raw streams */
	{ /* CE0 */ CE_ATTR_FLAGS, 0, 16, 2048, 0, NULL,},
	/* target->host HTT + HTC control */
	{ /* CE1 */ (CE_ATTR_FLAGS | CE_ATTR_ENABLE_POLL), 0, 0,  2048,
		512, NULL,},
	/* target->host WMI */
	{ /* CE2 */ (CE_ATTR_FLAGS | CE_ATTR_ENABLE_POLL), 0, 0,  2048,
		32, NULL,},
	/* host->target WMI */
	{ /* CE3 */ CE_ATTR_FLAGS, 0, 32, 2048, 0, NULL,},
	/* host->target HTT */
	{ /* CE4 */ (CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR), 0,
		CE_HTT_H2T_MSG_SRC_NENTRIES, 256, 0, NULL,},
	/* target -> host PKTLOG */
	{ /* CE5 */ (CE_ATTR_FLAGS | CE_ATTR_ENABLE_POLL), 0, 0, 2048,
		512, NULL,},
	/* Target autonomous HIF_memcpy */
	{ /* CE6 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL,},
	/* host->target WMI (mac1) */
	{ /* CE7 */ CE_ATTR_FLAGS, 0, 32, 2048, 0, NULL,},
	/* Reserved for target */
	{ /* CE8 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL,},
	/* CE 9, 10, 11 belong to CoreBsp & MHI driver */
};

static struct CE_pipe_config target_ce_config_wlan_qca6290[] = {
	/* host->target HTC control and raw streams */
	{ /* CE0 */ 0, PIPEDIR_OUT, 32, 2048, CE_ATTR_FLAGS, 0,},
	/* target->host HTT */
	{ /* CE1 */ 1, PIPEDIR_IN,  32, 2048, CE_ATTR_FLAGS, 0,},
	/* target->host WMI  + HTC control */
	{ /* CE2 */ 2, PIPEDIR_IN,  32, 2048, CE_ATTR_FLAGS, 0,},
	/* host->target WMI */
	{ /* CE3 */ 3, PIPEDIR_OUT, 32, 2048, CE_ATTR_FLAGS, 0,},
	/* host->target HTT */
	{ /* CE4 */ 4, PIPEDIR_OUT, 256, 256,
		(CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR), 0,},
	/* Target -> host PKTLOG */
	{ /* CE5 */ 5, PIPEDIR_IN,  32, 2048, CE_ATTR_FLAGS, 0,},
	/* Reserved for target autonomous HIF_memcpy */
	{ /* CE6 */ 6, PIPEDIR_INOUT, 32, 16384, CE_ATTR_FLAGS, 0,},
	/* CE7 used only by Host */
	{ /* CE7 */ 7, PIPEDIR_OUT, 32, 2048,
		8192, 0,},
	/* Reserved for target */
	{ /* CE8 */ 8, PIPEDIR_INOUT, 32, 16384, CE_ATTR_FLAGS, 0,},
	/* CE 9, 10, 11 belong to CoreBsp & MHI driver */
};
#else
static struct CE_attr host_ce_config_wlan_qca6290[] = {
	/* host->target HTC control and raw streams */
	{ /* CE0 */ CE_ATTR_FLAGS, 0, 16, 2048, 0, NULL,},
	/* target->host HTT + HTC control */
	{ /* CE1 */ CE_ATTR_FLAGS, 0, 0,  2048, 512, NULL,},
	/* target->host WMI */
	{ /* CE2 */ CE_ATTR_FLAGS, 0, 0,  2048, 32, NULL,},
	/* host->target WMI */
	{ /* CE3 */ CE_ATTR_FLAGS, 0, 32, 2048, 0, NULL,},
	/* host->target HTT */
	{ /* CE4 */ (CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR), 0,
		CE_HTT_H2T_MSG_SRC_NENTRIES, 256, 0, NULL,},
	/* target -> host PKTLOG */
	{ /* CE5 */ CE_ATTR_FLAGS, 0, 0, 2048, 512, NULL,},
	/* Target autonomous HIF_memcpy */
	{ /* CE6 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL,},
	/* ce_diag, the Diagnostic Window */
	{ /* CE7 */ (CE_ATTR_DIAG_FLAGS | CE_ATTR_DISABLE_INTR), 0,
		2, DIAG_TRANSFER_LIMIT, 2, NULL,},
	/* Reserved for target */
	{ /* CE8 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL,},
	/* CE 9, 10, 11 belong to CoreBsp & MHI driver */
};

static struct CE_pipe_config target_ce_config_wlan_qca6290[] = {
	/* host->target HTC control and raw streams */
	{ /* CE0 */ 0, PIPEDIR_OUT, 32, 2048, CE_ATTR_FLAGS, 0,},
	/* target->host HTT */
	{ /* CE1 */ 1, PIPEDIR_IN,  32, 2048, CE_ATTR_FLAGS, 0,},
	/* target->host WMI  + HTC control */
	{ /* CE2 */ 2, PIPEDIR_IN,  32, 2048, CE_ATTR_FLAGS, 0,},
	/* host->target WMI */
	{ /* CE3 */ 3, PIPEDIR_OUT, 32, 2048, CE_ATTR_FLAGS, 0,},
	/* host->target HTT */
	{ /* CE4 */ 4, PIPEDIR_OUT, 256, 256,
		(CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR), 0,},
	/* Target -> host PKTLOG */
	{ /* CE5 */ 5, PIPEDIR_IN,  32, 2048, CE_ATTR_FLAGS, 0,},
	/* Reserved for target autonomous HIF_memcpy */
	{ /* CE6 */ 6, PIPEDIR_INOUT, 32, 16384, CE_ATTR_FLAGS, 0,},
	/* CE7 used only by Host */
	{ /* CE7 */ 7, PIPEDIR_INOUT_H2H, 0, 0,
		(CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR), 0,},
	/* Reserved for target */
	{ /* CE8 */ 8, PIPEDIR_INOUT, 32, 16384, CE_ATTR_FLAGS, 0,},
	/* CE 9, 10, 11 belong to CoreBsp & MHI driver */
};
#endif

#define QCA_6390_CE_COUNT 9
static struct CE_attr host_ce_config_wlan_qca6390[] = {
	/* host->target HTC control and raw streams */
	{ /* CE0 */ CE_ATTR_FLAGS, 0, 16, 2048, 0, NULL,},
	/* target->host HTT + HTC control */
	{ /* CE1 */ CE_ATTR_FLAGS, 0, 0,  2048, 512, NULL,},
	/* target->host WMI */
	{ /* CE2 */ CE_ATTR_FLAGS, 0, 0,  2048, 32, NULL,},
	/* host->target WMI */
	{ /* CE3 */ CE_ATTR_FLAGS, 0, 32, 2048, 0, NULL,},
	/* host->target HTT */
	{ /* CE4 */ (CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR), 0,
		CE_HTT_H2T_MSG_SRC_NENTRIES, 256, 0, NULL,},
	/* target -> host PKTLOG */
	{ /* CE5 */ CE_ATTR_FLAGS, 0, 0, 2048, 512, NULL,},
	/* Target autonomous HIF_memcpy */
	{ /* CE6 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL,},
	/* ce_diag, the Diagnostic Window */
	{ /* CE7 */ (CE_ATTR_DIAG_FLAGS | CE_ATTR_DISABLE_INTR), 0,
		0, DIAG_TRANSFER_LIMIT, 0, NULL,},
	/* Reserved for target */
	{ /* CE8 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL,},
	/* CE 9, 10, 11 belong to CoreBsp & MHI driver */
};

static struct CE_pipe_config target_ce_config_wlan_qca6390[] = {
	/* host->target HTC control and raw streams */
	{ /* CE0 */ 0, PIPEDIR_OUT, 32, 2048, CE_ATTR_FLAGS, 0,},
	/* target->host HTT */
	{ /* CE1 */ 1, PIPEDIR_IN,  32, 2048, CE_ATTR_FLAGS, 0,},
	/* target->host WMI  + HTC control */
	{ /* CE2 */ 2, PIPEDIR_IN,  32, 2048, CE_ATTR_FLAGS, 0,},
	/* host->target WMI */
	{ /* CE3 */ 3, PIPEDIR_OUT, 32, 2048, CE_ATTR_FLAGS, 0,},
	/* host->target HTT */
	{ /* CE4 */ 4, PIPEDIR_OUT, 256, 256,
		(CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR), 0,},
	/* Target -> host PKTLOG */
	{ /* CE5 */ 5, PIPEDIR_IN,  32, 2048, CE_ATTR_FLAGS, 0,},
	/* Reserved for target autonomous HIF_memcpy */
	{ /* CE6 */ 6, PIPEDIR_INOUT, 32, 16384, CE_ATTR_FLAGS, 0,},
	/* CE7 used only by Host */
	{ /* CE7 */ 7, PIPEDIR_INOUT_H2H, 0, 0,
		(CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR), 0,},
	/* Reserved for target */
	{ /* CE8 */ 8, PIPEDIR_INOUT, 32, 16384, CE_ATTR_FLAGS, 0,},
	/* CE 9, 10, 11 belong to CoreBsp & MHI driver */
};
#endif /* __HIF_PCI_INTERNAL_H__ */
