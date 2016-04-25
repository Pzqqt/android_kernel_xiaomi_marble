/*
 * Copyright (c) 2014 The Linux Foundation. All rights reserved.
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

/*
 * Implementation of the Host-side Host InterFace (HIF) API
 * for a Host/Target interconnect using Copy Engines over PCIe.
 */

#ifndef __HIF_PCI_INTERNAL_H__
#define __HIF_PCI_INTERNAL_H__

#define HIF_PCI_DEBUG   ATH_DEBUG_MAKE_MODULE_MASK(0)
#define HIF_PCI_IPA_UC_ASSIGNED_CE  5

#if defined(DEBUG)
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
HIF_ACCESS_LOG pcie_access_log[PCIE_ACCESS_LOG_NUM];
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

#define DIAG_CE_ID           7
#define EPPING_CE_FLAGS_POLL \
	(CE_ATTR_DISABLE_INTR|CE_ATTR_ENABLE_POLL|CE_ATTR_FLAGS)
#ifdef QCA_WIFI_3_0
static struct CE_attr host_ce_config_wlan[] = {
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
	/* ipa_uc->target HTC control */
	{ /* CE5 */ (CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR), 0,
		1024, 512, 0, NULL,},
	/* Target autonomous HIF_memcpy */
	{ /* CE6 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL,},
	/* ce_diag, the Diagnostic Window */
	{ /* CE7 */ (CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR), 0,
		2, DIAG_TRANSFER_LIMIT, 2, NULL,},
	/* Target to uMC */
	{ /* CE8 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL,},
	/* target->host HTT */
	{ /* CE9 */ CE_ATTR_FLAGS, 0, 0,  2048, 512, NULL,},
	/* target->host HTT */
	{ /* CE10 */ CE_ATTR_FLAGS, 0, 0,  2048, 512, NULL,},
	/*The following CEs are not being used yet */
	{ /* CE11 */ CE_ATTR_FLAGS, 0, 0, 0, 0, NULL,},
};

static struct CE_pipe_config target_ce_config_wlan[] = {
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
	/*The following CEs are not being used yet*/
	{ /* CE11 */ 11, PIPEDIR_IN,  0, 0, CE_ATTR_FLAGS, 0,},
};

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
	{ /* CE7 */ CE_ATTR_FLAGS, 0, 2,   DIAG_TRANSFER_LIMIT, 2, NULL,},
};

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
	{ /* CE7 */ CE_ATTR_FLAGS, 0,   2, DIAG_TRANSFER_LIMIT, 2, NULL,},
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
	{ /* CE7 */ CE_ATTR_FLAGS | CE_ATTR_DISABLE_INTR,
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
	{ /* CE7 */ CE_ATTR_FLAGS, 0, 2,   DIAG_TRANSFER_LIMIT, 2, NULL,},
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
	{ /* CE7 */ CE_ATTR_FLAGS, 0, 2, DIAG_TRANSFER_LIMIT, 2, NULL,},
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

static struct CE_attr *host_ce_config = host_ce_config_wlan;
static struct CE_pipe_config *target_ce_config = target_ce_config_wlan;
static int target_ce_config_sz = sizeof(target_ce_config_wlan);
#endif /* __HIF_PCI_INTERNAL_H__ */
