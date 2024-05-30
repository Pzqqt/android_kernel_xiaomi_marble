load("//build/bazel_common_rules/dist:dist.bzl", "copy_to_dist_dir")
load("//build/kernel/kleaf:kernel.bzl", "ddk_module")
load("//msm-kernel:target_variants.bzl", "get_all_variants")

_target_chipset_map = {
	"blair":[
		"wlan",
	],
	"monaco":[
		"wlan",
	],
	"pitti":[
		"adrastea",
	],
	"pitti_32go":[
                "adrastea",
        ],
}

_chipset_hw_map = {
	"wlan"   : "ADRASTEA",
	"adrastea" : "ADRASTEA"
}

_chipset_header_map = {
	"wlan" : [
    ],
	"adrastea" : [
    ],
}

_hw_header_map = {
	"ADRASTEA" : [
	],
}

_fixed_includes = [
    "configs/default_config.h",
    "configs/config_to_feature.h",
]

_fixed_ipaths = [
	"api/fw",
	"api/hw/kiwi/v1",
	"cmn/hal/wifi3.0",
	"components/cfg",
	"os_if/coap/inc",
	"os_if/cp_stats/inc",
	"os_if/sync/src",
	"os_if/sync/inc",
	"os_if/tdls/inc",
	"os_if/fw_offload/inc",
	"os_if/coex/inc",
	"os_if/twt/inc",
	"os_if/son/inc",
	"os_if/nan/inc",
	"os_if/pkt_capture/inc",
	"os_if/p2p/inc",
	"os_if/interop_issues_ap/inc",
	"uapi/linux",
	"core/dp/htt",
	"core/dp/ol/inc",
	"core/dp/txrx",
	"core/hdd/src",
	"core/hdd/inc",
	"core/sap/src",
	"core/sap/inc",
	"core/sme/inc",
	"core/bmi/src",
	"core/bmi/inc",
	"core/cds/src",
	"core/cds/inc",
	"core/mac/inc",
	"core/wma/src",
	"core/wma/inc",
	"core/pld/src",
	"core/pld/inc",
	"components/pmo/dispatcher/inc",
	"components/pmo/core/inc",
	"components/disa/dispatcher/inc",
	"components/disa/core/inc",
	"components/coap/dispatcher/inc",
	"components/coap/core/inc",
	"components/cp_stats/dispatcher/inc",
	"components/cmn_services/logging/inc",
	"components/cmn_services/policy_mgr/src",
	"components/cmn_services/policy_mgr/inc",
	"components/cmn_services/interface_mgr/inc",
	"components/tdls/dispatcher/inc",
	"components/tdls/core/src",
	"components/wmi/inc",
	"components/fw_offload/dispatcher/inc",
	"components/fw_offload/core/inc",
	"components/coex/dispatcher/inc",
	"components/coex/core/inc",
	"components/target_if/pmo/inc",
	"components/target_if/disa/inc",
	"components/target_if/coap/inc",
	"components/target_if/tdls/inc",
	"components/target_if/fw_offload/inc",
	"components/target_if/coex/inc",
	"components/target_if/action_oui/inc",
	"components/target_if/nan/inc",
	"components/target_if/ocb/inc",
	"components/target_if/pkt_capture/inc",
	"components/target_if/connection_mgr/inc",
	"components/target_if/p2p/inc",
	"components/target_if/ftm_time_sync/inc",
	"components/target_if/interop_issues_ap/inc",
	"components/target_if/wfa_config/inc",
	"components/cfg/cfg_all.h",
	"components/action_oui/dispatcher/inc",
	"components/action_oui/core/inc",
	"components/umac/twt/dispatcher/inc",
	"components/umac/twt/core/src",
	"components/umac/mlme/mlo_mgr/inc",
	"components/umac/mlme/connection_mgr/utf/inc",
	"components/umac/mlme/connection_mgr/dispatcher/inc",
	"components/umac/mlme/connection_mgr/core/src",
	"components/umac/mlme/wfa_config/dispatcher/inc",
	"components/son/dispatcher/inc",
	"components/dsc/src",
	"components/dsc/test",
	"components/dsc/inc",
	"components/nan/dispatcher/inc",
	"components/nan/core/src",
	"components/nan/core/inc",
	"components/ocb/dispatcher/inc",
	"components/ocb/core/inc",
	"components/pkt_capture/dispatcher/inc",
	"components/pkt_capture/core/inc",
	"components/p2p/dispatcher/inc",
	"components/p2p/core/src",
	"components/ftm_time_sync/dispatcher/inc",
	"components/ftm_time_sync/core/inc",
	"components/interop_issues_ap/dispatcher/inc",
	"components/interop_issues_ap/core/inc",
	"components/mlme/dispatcher/inc",
	"components/mlme/core/inc",
	"cmn/utils/nlink/inc",
	"cmn/utils/host_diag_log/src",
	"cmn/utils/host_diag_log/inc",
	"cmn/utils/qld/inc",
	"cmn/utils/pktlog/include/pktlog_ac.h",
	"cmn/utils/pktlog/include/pktlog_ac_i.h",
	"cmn/utils/pktlog/include/pktlog.h",
	"cmn/utils/pktlog/include/pktlog_wifi2.h",
	"cmn/utils/pktlog/include/pktlog_wifi3.h",
	"cmn/utils/pktlog/include/pktlog_ac_api.h",
	"cmn/utils/sys/queue.h",
	"cmn/utils/logging/inc",
	"cmn/utils/fwlog/dbglog_host.h",
	"cmn/utils/fwlog/inc",
	"cmn/utils/ptt/inc",
	"cmn/utils/epping/inc",
	"cmn/utils/sys",
	"cmn/qal/inc",
	"cmn/qal/linux/src",
	"cmn/dp/wifi3.0",
	"cmn/dp/inc",
	"cmn/dp/cmn_dp_api",
	"cmn/ftm/dispatcher/inc",
	"cmn/ftm/core/src",
	"cmn/gpio/dispatcher/inc",
	"cmn/gpio/core/inc",
	"cmn/htc",
	"cmn/qdf/test",
	"cmn/qdf/inc",
	"cmn/qdf/linux/src",
	"cmn/hal/wifi3.0",
	"cmn/wmi/inc",
	"cmn/target_if/mlo_mgr/inc",
	"cmn/target_if/dp/inc",
	"cmn/target_if/green_ap/inc",
	"cmn/target_if/scan/inc",
	"cmn/target_if/cp_stats/inc",
	"cmn/target_if/ftm/inc",
	"cmn/target_if/gpio",
	"cmn/target_if/wifi_pos/inc",
	"cmn/target_if/twt/inc",
	"cmn/target_if/dispatcher/inc",
	"cmn/target_if/son/inc",
	"cmn/target_if/core/inc",
	"cmn/target_if/regulatory/inc",
	"cmn/target_if/dcs/inc",
	"cmn/target_if/cfr/inc",
	"cmn/target_if/spectral",
	"cmn/target_if/mgmt_txrx/inc",
	"cmn/target_if/iot_sim",
	"cmn/target_if/init_deinit/inc",
	"cmn/target_if/dfs/inc",
	"cmn/target_if/direct_buf_rx/src",
	"cmn/target_if/direct_buf_rx/inc",
	"cmn/target_if/mlme/psoc/inc",
	"cmn/target_if/mlme/vdev_mgr/inc",
	"cmn/target_if/crypto/inc",
	"cmn/os_if/linux",
	"cmn/cfg/inc",
	"cmn/umac/mlo_mgr/inc",
	"cmn/umac/green_ap/dispatcher/inc",
	"cmn/umac/green_ap/core/src",
	"cmn/umac/scan/dispatcher/inc",
	"cmn/umac/scan/core/src",
	"cmn/umac/cp_stats/dispatcher/inc",
	"cmn/umac/cp_stats/core/src",
	"cmn/umac/cmn_services/utils/inc",
	"cmn/umac/cmn_services/serialization/src",
	"cmn/umac/cmn_services/serialization/inc",
	"cmn/umac/cmn_services/inc",
	"cmn/umac/cmn_services/sm_engine/inc",
	"cmn/umac/cmn_services/cmn_defs/inc",
	"cmn/umac/cmn_services/obj_mgr/src",
	"cmn/umac/cmn_services/obj_mgr/inc",
	"cmn/umac/cmn_services/regulatory/inc",
	"cmn/umac/cmn_services/mgmt_txrx",
	"cmn/umac/cmn_services/interface_mgr/inc",
	"cmn/umac/cmn_services/crypto/src",
	"cmn/umac/cmn_services/crypto/inc",
	"cmn/umac/wifi_pos/src",
	"cmn/umac/wifi_pos/inc",
	"cmn/umac/twt/dispatcher/inc",
	"cmn/umac/twt/core/src",
	"cmn/umac/global_umac_dispatcher/lmac_if/inc",
	"cmn/umac/regulatory/dispatcher/inc",
	"cmn/umac/regulatory/core/src",
	"cmn/umac/dcs/dispatcher/inc",
	"cmn/umac/dcs/core/src",
	"cmn/umac/cfr/dispatcher/inc",
	"cmn/umac/cfr/core/inc",
	"cmn/umac/thermal/dispatcher/inc",
	"cmn/umac/dfs/dispatcher/inc",
	"cmn/umac/dfs/core/src",
	"cmn/umac/mlme",
	"cmn/umac/mlme/pdev_mgr/dispatcher/inc",
	"cmn/umac/mlme/mlme_utils",
	"cmn/umac/mlme/connection_mgr/utf/inc",
	"cmn/umac/mlme/connection_mgr/dispatcher/inc",
	"cmn/umac/mlme/connection_mgr/core/src",
	"cmn/umac/mlme/mlme_objmgr/dispatcher/inc",
	"cmn/umac/mlme/vdev_mgr/dispatcher/inc",
	"cmn/umac/mlme/vdev_mgr/core/src",
	"cmn/umac/mlme/psoc_mgr/dispatcher/inc",
	"cmn/hif/src",
	"cmn/hif/src/pcie",
	"cmn/hif/src/ce",
	"cmn/hif/inc",
	"cmn/wlan_cfg",
	"cmn/wbuff/src",
	"cmn/wbuff/inc",
	"cmn/spectral",
	"cmn/iot_sim",
	"cmn/init_deinit/dispatcher/inc",
	"cmn/global_lmac_if/inc",
	"cmn/scheduler/inc",
	"core/mac/src/sys/common/inc",
	"core/mac/src/pe/include",
	"core/mac/src/include",
	"cmn/umac/cmn_services/mgmt_txrx/dispatcher/inc",
	"cmn/spectral/dispatcher/inc",
	"components/blacklist_mgr/dispatcher/inc",
	"core/mac/src/sys/legacy/src/platform/inc",
	"core/mac/src/sys/legacy/src/system/inc",
	"components/ipa/dispatcher/inc",
	"core/mac/src/dph",
	"cmn/os_if/linux/scan/inc",
	"cmn/os_if/linux/spectral/inc",
	"cmn/os_if/linux/wifi_pos/inc",
	"core/dp/txrx3.0",
	"cmn/os_if/linux/mlme/inc",
	"cmn/utils/pktlog/include",
	"core/sme/src/csr",
	"core/mac/src/pe/lim",
	"cmn/os_if/linux/ftm/inc",
	"components/ipa/core/inc",
	"cmn/os_if/linux/crypto/inc",
	"cmn/utils/fwlog",
	"cmn/os_if/linux/gpio/inc",
	"cmn/os_if/linux/twt/inc",
	"core/mac/src/pe/nan",
	"core/mac/src/sys/legacy/src/utils/inc",
	"cmn/hif/src/dispatcher",
	"cmn/spectral/core",
	"cmn/os_if/linux/cp_stats/inc",
	"cmn/hif/src/snoc",
	"cmn/hal/wifi3.0",
	"cmn/umac/dfs/dispatcher/inc"
]

# paths where include files are private in src folders
_private_ipaths = [
    "cmn/os_if/linux/mlme/src",
]

_fixed_srcs = [
	"core/hdd/src/wlan_hdd_cfg.c",
	"core/hdd/src/wlan_hdd_cfg80211.c",
	"core/hdd/src/wlan_hdd_data_stall_detection.c",
	"core/hdd/src/wlan_hdd_driver_ops.c",
	"core/hdd/src/wlan_hdd_ftm.c",
	"core/hdd/src/wlan_hdd_hostapd.c",
	"core/hdd/src/wlan_hdd_ioctl.c",
	"core/hdd/src/wlan_hdd_main.c",
	"core/hdd/src/wlan_hdd_object_manager.c",
	"core/hdd/src/wlan_hdd_oemdata.c",
	"core/hdd/src/wlan_hdd_p2p.c",
	"core/hdd/src/wlan_hdd_power.c",
	"core/hdd/src/wlan_hdd_regulatory.c",
	"core/hdd/src/wlan_hdd_scan.c",
	"core/hdd/src/wlan_hdd_softap_tx_rx.c",
	"core/hdd/src/wlan_hdd_sta_info.c",
	"core/hdd/src/wlan_hdd_stats.c",
	"core/hdd/src/wlan_hdd_trace.c",
	"core/hdd/src/wlan_hdd_tx_rx.c",
	"core/hdd/src/wlan_hdd_wmm.c",
	"core/hdd/src/wlan_hdd_wowl.c",
	"core/hdd/src/wlan_hdd_cm_connect.c",
	"core/hdd/src/wlan_hdd_cm_disconnect.c",
	"core/hdd/src/wlan_hdd_assoc.c",
	"os_if/sync/src/osif_sync.c",
	"os_if/sync/src/osif_driver_sync.c",
	"os_if/sync/src/osif_psoc_sync.c",
	"os_if/sync/src/osif_vdev_sync.c",
	"components/dsc/src/__wlan_dsc.c",
	"components/dsc/src/wlan_dsc_driver.c",
	"components/dsc/src/wlan_dsc_psoc.c",
	"components/dsc/src/wlan_dsc_vdev.c",
	"core/mac/src/dph/dph_hash_table.c",
	"core/mac/src/pe/lim/lim_aid_mgmt.c",
	"core/mac/src/pe/lim/lim_admit_control.c",
	"core/mac/src/pe/lim/lim_api.c",
	"core/mac/src/pe/lim/lim_assoc_utils.c",
	"core/mac/src/pe/lim/lim_ft.c",
	"core/mac/src/pe/lim/lim_link_monitoring_algo.c",
	"core/mac/src/pe/lim/lim_process_action_frame.c",
	"core/mac/src/pe/lim/lim_process_assoc_req_frame.c",
	"core/mac/src/pe/lim/lim_process_assoc_rsp_frame.c",
	"core/mac/src/pe/lim/lim_process_auth_frame.c",
	"core/mac/src/pe/lim/lim_process_beacon_frame.c",
	"core/mac/src/pe/lim/lim_process_cfg_updates.c",
	"core/mac/src/pe/lim/lim_process_deauth_frame.c",
	"core/mac/src/pe/lim/lim_process_disassoc_frame.c",
	"core/mac/src/pe/lim/lim_process_message_queue.c",
	"core/mac/src/pe/lim/lim_process_mlm_req_messages.c",
	"core/mac/src/pe/lim/lim_process_mlm_rsp_messages.c",
	"core/mac/src/pe/lim/lim_process_probe_rsp_frame.c",
	"core/mac/src/pe/lim/lim_process_sme_req_messages.c",
	"core/mac/src/pe/lim/lim_prop_exts_utils.c",
	"core/mac/src/pe/lim/lim_scan_result_utils.c",
	"core/mac/src/pe/lim/lim_security_utils.c",
	"core/mac/src/pe/lim/lim_send_management_frames.c",
	"core/mac/src/pe/lim/lim_send_messages.c",
	"core/mac/src/pe/lim/lim_send_sme_rsp_messages.c",
	"core/mac/src/pe/lim/lim_session.c",
	"core/mac/src/pe/lim/lim_session_utils.c",
	"core/mac/src/pe/lim/lim_sme_req_utils.c",
	"core/mac/src/pe/lim/lim_timer_utils.c",
	"core/mac/src/pe/lim/lim_trace.c",
	"core/mac/src/pe/lim/lim_utils.c",
	"core/mac/src/pe/lim/lim_process_probe_req_frame.c",
	"core/mac/src/pe/sch/sch_api.c",
	"core/mac/src/pe/sch/sch_beacon_gen.c",
	"core/mac/src/pe/sch/sch_beacon_process.c",
	"core/mac/src/pe/sch/sch_message.c",
	"core/mac/src/pe/rrm/rrm_api.c",
	"core/sap/src/sap_api_link_cntl.c",
	"core/sap/src/sap_ch_select.c",
	"core/sap/src/sap_fsm.c",
	"core/sap/src/sap_module.c",
	"cmn/cfg/src/cfg.c",
	"core/sme/src/csr/csr_api_roam.c",
	"core/sme/src/csr/csr_api_scan.c",
	"core/sme/src/csr/csr_cmd_process.c",
	"core/sme/src/csr/csr_link_list.c",
	"core/sme/src/csr/csr_util.c",
	"core/sme/src/qos/sme_qos.c",
	"core/sme/src/common/sme_api.c",
	"core/sme/src/common/sme_power_save.c",
	"core/sme/src/common/sme_trace.c",
	"core/sme/src/rrm/sme_rrm.c",
	"cmn/utils/nlink/src/wlan_nlink_srv.c",
	"cmn/utils/ptt/src/wlan_ptt_sock_svc.c",
	"cmn/utils/logging/src/wlan_logging_sock_svc.c",
	"cmn/utils/logging/src/wlan_roam_debug.c",
	"core/mac/src/sys/common/src/wlan_qct_sys.c",
	"core/mac/src/sys/legacy/src/platform/src/sys_wrapper.c",
	"core/mac/src/sys/legacy/src/system/src/mac_init_api.c",
	"core/mac/src/sys/legacy/src/system/src/sys_entry_func.c",
	"core/mac/src/sys/legacy/src/utils/src/mac_trace.c",
	"core/mac/src/sys/legacy/src/utils/src/dot11f.c",
	"core/mac/src/sys/legacy/src/utils/src/parser_api.c",
	"core/mac/src/sys/legacy/src/utils/src/utils_parser.c",
	"cmn/qdf/linux/src/qdf_crypto.c",
	"cmn/qdf/linux/src/qdf_defer.c",
	"cmn/qdf/linux/src/qdf_delayed_work.c",
	"cmn/qdf/linux/src/qdf_event.c",
	"cmn/qdf/linux/src/qdf_file.c",
	"cmn/qdf/linux/src/qdf_func_tracker.c",
	"cmn/qdf/linux/src/qdf_idr.c",
	"cmn/qdf/linux/src/qdf_list.c",
	"cmn/qdf/linux/src/qdf_lock.c",
	"cmn/qdf/linux/src/qdf_mc_timer.c",
	"cmn/qdf/linux/src/qdf_mem.c",
	"cmn/qdf/linux/src/qdf_nbuf.c",
	"cmn/qdf/linux/src/qdf_periodic_work.c",
	"cmn/qdf/linux/src/qdf_status.c",
	"cmn/qdf/linux/src/qdf_threads.c",
	"cmn/qdf/linux/src/qdf_trace.c",
	"cmn/qdf/linux/src/qdf_nbuf_frag.c",
	"cmn/qdf/src/qdf_flex_mem.c",
	"cmn/qdf/src/qdf_parse.c",
	"cmn/qdf/src/qdf_platform.c",
	"cmn/qdf/src/qdf_str.c",
	"cmn/qdf/src/qdf_talloc.c",
	"cmn/qdf/src/qdf_types.c",
	"cmn/os_if/linux/wlan_osif_request_manager.c",
	"cmn/os_if/linux/crypto/src/wlan_nl_to_crypto_params.c",
	"cmn/os_if/linux/mlme/src/osif_cm_util.c",
	"cmn/os_if/linux/mlme/src/osif_cm_connect_rsp.c",
	"cmn/os_if/linux/mlme/src/osif_cm_disconnect_rsp.c",
	"cmn/os_if/linux/mlme/src/osif_cm_req.c",
	"cmn/os_if/linux/mlme/src/osif_cm_roam_rsp.c",
	"cmn/os_if/linux/mlme/src/osif_vdev_mgr_util.c",
	"cmn/os_if/linux/crypto/src/wlan_cfg80211_crypto.c",
	"cmn/umac/global_umac_dispatcher/lmac_if/src/wlan_lmac_if.c",
	"cmn/umac/scan/core/src/wlan_scan_cache_db.c",
	"cmn/umac/scan/core/src/wlan_scan_11d.c",
	"cmn/umac/scan/core/src/wlan_scan_filter.c",
	"cmn/umac/scan/core/src/wlan_scan_main.c",
	"cmn/umac/scan/core/src/wlan_scan_manager.c",
	"cmn/umac/scan/dispatcher/src/wlan_scan_tgt_api.c",
	"cmn/umac/scan/dispatcher/src/wlan_scan_ucfg_api.c",
	"cmn/umac/scan/dispatcher/src/wlan_scan_api.c",
	"cmn/umac/scan/dispatcher/src/wlan_scan_utils_api.c",
	"cmn/os_if/linux/scan/src/wlan_cfg80211_scan.c",
	"cmn/os_if/linux/wlan_cfg80211.c",
	"cmn/target_if/scan/src/target_if_scan.c",
	"cmn/umac/cmn_services/crypto/src/wlan_crypto_main.c",
	"cmn/umac/cmn_services/crypto/src/wlan_crypto_ucfg_api.c",
	"cmn/umac/cmn_services/crypto/src/wlan_crypto_param_handling.c",
	"cmn/umac/cmn_services/crypto/src/wlan_crypto_obj_mgr.c",
	"cmn/umac/cmn_services/crypto/src/wlan_crypto_global_api.c",
	"core/cds/src/cds_regdomain.c",
	"core/cds/src/cds_sched.c",
	"core/cds/src/cds_api.c",
	"core/cds/src/cds_reg_service.c",
	"core/cds/src/cds_utils.c",
	"core/cds/src/cds_packet.c",
	"cmn/umac/cmn_services/obj_mgr/src/wlan_objmgr_psoc_obj.c",
	"cmn/umac/cmn_services/obj_mgr/src/wlan_objmgr_pdev_obj.c",
	"cmn/umac/cmn_services/obj_mgr/src/wlan_objmgr_peer_obj.c",
	"cmn/umac/cmn_services/obj_mgr/src/wlan_objmgr_vdev_obj.c",
	"cmn/umac/cmn_services/obj_mgr/src/wlan_objmgr_global_obj.c",
	"components/cmn_services/interface_mgr/src/wlan_if_mgr_sta.c",
	"components/cmn_services/interface_mgr/src/wlan_if_mgr_sap.c",
	"components/cmn_services/interface_mgr/src/wlan_if_mgr_roam.c",
	"cmn/umac/cmn_services/interface_mgr/src/wlan_if_mgr_core.c",
	"cmn/umac/cmn_services/interface_mgr/src/wlan_if_mgr_main.c",
	"os_if/cp_stats/src/wlan_cfg80211_mc_cp_stats.c",
	"os_if/twt/src/osif_twt_ext_util.c",
	"os_if/twt/src/osif_twt_ext_rsp.c",
	"os_if/twt/src/osif_twt_ext_req.c",
	"os_if/nan/src/os_if_nan.c",
	"os_if/p2p/src/wlan_cfg80211_p2p.c",
	"os_if/interop_issues_ap/src/wlan_cfg80211_interop_issues_ap.c",
	"core/dp/htt/htt.c",
	"core/dp/htt/htt_h2t.c",
	"core/dp/htt/htt_t2h.c",
	"core/dp/htt/htt_rx.c",
	"core/dp/htt/htt_fw_stats.c",
	"core/dp/htt/htt_tx.c",
	"core/dp/txrx/ol_rx_pn.c",
	"core/dp/txrx/ol_rx_reorder.c",
	"core/dp/txrx/ol_cfg.c",
	"core/dp/txrx/ol_txrx_encap.c",
	"core/dp/txrx/ol_txrx.c",
	"core/dp/txrx/ol_txrx_peer_find.c",
	"core/dp/txrx/ol_tx.c",
	"core/dp/txrx/ol_tx_send.c",
	"core/dp/txrx/ol_tx_desc.c",
	"core/dp/txrx/ol_rx.c",
	"core/dp/txrx/ol_rx_fwd.c",
	"core/dp/txrx/ol_rx_reorder_timeout.c",
	"core/dp/txrx/ol_rx_defrag.c",
	"core/sme/src/nan/nan_datapath_api.c",
	"core/mac/src/pe/nan/nan_datapath.c",
	"core/wma/src/wma_power.c",
	"core/wma/src/wma_mgmt.c",
	"core/wma/src/wlan_qct_wma_legacy.c",
	"core/wma/src/wma_scan_roam.c",
	"core/wma/src/wma_features.c",
	"core/wma/src/wma_fw_state.c",
	"core/wma/src/wma_utils.c",
	"core/wma/src/wma_data.c",
	"core/wma/src/wma_dev_if.c",
	"core/wma/src/wma_main.c",
	"core/pld/src/pld_snoc.c",
	"core/pld/src/pld_common.c",
	"components/cmn_services/policy_mgr/src/wlan_policy_mgr_action.c",
	"components/cmn_services/policy_mgr/src/wlan_policy_mgr_init_deinit.c",
	"components/cmn_services/policy_mgr/src/wlan_policy_mgr_pcl.c",
	"components/cmn_services/policy_mgr/src/wlan_policy_mgr_get_set_utils.c",
	"components/cmn_services/policy_mgr/src/wlan_policy_mgr_core.c",
	"components/cmn_services/policy_mgr/src/wlan_policy_mgr_ucfg.c",
	"components/target_if/connection_mgr/src/target_if_cm_roam_event.c",
	"components/target_if/connection_mgr/src/target_if_cm_roam_offload.c",
	"components/target_if/p2p/src/target_if_p2p.c",
	"components/target_if/wfa_config/src/target_if_wfa_testcmd.c",
	"components/umac/mlme/connection_mgr/dispatcher/src/wlan_cm_tgt_if_tx_api.c",
	"components/umac/mlme/connection_mgr/dispatcher/src/wlan_cm_roam_api.c",
	"components/umac/mlme/connection_mgr/dispatcher/src/wlan_cm_roam_ucfg_api.c",
	"components/umac/mlme/connection_mgr/core/src/wlan_cm_roam_offload.c",
	"components/umac/mlme/connection_mgr/core/src/wlan_cm_vdev_connect.c",
	"components/umac/mlme/connection_mgr/core/src/wlan_cm_roam_offload_event.c",
	"components/umac/mlme/connection_mgr/core/src/wlan_cm_vdev_disconnect.c",
	"components/umac/mlme/wfa_config/dispatcher/src/wlan_wfa_tgt_if_tx_api.c",
	"components/p2p/dispatcher/src/wlan_p2p_api.c",
	"components/p2p/dispatcher/src/wlan_p2p_tgt_api.c",
	"components/p2p/dispatcher/src/wlan_p2p_ucfg_api.c",
	"components/p2p/dispatcher/src/wlan_p2p_cfg.c",
	"components/p2p/core/src/wlan_p2p_off_chan_tx.c",
	"components/p2p/core/src/wlan_p2p_roc.c",
	"components/p2p/core/src/wlan_p2p_main.c",
	"components/mlme/dispatcher/src/wlan_mlme_ucfg_api.c",
	"components/mlme/dispatcher/src/wlan_mlme_api.c",
	"components/mlme/dispatcher/src/wlan_mlme_twt_ucfg_api.c",
	"components/mlme/core/src/wlan_mlme_vdev_mgr_interface.c",
	"components/mlme/core/src/wlan_mlme_main.c",
	"components/mlme/core/src/wlan_mlme_twt_api.c",
	"cmn/htc/htc.c",
	"cmn/htc/htc_send.c",
	"cmn/htc/htc_recv.c",
	"cmn/htc/htc_services.c",
	"cmn/wmi/src/wmi_unified_p2p_api.c",
	"cmn/wmi/src/wmi_unified_tlv.c",
	"cmn/wmi/src/wmi_unified_p2p_tlv.c",
	"cmn/wmi/src/wmi_unified.c",
	"cmn/wmi/src/wmi_unified_api.c",
	"cmn/wmi/src/wmi_tlv_helper.c",
	"cmn/wmi/src/wmi_tlv_platform.c",
	"cmn/wmi/src/wmi_unified_concurrency_api.c",
	"cmn/wmi/src/wmi_unified_vdev_tlv.c",
	"cmn/wmi/src/wmi_unified_crypto_api.c",
	"cmn/wmi/src/wmi_unified_vdev_api.c",
	"cmn/wmi/src/wmi_unified_reg_api.c",
	"cmn/target_if/core/src/target_if_main.c",
	"cmn/target_if/regulatory/src/target_if_reg_11d.c",
	"cmn/target_if/regulatory/src/target_if_reg.c",
	"cmn/target_if/regulatory/src/target_if_reg_lte.c",
	"cmn/target_if/init_deinit/src/init_event_handler.c",
	"cmn/target_if/init_deinit/src/init_cmd_api.c",
	"cmn/target_if/init_deinit/src/service_ready_util.c",
	"cmn/target_if/init_deinit/src/init_deinit_lmac.c",
	"cmn/target_if/mlme/psoc/src/target_if_psoc_timer_tx_ops.c",
	"cmn/target_if/mlme/vdev_mgr/src/target_if_vdev_mgr_tx_ops.c",
	"cmn/target_if/mlme/vdev_mgr/src/target_if_vdev_mgr_rx_ops.c",
	"cmn/target_if/crypto/src/target_if_crypto.c",
	"cmn/umac/cp_stats/dispatcher/src/wlan_cp_stats_utils_api.c",
	"cmn/umac/cp_stats/dispatcher/src/wlan_cp_stats_ucfg_api.c",
	"cmn/umac/cp_stats/core/src/wlan_cp_stats_obj_mgr_handler.c",
	"cmn/umac/cp_stats/core/src/wlan_cp_stats_comp_handler.c",
	"cmn/umac/cp_stats/core/src/wlan_cp_stats_ol_api.c",
	"cmn/umac/cmn_services/utils/src/wlan_utility.c",
	"cmn/umac/cmn_services/serialization/src/wlan_serialization_non_scan.c",
	"cmn/umac/cmn_services/serialization/src/wlan_serialization_legacy_api.c",
	"cmn/umac/cmn_services/serialization/src/wlan_serialization_api.c",
	"cmn/umac/cmn_services/serialization/src/wlan_serialization_main.c",
	"cmn/umac/cmn_services/serialization/src/wlan_serialization_rules.c",
	"cmn/umac/cmn_services/serialization/src/wlan_serialization_queue.c",
	"cmn/umac/cmn_services/serialization/src/wlan_serialization_utils.c",
	"cmn/umac/cmn_services/serialization/src/wlan_serialization_internal.c",
	"cmn/umac/cmn_services/serialization/src/wlan_serialization_scan.c",
	"cmn/umac/cmn_services/sm_engine/src/wlan_sm_engine.c",
	"cmn/umac/cmn_services/mgmt_txrx/dispatcher/src/wlan_mgmt_txrx_utils_api.c",
	"cmn/umac/cmn_services/mgmt_txrx/dispatcher/src/wlan_mgmt_txrx_tgt_api.c",
	"cmn/umac/cmn_services/mgmt_txrx/core/src/wlan_mgmt_txrx_main.c",
	"cmn/umac/regulatory/dispatcher/src/wlan_reg_tgt_api.c",
	"cmn/umac/regulatory/dispatcher/src/wlan_reg_ucfg_api.c",
	"cmn/umac/regulatory/dispatcher/src/wlan_reg_services_api.c",
	"cmn/umac/regulatory/core/src/reg_offload_11d_scan.c",
	"cmn/umac/regulatory/core/src/reg_db_parser.c",
	"cmn/umac/regulatory/core/src/reg_utils.c",
	"cmn/umac/regulatory/core/src/reg_lte.c",
	"cmn/umac/regulatory/core/src/reg_priv_objs.c",
	"cmn/umac/regulatory/core/src/reg_callbacks.c",
	"cmn/umac/regulatory/core/src/reg_opclass.c",
	"cmn/umac/regulatory/core/src/reg_db.c",
	"cmn/umac/regulatory/core/src/reg_services_common.c",
	"cmn/umac/regulatory/core/src/reg_build_chan_list.c",
	"cmn/umac/mlme/pdev_mgr/dispatcher/src/wlan_pdev_mlme_api.c",
	"cmn/umac/mlme/connection_mgr/dispatcher/src/wlan_cm_api.c",
	"cmn/umac/mlme/connection_mgr/dispatcher/src/wlan_cm_ucfg_api.c",
	"cmn/umac/mlme/connection_mgr/core/src/wlan_cm_roam_sm.c",
	"cmn/umac/mlme/connection_mgr/core/src/wlan_cm_disconnect.c",
	"cmn/umac/mlme/connection_mgr/core/src/wlan_cm_connect.c",
	"cmn/umac/mlme/connection_mgr/core/src/wlan_cm_main.c",
	"cmn/umac/mlme/connection_mgr/core/src/wlan_cm_sm.c",
	"cmn/umac/mlme/connection_mgr/core/src/wlan_cm_bss_scoring.c",
	"cmn/umac/mlme/connection_mgr/core/src/wlan_cm_connect_scan.c",
	"cmn/umac/mlme/connection_mgr/core/src/wlan_cm_util.c",
	"cmn/umac/mlme/mlme_objmgr/dispatcher/src/wlan_vdev_mlme_main.c",
	"cmn/umac/mlme/mlme_objmgr/dispatcher/src/wlan_psoc_mlme_main.c",
	"cmn/umac/mlme/mlme_objmgr/dispatcher/src/wlan_pdev_mlme_main.c",
	"cmn/umac/mlme/mlme_objmgr/dispatcher/src/wlan_cmn_mlme_main.c",
	"cmn/umac/mlme/vdev_mgr/dispatcher/src/wlan_vdev_mgr_utils_api.c",
	"cmn/umac/mlme/vdev_mgr/dispatcher/src/wlan_vdev_mgr_ucfg_api.c",
	"cmn/umac/mlme/vdev_mgr/dispatcher/src/wlan_vdev_mgr_tgt_if_tx_api.c",
	"cmn/umac/mlme/vdev_mgr/dispatcher/src/wlan_vdev_mlme_api.c",
	"cmn/umac/mlme/vdev_mgr/dispatcher/src/wlan_vdev_mgr_tgt_if_rx_api.c",
	"cmn/umac/mlme/vdev_mgr/core/src/vdev_mgr_ops.c",
	"cmn/umac/mlme/vdev_mgr/core/src/vdev_mlme_sm.c",
	"cmn/umac/mlme/psoc_mgr/dispatcher/src/wlan_psoc_mlme_api.c",
	"cmn/hif/src/hif_runtime_pm.c",
	"cmn/hif/src/mp_dev.c",
	"cmn/hif/src/regtable.c",
	"cmn/hif/src/ath_procfs.c",
	"cmn/hif/src/ce/ce_tasklet.c",
	"cmn/hif/src/ce/ce_service.c",
	"cmn/hif/src/ce/ce_service_legacy.c",
	"cmn/hif/src/ce/ce_diag.c",
	"cmn/hif/src/ce/ce_main.c",
	"cmn/hif/src/snoc/if_snoc.c",
	"cmn/hif/src/hif_main.c",
	"cmn/hif/src/hif_exec.c",
	"cmn/hif/src/hif_main_legacy.c",
	"cmn/hif/src/dispatcher/multibus.c",
	"cmn/hif/src/dispatcher/dummy.c",
	"cmn/init_deinit/dispatcher/src/dispatcher_init_deinit.c",
	"cmn/global_lmac_if/src/wlan_global_lmac_if.c",
	"cmn/scheduler/src/scheduler_api.c",
	"cmn/scheduler/src/scheduler_core.c",
	"core/dp/txrx/ol_tx_throttle.c",
	"cmn/hif/src/dispatcher/multibus_snoc.c"
]

_conditional_srcs = {
    "CONFIG_UNIT_TEST": {
        True: [
            "core/hdd/src/wlan_hdd_unit_test.c",
        ],
    },
    "CONFIG_WLAN_WEXT_SUPPORT_ENABLE": {
        True: [
            "core/hdd/src/wlan_hdd_hostapd_wext.c",
            "core/hdd/src/wlan_hdd_wext.c",
        ],
    },
    "CONFIG_BAND_6GHZ": {
        True: [
            "cmn/umac/scan/core/src/wlan_scan_manager_6ghz.c",
        ],
    },
    "CONFIG_BERYLLIUM": {
        True: [
            # TODO: how to handle Kbuild logic
            #ifneq (y,$(filter y,$(CONFIG_LITHIUM) $(CONFIG_BERYLLIUM) $(CONFIG_RHINE)))
            #ifeq (y,$(filter y,$(CONFIG_LITHIUM) $(CONFIG_BERYLLIUM) $(CONFIG_RHINE)))
            "cmn/dp/cmn_dp_api/dp_ratetable.c",
            "cmn/dp/wifi3.0/be/dp_be.c",
            "cmn/dp/wifi3.0/be/dp_be_rx.c",
            "cmn/dp/wifi3.0/be/dp_be_tx.c",
            "cmn/dp/wifi3.0/dp_reo.c",
            "cmn/dp/wifi3.0/dp_rx_err.c",
            "cmn/hal/wifi3.0/be/hal_be_generic_api.c",
            "cmn/hal/wifi3.0/be/hal_be_reo.c",
            "cmn/hal/wifi3.0/hal_reo.c",
            "cmn/hal/wifi3.0/hal_srng.c",
            "cmn/hif/src/ce/ce_service_srng.c",
            "cmn/wlan_cfg/wlan_cfg.c",
        ],
    },
    "CONFIG_CM_UTF_ENABLE": {
        True: [
            "components/umac/mlme/connection_mgr/utf/src/cm_utf.c",
            "cmn/umac/mlme/connection_mgr/utf/src/wlan_cm_utf_main.c",
            "cmn/umac/mlme/connection_mgr/utf/src/wlan_cm_utf_scan.c",
        ],
    },
    "CONFIG_CNSS_KIWI_V2": {
        True: [
            "cmn/hal/wifi3.0/kiwi/hal_kiwi.c",
            "cmn/hif/src/kiwidef.c",
        ],
    },
    "CONFIG_CNSS_PEACH": {
        True: [
            "cmn/hal/wifi3.0/kiwi/hal_kiwi.c",
        ],
    },
   "CONFIG_CP_STATS": {
        True: [
            "cmn/target_if/cp_stats/src/target_if_cp_stats.c",
            "cmn/umac/cp_stats/core/src/wlan_cp_stats_comp_handler.c",
            "cmn/umac/cp_stats/core/src/wlan_cp_stats_obj_mgr_handler.c",
            "cmn/umac/cp_stats/core/src/wlan_cp_stats_ol_api.c",
            "cmn/umac/cp_stats/dispatcher/src/wlan_cp_stats_ucfg_api.c",
            "cmn/umac/cp_stats/dispatcher/src/wlan_cp_stats_utils_api.c",
            "cmn/wmi/src/wmi_unified_cp_stats_api.c",
            "cmn/wmi/src/wmi_unified_cp_stats_tlv.c",
            "components/cp_stats/dispatcher/src/wlan_cp_stats_mc_tgt_api.c",
            "components/cp_stats/dispatcher/src/wlan_cp_stats_mc_ucfg_api.c",
            "components/target_if/cp_stats/src/target_if_mc_cp_stats.c",
            "components/wmi/src/wmi_unified_mc_cp_stats_api.c",
            "components/wmi/src/wmi_unified_mc_cp_stats_tlv.c",
            "os_if/cp_stats/src/wlan_cfg80211_mc_cp_stats.c",
        ],
    },
    "CONFIG_DCS": {
        True: [
            "cmn/target_if/dcs/src/target_if_dcs.c",
            "cmn/umac/dcs/core/src/wlan_dcs.c",
            "cmn/umac/dcs/dispatcher/src/wlan_dcs_init_deinit_api.c",
            "cmn/umac/dcs/dispatcher/src/wlan_dcs_tgt_api.c",
            "cmn/umac/dcs/dispatcher/src/wlan_dcs_ucfg_api.c",
            "cmn/wmi/src/wmi_unified_dcs_api.c",
            "cmn/wmi/src/wmi_unified_dcs_tlv.c",
            "core/hdd/src/wlan_hdd_dcs.c",
        ],
    },
    "CONFIG_DIRECT_BUF_RX_ENABLE": {
        True: [
            "cmn/target_if/direct_buf_rx/src/target_if_direct_buf_rx_api.c",
            "cmn/target_if/direct_buf_rx/src/target_if_direct_buf_rx_main.c",
            "cmn/wmi/src/wmi_unified_dbr_api.c",
            "cmn/wmi/src/wmi_unified_dbr_tlv.c",
        ],
    },
    "CONFIG_DP_HW_TX_DELAY_STATS_ENABLE": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs_dp_tx_delay_stats.c",
        ],
    },
    "CONFIG_DP_PKT_ADD_TIMESTAMP": {
        True: [
            # TODO: need separate cfg for sysfs
            "core/hdd/src/wlan_hdd_sysfs_add_timestamp.c",
            "cmn/qdf/linux/src/qdf_pkt_add_timestamp.c",
        ],
    },
   "CONFIG_DSC_TEST": {
        True: [
            "components/dsc/test/wlan_dsc_test.c",
        ],
    },
    "CONFIG_FEATURE_ACTIVE_TOS": {
        True: [
            "core/hdd/src/wlan_hdd_active_tos.c",
        ],
    },
    "CONFIG_FEATURE_BSS_TRANSITION": {
        True: [
            "core/hdd/src/wlan_hdd_bss_transition.c",
        ],
    },
    "CONFIG_FEATURE_COEX": {
        True: [
            "components/coex/core/src/wlan_coex_main.c",
            "components/coex/dispatcher/src/wlan_coex_tgt_api.c",
            "components/coex/dispatcher/src/wlan_coex_ucfg_api.c",
            "components/coex/dispatcher/src/wlan_coex_utils_api.c",
            "components/target_if/coex/src/target_if_coex.c",
            "os_if/coex/src/wlan_cfg80211_coex.c",
        ],
    },
   "CONFIG_FEATURE_CONCURRENCY_MATRIX": {
        True: [
            "core/hdd/src/wlan_hdd_concurrency_matrix.c",
        ],
    },
    "CONFIG_FEATURE_EPPING": {
        True: [
            "cmn/utils/epping/src/epping_helper.c",
            "cmn/utils/epping/src/epping_main.c",
            "cmn/utils/epping/src/epping_rx.c",
            "cmn/utils/epping/src/epping_tx.c",
            "cmn/utils/epping/src/epping_txrx.c",
        ],
    },
    "CONFIG_FEATURE_FW_LOG_PARSING": {
        True: [
            "cmn/utils/fwlog/dbglog_host.c",
        ],
    },
    "CONFIG_FEATURE_GPIO_CFG": {
       True: [
            "core/hdd/src/wlan_hdd_gpio.c",
            "cmn/gpio/core/src/wlan_gpio_api.c",
            "cmn/gpio/dispatcher/src/wlan_gpio_tgt_api.c",
            "cmn/gpio/dispatcher/src/wlan_gpio_ucfg_api.c",
            "cmn/os_if/linux/gpio/src/wlan_cfg80211_gpio.c",
            "cmn/target_if/gpio/target_if_gpio.c",
            "cmn/wmi/src/wmi_unified_gpio_api.c",
            "cmn/wmi/src/wmi_unified_gpio_tlv.c",
        ],
    },
    "CONFIG_FEATURE_HTC_CREDIT_HISTORY": {
        True: [
            "cmn/htc/htc_credit_history.c",
        ],
    },
    "CONFIG_FEATURE_INTEROP_ISSUES_AP": {
        True: [
            "cmn/wmi/src/wmi_unified_interop_issues_ap_api.c",
            "cmn/wmi/src/wmi_unified_interop_issues_ap_tlv.c",
            "components/interop_issues_ap/core/src/wlan_interop_issues_ap_api.c",
            "components/interop_issues_ap/dispatcher/src/wlan_interop_issues_ap_tgt_api.c",
            "components/interop_issues_ap/dispatcher/src/wlan_interop_issues_ap_ucfg_api.c",
            "components/target_if/interop_issues_ap/src/target_if_interop_issues_ap.c",
            "os_if/interop_issues_ap/src/wlan_cfg80211_interop_issues_ap.c",
        ],
    },
    "CONFIG_FEATURE_MEC": {
        True: [
            "cmn/dp/wifi3.0/dp_txrx_wds.c",
        ],
    },
    "CONFIG_FEATURE_MEMDUMP_ENABLE": {
        True: [
            "core/hdd/src/wlan_hdd_memdump.c",
        ],
    },
    "CONFIG_FEATURE_MONITOR_MODE_SUPPORT": {
        True: [
            "core/hdd/src/wlan_hdd_rx_monitor.c",
			"core/dp/htt/htt_monitor_rx.c",
        ],
    },
  "CONFIG_FEATURE_MOTION_DETECTION": {
      True: [
          "core/hdd/src/wlan_hdd_sysfs_motion_detection.c",
      ],
  },
    "CONFIG_FEATURE_OTA_TEST": {
        True: [
            "core/hdd/src/wlan_hdd_ota_test.c",
        ],
    },
    "CONFIG_FEATURE_P2P_LISTEN_OFFLOAD": {
        True: [
            "core/hdd/src/wlan_hdd_p2p_listen_offload.c",
        ],
    },
    "CONFIG_FEATURE_RSSI_MONITOR": {
        True: [
            "core/hdd/src/wlan_hdd_rssi_monitor.c",
        ],
    },
    "CONFIG_FEATURE_SAP_COND_CHAN_SWITCH": {
        True: [
            "core/hdd/src/wlan_hdd_sap_cond_chan_switch.c",
        ],
    },
    "CONFIG_FEATURE_SAR_LIMITS": {
        True: [
            "core/hdd/src/wlan_hdd_sar_limits.c",
        ],
    },
    "CONFIG_FEATURE_STATION_INFO": {
        True: [
            "core/hdd/src/wlan_hdd_station_info.c",
        ],
    },
    "CONFIG_FEATURE_TX_POWER": {
        True: [
            "core/hdd/src/wlan_hdd_tx_power.c",
        ],
    },
    "CONFIG_FEATURE_UNIT_TEST_SUSPEND": {
        True: [
            "cmn/hif/src/hif_unit_test_suspend.c",
            #TODO: need separate flag for sysfs
            "core/hdd/src/wlan_hdd_sysfs_suspend_resume.c",
        ],
    },
    "CONFIG_FEATURE_VDEV_OPS_WAKELOCK": {
        True: [
            "cmn/target_if/mlme/psoc/src/target_if_psoc_wake_lock.c",
        ],
    },
    "CONFIG_FEATURE_WLAN_CH_AVOID_EXT": {
        True: [
            "core/hdd/src/wlan_hdd_avoid_freq_ext.c",
        ],
    },
    "CONFIG_FEATURE_WLAN_EXTSCAN": {
        True: [
            "cmn/umac/scan/dispatcher/src/wlan_extscan_api.c",
            "cmn/wmi/src/wmi_unified_extscan_api.c",
            "cmn/wmi/src/wmi_unified_extscan_tlv.c",
            "core/hdd/src/wlan_hdd_ext_scan.c",
        ],
    },
    "CONFIG_FEATURE_WLAN_TIME_SYNC_FTM": {
        True: [
           "components/ftm_time_sync/core/src/ftm_time_sync_main.c",
           "components/ftm_time_sync/dispatcher/src/ftm_time_sync_ucfg_api.c",
           "components/ftm_time_sync/dispatcher/src/wlan_ftm_time_sync_tgt_api.c",
           "components/target_if/ftm_time_sync/src/target_if_ftm_time_sync.c",
           "core/hdd/src/wlan_hdd_ftm_time_sync.c",
       ],
    },
    "CONFIG_FW_THERMAL_THROTTLE": {
        True: [
            "core/hdd/src/wlan_hdd_thermal.c",
        ],
    },
    "CONFIG_HIF_IPCI": {
        True: [
            "cmn/hif/src/dispatcher/multibus_ipci.c",
        ],
    },
    "CONFIG_HIF_PCI": {
        True: [
            "cmn/hif/src/dispatcher/multibus_pci.c",
            "cmn/hif/src/pcie/if_pci.c",
            "core/pld/src/pld_pcie.c",
        ],
    },
"CONFIG_HIF_SDIO": {
    True: [
        "cmn/hif/src/dispatcher/multibus_sdio.c",
    ],
},
    "CONFIG_HIF_SNOC": {
        True: [
            "cmn/hif/src/dispatcher/multibus_snoc.c",
        ],
    },
    "CONFIG_HIF_USB": {
        True: [
            "cmn/hif/src/dispatcher/multibus_usb.c",
            "core/pld/src/pld_usb.c",
        ],
    },
    "CONFIG_HL_DP_SUPPORT": {
        True: [
            "core/dp/txrx/ol_tx_classify.c",
            "core/dp/txrx/ol_tx_hl.c",
            "core/dp/txrx/ol_tx_queue.c",
            "core/dp/txrx/ol_tx_sched.c",
			"core/dp/htt/htt_rx_hl.c",
        ],
    },
    "CONFIG_HOST_11D_SCAN": {
        True: [
            "cmn/umac/regulatory/core/src/reg_host_11d.c",
        ],
    },
   "CONFIG_IPA_OFFLOAD": {
        True: [
			"core/dp/txrx/ol_txrx_ipa.c",
            "cmn/qdf/linux/src/qdf_ipa.c",
            "core/hdd/src/wlan_hdd_ipa.c",
            # TODO: need a separate flag for sysfs
            "core/hdd/src/wlan_hdd_sysfs_ipa.c",
        ],
    },
    "CONFIG_IPCIE_FW_SIM": {
        True: [
            "core/pld/src/pld_pcie_fw_sim.c",
        ],
    },
    "CONFIG_LEAK_DETECTION": {
        True: [
            "cmn/qdf/src/qdf_debug_domain.c",
            "cmn/qdf/src/qdf_tracker.c",
        ],
    },
    "CONFIG_LFR_SUBNET_DETECTION": {
        True: [
            "core/hdd/src/wlan_hdd_subnet_detect.c",
        ],
    },
    "CONFIG_LINUX_QCMBR": {
        True: [
            "cmn/os_if/linux/ftm/src/wlan_ioctl_ftm.c",
        ],
    },
  "CONFIG_LITHIUM": {
      True: [
          # TODO: how to handle Kbuild logic
          #ifneq (y,$(filter y,$(CONFIG_LITHIUM) $(CONFIG_BERYLLIUM) $(CONFIG_RHINE)))
          #ifeq (y,$(filter y,$(CONFIG_LITHIUM) $(CONFIG_BERYLLIUM) $(CONFIG_RHINE)))
          "cmn/dp/cmn_dp_api/dp_ratetable.c",
          "cmn/dp/wifi3.0/li/dp_li.c",
          "cmn/dp/wifi3.0/li/dp_li_rx.c",
          "cmn/dp/wifi3.0/li/dp_li_tx.c",
          "cmn/dp/wifi3.0/dp_reo.c",
          "cmn/dp/wifi3.0/dp_rx_err.c",
          "cmn/hal/wifi3.0/hal_reo.c",
          "cmn/hal/wifi3.0/hal_srng.c",
          "cmn/hal/wifi3.0/li/hal_li_generic_api.c",
          "cmn/hal/wifi3.0/li/hal_li_reo.c",
          "cmn/hif/src/ce/ce_service_srng.c",
          "cmn/wlan_cfg/wlan_cfg.c",
      ],
  },
    "CONFIG_LL_DP_SUPPORT": {
        True: [
            "core/dp/txrx/ol_tx_ll.c",
			"core/dp/htt/htt_rx_ll.c",
        ],
    },
    "CONFIG_OCB_UT_FRAMEWORK": {
        True: [
            "cmn/wmi/src/wmi_unified_ocb_ut.c",
        ],
    },
    "CONFIG_PCIE_FW_SIM": {
        True: [
            "core/pld/src/pld_pcie_fw_sim.c",
        ],
    },
    "CONFIG_PKTLOG_LEGACY": {
        True: [
            "cmn/utils/pktlog/pktlog_wifi2.c",
        ],
    },
    "CONFIG_FEATURE_PKTLOG_EN_NON_LEGACY": {
        True: [
            "cmn/utils/pktlog/pktlog_wifi3.c",
        ],
    },
    "CONFIG_PKT_LOG": {
        #TODO: Currently this is CONFIG_REMOVE_PKT_LOG but expect it to change
        #      Also need a separate config for sysfs
        True: [
            "cmn/utils/pktlog/linux_ac.c",
            "cmn/utils/pktlog/pktlog_ac.c",
            "cmn/utils/pktlog/pktlog_internal.c",
            "core/hdd/src/wlan_hdd_sysfs_pktlog.c",
        ],
    },
    "CONFIG_PLD_IPCI_ICNSS_FLAG": {
        True: [
            "core/pld/src/pld_ipci.c",
        ],
    },
   "CONFIG_PLD_SNOC_ICNSS_FLAG": {
        True: [
            "core/pld/src/pld_snoc.c",
        ],
    },
    "CONFIG_POWER_MANAGEMENT_OFFLOAD": {
        True: [
            "cmn/wmi/src/wmi_unified_pmo_api.c",
            "cmn/wmi/src/wmi_unified_pmo_tlv.c",
            "components/pmo/core/src/wlan_pmo_apf.c",
            "components/pmo/core/src/wlan_pmo_arp.c",
            "components/pmo/core/src/wlan_pmo_gtk.c",
            "components/pmo/core/src/wlan_pmo_hw_filter.c",
            "components/pmo/core/src/wlan_pmo_lphb.c",
            "components/pmo/core/src/wlan_pmo_main.c",
            "components/pmo/core/src/wlan_pmo_mc_addr_filtering.c",
            "components/pmo/core/src/wlan_pmo_static_config.c",
            "components/pmo/core/src/wlan_pmo_suspend_resume.c",
            "components/pmo/core/src/wlan_pmo_wow.c",
            "components/pmo/dispatcher/src/wlan_pmo_obj_mgmt_api.c",
            "components/pmo/dispatcher/src/wlan_pmo_tgt_arp.c",
            "components/pmo/dispatcher/src/wlan_pmo_tgt_gtk.c",
            "components/pmo/dispatcher/src/wlan_pmo_tgt_hw_filter.c",
            "components/pmo/dispatcher/src/wlan_pmo_tgt_lphb.c",
            "components/pmo/dispatcher/src/wlan_pmo_tgt_mc_addr_filtering.c",
            "components/pmo/dispatcher/src/wlan_pmo_tgt_static_config.c",
            "components/pmo/dispatcher/src/wlan_pmo_tgt_suspend_resume.c",
            "components/pmo/dispatcher/src/wlan_pmo_tgt_wow.c",
            "components/pmo/dispatcher/src/wlan_pmo_ucfg_api.c",
            "components/target_if/pmo/src/target_if_pmo_arp.c",
            "components/target_if/pmo/src/target_if_pmo_gtk.c",
            "components/target_if/pmo/src/target_if_pmo_hw_filter.c",
            "components/target_if/pmo/src/target_if_pmo_lphb.c",
            "components/target_if/pmo/src/target_if_pmo_main.c",
            "components/target_if/pmo/src/target_if_pmo_mc_addr_filtering.c",
            "components/target_if/pmo/src/target_if_pmo_static_config.c",
            "components/target_if/pmo/src/target_if_pmo_suspend_resume.c",
            "components/target_if/pmo/src/target_if_pmo_wow.c",
        ],
    },
    "CONFIG_QCACLD_FEATURE_APF": {
        True: [
            "core/hdd/src/wlan_hdd_apf.c",
            "cmn/wmi/src/wmi_unified_apf_tlv.c",
        ],
    },
    "CONFIG_QCACLD_FEATURE_BTC_CHAIN_MODE": {
        True: [
            "core/hdd/src/wlan_hdd_btc_chain_mode.c",
        ],
    },
    "CONFIG_QCACLD_FEATURE_COEX_CONFIG": {
        True: [
            "core/hdd/src/wlan_hdd_coex_config.c",
        ],
    },
    "CONFIG_QCACLD_FEATURE_FW_STATE": {
        True: [
            "core/hdd/src/wlan_hdd_fw_state.c",
            "core/wma/src/wma_fw_state.c",
        ],
    },
    "CONFIG_QCACLD_FEATURE_GREEN_AP": {
        True: [
            "cmn/target_if/green_ap/src/target_if_green_ap.c",
            "cmn/umac/green_ap/core/src/wlan_green_ap_main.c",
            "cmn/umac/green_ap/dispatcher/src/wlan_green_ap_api.c",
            "cmn/umac/green_ap/dispatcher/src/wlan_green_ap_ucfg_api.c",
            "core/hdd/src/wlan_hdd_green_ap.c",
        ],
    },
    "CONFIG_QCACLD_FEATURE_HW_CAPABILITY": {
        True: [
            "core/hdd/src/wlan_hdd_hw_capability.c",
        ],
    },
    "CONFIG_QCACLD_FEATURE_MPTA_HELPER": {
        True: [
            "core/hdd/src/wlan_hdd_mpta_helper.c",
        ],
    },
    "CONFIG_QCACLD_FEATURE_NAN": {
        True: [
            "cmn/wmi/src/wmi_unified_nan_api.c",
            "cmn/wmi/src/wmi_unified_nan_tlv.c",
            "components/nan/core/src/nan_api.c",
            "components/nan/core/src/nan_main.c",
            "components/nan/dispatcher/src/cfg_nan.c",
            "components/nan/dispatcher/src/nan_ucfg_api.c",
            "components/target_if/nan/src/target_if_nan.c",
            "core/hdd/src/wlan_hdd_nan.c",
            "core/hdd/src/wlan_hdd_nan_datapath.c",
            "core/mac/src/pe/nan/nan_datapath.c",
            "core/sme/src/nan/nan_datapath_api.c",
            "core/wma/src/wma_nan_datapath.c",
            "os_if/nan/src/os_if_nan.c",
        ],
    },
    "CONFIG_QCACLD_FEATURE_SON": {
        True: [
            "cmn/target_if/son/src/target_if_son.c",
            "components/son/dispatcher/src/son_api.c",
            "components/son/dispatcher/src/son_ucfg_api.c",
            "core/hdd/src/wlan_hdd_son.c",
            "os_if/son/src/os_if_son.c",
        ],
    },
    "CONFIG_QCACLD_WLAN_CONNECTIVITY_DIAG_EVENT": {
        True: [
            "core/hdd/src/wlan_hdd_connectivity_logging.c",
            "components/cmn_services/logging/src/wlan_connectivity_logging.c",
        ],
    },
    "CONFIG_QCACLD_WLAN_CONNECTIVITY_DIAG_LOGGING": {
        True: [
            "core/hdd/src/wlan_hdd_connectivity_logging.c",
            "components/cmn_services/logging/src/wlan_connectivity_logging.c",
        ],
    },
    "CONFIG_QCACLD_WLAN_LFR2": {
        True: [
            "cmn/umac/mlme/connection_mgr/core/src/wlan_cm_host_roam.c",
            "cmn/umac/mlme/connection_mgr/core/src/wlan_cm_roam_util.c",
            "components/umac/mlme/connection_mgr/core/src/wlan_cm_host_roam_preauth.c",
            "components/umac/mlme/connection_mgr/core/src/wlan_cm_host_util.c",
            "core/mac/src/pe/lim/lim_ft_preauth.c",
            "core/mac/src/pe/lim/lim_process_mlm_host_roam.c",
            "core/mac/src/pe/lim/lim_reassoc_utils.c",
            "core/mac/src/pe/lim/lim_roam_timer_utils.c",
            "core/mac/src/pe/lim/lim_send_frames_host_roam.c",
        ],
    },
   "CONFIG_QCACLD_WLAN_LFR3": {
        True: [
            "cmn/umac/mlme/connection_mgr/core/src/wlan_cm_roam_util.c",
            "components/umac/mlme/connection_mgr/core/src/wlan_cm_roam_fw_sync.c",
            "components/umac/mlme/connection_mgr/core/src/wlan_cm_roam_offload_event.c",
        ],
    },
    "LEGACY_CONFIG_QCA_SUPPORT_TX_THROTTLE": {
        False: [
            "core/dp/txrx/ol_tx_throttle.c",
        ],
    },
    "CONFIG_QCA_WIFI_FTM": {
        True: [
            "cmn/ftm/core/src/wlan_ftm_svc.c",
            "cmn/ftm/dispatcher/src/wlan_ftm_init_deinit.c",
            "cmn/ftm/dispatcher/src/wlan_ftm_ucfg_api.c",
            "cmn/target_if/ftm/src/target_if_ftm.c",
        ],
    },
    "CONFIG_QCA_WIFI_FTM_NL80211": {
        True: [
            "cmn/os_if/linux/ftm/src/wlan_cfg80211_ftm.c",
        ],
    },
    "CONFIG_QCA_WIFI_SDIO": {
        True: [
            "core/pld/src/pld_sdio.c",
        ],
    },
    "CONFIG_QCOM_TDLS": {
        True: [
            "components/target_if/tdls/src/target_if_tdls.c",
            "components/tdls/core/src/wlan_tdls_cmds_process.c",
            "components/tdls/core/src/wlan_tdls_ct.c",
            "components/tdls/core/src/wlan_tdls_main.c",
            "components/tdls/core/src/wlan_tdls_mgmt.c",
            "components/tdls/core/src/wlan_tdls_peer.c",
            "components/tdls/dispatcher/src/wlan_tdls_api.c",
            "components/tdls/dispatcher/src/wlan_tdls_cfg.c",
            "components/tdls/dispatcher/src/wlan_tdls_tgt_api.c",
            "components/tdls/dispatcher/src/wlan_tdls_ucfg_api.c",
            "components/tdls/dispatcher/src/wlan_tdls_utils_api.c",
            "core/hdd/src/wlan_hdd_tdls.c",
            "core/mac/src/pe/lim/lim_process_tdls.c",
            "os_if/tdls/src/wlan_cfg80211_tdls.c",
        ],
    },
    "CONFIG_QDF_TEST": {
        True: [
            "cmn/qdf/test/qdf_delayed_work_test.c",
            "cmn/qdf/test/qdf_hashtable_test.c",
            "cmn/qdf/test/qdf_periodic_work_test.c",
            "cmn/qdf/test/qdf_ptr_hash_test.c",
            "cmn/qdf/test/qdf_slist_test.c",
            "cmn/qdf/test/qdf_talloc_test.c",
            "cmn/qdf/test/qdf_tracker_test.c",
            "cmn/qdf/test/qdf_types_test.c",
        ],
    },
    "CONFIG_RHINE": {
        True: [
            # TODO: how to handle Kbuild logic
            #ifneq (y,$(filter y,$(CONFIG_LITHIUM) $(CONFIG_BERYLLIUM) $(CONFIG_RHINE)))
            #ifeq (y,$(filter y,$(CONFIG_LITHIUM) $(CONFIG_BERYLLIUM) $(CONFIG_RHINE)))
            "cmn/dp/cmn_dp_api/dp_ratetable.c",
            "cmn/hal/wifi3.0/hal_reo.c",
            "cmn/hal/wifi3.0/hal_srng.c",
            "cmn/wlan_cfg/wlan_cfg.c",
        ],
    },
   "CONFIG_SMP": {
        True: [
            "cmn/qdf/linux/src/qdf_cpuhp.c",
            "cmn/qdf/src/qdf_cpuhp.c",
        ],
    },
    "CONFIG_SM_ENG_HIST": {
        True: [
            "cmn/umac/cmn_services/sm_engine/src/wlan_sm_engine_dbg.c",
        ],
    },
    "CONFIG_SNOC_FW_SIM": {
        True: [
            "core/pld/src/pld_snoc_fw_sim.c",
        ],
    },
    "CONFIG_WDI_EVENT_ENABLE": {
        True: [
            "core/dp/txrx/ol_txrx_event.c",
            "cmn/dp/wifi3.0/dp_wdi_event.c",
        ],
   },
    "CONFIG_WIFI_MONITOR_SUPPORT": {
        True: [
            "cmn/dp/wifi3.0/monitor/1.0/dp_mon_1.0.c",
            "cmn/dp/wifi3.0/monitor/1.0/dp_mon_filter_1.0.c",
            "cmn/dp/wifi3.0/monitor/1.0/dp_rx_mon_dest_1.0.c",
            "cmn/dp/wifi3.0/monitor/1.0/dp_rx_mon_status_1.0.c",
            "cmn/dp/wifi3.0/monitor/dp_mon.c",
            "cmn/dp/wifi3.0/monitor/dp_mon_filter.c",
            "cmn/dp/wifi3.0/monitor/dp_rx_mon.c",
        ],
    },
    "CONFIG_WIFI_MONITOR_SUPPORT_Y_WLAN_TX_MON_2_0": {
        True: [
            "cmn/dp/wifi3.0/monitor/2.0/dp_mon_2.0.c",
            "cmn/dp/wifi3.0/monitor/2.0/dp_mon_filter_2.0.c",
            "cmn/dp/wifi3.0/monitor/2.0/dp_tx_mon_2.0.c",
            "cmn/dp/wifi3.0/monitor/2.0/dp_tx_mon_status_2.0.c",
        ],
    },
    "CONFIG_WIFI_POS_CONVERGED": {
        True: [
            "cmn/os_if/linux/wifi_pos/src/os_if_wifi_pos.c",
            "cmn/target_if/wifi_pos/src/target_if_wifi_pos.c",
            "cmn/umac/wifi_pos/src/wifi_pos_api.c",
            "cmn/umac/wifi_pos/src/wifi_pos_main.c",
            "cmn/umac/wifi_pos/src/wifi_pos_ucfg.c",
            "cmn/umac/wifi_pos/src/wifi_pos_utils.c",
        ],
    },
    "CONFIG_WLAN_BCN_RECV_FEATURE": {
        True: [
            "core/hdd/src/wlan_hdd_bcn_recv.c",
        ],
    },
    "CONFIG_WLAN_BMISS": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs_bmiss.c",
        ],
    },
    "CONFIG_WLAN_BOOTUP_MARKER": {
        True: [
            "core/hdd/src/wlan_hdd_bootup_marker.c",
        ],
    },
    "CONFIG_WLAN_CFR_ADRASTEA": {
        True: [
            "cmn/target_if/cfr/src/target_if_cfr_adrastea.c",
        ],
    },
    "CONFIG_WLAN_CFR_DBR": {
        True: [
            "cmn/target_if/cfr/src/target_if_cfr_dbr.c",
        ],
    },
    "CONFIG_WLAN_CFR_ENABLE": {
        True: [
            "cmn/target_if/cfr/src/target_if_cfr.c",
            "cmn/target_if/cfr/src/target_if_cfr_6490.c",
            "cmn/umac/cfr/core/src/cfr_common.c",
            "cmn/umac/cfr/dispatcher/src/wlan_cfr_tgt_api.c",
            "cmn/umac/cfr/dispatcher/src/wlan_cfr_ucfg_api.c",
            "cmn/umac/cfr/dispatcher/src/wlan_cfr_utils_api.c",
            "cmn/wmi/src/wmi_unified_cfr_api.c",
            "cmn/wmi/src/wmi_unified_cfr_tlv.c",
            "core/hdd/src/wlan_hdd_cfr.c",
        ],
    },
    "CONFIG_WLAN_CONV_SPECTRAL_ENABLE": {
        True: [
            "core/hdd/src/wlan_hdd_spectralscan.c",
            "cmn/spectral/core/spectral_common.c",
            "cmn/spectral/core/spectral_offload.c",
            "cmn/spectral/dispatcher/src/wlan_spectral_tgt_api.c",
            "cmn/spectral/dispatcher/src/wlan_spectral_ucfg_api.c",
            "cmn/spectral/dispatcher/src/wlan_spectral_utils_api.c",
            "cmn/os_if/linux/spectral/src/os_if_spectral_netlink.c",
            "cmn/os_if/linux/spectral/src/wlan_cfg80211_spectral.c",
            "cmn/target_if/spectral/target_if_spectral.c",
            "cmn/target_if/spectral/target_if_spectral_netlink.c",
            "cmn/target_if/spectral/target_if_spectral_phyerr.c",
            "cmn/target_if/spectral/target_if_spectral_sim.c",
        ],
    },
   "CONFIG_WLAN_DEBUGFS": {
        True: [
            "core/hdd/src/wlan_hdd_debugfs.c",
            "core/hdd/src/wlan_hdd_debugfs_config.c",
            "core/hdd/src/wlan_hdd_debugfs_csr.c",
            "core/hdd/src/wlan_hdd_debugfs_offload.c",
            "core/hdd/src/wlan_hdd_debugfs_roam.c",
            "core/hdd/src/wlan_hdd_debugfs_unit_test.c",
            "cmn/qdf/linux/src/qdf_debugfs.c",
        ],
    },
    "CONFIG_WLAN_DEBUG_CRASH_INJECT": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs_crash_inject.c",
        ],
    },
    "CONFIG_WLAN_DFS_MASTER_ENABLE": {
        True: [
            "cmn/target_if/dfs/src/target_if_dfs.c",
            "cmn/umac/dfs/core/src/misc/dfs.c",
            "cmn/umac/dfs/core/src/misc/dfs_nol.c",
            "cmn/umac/dfs/core/src/misc/dfs_process_radar_found_ind.c",
            "cmn/umac/dfs/core/src/misc/dfs_random_chan_sel.c",
            "cmn/umac/dfs/dispatcher/src/wlan_dfs_init_deinit_api.c",
            "cmn/umac/dfs/dispatcher/src/wlan_dfs_lmac_api.c",
            "cmn/umac/dfs/dispatcher/src/wlan_dfs_mlme_api.c",
            "cmn/umac/dfs/dispatcher/src/wlan_dfs_tgt_api.c",
            "cmn/umac/dfs/dispatcher/src/wlan_dfs_ucfg_api.c",
            "cmn/umac/dfs/dispatcher/src/wlan_dfs_utils_api.c",
            "cmn/wmi/src/wmi_unified_dfs_api.c",
        ],
    },
    "CONFIG_WLAN_DIAG_VERSION": {
        True: [
            "cmn/utils/host_diag_log/src/host_diag_log.c",
        ],
    },
   "CONFIG_WLAN_DL_MODES": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs_dl_modes.c",
        ],
    },
    "CONFIG_WLAN_DUMP_IN_PROGRESS": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs_dump_in_progress.c",
        ],
    },
    "CONFIG_WLAN_ENABLE_GPIO_WAKEUP": {
        True: [
            "core/hdd/src/wlan_hdd_gpio_wakeup.c",
        ],
    },
    "CONFIG_WLAN_ENH_CFR_ENABLE": {
        True: [
            "cmn/target_if/cfr/src/target_if_cfr_enh.c",
        ],
    },
    "CONFIG_WLAN_FASTPATH": {
    True: [
        "core/dp/txrx/ol_tx_ll_fastpath.c",
    ],
    False: [
        "core/dp/txrx/ol_tx_ll_legacy.c",
    ],
    },
	"CONFIG_WLAN_TX_FLOW_CONTROL_V2": {
		True: [
			"core/dp/txrx/ol_txrx_flow_control.c",
		],
	},
    "CONFIG_WLAN_FEATURE_11AX": {
        True: [
            "core/hdd/src/wlan_hdd_he.c",
            "core/wma/src/wma_he.c",
        ],
    },
    "CONFIG_WLAN_FEATURE_11BE": {
        True: [
            "core/hdd/src/wlan_hdd_eht.c",
            "core/wma/src/wma_eht.c",
        ],
    },
    "CONFIG_WLAN_FEATURE_11BE_MLO": {
        True: [
            "core/hdd/src/wlan_hdd_mlo.c",
            "core/mac/src/pe/lim/lim_mlo.c",
            "cmn/target_if/mlo_mgr/src/target_if_mlo_mgr.c",
            "cmn/umac/mlo_mgr/src/utils_mlo.c",
            "cmn/umac/mlo_mgr/src/wlan_mlo_mgr_aid.c",
            "cmn/umac/mlo_mgr/src/wlan_mlo_mgr_ap.c",
            "cmn/umac/mlo_mgr/src/wlan_mlo_mgr_cmn.c",
            "cmn/umac/mlo_mgr/src/wlan_mlo_mgr_main.c",
            "cmn/umac/mlo_mgr/src/wlan_mlo_mgr_msgq.c",
            "cmn/umac/mlo_mgr/src/wlan_mlo_mgr_peer.c",
            "cmn/umac/mlo_mgr/src/wlan_mlo_mgr_peer_list.c",
            "cmn/umac/mlo_mgr/src/wlan_mlo_mgr_primary_umac.c",
            "cmn/umac/mlo_mgr/src/wlan_mlo_mgr_sta.c",
            "cmn/wmi/src/wmi_unified_11be_api.c",
            "cmn/wmi/src/wmi_unified_11be_tlv.c",
            "components/umac/mlme/mlo_mgr/src/wlan_mlo_mgr_roam.c",
        ],
    },
   "CONFIG_WLAN_FEATURE_ACTION_OUI": {
        True: [
            "components/action_oui/core/src/wlan_action_oui_main.c",
            "components/action_oui/core/src/wlan_action_oui_parse.c",
            "components/action_oui/dispatcher/src/wlan_action_oui_tgt_api.c",
            "components/action_oui/dispatcher/src/wlan_action_oui_ucfg_api.c",
            "components/target_if/action_oui/src/target_if_action_oui.c",
            "cmn/wmi/src/wmi_unified_action_oui_tlv.c",
        ],
    },
    "CONFIG_WLAN_FEATURE_BMI": {
        True: [
            "cmn/hif/src/ce/ce_bmi.c",
            "cmn/hif/src/sdio/hif_bmi_reg_access.c",
            "core/bmi/src/bmi.c",
            "core/bmi/src/bmi_1.c",
            "core/bmi/src/ol_fw.c",
            "core/bmi/src/ol_fw_common.c",
        ],
    },
    "CONFIG_WLAN_FEATURE_COAP": {
        True: [
            "components/coap/core/src/wlan_coap_main.c",
            "components/coap/dispatcher/src/wlan_coap_tgt_api.c",
            "components/coap/dispatcher/src/wlan_coap_ucfg_api.c",
            "components/target_if/coap/src/target_if_coap.c",
            "components/wmi/src/wmi_unified_coap_tlv.c",
            "core/hdd/src/wlan_hdd_coap.c",
            "os_if/coap/src/wlan_cfg80211_coap.c",
        ],
    },
    "CONFIG_WLAN_FEATURE_DFS_OFFLOAD": {
       True: [
            "cmn/target_if/dfs/src/target_if_dfs_full_offload.c",
        ],
        #TODO: need a separate flag, otherwise the below are added
        #      even when DFS is disabled
        False: [
            "cmn/target_if/dfs/src/target_if_dfs_partial_offload.c",
            "cmn/umac/dfs/core/src/filtering/dfs_bindetects.c",
            "cmn/umac/dfs/core/src/filtering/dfs_debug.c",
            "cmn/umac/dfs/core/src/filtering/dfs_fcc_bin5.c",
            "cmn/umac/dfs/core/src/filtering/dfs_init.c",
            "cmn/umac/dfs/core/src/filtering/dfs_misc.c",
            "cmn/umac/dfs/core/src/filtering/dfs_partial_offload_radar.c",
            "cmn/umac/dfs/core/src/filtering/dfs_phyerr_tlv.c",
            "cmn/umac/dfs/core/src/filtering/dfs_process_phyerr.c",
            "cmn/umac/dfs/core/src/filtering/dfs_process_radarevent.c",
            "cmn/umac/dfs/core/src/filtering/dfs_radar.c",
            "cmn/umac/dfs/core/src/filtering/dfs_staggered.c",
            "cmn/umac/dfs/core/src/misc/dfs_filter_init.c",
        ],
    },
    "CONFIG_WLAN_FEATURE_DISA": {
        True: [
            "components/disa/core/src/wlan_disa_main.c",
            "components/disa/dispatcher/src/wlan_disa_obj_mgmt_api.c",
            "components/disa/dispatcher/src/wlan_disa_tgt_api.c",
            "components/disa/dispatcher/src/wlan_disa_ucfg_api.c",
            "components/target_if/disa/src/target_if_disa.c",
            "core/hdd/src/wlan_hdd_disa.c",
        ],
    },
    "CONFIG_WLAN_FEATURE_DSRC": {
        True: [
            "components/ocb/core/src/wlan_ocb_main.c",
            "components/ocb/dispatcher/src/wlan_ocb_tgt_api.c",
            "components/ocb/dispatcher/src/wlan_ocb_ucfg_api.c",
            "components/target_if/ocb/src/target_if_ocb.c",
            "core/hdd/src/wlan_hdd_ocb.c",
            "core/wma/src/wma_ocb.c",
            "cmn/wmi/src/wmi_unified_ocb_api.c",
            "cmn/wmi/src/wmi_unified_ocb_tlv.c",
        ],
    },
   "CONFIG_WLAN_FEATURE_FILS": {
        True: [
            "core/mac/src/pe/lim/lim_process_fils.c",
        ],
    },
    "CONFIG_WLAN_FEATURE_FIPS": {
        True: [
            "core/hdd/src/wlan_hdd_fips.c",
            "core/wma/src/wma_fips_api.c",
        ],
    },
    "CONFIG_WLAN_FEATURE_ICMP_OFFLOAD": {
        True: [
            "components/pmo/core/src/wlan_pmo_icmp.c",
            "components/pmo/dispatcher/src/wlan_pmo_tgt_icmp.c",
            "components/target_if/pmo/src/target_if_pmo_icmp.c",
        ],
    },
    "CONFIG_WLAN_FEATURE_LINK_LAYER_STATS": {
        True: [
            "core/hdd/src/wlan_hdd_debugfs_llstat.c",
        ],
    },
    "CONFIG_WLAN_FEATURE_LPSS": {
        True: [
            "core/hdd/src/wlan_hdd_lpass.c",
        ],
    },
    "CONFIG_WLAN_FEATURE_MDNS_OFFLOAD": {
        True: [
            "core/hdd/src/wlan_hdd_mdns_offload.c",
        ],
    },
    "CONFIG_WLAN_FEATURE_MEDIUM_ASSESS": {
        True: [
            "core/hdd/src/wlan_hdd_medium_assess.c",
        ],
    },
    "CONFIG_WLAN_FEATURE_MIB_STATS": {
        True: [
            "core/hdd/src/wlan_hdd_debugfs_mibstat.c",
        ],
    },
    "CONFIG_WLAN_FEATURE_PACKET_FILTERING": {
        True: [
            "components/pmo/core/src/wlan_pmo_pkt_filter.c",
            "components/pmo/dispatcher/src/wlan_pmo_tgt_pkt_filter.c",
            "components/target_if/pmo/src/target_if_pmo_pkt_filter.c",
            "core/hdd/src/wlan_hdd_packet_filter.c",
        ],
    },
    "CONFIG_WLAN_FEATURE_PKT_CAPTURE": {
        True: [
            "components/pkt_capture/core/src/wlan_pkt_capture_data_txrx.c",
            "components/pkt_capture/core/src/wlan_pkt_capture_main.c",
            "components/pkt_capture/core/src/wlan_pkt_capture_mgmt_txrx.c",
            "components/pkt_capture/core/src/wlan_pkt_capture_mon_thread.c",
            "components/pkt_capture/dispatcher/src/wlan_pkt_capture_api.c",
            "components/pkt_capture/dispatcher/src/wlan_pkt_capture_tgt_api.c",
            "components/pkt_capture/dispatcher/src/wlan_pkt_capture_ucfg_api.c",
            "components/target_if/pkt_capture/src/target_if_pkt_capture.c",
            "os_if/pkt_capture/src/os_if_pkt_capture.c",
        ],
    },
    "CONFIG_WLAN_FEATURE_RX_BUFFER_POOL": {
        True: [
            "cmn/dp/wifi3.0/dp_rx_buffer_pool.c",
        ],
    },
    "CONFIG_WLAN_FEATURE_TWT": {
        True: [
            "cmn/os_if/linux/twt/src/osif_twt_req.c",
            "cmn/os_if/linux/twt/src/osif_twt_rsp.c",
            "cmn/target_if/twt/src/target_if_twt.c",
            "cmn/target_if/twt/src/target_if_twt_cmd.c",
            "cmn/target_if/twt/src/target_if_twt_evt.c",
            "cmn/umac/twt/core/src/wlan_twt_common.c",
            "cmn/umac/twt/core/src/wlan_twt_objmgr.c",
            "cmn/umac/twt/dispatcher/src/wlan_twt_api.c",
            "cmn/umac/twt/dispatcher/src/wlan_twt_tgt_if_rx_api.c",
            "cmn/umac/twt/dispatcher/src/wlan_twt_tgt_if_tx_api.c",
            "cmn/umac/twt/dispatcher/src/wlan_twt_ucfg_api.c",
            "cmn/wmi/src/wmi_unified_twt_api.c",
            "cmn/wmi/src/wmi_unified_twt_tlv.c",
            "components/target_if/twt/src/target_if_ext_twt_cmd.c",
            "components/target_if/twt/src/target_if_ext_twt_evt.c",
            "components/umac/twt/core/src/wlan_twt_cfg.c",
            "components/umac/twt/core/src/wlan_twt_main.c",
            "components/umac/twt/dispatcher/src/wlan_twt_cfg_ext_api.c",
            "components/umac/twt/dispatcher/src/wlan_twt_tgt_if_ext_rx_api.c",
            "components/umac/twt/dispatcher/src/wlan_twt_tgt_if_ext_tx_api.c",
            "components/umac/twt/dispatcher/src/wlan_twt_ucfg_ext_api.c",
            "core/hdd/src/wlan_hdd_twt.c",
            "os_if/twt/src/osif_twt_ext_req.c",
            "os_if/twt/src/osif_twt_ext_rsp.c",
            "os_if/twt/src/osif_twt_ext_util.c",
            # TODO: rest being removed by David's TWT change
            #            "components/mlme/core/src/wlan_mlme_twt_api.c",
            "components/mlme/dispatcher/src/wlan_mlme_twt_ucfg_api.c",
            "core/wma/src/wma_twt.c",
        ],
    },
    "CONFIG_WLAN_FREQ_LIST": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs_get_freq_for_pwr.c",
        ],
    },
    "CONFIG_WLAN_FW_OFFLOAD": {
        True: [
            "components/fw_offload/core/src/wlan_fw_offload_main.c",
            "components/fw_offload/dispatcher/src/wlan_fwol_tgt_api.c",
            "components/fw_offload/dispatcher/src/wlan_fwol_ucfg_api.c",
            "components/target_if/fw_offload/src/target_if_fwol.c",
            "os_if/fw_offload/src/os_if_fwol.c",
            "cmn/wmi/src/wmi_unified_fwol_api.c",
            "cmn/wmi/src/wmi_unified_fwol_tlv.c",
        ],
    },
    "CONFIG_WLAN_GTX_BW_MASK": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs_gtx_bw_mask.c",
        ],
    },
    "CONFIG_WLAN_HANG_EVENT": {
        True: [
            "core/hdd/src/wlan_hdd_hang_event.c",
            "cmn/htc/htc_hang_event.c",
            "cmn/qdf/src/qdf_hang_event_notifier.c",
            "cmn/qdf/src/qdf_notifier.c",
            "cmn/wmi/src/wmi_hang_event.c",
        ],
    },
    "CONFIG_WLAN_LRO": {
        True: [
            "cmn/qdf/linux/src/qdf_lro.c",
        ],
    },
    "CONFIG_WLAN_MWS_INFO_DEBUGFS": {
        True: [
            "core/hdd/src/wlan_hdd_debugfs_coex.c",
            "core/wma/src/wma_coex.c",
        ],
    },
    "CONFIG_WLAN_NAPI": {
        True: [
            "core/hdd/src/wlan_hdd_napi.c",
            "cmn/hif/src/hif_irq_affinity.c",
            "cmn/hif/src/hif_napi.c",
        ],
    },
    "CONFIG_WLAN_NS_OFFLOAD": {
        True: [
            "components/pmo/core/src/wlan_pmo_ns.c",
            "components/pmo/dispatcher/src/wlan_pmo_tgt_ns.c",
            "components/target_if/pmo/src/target_if_pmo_ns.c",
        ],
    },
    "CONFIG_WLAN_NUD_TRACKING": {
        True: [
            "core/hdd/src/wlan_hdd_nud_tracking.c",
        ],
    },
    "CONFIG_WLAN_OBJMGR_DEBUG": {
        True: [
            "cmn/umac/cmn_services/obj_mgr/src/wlan_objmgr_debug.c",
        ],
    },
    "CONFIG_WLAN_REASSOC": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs_reassoc.c",
        ],
    },
    "CONFIG_WLAN_SCAN_DISABLE": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs_scan_disable.c",
        ],
    },
    "CONFIG_WLAN_STREAMFS": {
        True: [
            "cmn/qdf/linux/src/qdf_streamfs.c",
        ],
    },
    "CONFIG_WLAN_SYNC_TSF": {
        True: [
            "core/hdd/src/wlan_hdd_tsf.c",
        ],
    },
    "CONFIG_WLAN_SYSFS": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs.c",
            "core/hdd/src/wlan_hdd_sysfs_unit_test.c",
            "core/hdd/src/wlan_hdd_sysfs_modify_acl.c",
            "core/hdd/src/wlan_hdd_sysfs_policy_mgr.c",
            "core/hdd/src/wlan_hdd_sysfs_dp_aggregation.c",
        ],
    },
    "CONFIG_WLAN_SYSFS_CHANNEL": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs_channel.c",
        ],
    },
    "CONFIG_WLAN_SYSFS_CONNECT_INFO": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs_connect_info.c",
        ],
    },
    "CONFIG_WLAN_SYSFS_DCM": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs_dcm.c",
        ],
    },
    "CONFIG_WLAN_SYSFS_DP_STATS": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs_txrx_stats_console.c",
        ],
    },
    "CONFIG_WLAN_SYSFS_DP_TRACE": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs_dp_trace.c",
        ],
    },
    "CONFIG_WLAN_SYSFS_FW_MODE_CFG": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs_fw_mode_config.c",
        ],
    },
    "CONFIG_WLAN_SYSFS_HE_BSS_COLOR": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs_he_bss_color.c",
        ],
    },
    "CONFIG_WLAN_SYSFS_MEM_STATS": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs_mem_stats.c",
        ],
    },
    "CONFIG_WLAN_SYSFS_MONITOR_MODE_CHANNEL": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs_monitor_mode_channel.c",
        ],
    },
    "CONFIG_WLAN_SYSFS_RADAR": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs_radar.c",
        ],
    },
    "CONFIG_WLAN_SYSFS_RANGE_EXT": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs_range_ext.c",
        ],
    },
    "CONFIG_WLAN_SYSFS_RTS_CTS": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs_rts_cts.c",
        ],
    },
    "CONFIG_WLAN_SYSFS_SCAN_CFG": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs_scan_config.c",
        ],
    },
    "CONFIG_WLAN_SYSFS_STATS": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs_stats.c",
        ],
    },
    "CONFIG_WLAN_SYSFS_STA_INFO": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs_sta_info.c",
        ],
    },
    "CONFIG_WLAN_SYSFS_TDLS_PEERS": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs_tdls_peers.c",
        ],
    },
    "CONFIG_WLAN_SYSFS_TEMPERATURE": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs_temperature.c",
        ],
    },
    "CONFIG_WLAN_SYSFS_TX_STBC": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs_tx_stbc.c",
        ],
    },
    "CONFIG_WLAN_SYSFS_WLAN_DBG": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs_wlan_dbg.c",
        ],
    },
    "CONFIG_WLAN_THERMAL_CFG": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs_thermal_cfg.c",
        ],
    },
    "CONFIG_WLAN_TRACEPOINTS": {
        True: [
            "cmn/qdf/linux/src/qdf_tracepoint.c",
        ],
    },
    "CONFIG_WLAN_TXRX_FW_STATS": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs_txrx_fw_stats.c",
        ],
    },
    "CONFIG_WLAN_TXRX_FW_ST_RST": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs_txrx_fw_st_rst.c",
        ],
    },
    "CONFIG_WLAN_TXRX_STATS": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs_txrx_stats.c",
        ],
    },
   "CONFIG_WLAN_WBUFF": {
        True: [
            "cmn/wbuff/src/wbuff.c",
        ],
    },
    "CONFIG_WLAN_WOWL_ADD_PTRN": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs_wowl_add_ptrn.c",
        ],
    },
    "CONFIG_WLAN_WOWL_DEL_PTRN": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs_wowl_del_ptrn.c",
        ],
    },
    "CONFIG_WLAN_WOW_ITO": {
        True: [
            "core/hdd/src/wlan_hdd_sysfs_wow_ito.c",
        ],
    },
    "CONFIG_WMI_BCN_OFFLOAD": {
        True: [
            "cmn/wmi/src/wmi_unified_bcn_api.c",
            "cmn/wmi/src/wmi_unified_bcn_tlv.c",
        ],
    },
    "CONFIG_WMI_CONCURRENCY_SUPPORT": {
        True: [
            "cmn/wmi/src/wmi_unified_concurrency_api.c",
            "cmn/wmi/src/wmi_unified_concurrency_tlv.c",
        ],
    },
    "CONFIG_WMI_ROAM_SUPPORT": {
        True: [
            "components/wmi/src/wmi_unified_roam_api.c",
            "components/wmi/src/wmi_unified_roam_tlv.c",
        ],
    },
    "CONFIG_WMI_STA_SUPPORT": {
        True: [
            "cmn/wmi/src/wmi_unified_sta_api.c",
            "cmn/wmi/src/wmi_unified_sta_tlv.c",
        ],
    },
	"CONFIG_WLAN_TX_FLOW_CONTROL_LEGACY": {
		True: [
			"core/dp/txrx/ol_txrx_legacy_flow_control.c"
		],
	}
}

def _define_module_for_target_variant_chipset(target, variant, chipset):
    tvc = "{}_{}_{}".format(target, variant, chipset)
    tv = "{}_{}".format(target, variant)
    name = "{}_qca_cld_{}".format(tv, chipset)
    hw = _chipset_hw_map[chipset]
    chipset_ipaths = _chipset_header_map[chipset]
    hw_ipaths = _hw_header_map[hw]

    ipaths = chipset_ipaths + hw_ipaths + _fixed_ipaths

    iglobs = []
    for i in _fixed_includes:
        iglobs.append(i)
    for i in ipaths:
        iglobs.append("{}/*.h".format(i))
        iglobs.append("{}/**/*.h".format(i))
    for i in _private_ipaths:
        iglobs.append("{}/*.h".format(i))
        iglobs.append("{}/**/*.h".format(i))
    copts = []
    for i in _fixed_includes:
        copts.append("-include")
        copts.append(i)

    feature_grep_map = [
        {
            "pattern": "walt_get_cpus_taken",
            "file": "kernel/sched/walt/walt.c",
            "flag": "WALT_GET_CPU_TAKEN_SUPPORT",
        },
        {
            "pattern": "nl80211_validate_key_link_id",
            "file": "net/wireless/nl80211.c",
            "flag": "CFG80211_MLO_KEY_OPERATION_SUPPORT",
        },
        {
            "pattern": "struct link_station_parameters",
            "file": "include/net/cfg80211.h",
            "flag": "CFG80211_LINK_STA_PARAMS_PRESENT",
        },
        {
            "pattern": "NL80211_EXT_FEATURE_PUNCT",
            "file": "include/uapi/linux/nl80211.h",
            "flag": "NL80211_EXT_FEATURE_PUNCT_SUPPORT",
        },
        {
            "pattern": "unsigned int link_id, u16 punct_bitmap",
            "file": "include/net/cfg80211.h",
            "flag": "CFG80211_RU_PUNCT_NOTIFY",
        },
        {
            "pattern": "NL80211_EXT_FEATURE_AUTH_AND_DEAUTH_RANDOM_TA",
            "file": "include/uapi/linux/nl80211.h",
            "flag": "CFG80211_EXT_FEATURE_AUTH_AND_DEAUTH_RANDOM_TA",
        },
    ]

    cmd = 'touch "$@"\n'
    for feature_grep in feature_grep_map:
        cmd += """
          if grep -qF "{pattern}" $(location //msm-kernel:{file}); then
            echo "#define {flag} (1)" >> "$@"
          fi
        """.format(
            pattern = feature_grep["pattern"],
            file = feature_grep["file"],
            flag = feature_grep["flag"],
        )

    grepSrcFiles = []
    for e in feature_grep_map:
        grepSrcFiles.append("//msm-kernel:{}".format(e["file"]))

    depsetSrc = depset(grepSrcFiles)
    native.genrule(
        name = "{}_grep_defines".format(tvc),
        outs = ["configs/grep_defines_{}.h".format(tvc)],
        srcs = depsetSrc.to_list(),
        cmd = cmd,
    )

    copts.append("-include")
    copts.append("$(location :{}_grep_defines)".format(tvc))

    native.genrule(
        name = "configs/{}_defconfig_generate_consolidate".format(tvc),
        outs = ["configs/{}_defconfig.generated_consolidate".format(tvc)],
        srcs = [
            "configs/{}_gki_{}_defconfig".format(target, chipset),
            "configs/{}_consolidate_{}_defconfig".format(target, chipset),
        ],
        cmd = "cat $(SRCS) > $@",
    )
    native.genrule(
        name = "configs/{}_defconfig_generate_gki".format(tvc),
        outs = ["configs/{}_defconfig.generated_gki".format(tvc)],
        srcs = [
            "configs/{}_gki_{}_defconfig".format(target, chipset),
        ],
        cmd = "cat $(SRCS) > $@",
    )

    srcs = native.glob(iglobs) + _fixed_srcs

    if chipset == "wlan":
        out = "{}.ko".format(chipset.replace("-", "_"))
    else:
        out = "qca_cld3_{}.ko".format(chipset.replace("-", "_"))
    kconfig = "Kconfig"
    defconfig = ":configs/{}_defconfig_generate_{}".format(tvc, variant)

    print("name= ", name)
    print("hw= ", hw)
    print("ipaths= ", ipaths)
    print("srcs= ", srcs)
    print("conditional_srcs= ",_conditional_srcs)
    print("out=", out)
    print("iglobs= ", iglobs)
    print("copts= ", copts)
    print("kconfig= ", kconfig)
    print("defconfig= ", defconfig)
    print("name = ", "{}_modules_dist".format(tvc))
    ddk_module(
        name = name,
        srcs = srcs + [":{}_grep_defines".format(tvc)],
        includes = ipaths + ["."],
        kconfig = kconfig,
        defconfig = defconfig,
        conditional_srcs = _conditional_srcs,
        copts = copts,
        out = out,
        kernel_build = "//msm-kernel:{}".format(tv),
        deps = [
            "//vendor/qcom/opensource/wlan/platform:{}_icnss2".format(tv),
            "//vendor/qcom/opensource/wlan/platform:{}_cnss_prealloc".format(tv),
            "//vendor/qcom/opensource/wlan/platform:{}_cnss_utils".format(tv),
            "//vendor/qcom/opensource/wlan/platform:{}_cnss_nl".format(tv),
            "//msm-kernel:all_headers",
            "//vendor/qcom/opensource/wlan/platform:wlan-platform-headers",
        ],
    )

def define_dist(target, variant, chipsets):
    tv = "{}_{}".format(target, variant)
    dataList = []
    for c in chipsets:
        tvc = "{}_{}_{}".format(target, variant, c)
        name = "{}_qca_cld_{}".format(tv, c)
        dataList.append(":{}".format(name))
        copy_to_dist_dir(
            name = "{}_modules_dist".format(tvc),
            data =  [":{}".format(name)],
            dist_dir = "out/target/product/{}/dlkm/lib/modules/".format(target),
            flat = True,
            wipe_dist_dir = False,
            allow_duplicate_filenames = False,
            mode_overrides = {"**/*": "644"},
            log = "info",
        )
    if target == "blair" or target == "monaco":
        return
    copy_to_dist_dir(
        name = "{}_all_modules_dist".format(tv),
        data = dataList,
        dist_dir = "out/target/product/{}/dlkm/lib/modules/".format(target),
        flat = True,
        wipe_dist_dir = False,
        allow_duplicate_filenames = False,
        mode_overrides = {"**/*": "644"},
        log = "info",
    )

def define_modules():
    for (t, v) in get_all_variants():
        chipsets = _target_chipset_map.get(t)
        if chipsets:
            for c in chipsets:
                _define_module_for_target_variant_chipset(t, v, c)
            define_dist(t, v, chipsets)
