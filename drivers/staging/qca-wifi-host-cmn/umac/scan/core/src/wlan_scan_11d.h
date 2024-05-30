/*
 * Copyright (c) 2011-2018, 2020 The Linux Foundation. All rights reserved.
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
 * DOC: contains scan 11d entry api
 */

#ifndef _WLAN_SCAN_11D_H_
#define _WLAN_SCAN_11D_H_

#define SCAN_MAX_NUM_COUNTRY_CODE 100
#define MIN_11D_AP_COUNT 3

/**
 * struct scan_country_code_votes - votes to country code mapping structure
 * @votes: votes
 * @cc: country code
 */
struct scan_country_code_votes {
	uint16_t votes;
	uint8_t cc[REG_ALPHA2_LEN + 1];
};

/**
 * struct scan_country_code_db - country code data base definition
 * @elected_cc: elected country code
 * @num_country_codes: number of country codes encountered
 * @votes: votes to country code mapping array
 */
struct scan_country_code_db {
	uint8_t elected_cc[REG_ALPHA2_LEN + 1];
	uint8_t num_country_codes;
	struct scan_country_code_votes votes[SCAN_MAX_NUM_COUNTRY_CODE];
};

/**
 * scm_11d_cc_db_init() - API to init 11d country code db
 * @psoc: psoc object
 *
 * Initialize the country code database.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS scm_11d_cc_db_init(struct wlan_objmgr_psoc *psoc);

/**
 * scm_11d_cc_db_deinit() - API to deinit 11d country code db
 * @psoc: psoc object
 *
 * free the country code database.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS scm_11d_cc_db_deinit(struct wlan_objmgr_psoc *psoc);

/**
 * scm_11d_decide_country_code() - API to decide the country code per 11d
 * @vdev: vdev object
 *
 * Decide which country will be elected from the country database. If one
 * cadidate country is found, then it set the country code.
 *
 * Return: void
 */
void scm_11d_decide_country_code(struct wlan_objmgr_vdev *vdev);
#endif
