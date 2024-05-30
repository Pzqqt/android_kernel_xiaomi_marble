/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
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

#include "qdf_mem.h"
#include "qdf_module.h"
#include "wlan_lmac_if_def.h"
#include "wlan_lmac_if_api.h"
#include "wlan_global_lmac_if_api.h"
#ifdef WLAN_CONV_SPECTRAL_ENABLE
#include <wlan_spectral_utils_api.h>
#endif
#include <target_if_psoc_wake_lock.h>

/* Function pointer to call DA/OL specific tx_ops registration function */
QDF_STATUS (*wlan_global_lmac_if_tx_ops_register[MAX_DEV_TYPE])
				(struct wlan_lmac_if_tx_ops *tx_ops);

/*
 * spectral scan is built as separate .ko for WIN where
 * MCL it is part of wlan.ko so the registration of
.* rx ops to global lmac if layer is different between WIN
 * and MCL
 */
#ifdef WLAN_CONV_SPECTRAL_ENABLE
/**
 * wlan_spectral_register_rx_ops() - Register spectral component RX OPS
 * @rx_ops: lmac if receive ops
 *
 * Return: None
 */
#ifdef SPECTRAL_MODULIZED_ENABLE
/* Function pointer for spectral rx_ops registration function */
void (*wlan_lmac_if_sptrl_rx_ops)(struct wlan_lmac_if_rx_ops *rx_ops);

QDF_STATUS wlan_lmac_if_sptrl_set_rx_ops_register_cb(void (*handler)
				(struct wlan_lmac_if_rx_ops *))
{
	wlan_lmac_if_sptrl_rx_ops = handler;

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(wlan_lmac_if_sptrl_set_rx_ops_register_cb);

static void wlan_spectral_register_rx_ops(struct wlan_lmac_if_rx_ops *rx_ops)
{
	wlan_lmac_if_sptrl_rx_ops(rx_ops);
}
#else
static void wlan_spectral_register_rx_ops(struct wlan_lmac_if_rx_ops *rx_ops)
{
	wlan_lmac_if_sptrl_register_rx_ops(rx_ops);
}
#endif /* SPECTRAL_MODULIZED_ENABLE */
#else
/**
 * wlan_spectral_register_rx_ops() - Dummy api to register spectral RX OPS
 * @rx_ops: lmac if receive ops
 *
 * Return: None
 */
static void wlan_spectral_register_rx_ops(struct wlan_lmac_if_rx_ops *rx_ops)
{
}
#endif /*WLAN_CONV_SPECTRAL_ENABLE*/

#ifdef WLAN_IOT_SIM_SUPPORT
/* Function pointer for iot_sim rx_ops registration function */
void (*wlan_lmac_if_iot_sim_rx_ops)(struct wlan_lmac_if_rx_ops *rx_ops);

QDF_STATUS wlan_lmac_if_iot_sim_set_rx_ops_register_cb(void (*handler)
				(struct wlan_lmac_if_rx_ops *))
{
	wlan_lmac_if_iot_sim_rx_ops = handler;

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(wlan_lmac_if_iot_sim_set_rx_ops_register_cb);

static void wlan_iot_sim_register_rx_ops(struct wlan_lmac_if_rx_ops *rx_ops)
{
	if (wlan_lmac_if_iot_sim_rx_ops)
		wlan_lmac_if_iot_sim_rx_ops(rx_ops);
	else
		qdf_print("\n***** IOT SIM MODULE NOT LOADED *****\n");
}

#else
static void wlan_iot_sim_register_rx_ops(struct wlan_lmac_if_rx_ops *rx_ops)
{
}
#endif

/* Function pointer for son rx_ops registration function */
void (*wlan_lmac_if_son_rx_ops)(struct wlan_lmac_if_rx_ops *rx_ops);

QDF_STATUS wlan_lmac_if_son_set_rx_ops_register_cb(void (*handler)
				(struct wlan_lmac_if_rx_ops *))
{
	wlan_lmac_if_son_rx_ops = handler;

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(wlan_lmac_if_son_set_rx_ops_register_cb);

static void wlan_son_register_rx_ops(struct wlan_lmac_if_rx_ops *rx_ops)
{
	if (wlan_lmac_if_son_rx_ops)
		wlan_lmac_if_son_rx_ops(rx_ops);
	else
		qdf_info("\n***** SON MODULE NOT LOADED *****\n");
}

/**
 * wlan_global_lmac_if_rx_ops_register() - Global lmac_if
 * rx handler register
 * @rx_ops: Pointer to rx_ops structure to be populated
 *
 * Register lmac_if RX callabacks which will be called by DA/OL/WMA/WMI
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS
wlan_global_lmac_if_rx_ops_register(struct wlan_lmac_if_rx_ops *rx_ops)
{
	/*
	 * Component specific public api's to be called to register
	 * respective callbacks
	 * Ex: rx_ops->fp = function;
	 */
	if (!rx_ops) {
		qdf_err("lmac if rx ops pointer is NULL");
		return QDF_STATUS_E_INVAL;
	}
	/* Registeration for UMAC componets */
	wlan_lmac_if_umac_rx_ops_register(rx_ops);

	/* spectral rx_ops registration*/
	wlan_spectral_register_rx_ops(rx_ops);

	/* iot_sim rx_ops registration*/
	wlan_iot_sim_register_rx_ops(rx_ops);

	/* son rx_ops registration*/
	wlan_son_register_rx_ops(rx_ops);

	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_global_lmac_if_open() - global lmac_if open
 * @psoc: psoc context
 *
 * Opens up lmac_if southbound layer. This function calls OL,DA and UMAC
 * modules to register respective tx and rx callbacks.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_global_lmac_if_open(struct wlan_objmgr_psoc *psoc)
{
	WLAN_DEV_TYPE dev_type;

	struct wlan_lmac_if_tx_ops *tx_ops;
	struct wlan_lmac_if_rx_ops *rx_ops;

	if (!psoc) {
		qdf_err("psoc is NULL");
		return QDF_STATUS_E_INVAL;
	}

	tx_ops = qdf_mem_malloc(sizeof(*tx_ops));
	if (!tx_ops) {
		qdf_err("tx_ops is NULL");
		return QDF_STATUS_E_NOMEM;
	}

	rx_ops = qdf_mem_malloc(sizeof(*rx_ops));
	if (!rx_ops) {
		qdf_err("rx_ops is NULL");
		qdf_mem_free(tx_ops);
		return QDF_STATUS_E_NOMEM;
	}

	wlan_psoc_set_lmac_if_txops(psoc, tx_ops);
	wlan_psoc_set_lmac_if_rxops(psoc, rx_ops);

	dev_type = psoc->soc_nif.phy_type;

	if (dev_type == WLAN_DEV_OL) {
		wlan_global_lmac_if_tx_ops_register[dev_type]
					(tx_ops);
	} else {
		/* Control should ideally not reach here */
		qdf_print("Invalid device type");
		qdf_mem_free(tx_ops);
		qdf_mem_free(rx_ops);
		return QDF_STATUS_E_INVAL;
	}

	/* Function call to register rx-ops handlers */
	wlan_global_lmac_if_rx_ops_register(rx_ops);

	target_if_wake_lock_init(psoc);

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(wlan_global_lmac_if_open);

/**
 * wlan_global_lmac_if_close() - Close global lmac_if
 * @psoc: psoc context
 *
 * Deregister lmac_if TX and RX handlers
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_global_lmac_if_close(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_tx_ops *tx_ops;
	struct wlan_lmac_if_rx_ops *rx_ops;

	if (!psoc) {
		qdf_err("psoc is NULL");
		return QDF_STATUS_E_INVAL;
	}

	target_if_wake_lock_deinit(psoc);
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	rx_ops = wlan_psoc_get_lmac_if_rxops(psoc);

	wlan_psoc_set_lmac_if_txops(psoc, NULL);
	wlan_psoc_set_lmac_if_rxops(psoc, NULL);

	qdf_mem_free(tx_ops);
	qdf_mem_free(rx_ops);

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(wlan_global_lmac_if_close);

/**
 * wlan_global_lmac_if_set_txops_registration_cb() - tx
 * registration callback assignment
 * @dev_type: Dev type can be either Direct attach or Offload
 * @handler: handler to be called for LMAC tx ops registration
 *
 * API to assign appropriate tx registration callback handler based on the
 * device type(Offload or Direct attach)
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_global_lmac_if_set_txops_registration_cb(WLAN_DEV_TYPE dev_type,
			QDF_STATUS (*handler)(struct wlan_lmac_if_tx_ops *))
{
	wlan_global_lmac_if_tx_ops_register[dev_type] = handler;

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(wlan_global_lmac_if_set_txops_registration_cb);
