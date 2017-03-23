/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 * DOC : wlan_hdd_he.h
 *
 * WLAN Host Device Driver file for 802.11ax (High Efficiency) support.
 *
 */

#if !defined(WLAN_HDD_HE_H)
#define WLAN_HDD_HE_H

struct hdd_context_s;
struct wma_tgt_cfg;
struct beacon_data_s;
struct sap_Config;

#define HE_CAP_OUI_TYPE "\x00\x13\x74\x01"
#define HE_CAP_OUI_SIZE 4

#ifdef WLAN_FEATURE_11AX
void hdd_update_tgt_he_cap(struct hdd_context_s *hdd_ctx,
			   struct wma_tgt_cfg *cfg);
void wlan_hdd_check_11ax_support(struct beacon_data_s *beacon,
				 struct sap_Config *config);
void hdd_he_print_ini_config(hdd_context_t *hdd_ctx);
int hdd_update_he_cap_in_cfg(hdd_context_t *hdd_ctx);
void hdd_he_set_sme_config(tSmeConfigParams *sme_config,
			   struct hdd_config *config);
#else
static inline void hdd_update_tgt_he_cap(struct hdd_context_s *hdd_ctx,
					 struct wma_tgt_cfg *cfg)
{
}

static inline void wlan_hdd_check_11ax_support(struct beacon_data_s *beacon,
					       struct sap_Config *config)
{
}

static inline void hdd_he_print_ini_config(hdd_context_t *hdd_ctx)
{
}

static inline int hdd_update_he_cap_in_cfg(hdd_context_t *hdd_ctx)
{
	return 0;
}

static inline void hdd_he_set_sme_config(tSmeConfigParams *sme_config,
					 struct hdd_config *config)
{
}
#endif
#endif /* if !defined(WLAN_HDD_HE_H)*/
