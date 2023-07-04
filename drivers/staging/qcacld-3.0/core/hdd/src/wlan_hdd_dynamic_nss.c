#include <linux/string.h>
#include "wlan_hdd_dynamic_nss.h"
#include "wlan_hdd_ioctl.h"
#include "wlan_hdd_cm_api.h"

#define NSS_SWITCH_TPUT_THRESH_1  200 //200Mbps
#define NSS_SWITCH_TPUT_THRESH_2  65 //65Mbps

#define NSS_SWITCH_RSSI_THRESH_1  -60  // tcp_ul is 443Mbps in sm8250 5g RVR test
#define NSS_SWITCH_RSSI_THRESH_2  -56  // tcp_ul is 220Mbps in sm8250 24g RVR test

#define NSS_SWITCH_2T1_MIN_TIME 5  //5 seconds, nss2->1 slow
#define NSS_SWITCH_1T2_MIN_TIME 2  //2 seconds, nss1->2 fast

enum {
	DNSS_MODE_DYNAMIC,  // dynamic nss mode
	DNSS_MODE_1x1,      // 1x1 mode
	DNSS_MODE_2x2,      // 2x2 mode
	DNSS_MODE_PERIOD    // switch 1x1, 2x2 in period
} dnss_mode_type;

enum {
	DNSS_USER_MODE,
	DNSS_DEBUG_MODE
} dnss_debug_type;

struct dynamic_nss_context *g_dynamic_nss_list = NULL;
//char dynamic_nss_oui_black_list[MAX_OUI_SIZE] = "";
static bool s_config_dynamic_nss = 0;
static bool s_bt_active;
static bool s_bt_connected; //flag indicate if bt connected
static int dynamic_nss_mode = DNSS_MODE_DYNAMIC;
static int dnss_debug_mode = DNSS_USER_MODE;

module_param(dynamic_nss_mode, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
module_param(dnss_debug_mode, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

void wlan_hdd_dynamic_nss_list_node_add(struct dynamic_nss_context *node)
{
	struct dynamic_nss_context *p = g_dynamic_nss_list;

	if (p == NULL) {
		g_dynamic_nss_list = node;
		node->next = NULL;
		return;
	}

	while (p->next != NULL) {
		p = p->next;
	}

	p->next = node;
	node->next = NULL;
}

void wlan_hdd_dynamic_nss_list_node_del(struct dynamic_nss_context *node)
{
	struct dynamic_nss_context *pre, *cur;

	pre = cur = g_dynamic_nss_list;
	while (cur != NULL) {
		if (cur == node) {
			if (pre == cur) {
				//the deleted node is head
				g_dynamic_nss_list = cur->next;
			} else {
				pre->next = cur->next;
			}
			qdf_mem_free(cur);
			break;
		} else {
			pre = cur;
			cur = cur->next;
		}
	}
}

struct dynamic_nss_context *wlan_hdd_get_dynamic_nss(struct net_device *dev)
{
	struct dynamic_nss_context *p = g_dynamic_nss_list;

	if (dev == NULL) {
		hdd_err("net_device is null");
		return NULL;
	}

	while (p != NULL) {
		if (memcmp(p->intf_name, dev->name, IFNAMSIZ) == 0)
			return p;
		else
			p = p->next;
	}

	return NULL;
}

#if 0
static bool wlan_hdd_check_oui_in_black_list(char *oui, char *oui_list)
{
	int i = 0, j = 0, oui_start = 0;
	int oui_len = 0, oui_list_len = 0;

	oui_len = strlen(oui);
	oui_list_len = strlen(oui_list);
	while(i < oui_list_len) {
		oui_start = i;
		j = 0;

		while(j < oui_len) {
			if(oui_list[i] != oui[j]) {
				break;
			} else {
				i++;
				j++;
			}
		}

		if(j == OUI_LEN)
			return true;

		i = oui_start + OUI_LEN;
	}

	return false;
}
#endif

void wlan_hdd_config_enable_dynamic_nss(bool enable)
{
	s_config_dynamic_nss = enable;
}

void wlan_hdd_config_set_bt_active(bool active)
{
	s_bt_active = active;
        wlan_hdd_set_bt_connected_mode(active);
}


void wlan_hdd_set_bt_connected_mode(bool connected)
{
	if(connected)
		s_bt_connected = true;
	else
		s_bt_connected = false;

	hdd_debug("bt_connected = %d", s_bt_connected);
}

static void wlan_hdd_get_nss_switch_tput_and_rssi_th(struct hdd_adapter *adapter, uint32_t *tput_th, int8_t *rssi_th) {
	struct hdd_station_ctx *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	if (WLAN_REG_IS_5GHZ_CH_FREQ(sta_ctx->conn_info.chan_freq)
		&& (sta_ctx->conn_info.dot11mode >= eCSR_CFG_DOT11_MODE_11AC)) {
		//max tput is 600+Mbps
		*tput_th = NSS_SWITCH_TPUT_THRESH_1;
		*rssi_th = NSS_SWITCH_RSSI_THRESH_1;
	} else {
		//2g && 11ac mode, 5g && 11n mode. max tput is 300+Mbps
		*tput_th = NSS_SWITCH_TPUT_THRESH_2;
		*rssi_th = NSS_SWITCH_RSSI_THRESH_2;
	}
}

void wlan_hdd_init_dynamic_nss_ctx(struct hdd_adapter *adapter, struct dynamic_nss_context *dynamic_nss) {
	struct hdd_station_ctx *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	dynamic_nss->last_txrx_bytes = 0;
	dynamic_nss->last_time_in_millisec = 0;
	dynamic_nss->consct_low_tput_cnt = 0;
	dynamic_nss->consct_high_tput_cnt = 0;
	dynamic_nss->consct_low_rssi_cnt = 0;
	dynamic_nss->is_dnss_2x2 = true;

	if ((sta_ctx != NULL) && hdd_cm_is_connected(adapter)) {
		wlan_hdd_get_nss_switch_tput_and_rssi_th(adapter, &dynamic_nss->tput_th, &dynamic_nss->rssi_th);
		hdd_debug("flag = %d, rssi_th = %d, tput_th = %d, intf_name = %s",
			dynamic_nss->is_dnss_2x2, dynamic_nss->rssi_th,
			dynamic_nss->tput_th, adapter->dev->name);
	}
}

int wlan_hdd_set_nss_and_antenna_mode(struct hdd_adapter *adapter, int nss, int mode)
{
	QDF_STATUS status;

	hdd_debug("NSS = %d, antenna mode = %d", nss, mode);

	if ((nss > 2) || (nss <= 0)) {
		hdd_err("Invalid NSS: %d", nss);
		return -EINVAL;
	}

	if ((mode > HDD_ANTENNA_MODE_2X2) || (mode < HDD_ANTENNA_MODE_1X1)) {
		hdd_err("Invalid antenna mode: %d", mode);
		return -EINVAL;
	}

	if (((nss == NSS_1x1_MODE) && (mode == HDD_ANTENNA_MODE_2X2))
		|| ((nss == NSS_2x2_MODE) && (mode == HDD_ANTENNA_MODE_1X1))) {
		hdd_err("nss and antenna mode is not match: nss = %d, mode = %d", nss, mode);
		return -EINVAL;
	}

	//iwpriv wlan0 nss 1/2
	status = hdd_update_nss(adapter, nss, nss);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("cfg set nss failed, value %d status %d", nss, status);
		return qdf_status_to_os_return(status);
	}

	//wpa_cli -i wlan0 driver SET ANTENNAMODE 1/2
	status = hdd_set_antenna_mode(adapter, adapter->hdd_ctx, mode);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("cfg set antenna mode failed, value %d status %d", mode, status);
		return qdf_status_to_os_return(status);
	}

	hdd_exit();

	return 0;
}

void wlan_hdd_start_dynamic_nss(struct hdd_adapter *adapter)
{
	struct hdd_station_ctx *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	hdd_enter_dev(adapter->dev);
	if (sta_ctx == NULL)
		return;

	hdd_debug("enable_dynamic_nss = %d", s_config_dynamic_nss);
	hdd_debug("connection info: dev_moed = %d, chan = %d, dot11mode = %d, ch_width = %d",
		adapter->device_mode, sta_ctx->conn_info.chan_freq,
		sta_ctx->conn_info.dot11mode, sta_ctx->conn_info.ch_width);
		hdd_debug("dynamic_nss_mode= %d", dynamic_nss_mode);

	if (s_config_dynamic_nss && (adapter->device_mode == QDF_STA_MODE)
		&& hdd_cm_is_connected(adapter)
		&& (WLAN_REG_IS_5GHZ_CH_FREQ(sta_ctx->conn_info.chan_freq)
		|| (sta_ctx->conn_info.dot11mode >= eCSR_CFG_DOT11_MODE_11AC)
		|| (sta_ctx->conn_info.ch_width >= CH_WIDTH_40MHZ))
		&& (wlan_hdd_get_dynamic_nss(adapter->dev) == NULL)) {

		if (dynamic_nss_mode == DNSS_MODE_DYNAMIC
			|| dynamic_nss_mode == DNSS_MODE_PERIOD) {
			//normal dynamic nss mode
			struct dynamic_nss_context *dynamic_nss;
			QDF_STATUS status;

			//setp 1: init timer
			dynamic_nss = qdf_mem_malloc(sizeof(struct dynamic_nss_context));
			if (dynamic_nss == NULL) {
				hdd_debug("tput_stats_timer: malloc fail");
				return;
			}

			memcpy(dynamic_nss->intf_name, adapter->dev->name, IFNAMSIZ);
			wlan_hdd_dynamic_nss_list_node_add(dynamic_nss);
			qdf_mc_timer_init(&dynamic_nss->tput_stats_timer,
					QDF_TIMER_TYPE_SW, wlan_hdd_check_if_nss_switch, adapter);
			hdd_debug("tput_stats_timer: init, intf_name = %s", adapter->dev->name);

			//step 2: start timer, interval: 1s
			status = qdf_mc_timer_start(&dynamic_nss->tput_stats_timer, 1000);
			if (QDF_IS_STATUS_ERROR(status)) {
				hdd_err("cannot start tput_stats_timer");
				return;
			}
			hdd_debug("tput_stats_timer: start, intf_name = %s", adapter->dev->name);

			wlan_hdd_init_dynamic_nss_ctx(adapter, dynamic_nss);
		} else if (dynamic_nss_mode == DNSS_MODE_1x1) {
			//force set nss to 1x1
			wlan_hdd_set_nss_and_antenna_mode(adapter,
						NSS_1x1_MODE, HDD_ANTENNA_MODE_1X1);
			hdd_debug("tput_stats_timer: forced nss=1, intf_name = %s",
				adapter->dev->name);
		} else if (dynamic_nss_mode == DNSS_MODE_2x2) {
			//force set nss to 2x2
			wlan_hdd_set_nss_and_antenna_mode(adapter,
						NSS_2x2_MODE, HDD_ANTENNA_MODE_2X2);
			hdd_debug("tput_stats_timer: forced nss=2, intf_name = %s",
			adapter->dev->name);
		}
	}

	hdd_exit();
}

void wlan_hdd_stop_dynamic_nss(struct hdd_adapter *adapter)
{

	if(!adapter || hdd_validate_adapter(adapter)){
        	hdd_debug("wlan_hdd_stop_dynamic_nss: adapter is not valid, return.");
        	return ;
        }
	hdd_enter_dev(adapter->dev);

	if (adapter->device_mode == QDF_STA_MODE) {
		struct dynamic_nss_context *dynamic_nss = wlan_hdd_get_dynamic_nss(adapter->dev);
		if (dynamic_nss != NULL) {
			qdf_mc_timer_stop(&dynamic_nss->tput_stats_timer);
			wlan_hdd_init_dynamic_nss_ctx(adapter, dynamic_nss);
			wlan_hdd_set_nss_and_antenna_mode(adapter, NSS_2x2_MODE, HDD_ANTENNA_MODE_2X2);
			hdd_debug("tput_stats_timer: stop, intf_name = %s", adapter->dev->name);

			qdf_mc_timer_destroy(&dynamic_nss->tput_stats_timer);
			memset(dynamic_nss->intf_name, 0, IFNAMSIZ);
			wlan_hdd_dynamic_nss_list_node_del(dynamic_nss);
			hdd_debug("tput_stats_timer: destory, intf_name = %s", adapter->dev->name);
		}
	}

	hdd_exit();
}

void wlan_hdd_tput_stats_timer_deinit(struct hdd_context *hdd_ctx)
{
	struct hdd_adapter *adapter = NULL;
	struct hdd_adapter *next = NULL;
	QDF_STATUS status = -1;

	hdd_enter();
	status = hdd_get_front_adapter(hdd_ctx, &adapter);
	while (adapter && (status == QDF_STATUS_SUCCESS)) {
		wlan_hdd_stop_dynamic_nss(adapter);
		status = hdd_get_next_adapter(hdd_ctx, adapter, &next);
		adapter = next;
	}

	hdd_exit();
}

//tput format: Mbps
static uint32_t wlan_hdd_get_tput_stats(struct hdd_adapter *adapter, struct dynamic_nss_context *dynamic_nss)
{
	unsigned long cur_txrx_bytes = 0;
	unsigned long cur_time_in_millisec = 0;
	unsigned long tput_stats_in_Mbps = 0;

	cur_txrx_bytes = adapter->stats.tx_bytes;
	cur_txrx_bytes += adapter->stats.rx_bytes;
	cur_time_in_millisec = qdf_mc_timer_get_system_time();

	if ((cur_txrx_bytes > dynamic_nss->last_txrx_bytes)
		&& (cur_time_in_millisec > dynamic_nss->last_time_in_millisec)) {
		tput_stats_in_Mbps = ((cur_txrx_bytes - dynamic_nss->last_txrx_bytes) * 8)/ \
			((cur_time_in_millisec - dynamic_nss->last_time_in_millisec) * 1000); //Bpms to Mbps
	}

	dynamic_nss->last_txrx_bytes = cur_txrx_bytes;
	dynamic_nss->last_time_in_millisec = cur_time_in_millisec;

	if(dnss_debug_mode == DNSS_DEBUG_MODE){
		hdd_debug("tput = %d, intf_name = %s", tput_stats_in_Mbps, adapter->dev->name);
        }
	return tput_stats_in_Mbps;
}

static void wlan_hdd_do_nss_switch_dynamic(struct hdd_adapter *adapter,
			struct dynamic_nss_context *dynamic_nss) {
	uint32_t tput = 0;
	bool concur_mode = false; //coex with bt, or sta, p2p, softAp concurrency
	struct hdd_station_ctx *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);

	hdd_enter_dev(adapter->dev);
	/*
	 * coex senarios:
	 * case 1: sta coex with sap or p2p, then set nss 2; //todo: dbs 1x1
	 * case 2: sta works on 2.4G and coex with bt, when bt is active, set nss 2;
	 * case 3: in dual wifi case, due to dbs 2x2, each mac set nss seperately.
	 */
	if (((policy_mgr_get_connection_count(adapter->hdd_ctx->psoc) > 1) &&
		(policy_mgr_get_concurrency_mode(adapter->hdd_ctx->psoc) != 1))
		|| (s_bt_connected && WLAN_REG_IS_24GHZ_CH_FREQ(sta_ctx->conn_info.chan_freq))) {
		concur_mode = true;
	}

	if (concur_mode) {
		dynamic_nss->consct_low_tput_cnt = 0;
		dynamic_nss->consct_high_tput_cnt = 0;
		dynamic_nss->consct_low_rssi_cnt = 0;

		if (!dynamic_nss->is_dnss_2x2) {
			hdd_debug("tput_stats_timer: nss=2, coex, intf_name = %s",
				adapter->dev->name);
			wlan_hdd_set_nss_and_antenna_mode(adapter,
							NSS_2x2_MODE, HDD_ANTENNA_MODE_2X2);
			dynamic_nss->is_dnss_2x2 = true;
		}
		goto out;
	}

	if (adapter->rssi < dynamic_nss->rssi_th) {
		dynamic_nss->consct_low_tput_cnt = 0;
		dynamic_nss->consct_high_tput_cnt = 0;
		dynamic_nss->consct_low_rssi_cnt++;

		if ((!dynamic_nss->is_dnss_2x2)
			&& (dynamic_nss->consct_low_rssi_cnt == NSS_SWITCH_1T2_MIN_TIME)) {
			hdd_debug("tput_stats_timer: nss=2, low rssi, intf_name = %s",
				adapter->dev->name);
			wlan_hdd_set_nss_and_antenna_mode(adapter, NSS_2x2_MODE,
							HDD_ANTENNA_MODE_2X2);

			dynamic_nss->is_dnss_2x2 = true;
			dynamic_nss->consct_low_rssi_cnt = 0;
		}
		goto out;
	}

	dynamic_nss->consct_low_rssi_cnt = 0;
	tput = wlan_hdd_get_tput_stats(adapter, dynamic_nss);

	if(dnss_debug_mode == DNSS_DEBUG_MODE){
        	hdd_debug("concur = %d, rssi = %d, rssi_th = %d, tput = %d",
                  concur_mode, adapter->rssi, dynamic_nss->rssi_th, tput);
        }

	// check tput
	if (tput >= dynamic_nss->tput_th) {
		dynamic_nss->consct_low_tput_cnt = 0;

		if (!dynamic_nss->is_dnss_2x2) {
			dynamic_nss->consct_high_tput_cnt++;
			if (dynamic_nss->consct_high_tput_cnt == NSS_SWITCH_1T2_MIN_TIME) {
				//set 2x2 mode
				hdd_debug("tput_stats_timer: nss=2, high tput, intf_name = %s",
					adapter->dev->name);
				wlan_hdd_set_nss_and_antenna_mode(adapter,
								NSS_2x2_MODE, HDD_ANTENNA_MODE_2X2);

				dynamic_nss->is_dnss_2x2 = true;
				dynamic_nss->consct_high_tput_cnt = 0;
			}
		}
	} else {
		dynamic_nss->consct_high_tput_cnt = 0;

		if (dynamic_nss->is_dnss_2x2) {
			dynamic_nss->consct_low_tput_cnt++;

			if (dynamic_nss->consct_low_tput_cnt == NSS_SWITCH_2T1_MIN_TIME) {
				//set 1x1 mode
				hdd_debug("tput_stats_timer: nss=1, low tput, intf_name = %s",
					adapter->dev->name);
				wlan_hdd_set_nss_and_antenna_mode(adapter,
								NSS_1x1_MODE, HDD_ANTENNA_MODE_1X1);

				dynamic_nss->is_dnss_2x2 = false;
				dynamic_nss->consct_low_tput_cnt = 0;
			}
		}
	}

out:
	hdd_exit();
}

static void wlan_hdd_do_nss_switch_period(struct hdd_adapter *adapter,
				struct dynamic_nss_context *dynamic_nss) {
		hdd_enter_dev(adapter->dev);

	if (dynamic_nss->is_dnss_2x2) {
		dynamic_nss->consct_low_tput_cnt++;
		if (dynamic_nss->consct_low_tput_cnt >= NSS_SWITCH_2T1_MIN_TIME) {
			//set 1x1 mode
			hdd_debug("tput_stats_timer: force nss change nss=1, intf_name = %s",
				adapter->dev->name);
			wlan_hdd_set_nss_and_antenna_mode(adapter,
							NSS_1x1_MODE, HDD_ANTENNA_MODE_1X1);
			dynamic_nss->is_dnss_2x2 = false;
			dynamic_nss->consct_low_tput_cnt = 0;
                }
	} else {
		dynamic_nss->consct_high_tput_cnt++;
		if (dynamic_nss->consct_high_tput_cnt >= NSS_SWITCH_1T2_MIN_TIME) {
			//set 2x2 mode
			hdd_debug("tput_stats_timer: force nss change nss=2, intf_name = %s",
				adapter->dev->name);
			wlan_hdd_set_nss_and_antenna_mode(adapter,
							NSS_2x2_MODE, HDD_ANTENNA_MODE_2X2);
			dynamic_nss->is_dnss_2x2 = true;
			dynamic_nss->consct_high_tput_cnt = 0;
		}
	}

	hdd_exit();
}

void wlan_hdd_check_if_nss_switch(void *data)
{
	struct hdd_adapter * adapter = (struct hdd_adapter *)data;
	struct dynamic_nss_context *dynamic_nss = wlan_hdd_get_dynamic_nss(adapter->dev);

	hdd_enter_dev(adapter->dev);
	//hdd_debug("tput_stats_timer: timeout, intf_name = %s", adapter->dev->name);

	if (dynamic_nss == NULL)
		return;

	if (adapter->device_mode == QDF_STA_MODE) {
		QDF_STATUS status;

		status = qdf_mc_timer_start(&dynamic_nss->tput_stats_timer, 1000);
		if (QDF_IS_STATUS_ERROR(status)) {
			hdd_err("cannot re_start tput_stats_timer");
			return;
		}
	}
	if (dynamic_nss_mode == DNSS_MODE_DYNAMIC)
		wlan_hdd_do_nss_switch_dynamic(adapter, dynamic_nss);
	else if (dynamic_nss_mode == DNSS_MODE_PERIOD)
		wlan_hdd_do_nss_switch_period(adapter, dynamic_nss);

	hdd_exit();
}
