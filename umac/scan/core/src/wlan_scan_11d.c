/*
 * Copyright (c) 2011-2018 The Linux Foundation. All rights reserved.
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
 * DOC: contains scan 11d api and functionality
 */
#include <qdf_status.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_scan_public_structs.h>
#include <wlan_scan_utils_api.h>
#include "wlan_scan_main.h"
#include "wlan_scan_11d.h"
#include "wlan_reg_services_api.h"
#include "wlan_reg_ucfg_api.h"

/**
 * wlan_pdevid_get_cc_db() - private API to get cc db from pdev id
 * @psoc: psoc object
 * @pdev_id: pdev id
 *
 * Return: cc db for the pdev id
 */
static struct scan_country_code_db *
wlan_pdevid_get_cc_db(struct wlan_objmgr_psoc *psoc, uint8_t pdev_id)
{
	struct wlan_scan_obj *scan_obj;

	if (pdev_id > WLAN_UMAC_MAX_PDEVS) {
		scm_err("invalid pdev_id %d", pdev_id);
		return NULL;
	}

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj)
		return NULL;

	return &scan_obj->cc_db[pdev_id];
}

/**
 * wlan_pdev_get_cc_db() - private API to get cc db from pdev
 * @psoc: psoc object
 * @pdev: Pdev object
 *
 * Return: cc db for the pdev
 */
static struct scan_country_code_db *
wlan_pdev_get_cc_db(struct wlan_objmgr_psoc *psoc,
		    struct wlan_objmgr_pdev *pdev)
{
	uint8_t pdev_id;

	if (!pdev) {
		scm_err("pdev is NULL");
		return NULL;
	}
	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);

	return wlan_pdevid_get_cc_db(psoc, pdev_id);
}

/**
 * scm_11d_elected_country_algo_fcc - private api to get cc per fcc algo
 * @cc_db: scan country code db
 *
 * Return: true or false
 */
static bool
scm_11d_elected_country_algo_fcc(struct scan_country_code_db *cc_db)
{
	uint8_t i;
	uint8_t country_idx;
	uint16_t max_votes;
	bool found = false;

	if (!cc_db->num_country_codes) {
		scm_err("No AP with 11d Country code is present in scan list");
		return false;
	}

	max_votes = cc_db->votes[0].votes;
	if (wlan_reg_is_us(cc_db->votes[0].cc)) {
		found = true;
		country_idx = 0;
		goto algo_done;
	} else if (max_votes >= MIN_11D_AP_COUNT) {
		found = true;
		country_idx = 0;
	}

	for (i = 1; i < cc_db->num_country_codes; i++) {
		if (wlan_reg_is_us(cc_db->votes[i].cc)) {
			found = true;
			country_idx = i;
			goto algo_done;
		}

		if ((max_votes < cc_db->votes[i].votes) &&
		    (cc_db->votes[i].votes >= MIN_11D_AP_COUNT)) {
			scm_debug("Votes for Country %c%c : %d",
				  cc_db->votes[i].cc[0],
				  cc_db->votes[i].cc[1],
				  cc_db->votes[i].votes);
			max_votes = cc_db->votes[i].votes;
			country_idx = i;
			found = true;
		}
	}

algo_done:
	if (found) {
		qdf_mem_copy(cc_db->elected_cc,
			     cc_db->votes[country_idx].cc,
			     REG_ALPHA2_LEN + 1);

		scm_debug("Selected Country is %c%c With count %d",
			  cc_db->votes[country_idx].cc[0],
			  cc_db->votes[country_idx].cc[1],
			  cc_db->votes[country_idx].votes);
	}

	return found;
}

/**
 * scm_11d_elected_country_info - private api to get cc
 * @cc_db: scan country code db
 *
 * Return: true or false
 */
static bool
scm_11d_elected_country_info(struct scan_country_code_db *cc_db)
{
	uint8_t i, j = 0;
	uint8_t max_votes;

	if (!cc_db->num_country_codes) {
		scm_err("No AP with 11d Country code is present in scan list");
		return false;
	}

	max_votes = cc_db->votes[0].votes;

	for (i = 1; i < cc_db->num_country_codes; i++) {
		/*
		 * If we have a tie for max votes for 2 different country codes,
		 * pick random.
		 */
		if (max_votes < cc_db->votes[i].votes) {
			scm_debug("Votes for Country %c%c : %d",
				  cc_db->votes[i].cc[0],
				  cc_db->votes[i].cc[1],
				  cc_db->votes[i].votes);

			max_votes = cc_db->votes[i].votes;
			j = i;
		}
	}

	qdf_mem_copy(cc_db->elected_cc, cc_db->votes[j].cc,
		     REG_ALPHA2_LEN + 1);

	scm_debug("Selected Country is %c%c With count %d",
		  cc_db->votes[j].cc[0],
		  cc_db->votes[j].cc[1],
		  cc_db->votes[j].votes);

	return true;
}

/**
 * scm_11d_set_country_code - private api to set cc per 11d learning
 * @pdev: pdev object
 * @elected_cc: elected country code
 * @current_cc: current country code
 *
 * Return: true or false
 */
static bool
scm_11d_set_country_code(struct wlan_objmgr_pdev *pdev,
			 uint8_t *elected_cc, uint8_t *current_cc)
{
	scm_debug("elected country %c%c, current country %c%c",
		  elected_cc[0], elected_cc[1], current_cc[0], current_cc[1]);

	if (!qdf_mem_cmp(elected_cc, current_cc, REG_ALPHA2_LEN + 1))
		return true;

	wlan_reg_set_11d_country(pdev, elected_cc);
	return true;
}

/**
 * scm_11d_reset_cc_db - reset the country code db
 * @cc_db: the pointer of country code db
 *
 * Return: void
 */
static void scm_11d_reset_cc_db(struct scan_country_code_db *cc_db)
{
	qdf_mem_zero(cc_db->votes, sizeof(cc_db->votes));
	qdf_mem_zero(cc_db->elected_cc, sizeof(cc_db->elected_cc));
	cc_db->num_country_codes = 0;
}

QDF_STATUS scm_11d_cc_db_init(struct wlan_objmgr_psoc *psoc)
{
	struct scan_country_code_db *cc_db;
	struct wlan_scan_obj *scan_obj;

	if (!psoc) {
		scm_err("psoc is NULL");
		return QDF_STATUS_E_INVAL;
	}

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj) {
		scm_err("scan_obj is NULL");
		return QDF_STATUS_E_INVAL;
	}

	cc_db = (struct scan_country_code_db *)qdf_mem_malloc_atomic(
		   sizeof(struct scan_country_code_db) * WLAN_UMAC_MAX_PDEVS);
	if (!cc_db) {
		scm_err("alloc country code db error");
		return QDF_STATUS_E_INVAL;
	}

	qdf_mem_zero(cc_db,
		     sizeof(struct scan_country_code_db) *
			    WLAN_UMAC_MAX_PDEVS);

	scan_obj->cc_db = cc_db;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS scm_11d_cc_db_deinit(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_scan_obj *scan_obj;

	if (!psoc) {
		scm_err("psoc is NULL");
		return QDF_STATUS_E_INVAL;
	}

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj) {
		scm_err("scan_obj is NULL");
		return QDF_STATUS_E_INVAL;
	}

	qdf_mem_free(scan_obj->cc_db);
	return QDF_STATUS_SUCCESS;
}

void scm_11d_handle_country_info(struct wlan_objmgr_psoc *psoc,
				 struct wlan_objmgr_pdev *pdev,
				 struct scan_cache_entry *scan_entry)
{
	uint8_t i;
	bool match = false;
	uint8_t num_country_codes;
	struct scan_country_code_db *cc_db;
	struct wlan_country_ie *cc_ie;

	cc_ie = util_scan_entry_country(scan_entry);
	if (!cc_ie)
		return;

	cc_db = wlan_pdev_get_cc_db(psoc, pdev);
	if (!cc_db)
		return;

	/* just to be sure, convert to UPPER case here */
	for (i = 0; i < 3; i++)
		cc_ie->cc[i] = qdf_toupper(cc_ie->cc[i]);

	num_country_codes = cc_db->num_country_codes;
	for (i = 0; i < num_country_codes; i++) {
		match = !qdf_mem_cmp(cc_db->votes[i].cc, cc_ie->cc,
				     REG_ALPHA2_LEN);
		if (match)
			break;
	}

	if (match) {
		cc_db->votes[i].votes++;
		return;
	}

	if (num_country_codes >= SCAN_MAX_NUM_COUNTRY_CODE) {
		scm_debug("country code db already full: %d",
			  num_country_codes);
		return;
	}

	/* add country code to end of the list */
	qdf_mem_copy(cc_db->votes[num_country_codes].cc, cc_ie->cc,
		     REG_ALPHA2_LEN + 1);
	cc_db->votes[num_country_codes].votes = 1;
	cc_db->num_country_codes++;
}

void scm_11d_decide_country_code(struct wlan_objmgr_vdev *vdev)
{
	uint8_t current_cc[REG_ALPHA2_LEN + 1];
	bool found;
	struct scan_country_code_db *cc_db;
	struct wlan_objmgr_pdev *pdev = wlan_vdev_get_pdev(vdev);
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);

	if (!wlan_reg_11d_enabled_on_host(psoc))
		return;

	if (SOURCE_UNKNOWN == ucfg_reg_get_cc_and_src(psoc, current_cc)) {
		scm_err("fail to get current country code");
		return;
	}

	cc_db = wlan_pdev_get_cc_db(psoc, pdev);
	if (!cc_db) {
		scm_err("scan_db is NULL");
		return;
	}

	if (wlan_reg_is_us(current_cc) || wlan_reg_is_world(current_cc))
		found = scm_11d_elected_country_algo_fcc(cc_db);
	else
		found = scm_11d_elected_country_info(cc_db);

	if (found)
		scm_11d_set_country_code(pdev, cc_db->elected_cc,
					 current_cc);
	scm_11d_reset_cc_db(cc_db);
}
