# We can build either as part of a standalone Kernel build or as
# an external module.  Determine which mechanism is being used
ifeq ($(MODNAME),)
	KERNEL_BUILD := 1
else
	KERNEL_BUILD := 0
endif

ifeq ($(CONFIG_CNSS_QCA6290), y)
	CONFIG_LITHIUM := y
	CONFIG_WLAN_FEATURE_11AX := y
endif

ifeq ($(CONFIG_CLD_HL_SDIO_CORE), y)
	CONFIG_QCA_WIFI_SDIO := 1
endif

ifeq ($(CONFIG_QCA_WIFI_SDIO), 1)
	CONFIG_ROME_IF = sdio
endif

ifdef CONFIG_ICNSS
	CONFIG_ROME_IF = snoc
endif

ifeq (y,$(findstring y,$(CONFIG_CNSS) $(CONFIG_CNSS2)))
ifndef CONFIG_ROME_IF
	#use pci as default interface
	CONFIG_ROME_IF = pci
endif
endif

ifeq ($(KERNEL_BUILD),1)
	# These are provided in external module based builds
	# Need to explicitly define for Kernel-based builds
	MODNAME := wlan
	WLAN_ROOT := drivers/staging/qcacld-3.0
	WLAN_COMMON_ROOT := ../qca-wifi-host-cmn
	WLAN_COMMON_INC := $(WLAN_ROOT)/$(WLAN_COMMON_ROOT)
endif

# Make WLAN as open-source driver by default
WLAN_OPEN_SOURCE := 1

ifeq ($(KERNEL_BUILD), 0)
	# These are configurable via Kconfig for kernel-based builds
	# Need to explicitly configure for Android-based builds

	ifeq ($(CONFIG_ARCH_MDM9630), y)
	CONFIG_MOBILE_ROUTER := y
	endif

	ifeq ($(CONFIG_ARCH_MDM9640), y)
	CONFIG_MOBILE_ROUTER := y
	endif

	ifeq ($(CONFIG_ARCH_SDXHEDGEHOG), y)
	CONFIG_MOBILE_ROUTER := y
	endif

	# As per target team, build is done as follows:
	# Defconfig : build with default flags
	# Slub      : defconfig  + CONFIG_SLUB_DEBUG=y +
	#	      CONFIG_SLUB_DEBUG_ON=y + CONFIG_PAGE_POISONING=y
	# Perf      : Using appropriate msmXXXX-perf_defconfig
	#
	# Shipment builds (user variants) should not have any debug feature
	# enabled. This is identified using 'TARGET_BUILD_VARIANT'. Slub builds
	# are identified using the CONFIG_SLUB_DEBUG_ON configuration. Since
	# there is no other way to identify defconfig builds, QCOMs internal
	# representation of perf builds (identified using the string 'perf'),
	# is used to identify if the build is a slub or defconfig one. This
	# way no critical debug feature will be enabled for perf and shipment
	# builds. Other OEMs are also protected using the TARGET_BUILD_VARIANT
	# config.
	ifneq ($(TARGET_BUILD_VARIANT),user)
		ifeq ($(CONFIG_LITHIUM), y)
			CONFIG_FEATURE_PKTLOG := n
		else
			CONFIG_FEATURE_PKTLOG := y
		endif
	endif

	#Flag to enable Legacy Fast Roaming2(LFR2)
	CONFIG_QCACLD_WLAN_LFR2 := y
	#Flag to enable Legacy Fast Roaming3(LFR3)
	CONFIG_QCACLD_WLAN_LFR3 := y

	# JB kernel has CPU enablement patches, so enable
	ifeq ($(CONFIG_ROME_IF),pci)
		CONFIG_PRIMA_WLAN_11AC_HIGH_TP := y
	endif
	ifeq ($(CONFIG_ROME_IF),usb)
		CONFIG_PRIMA_WLAN_11AC_HIGH_TP := n
	endif
	ifeq ($(CONFIG_ROME_IF),sdio)
		CONFIG_PRIMA_WLAN_11AC_HIGH_TP := n
	endif

	ifneq ($(CONFIG_MOBILE_ROUTER), y)
	#Flag to enable TDLS feature
	CONFIG_QCOM_TDLS := y
	endif

	ifeq ($(CONFIG_MOBILE_ROUTER), y)
	CONFIG_QCACLD_FEATURE_GREEN_AP := y
	endif
	ifeq ($(CONFIG_ARCH_MSM8998), y)
	CONFIG_QCACLD_FEATURE_GREEN_AP := y
	CONFIG_QCACLD_FEATURE_METERING := y
	endif

	ifeq ($(CONFIG_ARCH_SDM660), y)
	CONFIG_QCACLD_FEATURE_GREEN_AP := y
	CONFIG_QCACLD_FEATURE_METERING := y
	endif

	ifeq ($(CONFIG_ARCH_SDM630), y)
	CONFIG_QCACLD_FEATURE_METERING := y
	endif

	#Flag to enable Fast Transition (11r) feature
	CONFIG_QCOM_VOWIFI_11R := y

	ifneq ($(CONFIG_QCA_CLD_WLAN),)
		ifeq (y,$(findstring y,$(CONFIG_CNSS) $(CONFIG_CNSS2) $(CONFIG_ICNSS)))
		#Flag to enable Protected Managment Frames (11w) feature
		CONFIG_WLAN_FEATURE_11W := y
		#Flag to enable LTE CoEx feature
		CONFIG_QCOM_LTE_COEX := y
			ifneq ($(CONFIG_MOBILE_ROUTER), y)
			#Flag to enable LPSS feature
			CONFIG_WLAN_FEATURE_LPSS := y
			endif
		endif
	endif

	#Flag to enable Protected Managment Frames (11w) feature
	ifeq ($(CONFIG_ROME_IF),usb)
		CONFIG_WLAN_FEATURE_11W := y
	endif
	ifeq ($(CONFIG_ROME_IF),sdio)
		CONFIG_WLAN_FEATURE_11W := y
	endif

	#Flag to enable the tx desc sanity check
	ifeq ($(CONFIG_ROME_IF),usb)
		CONFIG_QCA_TXDESC_SANITY_CHECKS := y
	endif

	ifneq ($(CONFIG_MOBILE_ROUTER), y)
		#Flag to enable NAN
		CONFIG_QCACLD_FEATURE_NAN := y
	endif

	ifneq ($(CONFIG_MOBILE_ROUTER), y)
		#Flag to enable NAN Data path
		CONFIG_WLAN_FEATURE_NAN_DATAPATH := y
		CONFIG_NAN_CONVERGENCE := y
	endif

	#Flag to enable Linux QCMBR feature as default feature
	ifeq ($(CONFIG_ROME_IF),usb)
		CONFIG_LINUX_QCMBR :=y
	endif

	CONFIG_MPC_UT_FRAMEWORK := y

	#Flag to enable offload packets feature
	CONFIG_WLAN_OFFLOAD_PACKETS := y

	#enable TSF get feature
	CONFIG_WLAN_SYNC_TSF := y
	#Enable DSRC feature

	ifeq ($(CONFIG_QCA_WIFI_SDIO), 1)
	CONFIG_WLAN_FEATURE_DSRC := y
	endif

ifneq ($(CONFIG_ROME_IF),sdio)
	#Flag to enable memdump feature
	CONFIG_WLAN_FEATURE_MEMDUMP := n

	#Flag to enable DISA
	CONFIG_WLAN_FEATURE_DISA := y

	#Flag to enable Fast Path feature
	CONFIG_WLAN_FASTPATH := y

	# Flag to enable NAPI
ifeq (y,$(CONFIG_LITHIUM))
	CONFIG_WLAN_NAPI := n
	CONFIG_WLAN_NAPI_DEBUG := n
else
	CONFIG_WLAN_NAPI := y
	CONFIG_WLAN_NAPI_DEBUG := n
endif
	# Flag to enable FW based TX Flow control
	ifeq ($(CONFIG_CNSS_EOS),y)
		CONFIG_WLAN_TX_FLOW_CONTROL_V2 := y
	else
		CONFIG_WLAN_TX_FLOW_CONTROL_V2 := n
	endif

	# Flag to enable LRO (Large Receive Offload)
	ifeq ($(CONFIG_INET_LRO), y)
		CONFIG_WLAN_LRO := y
	else
		CONFIG_WLAN_LRO := n
	endif
endif

ifeq ($(CONFIG_ROME_IF), snoc)
	CONFIG_WLAN_TX_FLOW_CONTROL_V2 := y
endif

	# Flag to enable LFR Subnet Detection
	CONFIG_LFR_SUBNET_DETECTION := y

	# Flag to enable MCC to SCC switch feature
	CONFIG_MCC_TO_SCC_SWITCH := y

endif

#Enable WLAN/Power debugfs feature only if debug_fs is enabled
ifeq ($(CONFIG_DEBUG_FS), y)
       # Flag to enable debugfs. Depends on CONFIG_DEBUG_FS in kernel
       # configuration.
       CONFIG_WLAN_DEBUGFS := y

       CONFIG_WLAN_POWER_DEBUGFS := y
endif

# If not set, assume, Common driver is with in the build tree
WLAN_COMMON_ROOT ?= ../qca-wifi-host-cmn
WLAN_COMMON_INC ?= $(WLAN_ROOT)/$(WLAN_COMMON_ROOT)

ifneq ($(CONFIG_MOBILE_ROUTER), y)
CONFIG_QCOM_ESE := y
endif

# Feature flags which are not (currently) configurable via Kconfig

#Whether to build debug version
BUILD_DEBUG_VERSION := 1

#Enable this flag to build driver in diag version
BUILD_DIAG_VERSION := 1

#Do we panic on bug?  default is to warn
PANIC_ON_BUG := 1

#Enable OL debug and wmi unified functions
CONFIG_ATH_PERF_PWR_OFFLOAD := 1

#Disable packet log
CONFIG_REMOVE_PKT_LOG := 0

#Enable 11AC TX
ifeq ($(CONFIG_ROME_IF),pci)
	CONFIG_ATH_11AC_TXCOMPACT := 1
endif
ifeq ($(CONFIG_ROME_IF),usb)
	CONFIG_ATH_11AC_TXCOMPACT := 0
endif

#Enable OS specific IRQ abstraction
CONFIG_ATH_SUPPORT_SHARED_IRQ := 1

#Enable message based HIF instead of RAW access in BMI
ifeq ($(CONFIG_QCA_WIFI_SDIO), 1)
CONFIG_HIF_MESSAGE_BASED := 0
else
CONFIG_HIF_MESSAGE_BASED := 1
endif

#Enable PCI specific APIS (dma, etc)
ifeq ($(CONFIG_ROME_IF),pci)
	CONFIG_HIF_PCI := 1
endif

#Enable USB specific APIS
ifeq ($(CONFIG_ROME_IF),usb)
	CONFIG_HIF_USB := 1
	CONFIG_PLD_USB_CNSS := y
endif

#Enable SDIO specific APIS
ifeq ($(CONFIG_ROME_IF),sdio)
	CONFIG_HIF_SDIO := 1
endif

#Enable pci read/write config functions
ifeq ($(CONFIG_ROME_IF),pci)
	CONFIG_ATH_PCI := 1
endif

ifeq ($(CONFIG_ROME_IF),snoc)
	CONFIG_HIF_SNOC:= 1
endif

ifneq ($(CONFIG_MOBILE_ROUTER), y)
#Enable IBSS support on CLD
CONFIG_QCA_IBSS_SUPPORT := 1
endif

#Enable power management suspend/resume functionality to PCI
CONFIG_ATH_BUS_PM := 1

#Enable FLOWMAC module support
CONFIG_ATH_SUPPORT_FLOWMAC_MODULE := 0

#Enable spectral support
CONFIG_ATH_SUPPORT_SPECTRAL := 0

#Enable HOST statistics support
CONFIG_SUPPORT_HOST_STATISTICS := 0

#Enable WDI Event support
CONFIG_WDI_EVENT_ENABLE := 1

#Endianess selection
CONFIG_LITTLE_ENDIAN := 1

#Enable TX reclaim support
CONFIG_TX_CREDIT_RECLAIM_SUPPORT := 0

#Enable FTM support
CONFIG_QCA_WIFI_FTM := 1

#Enable Checksum Offload
CONFIG_CHECKSUM_OFFLOAD := 1

#Enable GTK offload
CONFIG_GTK_OFFLOAD := 1

#Enable EXT WOW
ifeq ($(CONFIG_ROME_IF),pci)
	CONFIG_EXT_WOW := 1
endif

#Set this to 1 to catch erroneous Target accesses during debug.
CONFIG_ATH_PCIE_ACCESS_DEBUG := 0

#Enable IPA offload
ifeq ($(CONFIG_IPA), y)
CONFIG_IPA_OFFLOAD := 1
endif

#Enable Signed firmware support for split binary format
CONFIG_QCA_SIGNED_SPLIT_BINARY_SUPPORT := 0

#Enable single firmware binary format
CONFIG_QCA_SINGLE_BINARY_SUPPORT := 0

#Enable collecting target RAM dump after kernel panic
CONFIG_TARGET_RAMDUMP_AFTER_KERNEL_PANIC := 1

#Flag to enable/disable secure firmware feature
CONFIG_FEATURE_SECURE_FIRMWARE := 0

#Flag to enable Stats Ext implementation
CONFIG_FEATURE_STATS_EXT := 1

ifeq ($(CONFIG_CFG80211),y)
HAVE_CFG80211 := 1
else
ifeq ($(CONFIG_CFG80211),m)
HAVE_CFG80211 := 1
else
HAVE_CFG80211 := 0
endif
endif

############ UAPI ############
UAPI_DIR :=	uapi
UAPI_INC :=	-I$(WLAN_ROOT)/$(UAPI_DIR)/linux

############ COMMON ############
COMMON_DIR :=	core/common
COMMON_INC :=	-I$(WLAN_ROOT)/$(COMMON_DIR)

############ HDD ############
HDD_DIR :=	core/hdd
HDD_INC_DIR :=	$(HDD_DIR)/inc
HDD_SRC_DIR :=	$(HDD_DIR)/src

HDD_INC := 	-I$(WLAN_ROOT)/$(HDD_INC_DIR) \
		-I$(WLAN_ROOT)/$(HDD_SRC_DIR)

HDD_OBJS := 	$(HDD_SRC_DIR)/wlan_hdd_assoc.o \
		$(HDD_SRC_DIR)/wlan_hdd_cfg.o \
		$(HDD_SRC_DIR)/wlan_hdd_driver_ops.o \
		$(HDD_SRC_DIR)/wlan_hdd_ftm.o \
		$(HDD_SRC_DIR)/wlan_hdd_hostapd.o \
		$(HDD_SRC_DIR)/wlan_hdd_ioctl.o \
		$(HDD_SRC_DIR)/wlan_hdd_main.o \
		$(HDD_SRC_DIR)/wlan_hdd_oemdata.o \
		$(HDD_SRC_DIR)/wlan_hdd_power.o \
		$(HDD_SRC_DIR)/wlan_hdd_regulatory.o \
		$(HDD_SRC_DIR)/wlan_hdd_request_manager.o \
		$(HDD_SRC_DIR)/wlan_hdd_scan.o \
		$(HDD_SRC_DIR)/wlan_hdd_softap_tx_rx.o \
		$(HDD_SRC_DIR)/wlan_hdd_tx_rx.o \
		$(HDD_SRC_DIR)/wlan_hdd_trace.o \
		$(HDD_SRC_DIR)/wlan_hdd_wext.o \
		$(HDD_SRC_DIR)/wlan_hdd_wmm.o \
		$(HDD_SRC_DIR)/wlan_hdd_wowl.o \
		$(HDD_SRC_DIR)/wlan_hdd_object_manager.o

ifeq ($(CONFIG_WLAN_DEBUGFS), y)
HDD_OBJS += $(HDD_SRC_DIR)/wlan_hdd_debugfs.o
endif

ifeq ($(CONFIG_WLAN_FEATURE_DSRC), y)
HDD_OBJS+=	$(HDD_SRC_DIR)/wlan_hdd_ocb.o
endif

ifeq ($(CONFIG_WLAN_FEATURE_LPSS),y)
HDD_OBJS +=	$(HDD_SRC_DIR)/wlan_hdd_lpass.o
endif

ifeq ($(CONFIG_WLAN_LRO), y)
HDD_OBJS +=     $(HDD_SRC_DIR)/wlan_hdd_lro.o
endif

ifeq ($(CONFIG_WLAN_NAPI), y)
HDD_OBJS +=     $(HDD_SRC_DIR)/wlan_hdd_napi.o
endif

ifeq ($(CONFIG_IPA_OFFLOAD), 1)
HDD_OBJS +=	$(HDD_SRC_DIR)/wlan_hdd_ipa.o
endif

ifeq ($(HAVE_CFG80211),1)
HDD_OBJS +=	$(HDD_SRC_DIR)/wlan_hdd_cfg80211.o \
		$(HDD_SRC_DIR)/wlan_hdd_ext_scan.o \
		$(HDD_SRC_DIR)/wlan_hdd_stats.o \
		$(HDD_SRC_DIR)/wlan_hdd_p2p.o
endif

ifeq ($(CONFIG_QCACLD_FEATURE_GREEN_AP),y)
HDD_OBJS +=	$(HDD_SRC_DIR)/wlan_hdd_green_ap.o
endif

ifeq ($(CONFIG_QCACLD_FEATURE_NAN),y)
HDD_OBJS +=	$(HDD_SRC_DIR)/wlan_hdd_nan.o
endif

ifeq ($(CONFIG_QCOM_TDLS),y)
HDD_OBJS +=	$(HDD_SRC_DIR)/wlan_hdd_tdls.o
endif

ifeq ($(CONFIG_WLAN_SYNC_TSF),y)
HDD_OBJS +=	$(HDD_SRC_DIR)/wlan_hdd_tsf.o
endif

ifeq ($(CONFIG_MPC_UT_FRAMEWORK),y)
HDD_OBJS +=	$(HDD_SRC_DIR)/wlan_hdd_conc_ut.o
endif

ifeq ($(CONFIG_WLAN_FEATURE_MEMDUMP),y)
HDD_OBJS += $(HDD_SRC_DIR)/wlan_hdd_memdump.o
endif

ifeq ($(CONFIG_WLAN_FEATURE_DISA),y)
HDD_OBJS += $(HDD_SRC_DIR)/wlan_hdd_disa.o
endif

ifeq ($(CONFIG_LFR_SUBNET_DETECTION), y)
HDD_OBJS +=	$(HDD_SRC_DIR)/wlan_hdd_subnet_detect.o
endif

ifeq ($(CONFIG_WLAN_FEATURE_NAN_DATAPATH), y)
HDD_OBJS += $(HDD_SRC_DIR)/wlan_hdd_nan_datapath.o
endif

ifeq ($(CONFIG_WLAN_FEATURE_11AX),y)
HDD_OBJS += $(HDD_SRC_DIR)/wlan_hdd_he.o
endif

ifeq ($(CONFIG_LITHIUM), y)
HDD_OBJS += $(HDD_SRC_DIR)/wlan_hdd_rx_monitor.o
endif

########### HOST DIAG LOG ###########
HOST_DIAG_LOG_DIR :=	$(WLAN_COMMON_ROOT)/utils/host_diag_log

HOST_DIAG_LOG_INC_DIR :=	$(HOST_DIAG_LOG_DIR)/inc
HOST_DIAG_LOG_SRC_DIR :=	$(HOST_DIAG_LOG_DIR)/src

HOST_DIAG_LOG_INC :=	-I$(WLAN_ROOT)/$(HOST_DIAG_LOG_INC_DIR) \
			-I$(WLAN_ROOT)/$(HOST_DIAG_LOG_SRC_DIR)

HOST_DIAG_LOG_OBJS +=	$(HOST_DIAG_LOG_SRC_DIR)/host_diag_log.o

############ EPPING ############
EPPING_DIR :=	$(WLAN_COMMON_ROOT)/utils/epping
EPPING_INC_DIR :=	$(EPPING_DIR)/inc
EPPING_SRC_DIR :=	$(EPPING_DIR)/src

EPPING_INC := 	-I$(WLAN_ROOT)/$(EPPING_INC_DIR)

EPPING_OBJS := $(EPPING_SRC_DIR)/epping_main.o \
		$(EPPING_SRC_DIR)/epping_txrx.o \
		$(EPPING_SRC_DIR)/epping_tx.o \
		$(EPPING_SRC_DIR)/epping_rx.o \
		$(EPPING_SRC_DIR)/epping_helper.o \


############ MAC ############
MAC_DIR :=	core/mac
MAC_INC_DIR :=	$(MAC_DIR)/inc
MAC_SRC_DIR :=	$(MAC_DIR)/src

MAC_INC := 	-I$(WLAN_ROOT)/$(MAC_INC_DIR) \
		-I$(WLAN_ROOT)/$(MAC_SRC_DIR)/dph \
		-I$(WLAN_ROOT)/$(MAC_SRC_DIR)/include \
		-I$(WLAN_ROOT)/$(MAC_SRC_DIR)/pe/include \
		-I$(WLAN_ROOT)/$(MAC_SRC_DIR)/pe/lim \
		-I$(WLAN_ROOT)/$(MAC_SRC_DIR)/pe/nan

MAC_CFG_OBJS := $(MAC_SRC_DIR)/cfg/cfg_api.o \
		$(MAC_SRC_DIR)/cfg/cfg_param_name.o \
		$(MAC_SRC_DIR)/cfg/cfg_proc_msg.o \
		$(MAC_SRC_DIR)/cfg/cfg_send_msg.o

MAC_DPH_OBJS :=	$(MAC_SRC_DIR)/dph/dph_hash_table.o

MAC_LIM_OBJS := $(MAC_SRC_DIR)/pe/lim/lim_aid_mgmt.o \
		$(MAC_SRC_DIR)/pe/lim/lim_admit_control.o \
		$(MAC_SRC_DIR)/pe/lim/lim_api.o \
		$(MAC_SRC_DIR)/pe/lim/lim_assoc_utils.o \
		$(MAC_SRC_DIR)/pe/lim/lim_ft.o \
		$(MAC_SRC_DIR)/pe/lim/lim_ibss_peer_mgmt.o \
		$(MAC_SRC_DIR)/pe/lim/lim_link_monitoring_algo.o \
		$(MAC_SRC_DIR)/pe/lim/lim_p2p.o \
		$(MAC_SRC_DIR)/pe/lim/lim_process_action_frame.o \
		$(MAC_SRC_DIR)/pe/lim/lim_process_assoc_req_frame.o \
		$(MAC_SRC_DIR)/pe/lim/lim_process_assoc_rsp_frame.o \
		$(MAC_SRC_DIR)/pe/lim/lim_process_auth_frame.o \
		$(MAC_SRC_DIR)/pe/lim/lim_process_beacon_frame.o \
		$(MAC_SRC_DIR)/pe/lim/lim_process_cfg_updates.o \
		$(MAC_SRC_DIR)/pe/lim/lim_process_deauth_frame.o \
		$(MAC_SRC_DIR)/pe/lim/lim_process_disassoc_frame.o \
		$(MAC_SRC_DIR)/pe/lim/lim_process_message_queue.o \
		$(MAC_SRC_DIR)/pe/lim/lim_process_mlm_req_messages.o \
		$(MAC_SRC_DIR)/pe/lim/lim_process_mlm_rsp_messages.o \
		$(MAC_SRC_DIR)/pe/lim/lim_process_probe_req_frame.o \
		$(MAC_SRC_DIR)/pe/lim/lim_process_probe_rsp_frame.o \
		$(MAC_SRC_DIR)/pe/lim/lim_process_sme_req_messages.o \
		$(MAC_SRC_DIR)/pe/lim/lim_prop_exts_utils.o \
		$(MAC_SRC_DIR)/pe/lim/lim_scan_result_utils.o \
		$(MAC_SRC_DIR)/pe/lim/lim_security_utils.o \
		$(MAC_SRC_DIR)/pe/lim/lim_send_management_frames.o \
		$(MAC_SRC_DIR)/pe/lim/lim_send_messages.o \
		$(MAC_SRC_DIR)/pe/lim/lim_send_sme_rsp_messages.o \
		$(MAC_SRC_DIR)/pe/lim/lim_ser_des_utils.o \
		$(MAC_SRC_DIR)/pe/lim/lim_session.o \
		$(MAC_SRC_DIR)/pe/lim/lim_session_utils.o \
		$(MAC_SRC_DIR)/pe/lim/lim_sme_req_utils.o \
		$(MAC_SRC_DIR)/pe/lim/lim_sta_hash_api.o \
		$(MAC_SRC_DIR)/pe/lim/lim_timer_utils.o \
		$(MAC_SRC_DIR)/pe/lim/lim_trace.o \
		$(MAC_SRC_DIR)/pe/lim/lim_utils.o

ifeq ($(CONFIG_QCOM_TDLS),y)
MAC_LIM_OBJS += $(MAC_SRC_DIR)/pe/lim/lim_process_tdls.o
endif

ifeq ($(CONFIG_WLAN_FEATURE_NAN_DATAPATH), y)
MAC_NDP_OBJS += $(MAC_SRC_DIR)/pe/nan/nan_datapath.o
endif

ifeq ($(CONFIG_QCACLD_WLAN_LFR2),y)
	MAC_LIM_OBJS += $(MAC_SRC_DIR)/pe/lim/lim_process_mlm_host_roam.o \
		$(MAC_SRC_DIR)/pe/lim/lim_send_frames_host_roam.o \
		$(MAC_SRC_DIR)/pe/lim/lim_roam_timer_utils.o \
		$(MAC_SRC_DIR)/pe/lim/lim_ft_preauth.o \
		$(MAC_SRC_DIR)/pe/lim/lim_reassoc_utils.o
endif

MAC_SCH_OBJS := $(MAC_SRC_DIR)/pe/sch/sch_api.o \
		$(MAC_SRC_DIR)/pe/sch/sch_beacon_gen.o \
		$(MAC_SRC_DIR)/pe/sch/sch_beacon_process.o \
		$(MAC_SRC_DIR)/pe/sch/sch_message.o

MAC_RRM_OBJS :=	$(MAC_SRC_DIR)/pe/rrm/rrm_api.o

MAC_OBJS := 	$(MAC_CFG_OBJS) \
		$(MAC_DPH_OBJS) \
		$(MAC_LIM_OBJS) \
		$(MAC_SCH_OBJS) \
		$(MAC_RRM_OBJS) \
		$(MAC_NDP_OBJS)

############ SAP ############
SAP_DIR :=	core/sap
SAP_INC_DIR :=	$(SAP_DIR)/inc
SAP_SRC_DIR :=	$(SAP_DIR)/src

SAP_INC := 	-I$(WLAN_ROOT)/$(SAP_INC_DIR) \
		-I$(WLAN_ROOT)/$(SAP_SRC_DIR)

SAP_OBJS :=	$(SAP_SRC_DIR)/sap_api_link_cntl.o \
		$(SAP_SRC_DIR)/sap_ch_select.o \
		$(SAP_SRC_DIR)/sap_fsm.o \
		$(SAP_SRC_DIR)/sap_module.o

############ DFS ############ 350
DFS_DIR :=     $(WLAN_COMMON_ROOT)/umac/dfs
DFS_CORE_INC_DIR := $(DFS_DIR)/core/inc
DFS_CORE_SRC_DIR := $(DFS_DIR)/core/src

DFS_DISP_INC_DIR := $(DFS_DIR)/dispatcher/inc
DFS_DISP_SRC_DIR := $(DFS_DIR)/dispatcher/src
DFS_TARGET_INC_DIR := $(WLAN_COMMON_ROOT)/target_if/dfs/inc
DFS_CMN_SERVICES_INC_DIR := $(WLAN_COMMON_ROOT)/umac/cmn_services/dfs/inc

DFS_INC :=	-I$(WLAN_ROOT)/$(DFS_DISP_INC_DIR) \
		-I$(WLAN_ROOT)/$(DFS_TARGET_INC_DIR) \
		-I$(WLAN_ROOT)/$(DFS_CMN_SERVICES_INC_DIR)

DFS_OBJS :=	$(DFS_CORE_SRC_DIR)/filtering/dfs_bindetects.o \
		$(DFS_CORE_SRC_DIR)/filtering/dfs_debug.o\
		$(DFS_CORE_SRC_DIR)/filtering/dfs_fcc_bin5.o\
		$(DFS_CORE_SRC_DIR)/filtering/dfs_init.o\
		$(DFS_CORE_SRC_DIR)/filtering/dfs_misc.o\
		$(DFS_CORE_SRC_DIR)/filtering/dfs_radar.o\
		$(DFS_CORE_SRC_DIR)/filtering/dfs_ar.o\
		$(DFS_CORE_SRC_DIR)/filtering/ar5416_radar.o\
		$(DFS_CORE_SRC_DIR)/filtering/ar9300_radar.o\
		$(DFS_CORE_SRC_DIR)/filtering/ar5212_radar.o\
		$(DFS_CORE_SRC_DIR)/filtering/dfs_phyerr_tlv.o\
		$(DFS_CORE_SRC_DIR)/filtering/dfs_process_phyerr.o\
		$(DFS_CORE_SRC_DIR)/filtering/dfs_process_radarevent.o\
		$(DFS_CORE_SRC_DIR)/filtering/dfs_staggered.o \
		$(DFS_CORE_SRC_DIR)/misc/dfs.o \
		$(DFS_CORE_SRC_DIR)/misc/dfs_cac.o\
		$(DFS_CORE_SRC_DIR)/misc/dfs_nol.o\
		$(DFS_CORE_SRC_DIR)/misc/dfs_nol.o\
		$(DFS_CORE_SRC_DIR)/misc/dfs_zero_cac.o\
		$(DFS_CORE_SRC_DIR)/misc/dfs_random_chan_sel.o\
		$(DFS_CORE_SRC_DIR)/misc/dfs_process_radar_found_ind.o\
		$(DFS_DISP_SRC_DIR)/wlan_dfs_init_deinit_api.o\
		$(DFS_DISP_SRC_DIR)/wlan_dfs_lmac_api.o\
		$(DFS_DISP_SRC_DIR)/wlan_dfs_mlme_api.o\
		$(DFS_DISP_SRC_DIR)/wlan_dfs_tgt_api.o\
		$(DFS_DISP_SRC_DIR)/wlan_dfs_ucfg_api.o\
		$(DFS_DISP_SRC_DIR)/wlan_dfs_tgt_api.o\
		$(DFS_DISP_SRC_DIR)/wlan_dfs_utils_api.o\
		$(WLAN_COMMON_ROOT)/target_if/dfs/src/target_if_dfs.o

############ SME ############
SME_DIR :=	core/sme
SME_INC_DIR :=	$(SME_DIR)/inc
SME_SRC_DIR :=	$(SME_DIR)/src

SME_INC := 	-I$(WLAN_ROOT)/$(SME_INC_DIR) \
		-I$(WLAN_ROOT)/$(SME_SRC_DIR)/csr

SME_CSR_OBJS := $(SME_SRC_DIR)/csr/csr_api_roam.o \
		$(SME_SRC_DIR)/csr/csr_api_scan.o \
		$(SME_SRC_DIR)/csr/csr_cmd_process.o \
		$(SME_SRC_DIR)/csr/csr_link_list.o \
		$(SME_SRC_DIR)/csr/csr_neighbor_roam.o \
		$(SME_SRC_DIR)/csr/csr_util.o \


ifeq ($(CONFIG_QCACLD_WLAN_LFR2),y)
SME_CSR_OBJS += $(SME_SRC_DIR)/csr/csr_roam_preauth.o \
		$(SME_SRC_DIR)/csr/csr_host_scan_roam.o
endif


ifeq ($(CONFIG_QCOM_TDLS),y)
SME_CSR_OBJS += $(SME_SRC_DIR)/csr/csr_tdls_process.o
endif

SME_QOS_OBJS := $(SME_SRC_DIR)/qos/sme_qos.o

SME_CMN_OBJS := $(SME_SRC_DIR)/common/sme_api.o \
		$(SME_SRC_DIR)/common/sme_ft_api.o \
		$(SME_SRC_DIR)/common/sme_power_save.o \
		$(SME_SRC_DIR)/common/sme_trace.o

SME_P2P_OBJS = $(SME_SRC_DIR)/p2p/p2p_api.o

SME_RRM_OBJS := $(SME_SRC_DIR)/rrm/sme_rrm.o

ifeq ($(CONFIG_QCACLD_FEATURE_NAN),y)
SME_NAN_OBJS = $(SME_SRC_DIR)/nan/nan_api.o
endif

ifeq ($(CONFIG_WLAN_FEATURE_NAN_DATAPATH), y)
SME_NDP_OBJS += $(SME_SRC_DIR)/nan/nan_datapath_api.o
endif

SME_OBJS :=	$(SME_CMN_OBJS) \
		$(SME_CSR_OBJS) \
		$(SME_P2P_OBJS) \
		$(SME_QOS_OBJS) \
		$(SME_RRM_OBJS) \
		$(SME_NAN_OBJS) \
		$(SME_NDP_OBJS)

############ NLINK ############
NLINK_DIR     :=	$(WLAN_COMMON_ROOT)/utils/nlink
NLINK_INC_DIR :=	$(NLINK_DIR)/inc
NLINK_SRC_DIR :=	$(NLINK_DIR)/src

NLINK_INC     := 	-I$(WLAN_ROOT)/$(NLINK_INC_DIR)
NLINK_OBJS    :=	$(NLINK_SRC_DIR)/wlan_nlink_srv.o

############ PTT ############
PTT_DIR     :=	$(WLAN_COMMON_ROOT)/utils/ptt
PTT_INC_DIR :=	$(PTT_DIR)/inc
PTT_SRC_DIR :=	$(PTT_DIR)/src

PTT_INC     := 	-I$(WLAN_ROOT)/$(PTT_INC_DIR)
PTT_OBJS    :=	$(PTT_SRC_DIR)/wlan_ptt_sock_svc.o

############ WLAN_LOGGING ############
WLAN_LOGGING_DIR     :=	$(WLAN_COMMON_ROOT)/utils/logging
WLAN_LOGGING_INC_DIR :=	$(WLAN_LOGGING_DIR)/inc
WLAN_LOGGING_SRC_DIR :=	$(WLAN_LOGGING_DIR)/src

WLAN_LOGGING_INC     := -I$(WLAN_ROOT)/$(WLAN_LOGGING_INC_DIR)
WLAN_LOGGING_OBJS    := $(WLAN_LOGGING_SRC_DIR)/wlan_logging_sock_svc.o

############ SYS ############
SYS_DIR :=	core/mac/src/sys

SYS_INC := 	-I$(WLAN_ROOT)/$(SYS_DIR)/common/inc \
		-I$(WLAN_ROOT)/$(SYS_DIR)/legacy/src/platform/inc \
		-I$(WLAN_ROOT)/$(SYS_DIR)/legacy/src/system/inc \
		-I$(WLAN_ROOT)/$(SYS_DIR)/legacy/src/utils/inc

SYS_COMMON_SRC_DIR := $(SYS_DIR)/common/src
SYS_LEGACY_SRC_DIR := $(SYS_DIR)/legacy/src
SYS_OBJS :=	$(SYS_COMMON_SRC_DIR)/wlan_qct_sys.o \
		$(SYS_LEGACY_SRC_DIR)/platform/src/sys_wrapper.o \
		$(SYS_LEGACY_SRC_DIR)/system/src/mac_init_api.o \
		$(SYS_LEGACY_SRC_DIR)/system/src/sys_entry_func.o \
		$(SYS_LEGACY_SRC_DIR)/utils/src/dot11f.o \
		$(SYS_LEGACY_SRC_DIR)/utils/src/mac_trace.o \
		$(SYS_LEGACY_SRC_DIR)/utils/src/parser_api.o \
		$(SYS_LEGACY_SRC_DIR)/utils/src/utils_parser.o

############ Qca-wifi-host-cmn ############
QDF_OS_DIR :=	qdf
QDF_OS_INC_DIR := $(QDF_OS_DIR)/inc
QDF_OS_SRC_DIR := $(QDF_OS_DIR)/linux/src
QDF_OBJ_DIR := $(WLAN_COMMON_ROOT)/$(QDF_OS_SRC_DIR)

QDF_INC :=	-I$(WLAN_COMMON_INC)/$(QDF_OS_INC_DIR) \
		-I$(WLAN_COMMON_INC)/$(QDF_OS_SRC_DIR)

QDF_OBJS := 	$(QDF_OBJ_DIR)/qdf_defer.o \
		$(QDF_OBJ_DIR)/qdf_event.o \
		$(QDF_OBJ_DIR)/qdf_list.o \
		$(QDF_OBJ_DIR)/qdf_lock.o \
		$(QDF_OBJ_DIR)/qdf_mc_timer.o \
		$(QDF_OBJ_DIR)/qdf_mem.o \
		$(QDF_OBJ_DIR)/qdf_nbuf.o \
		$(QDF_OBJ_DIR)/qdf_threads.o \
		$(QDF_OBJ_DIR)/qdf_trace.o

ifeq ($(CONFIG_WLAN_DEBUGFS), y)
QDF_OBJS += $(QDF_OBJ_DIR)/qdf_debugfs.o
endif


##########OS_IF #######
OS_IF_DIR := $(WLAN_COMMON_ROOT)/os_if

OS_IF_INC := -I$(WLAN_COMMON_INC)/os_if/linux \
            -I$(WLAN_COMMON_INC)/os_if/linux/scan/inc \
            -I$(WLAN_COMMON_INC)/os_if/linux/p2p/inc \
            -I$(WLAN_COMMON_INC)/os_if/linux/tdls/inc

OS_IF_OBJ := $(OS_IF_DIR)/linux/p2p/src/wlan_cfg80211_p2p.o

############ UMAC_DISP ############
UMAC_DISP_DIR := umac/global_umac_dispatcher/lmac_if
UMAC_DISP_INC_DIR := $(UMAC_DISP_DIR)/inc
UMAC_DISP_SRC_DIR := $(UMAC_DISP_DIR)/src
UMAC_DISP_OBJ_DIR := $(WLAN_COMMON_ROOT)/$(UMAC_DISP_SRC_DIR)

UMAC_DISP_INC := -I$(WLAN_COMMON_INC)/$(UMAC_DISP_INC_DIR)

UMAC_DISP_OBJS := $(UMAC_DISP_OBJ_DIR)/wlan_lmac_if.o

############# UMAC_SCAN ############
UMAC_SCAN_DIR := umac/scan
UMAC_SCAN_DISP_INC_DIR := $(UMAC_SCAN_DIR)/dispatcher/inc
UMAC_SCAN_CORE_DIR := $(WLAN_COMMON_ROOT)/$(UMAC_SCAN_DIR)/core/src
UMAC_SCAN_DISP_DIR := $(WLAN_COMMON_ROOT)/$(UMAC_SCAN_DIR)/dispatcher/src
UMAC_TARGET_SCAN_INC := -I$(WLAN_COMMON_INC)/target_if/scan/inc

UMAC_SCAN_INC := -I$(WLAN_COMMON_INC)/$(UMAC_SCAN_DISP_INC_DIR)
UMAC_SCAN_OBJS := $(UMAC_SCAN_CORE_DIR)/wlan_scan_cache_db.o \
		$(UMAC_SCAN_CORE_DIR)/wlan_scan_cache_db_ops.o \
		$(UMAC_SCAN_CORE_DIR)/wlan_scan_main.o \
		$(UMAC_SCAN_CORE_DIR)/wlan_scan_manager.o \
		$(UMAC_SCAN_DISP_DIR)/wlan_scan_tgt_api.o \
		$(UMAC_SCAN_DISP_DIR)/wlan_scan_ucfg_api.o \
		$(UMAC_SCAN_DISP_DIR)/wlan_scan_utils_api.o \
		$(WLAN_COMMON_ROOT)/os_if/linux/scan/src/wlan_cfg80211_scan.o \
		$(WLAN_COMMON_ROOT)/os_if/linux/wlan_cfg80211.o \
		$(WLAN_COMMON_ROOT)/target_if/scan/src/target_if_scan.o

############# UMAC_CMN_SERVICES ############
UMAC_COMMON_INC := -I$(WLAN_COMMON_INC)/umac/cmn_services/cmn_defs/inc \
		-I$(WLAN_COMMON_INC)/umac/cmn_services/utils/inc
UMAC_COMMON_OBJS := $(WLAN_COMMON_ROOT)/umac/cmn_services/utils/src/wlan_utility.o

ifeq ($(CONFIG_WLAN_LRO), y)
QDF_OBJS +=     $(QDF_OBJ_DIR)/qdf_lro.o
endif

############ CDS (Connectivity driver services) ############
CDS_DIR :=	core/cds
CDS_INC_DIR :=	$(CDS_DIR)/inc
CDS_SRC_DIR :=	$(CDS_DIR)/src

CDS_INC := 	-I$(WLAN_ROOT)/$(CDS_INC_DIR) \
		-I$(WLAN_ROOT)/$(CDS_SRC_DIR)

CDS_OBJS :=	$(CDS_SRC_DIR)/cds_api.o \
		$(CDS_SRC_DIR)/cds_reg_service.o \
		$(CDS_SRC_DIR)/cds_packet.o \
		$(CDS_SRC_DIR)/cds_regdomain.o \
		$(CDS_SRC_DIR)/cds_sched.o \
		$(CDS_SRC_DIR)/cds_utils.o


###### UMAC OBJMGR ########
UMAC_OBJMGR_DIR := $(WLAN_COMMON_ROOT)/umac/cmn_services/obj_mgr

UMAC_OBJMGR_INC := -I$(WLAN_COMMON_INC)/umac/cmn_services/obj_mgr/inc \
		-I$(WLAN_COMMON_INC)/umac/cmn_services/obj_mgr/src \
		-I$(WLAN_COMMON_INC)/umac/cmn_services/inc \
		-I$(WLAN_COMMON_INC)/umac/global_umac_dispatcher/lmac_if/inc

UMAC_OBJMGR_OBJS := $(UMAC_OBJMGR_DIR)/src/wlan_objmgr_global_obj.o \
		$(UMAC_OBJMGR_DIR)/src/wlan_objmgr_pdev_obj.o \
		$(UMAC_OBJMGR_DIR)/src/wlan_objmgr_peer_obj.o \
		$(UMAC_OBJMGR_DIR)/src/wlan_objmgr_psoc_obj.o \
		$(UMAC_OBJMGR_DIR)/src/wlan_objmgr_psoc_service_ready_api.o \
		$(UMAC_OBJMGR_DIR)/src/wlan_objmgr_vdev_obj.o

###########  UMAC MGMT TXRX ##########
UMAC_MGMT_TXRX_DIR := $(WLAN_COMMON_ROOT)/umac/cmn_services/mgmt_txrx

UMAC_MGMT_TXRX_INC := -I$(WLAN_COMMON_INC)/umac/cmn_services/mgmt_txrx/dispatcher/inc \

UMAC_MGMT_TXRX_OBJS := $(UMAC_MGMT_TXRX_DIR)/core/src/wlan_mgmt_txrx_main.o \
	$(UMAC_MGMT_TXRX_DIR)/dispatcher/src/wlan_mgmt_txrx_utils_api.o \
	$(UMAC_MGMT_TXRX_DIR)/dispatcher/src/wlan_mgmt_txrx_tgt_api.o

########## POWER MANAGEMENT OFFLOADS (PMO) ##########
PMO_DIR := $(WLAN_COMMON_ROOT)/pmo
PMO_INC :=      -I$(WLAN_COMMON_INC)/pmo/core/inc \
		-I$(WLAN_COMMON_INC)/pmo/dispatcher/inc \
		-I$(WLAN_COMMON_INC)/pmo/core/src \
		-I$(WLAN_COMMON_INC)/pmo/dispatcher/src

PMO_OBJS :=     $(PMO_DIR)/core/src/wlan_pmo_main.o \
		$(PMO_DIR)/core/src/wlan_pmo_arp.o \
		$(PMO_DIR)/core/src/wlan_pmo_ns.o \
		$(PMO_DIR)/core/src/wlan_pmo_gtk.o \
		$(PMO_DIR)/core/src/wlan_pmo_mc_addr_filtering.o \
		$(PMO_DIR)/core/src/wlan_pmo_static_config.o \
		$(PMO_DIR)/core/src/wlan_pmo_wow.o \
		$(PMO_DIR)/core/src/wlan_pmo_lphb.o \
		$(PMO_DIR)/core/src/wlan_pmo_suspend_resume.o \
		$(PMO_DIR)/core/src/wlan_pmo_hw_filter.o \
		$(PMO_DIR)/core/src/wlan_pmo_pkt_filter.o \
		$(PMO_DIR)/dispatcher/src/wlan_pmo_obj_mgmt_api.o \
		$(PMO_DIR)/dispatcher/src/wlan_pmo_ucfg_api.o \
		$(PMO_DIR)/dispatcher/src/wlan_pmo_tgt_arp.o \
		$(PMO_DIR)/dispatcher/src/wlan_pmo_tgt_ns.o \
		$(PMO_DIR)/dispatcher/src/wlan_pmo_tgt_gtk.o \
		$(PMO_DIR)/dispatcher/src/wlan_pmo_tgt_wow.o \
		$(PMO_DIR)/dispatcher/src/wlan_pmo_tgt_static_config.o \
		$(PMO_DIR)/dispatcher/src/wlan_pmo_tgt_mc_addr_filtering.o \
		$(PMO_DIR)/dispatcher/src/wlan_pmo_tgt_lphb.o \
		$(PMO_DIR)/dispatcher/src/wlan_pmo_tgt_suspend_resume.o \
		$(PMO_DIR)/dispatcher/src/wlan_pmo_tgt_hw_filter.o \
		$(PMO_DIR)/dispatcher/src/wlan_pmo_tgt_pkt_filter.o

############## UMAC P2P ###########
P2P_DIR := umac/p2p
P2P_CORE_DIR := $(P2P_DIR)/core
P2P_CORE_SRC_DIR := $(P2P_CORE_DIR)/src
P2P_CORE_OBJ_DIR := $(WLAN_COMMON_ROOT)/$(P2P_CORE_SRC_DIR)
P2P_DISPATCHER_DIR := $(P2P_DIR)/dispatcher
P2P_DISPATCHER_INC_DIR := $(P2P_DISPATCHER_DIR)/inc
P2P_DISPATCHER_SRC_DIR := $(P2P_DISPATCHER_DIR)/src
P2P_DISPATCHER_OBJ_DIR := $(WLAN_COMMON_ROOT)/$(P2P_DISPATCHER_SRC_DIR)
UMAC_P2P_INC := -I$(WLAN_COMMON_INC)/$(P2P_DISPATCHER_INC_DIR) \
		-I$(WLAN_COMMON_INC)/umac/scan/dispatcher/inc
UMAC_P2P_OBJS := $(P2P_DISPATCHER_OBJ_DIR)/wlan_p2p_ucfg_api.o \
                 $(P2P_DISPATCHER_OBJ_DIR)/wlan_p2p_tgt_api.o \
                 $(P2P_CORE_OBJ_DIR)/wlan_p2p_main.o \
                 $(P2P_CORE_OBJ_DIR)/wlan_p2p_roc.o \
                 $(P2P_CORE_OBJ_DIR)/wlan_p2p_off_chan_tx.o

###### UMAC POLICY MGR ########
UMAC_POLICY_MGR_DIR := $(WLAN_COMMON_ROOT)/umac/cmn_services/policy_mgr

UMAC_POLICY_MGR_INC := -I$(WLAN_COMMON_INC)/umac/cmn_services/policy_mgr/inc \
		-I$(WLAN_COMMON_INC)/umac/cmn_services/policy_mgr/src

UMAC_POLICY_MGR_OBJS := $(UMAC_POLICY_MGR_DIR)/src/wlan_policy_mgr_action.o \
	$(UMAC_POLICY_MGR_DIR)/src/wlan_policy_mgr_core.o \
	$(UMAC_POLICY_MGR_DIR)/src/wlan_policy_mgr_get_set_utils.o \
	$(UMAC_POLICY_MGR_DIR)/src/wlan_policy_mgr_init_deinit.o \
	$(UMAC_POLICY_MGR_DIR)/src/wlan_policy_mgr_pcl.o \

###### UMAC TDLS ########
UMAC_TDLS_DIR := $(WLAN_COMMON_ROOT)/umac/tdls

UMAC_TDLS_INC := -I$(WLAN_COMMON_INC)/umac/tdls/dispatcher/inc

UMAC_TDLS_OBJS := $(UMAC_TDLS_DIR)/core/src/wlan_tdls_main.o \
       $(UMAC_TDLS_DIR)/core/src/wlan_tdls_cmds_process.o \
       $(UMAC_TDLS_DIR)/core/src/wlan_tdls_peer.o \
       $(UMAC_TDLS_DIR)/core/src/wlan_tdls_mgmt.o \
       $(UMAC_TDLS_DIR)/core/src/wlan_tdls_ct.o \
       $(UMAC_TDLS_DIR)/dispatcher/src/wlan_tdls_tgt_api.o \
       $(UMAC_TDLS_DIR)/dispatcher/src/wlan_tdls_ucfg_api.o \
       $(UMAC_TDLS_DIR)/dispatcher/src/wlan_tdls_utils_api.o \
       $(WLAN_COMMON_ROOT)/os_if/linux/tdls/src/wlan_cfg80211_tdls.o

########### BMI ###########
BMI_DIR := core/bmi

BMI_INC := -I$(WLAN_ROOT)/$(BMI_DIR)/inc

BMI_OBJS := $(BMI_DIR)/src/bmi.o \
            $(BMI_DIR)/src/ol_fw.o \
            $(BMI_DIR)/src/ol_fw_common.o
BMI_OBJS += $(BMI_DIR)/src/bmi_1.o

##########  TARGET_IF #######
TARGET_IF_DIR := $(WLAN_COMMON_ROOT)/target_if

TARGET_IF_INC := -I$(WLAN_COMMON_INC)/target_if/core/inc \
		 -I$(WLAN_COMMON_INC)/target_if/core/src \
		 -I$(WLAN_COMMON_INC)/target_if/init_deinit/inc \
		 -I$(WLAN_COMMON_INC)/target_if/pmo/inc \
		 -I$(WLAN_COMMON_INC)/target_if/pmo/src \
		 -I$(WLAN_COMMON_INC)/target_if/p2p/inc \
		 -I$(WLAN_COMMON_INC)/target_if/regulatory/inc \
		 -I$(WLAN_COMMON_INC)/target_if/tdls/inc

TARGET_IF_OBJ := $(TARGET_IF_DIR)/core/src/target_if_main.o \
		$(TARGET_IF_DIR)/init_deinit/src/service_ready_event_handler.o \
		$(TARGET_IF_DIR)/pmo/src/target_if_pmo_main.o \
		$(TARGET_IF_DIR)/pmo/src/target_if_pmo_arp.o \
		$(TARGET_IF_DIR)/pmo/src/target_if_pmo_ns.o \
		$(TARGET_IF_DIR)/pmo/src/target_if_pmo_gtk.o \
		$(TARGET_IF_DIR)/pmo/src/target_if_pmo_wow.o \
		$(TARGET_IF_DIR)/pmo/src/target_if_pmo_hw_filter.o \
		$(TARGET_IF_DIR)/pmo/src/target_if_pmo_mc_addr_filtering.o \
		$(TARGET_IF_DIR)/pmo/src/target_if_pmo_static_config.o \
		$(TARGET_IF_DIR)/pmo/src/target_if_pmo_lphb.o \
		$(TARGET_IF_DIR)/pmo/src/target_if_pmo_suspend_resume.o \
		$(TARGET_IF_DIR)/pmo/src/target_if_pmo_pkt_filter.o \
		$(TARGET_IF_DIR)/p2p/src/target_if_p2p.o \
		$(TARGET_IF_DIR)/regulatory/src/target_if_reg.o \
		$(TARGET_IF_DIR)/tdls/src/target_if_tdls.o

########### GLOBAL_LMAC_IF ##########
GLOBAL_LMAC_IF_DIR := $(WLAN_COMMON_ROOT)/global_lmac_if

GLOBAL_LMAC_IF_INC := -I$(WLAN_COMMON_INC)/global_lmac_if/inc \
                      -I$(WLAN_COMMON_INC)/global_lmac_if/src

GLOBAL_LMAC_IF_OBJ := $(GLOBAL_LMAC_IF_DIR)/src/wlan_global_lmac_if.o

########### WMI ###########
WMI_ROOT_DIR := wmi

WMI_SRC_DIR := $(WMI_ROOT_DIR)/src
WMI_INC_DIR := $(WMI_ROOT_DIR)/inc
WMI_OBJ_DIR := $(WLAN_COMMON_ROOT)/$(WMI_SRC_DIR)

WMI_INC := -I$(WLAN_COMMON_INC)/$(WMI_INC_DIR)

WMI_OBJS := $(WMI_OBJ_DIR)/wmi_unified.o \
	    $(WMI_OBJ_DIR)/wmi_tlv_helper.o \
	    $(WMI_OBJ_DIR)/wmi_unified_tlv.o \
	    $(WMI_OBJ_DIR)/wmi_unified_api.o \
	    $(WMI_OBJ_DIR)/wmi_unified_pmo_api.o \
	    $(WMI_OBJ_DIR)/wmi_unified_reg_api.o \
	    $(WMI_OBJ_DIR)/wmi_unified_dfs_api.o \
	    $(WMI_OBJ_DIR)/wmi_unified_non_tlv.o

########### FWLOG ###########
FWLOG_DIR := $(WLAN_COMMON_ROOT)/utils/fwlog

FWLOG_INC := -I$(WLAN_ROOT)/$(FWLOG_DIR)

FWLOG_OBJS := $(FWLOG_DIR)/dbglog_host.o

############ TXRX ############
TXRX_DIR :=     core/dp/txrx
TXRX_INC :=     -I$(WLAN_ROOT)/$(TXRX_DIR)

TXRX_OBJS := $(TXRX_DIR)/ol_txrx.o \
                $(TXRX_DIR)/ol_cfg.o \
                $(TXRX_DIR)/ol_rx.o \
                $(TXRX_DIR)/ol_rx_fwd.o \
                $(TXRX_DIR)/ol_txrx.o \
                $(TXRX_DIR)/ol_rx_defrag.o \
                $(TXRX_DIR)/ol_tx_desc.o \
                $(TXRX_DIR)/ol_tx.o \
                $(TXRX_DIR)/ol_rx_reorder_timeout.o \
                $(TXRX_DIR)/ol_rx_reorder.o \
                $(TXRX_DIR)/ol_rx_pn.o \
                $(TXRX_DIR)/ol_tx_queue.o \
                $(TXRX_DIR)/ol_txrx_peer_find.o \
                $(TXRX_DIR)/ol_txrx_event.o \
                $(TXRX_DIR)/ol_txrx_encap.o \
                $(TXRX_DIR)/ol_tx_send.o \
                $(TXRX_DIR)/ol_tx_sched.o \
                $(TXRX_DIR)/ol_tx_classify.o

ifeq ($(CONFIG_WLAN_TX_FLOW_CONTROL_V2), y)
TXRX_OBJS +=     $(TXRX_DIR)/ol_txrx_flow_control.o
endif

ifeq ($(CONFIG_LITHIUM), y)
############ DP 3.0 ############
DP_INC := -I$(WLAN_COMMON_ROOT)/dp/inc \
	-I$(WLAN_COMMON_ROOT)/dp/wifi3.0

DP_SRC := $(WLAN_COMMON_ROOT)/dp/wifi3.0
DP_OBJS := $(DP_SRC)/dp_main.o \
		$(DP_SRC)/dp_tx.o \
		$(DP_SRC)/dp_tx_desc.o \
		$(DP_SRC)/dp_rx.o \
		$(DP_SRC)/dp_rx_err.o \
		$(DP_SRC)/dp_htt.o \
		$(DP_SRC)/dp_peer.o \
		$(DP_SRC)/dp_rx_desc.o \
		$(DP_SRC)/dp_reo.o \
		$(DP_SRC)/dp_rx_mon_dest.o \
		$(DP_SRC)/dp_rx_mon_status.o \
		$(DP_SRC)/dp_rx_defrag.o \
		$(DP_SRC)/dp_stats.o
endif

############ CFG ############
WCFG_DIR := wlan_cfg
WCFG_INC := -I$(WLAN_COMMON_INC)/$(WCFG_DIR)
WCFG_SRC := $(WLAN_COMMON_ROOT)/$(WCFG_DIR)
WCFG_OBJS := $(WCFG_SRC)/wlan_cfg.o

############ OL ############
OL_DIR :=     core/dp/ol
OL_INC :=     -I$(WLAN_ROOT)/$(OL_DIR)/inc

############ CDP ############
CDP_ROOT_DIR := dp
CDP_INC_DIR := $(CDP_ROOT_DIR)/inc
CDP_INC := -I$(WLAN_COMMON_INC)/$(CDP_INC_DIR)

############ PKTLOG ############
PKTLOG_DIR :=      $(WLAN_COMMON_ROOT)/utils/pktlog
PKTLOG_INC :=      -I$(WLAN_ROOT)/$(PKTLOG_DIR)/include

PKTLOG_OBJS :=	$(PKTLOG_DIR)/pktlog_ac.o \
		$(PKTLOG_DIR)/pktlog_internal.o \
		$(PKTLOG_DIR)/linux_ac.o

############ HTT ############
HTT_DIR :=      core/dp/htt
HTT_INC :=      -I$(WLAN_ROOT)/$(HTT_DIR)

HTT_OBJS := $(HTT_DIR)/htt_tx.o \
            $(HTT_DIR)/htt.o \
            $(HTT_DIR)/htt_t2h.o \
            $(HTT_DIR)/htt_h2t.o \
            $(HTT_DIR)/htt_fw_stats.o \
            $(HTT_DIR)/htt_rx.o


############## INIT-DEINIT ###########
INIT_DEINIT_DIR := init_deinit/dispatcher
INIT_DEINIT_INC_DIR := $(INIT_DEINIT_DIR)/inc
INIT_DEINIT_SRC_DIR := $(INIT_DEINIT_DIR)/src
INIT_DEINIT_OBJ_DIR := $(WLAN_COMMON_ROOT)/$(INIT_DEINIT_SRC_DIR)
INIT_DEINIT_INC := -I$(WLAN_COMMON_INC)/$(INIT_DEINIT_INC_DIR)
INIT_DEINIT_OBJS := $(INIT_DEINIT_OBJ_DIR)/dispatcher_init_deinit.o

############## REGULATORY ###########
REGULATORY_DIR := umac/regulatory
REGULATORY_CORE_INC_DIR := $(REGULATORY_DIR)/core/inc
REGULATORY_CORE_SRC_DIR := $(REGULATORY_DIR)/core/src
REG_DISPATCHER_INC_DIR := $(REGULATORY_DIR)/dispatcher/inc
REG_DISPATCHER_SRC_DIR := $(REGULATORY_DIR)/dispatcher/src
REG_CORE_OBJ_DIR := $(WLAN_COMMON_ROOT)/$(REGULATORY_CORE_SRC_DIR)
REG_DISPATCHER_OBJ_DIR := $(WLAN_COMMON_ROOT)/$(REG_DISPATCHER_SRC_DIR)
REGULATORY_INC := -I$(WLAN_COMMON_INC)/$(REGULATORY_CORE_INC_DIR)
REGULATORY_INC += -I$(WLAN_COMMON_INC)/$(REG_DISPATCHER_INC_DIR)
REGULATORY_OBJS := $(REG_CORE_OBJ_DIR)/reg_db.o \
                   $(REG_CORE_OBJ_DIR)/reg_services.o \
                   $(REG_CORE_OBJ_DIR)/reg_db_parser.o \
                   $(REG_DISPATCHER_OBJ_DIR)/wlan_reg_services_api.o \
                   $(REG_DISPATCHER_OBJ_DIR)/wlan_reg_tgt_api.o \
                   $(REG_DISPATCHER_OBJ_DIR)/wlan_reg_ucfg_api.o

############## Control path common scheduler ##########
SCHEDULER_DIR := scheduler
SCHEDULER_INC_DIR := $(SCHEDULER_DIR)/inc
SCHEDULER_SRC_DIR := $(SCHEDULER_DIR)/src
SCHEDULER_OBJ_DIR := $(WLAN_COMMON_ROOT)/$(SCHEDULER_SRC_DIR)
SCHEDULER_INC := -I$(WLAN_COMMON_INC)/$(SCHEDULER_INC_DIR)
SCHEDULER_OBJS := $(SCHEDULER_OBJ_DIR)/scheduler_api.o \
                  $(SCHEDULER_OBJ_DIR)/scheduler_core.o

###### UMAC SERIALIZATION ########
UMAC_SER_DIR := umac/cmn_services/serialization
UMAC_SER_INC_DIR := $(UMAC_SER_DIR)/inc
UMAC_SER_SRC_DIR := $(UMAC_SER_DIR)/src
UMAC_SER_OBJ_DIR := $(WLAN_COMMON_ROOT)/$(UMAC_SER_SRC_DIR)

UMAC_SER_INC := -I$(WLAN_COMMON_INC)/$(UMAC_SER_INC_DIR)
UMAC_SER_OBJS := $(UMAC_SER_OBJ_DIR)/wlan_serialization_dequeue.o \
		 $(UMAC_SER_OBJ_DIR)/wlan_serialization_enqueue.o \
		 $(UMAC_SER_OBJ_DIR)/wlan_serialization_main.o \
		 $(UMAC_SER_OBJ_DIR)/wlan_serialization_api.o \
		 $(UMAC_SER_OBJ_DIR)/wlan_serialization_utils.o \
		 $(UMAC_SER_OBJ_DIR)/wlan_serialization_legacy_api.o \
		 $(UMAC_SER_OBJ_DIR)/wlan_serialization_rules.o

###### WIFI POS ########
CONFIG_WIFI_POS_CONVERGED := y

WIFI_POS_OS_IF_DIR := $(WLAN_COMMON_ROOT)/os_if/linux/wifi_pos/src
WIFI_POS_OS_IF_INC := -I$(WLAN_COMMON_INC)/os_if/linux/wifi_pos/inc
WIFI_POS_TGT_DIR := $(WLAN_COMMON_ROOT)/target_if/wifi_pos/src
WIFI_POS_TGT_INC := -I$(WLAN_COMMON_INC)/target_if/wifi_pos/inc
WIFI_POS_CORE_DIR := $(WLAN_COMMON_ROOT)/umac/wifi_pos/src
WIFI_POS_API_INC := -I$(WLAN_COMMON_INC)/umac/wifi_pos/inc


ifeq ($(CONFIG_WIFI_POS_CONVERGED), y)
WIFI_POS_OBJS := $(WIFI_POS_CORE_DIR)/wifi_pos_api.o \
		 $(WIFI_POS_CORE_DIR)/wifi_pos_main.o \
		 $(WIFI_POS_CORE_DIR)/wifi_pos_ucfg.o \
		 $(WIFI_POS_CORE_DIR)/wifi_pos_utils.o \
		 $(WIFI_POS_OS_IF_DIR)/os_if_wifi_pos.o \
		 $(WIFI_POS_TGT_DIR)/target_if_wifi_pos.o
endif

######################### NAN #########################
NAN_CORE_DIR := $(WLAN_COMMON_ROOT)/umac/nan/core/src
NAN_CORE_INC := -I$(WLAN_COMMON_INC)/umac/nan/core/inc
NAN_UCFG_DIR := $(WLAN_COMMON_ROOT)/umac/nan/dispatcher/src
NAN_UCFG_INC := -I$(WLAN_COMMON_INC)/umac/nan/dispatcher/inc
NAN_TGT_DIR  := $(WLAN_COMMON_ROOT)/target_if/nan/src
NAN_TGT_INC  := -I$(WLAN_COMMON_INC)/target_if/nan/inc
NAN_OS_IF_DIR  := $(WLAN_COMMON_ROOT)/os_if/linux/nan/src
NAN_OS_IF_INC  := -I$(WLAN_COMMON_INC)/os_if/linux/nan/inc

ifeq ($(CONFIG_NAN_CONVERGENCE), y)
WLAN_NAN_OBJS := $(NAN_CORE_DIR)/nan_main.o \
		 $(NAN_CORE_DIR)/nan_api.o \
		 $(NAN_CORE_DIR)/nan_utils.o \
		 $(NAN_UCFG_DIR)/nan_ucfg_api.o \
		 $(NAN_TGT_DIR)/target_if_nan.o \
		 $(NAN_OS_IF_DIR)/os_if_nan.o
endif
#######################################################

############## HTC ##########
HTC_DIR := htc
HTC_INC := -I$(WLAN_COMMON_INC)/$(HTC_DIR)

HTC_OBJS := $(WLAN_COMMON_ROOT)/$(HTC_DIR)/htc.o \
            $(WLAN_COMMON_ROOT)/$(HTC_DIR)/htc_send.o \
            $(WLAN_COMMON_ROOT)/$(HTC_DIR)/htc_recv.o \
            $(WLAN_COMMON_ROOT)/$(HTC_DIR)/htc_services.o

########### HIF ###########
HIF_DIR := hif
HIF_CE_DIR := $(HIF_DIR)/src/ce

HIF_DISPATCHER_DIR := $(HIF_DIR)/src/dispatcher

HIF_PCIE_DIR := $(HIF_DIR)/src/pcie
HIF_SNOC_DIR := $(HIF_DIR)/src/snoc
HIF_USB_DIR := $(HIF_DIR)/src/usb
HIF_SDIO_DIR := $(HIF_DIR)/src/sdio

HIF_SDIO_NATIVE_DIR := $(HIF_SDIO_DIR)/native_sdio
HIF_SDIO_NATIVE_INC_DIR := $(HIF_SDIO_NATIVE_DIR)/include
HIF_SDIO_NATIVE_SRC_DIR := $(HIF_SDIO_NATIVE_DIR)/src

HIF_INC := -I$(WLAN_COMMON_INC)/$(HIF_DIR)/inc \
	   -I$(WLAN_COMMON_INC)/$(HIF_DIR)/src \
	   -I$(WLAN_COMMON_INC)/$(HIF_CE_DIR)

ifeq ($(CONFIG_HIF_PCI), 1)
HIF_INC += -I$(WLAN_COMMON_INC)/$(HIF_DISPATCHER_DIR)
HIF_INC += -I$(WLAN_COMMON_INC)/$(HIF_PCIE_DIR)
endif

ifeq ($(CONFIG_HIF_SNOC), 1)
HIF_INC += -I$(WLAN_COMMON_INC)/$(HIF_DISPATCHER_DIR)
HIF_INC += -I$(WLAN_COMMON_INC)/$(HIF_SNOC_DIR)
endif

ifeq ($(CONFIG_HIF_USB), 1)
HIF_INC += -I$(WLAN_COMMON_INC)/$(HIF_DISPATCHER_DIR)
HIF_INC += -I$(WLAN_COMMON_INC)/$(HIF_USB_DIR)
endif

ifeq ($(CONFIG_HIF_SDIO), 1)
HIF_INC += -I$(WLAN_COMMON_INC)/$(HIF_DISPATCHER_DIR)
HIF_INC += -I$(WLAN_COMMON_INC)/$(HIF_SDIO_DIR)
HIF_INC += -I$(WLAN_COMMON_INC)/$(HIF_SDIO_NATIVE_INC_DIR)
endif

HIF_COMMON_OBJS := $(WLAN_COMMON_ROOT)/$(HIF_DIR)/src/ath_procfs.o \
                $(WLAN_COMMON_ROOT)/$(HIF_DIR)/src/hif_main.o \
                $(WLAN_COMMON_ROOT)/$(HIF_DIR)/src/mp_dev.o

HIF_CE_OBJS :=  $(WLAN_COMMON_ROOT)/$(HIF_CE_DIR)/ce_bmi.o \
                $(WLAN_COMMON_ROOT)/$(HIF_CE_DIR)/ce_diag.o \
                $(WLAN_COMMON_ROOT)/$(HIF_CE_DIR)/ce_main.o \
                $(WLAN_COMMON_ROOT)/$(HIF_CE_DIR)/ce_service.o \
                $(WLAN_COMMON_ROOT)/$(HIF_CE_DIR)/ce_tasklet.o \
                $(WLAN_COMMON_ROOT)/$(HIF_DIR)/src/regtable.o

ifeq ($(CONFIG_LITHIUM), y)
HIF_CE_OBJS +=  $(WLAN_COMMON_ROOT)/$(HIF_DIR)/src/qca6290def.o \
                $(WLAN_COMMON_ROOT)/$(HIF_CE_DIR)/ce_service_srng.o
endif

HIF_USB_OBJS := $(WLAN_COMMON_ROOT)/$(HIF_USB_DIR)/usbdrv.o \
                $(WLAN_COMMON_ROOT)/$(HIF_USB_DIR)/hif_usb.o \
                $(WLAN_COMMON_ROOT)/$(HIF_USB_DIR)/if_usb.o \
                $(WLAN_COMMON_ROOT)/$(HIF_USB_DIR)/regtable_usb.o

HIF_SDIO_OBJS := $(WLAN_COMMON_ROOT)/$(HIF_SDIO_DIR)/hif_sdio_send.o \
                 $(WLAN_COMMON_ROOT)/$(HIF_SDIO_DIR)/hif_bmi_reg_access.o \
                 $(WLAN_COMMON_ROOT)/$(HIF_SDIO_DIR)/hif_diag_reg_access.o \
                 $(WLAN_COMMON_ROOT)/$(HIF_SDIO_DIR)/hif_sdio_dev.o \
                 $(WLAN_COMMON_ROOT)/$(HIF_SDIO_DIR)/hif_sdio.o \
                 $(WLAN_COMMON_ROOT)/$(HIF_SDIO_DIR)/hif_sdio_recv.o \
                 $(WLAN_COMMON_ROOT)/$(HIF_SDIO_DIR)/regtable_sdio.o

HIF_SDIO_NATIVE_OBJS := $(WLAN_COMMON_ROOT)/$(HIF_SDIO_NATIVE_SRC_DIR)/hif.o \
                        $(WLAN_COMMON_ROOT)/$(HIF_SDIO_NATIVE_SRC_DIR)/hif_scatter.o

ifeq ($(CONFIG_WLAN_NAPI), y)
HIF_OBJS += $(WLAN_COMMON_ROOT)/$(HIF_DIR)/src/hif_napi.o
endif

HIF_PCIE_OBJS := $(WLAN_COMMON_ROOT)/$(HIF_PCIE_DIR)/if_pci.o
HIF_SNOC_OBJS := $(WLAN_COMMON_ROOT)/$(HIF_SNOC_DIR)/if_snoc.o
HIF_SDIO_OBJS += $(WLAN_COMMON_ROOT)/$(HIF_SDIO_DIR)/if_sdio.o

HIF_OBJS += $(WLAN_COMMON_ROOT)/$(HIF_DISPATCHER_DIR)/multibus.o
HIF_OBJS += $(WLAN_COMMON_ROOT)/$(HIF_DISPATCHER_DIR)/dummy.o

ifeq ($(CONFIG_HIF_PCI), 1)
HIF_OBJS += $(HIF_PCIE_OBJS)
HIF_OBJS += $(HIF_COMMON_OBJS)
HIF_OBJS += $(HIF_CE_OBJS)
HIF_OBJS += $(WLAN_COMMON_ROOT)/$(HIF_DISPATCHER_DIR)/multibus_pci.o
endif

ifeq ($(CONFIG_HIF_SNOC), 1)
HIF_OBJS += $(HIF_SNOC_OBJS)
HIF_OBJS += $(HIF_COMMON_OBJS)
HIF_OBJS += $(HIF_CE_OBJS)
HIF_OBJS += $(WLAN_COMMON_ROOT)/$(HIF_DISPATCHER_DIR)/multibus_snoc.o
endif

ifeq ($(CONFIG_HIF_SDIO), 1)
HIF_OBJS += $(HIF_SDIO_OBJS)
HIF_OBJS += $(HIF_SDIO_NATIVE_OBJS)
HIF_OBJS += $(HIF_COMMON_OBJS)
HIF_OBJS += $(WLAN_COMMON_ROOT)/$(HIF_DISPATCHER_DIR)/multibus_sdio.o
endif

ifeq ($(CONFIG_HIF_USB), 1)
HIF_OBJS += $(HIF_USB_OBJS)
HIF_OBJS += $(HIF_COMMON_OBJS)
HIF_OBJS += $(WLAN_COMMON_ROOT)/$(HIF_DISPATCHER_DIR)/multibus_usb.o
endif

ifeq ($(CONFIG_LITHIUM), y)
############ HAL ############
HAL_DIR :=	hal
HAL_INC :=	-I$(WLAN_COMMON_INC)/$(HAL_DIR)/inc \
		-I$(WLAN_COMMON_INC)/$(HAL_DIR)/wifi3.0

HAL_OBJS :=	$(WLAN_COMMON_ROOT)/$(HAL_DIR)/wifi3.0/hal_srng.o \
		$(WLAN_COMMON_ROOT)/$(HAL_DIR)/wifi3.0/hal_rx.o \
		$(WLAN_COMMON_ROOT)/$(HAL_DIR)/wifi3.0/hal_wbm.o \
		$(WLAN_COMMON_ROOT)/$(HAL_DIR)/wifi3.0/hal_reo.o
endif

############ WMA ############
WMA_DIR :=	core/wma

WMA_INC_DIR :=  $(WMA_DIR)/inc
WMA_SRC_DIR :=  $(WMA_DIR)/src

WMA_INC :=	-I$(WLAN_ROOT)/$(WMA_INC_DIR) \
		-I$(WLAN_ROOT)/$(WMA_SRC_DIR)

ifeq ($(CONFIG_WLAN_FEATURE_NAN_DATAPATH), y)
WMA_NDP_OBJS += $(WMA_SRC_DIR)/wma_nan_datapath.o
endif

WMA_OBJS :=	$(WMA_SRC_DIR)/wma_main.o \
		$(WMA_SRC_DIR)/wma_scan_roam.o \
		$(WMA_SRC_DIR)/wma_dev_if.o \
		$(WMA_SRC_DIR)/wma_mgmt.o \
		$(WMA_SRC_DIR)/wma_power.o \
		$(WMA_SRC_DIR)/wma_data.o \
		$(WMA_SRC_DIR)/wma_utils.o \
		$(WMA_SRC_DIR)/wma_features.o \
		$(WMA_SRC_DIR)/wlan_qct_wma_legacy.o\
		$(WMA_NDP_OBJS)

ifeq ($(CONFIG_WLAN_FEATURE_DSRC), y)
WMA_OBJS+=	$(WMA_SRC_DIR)/wma_ocb.o
endif
ifeq ($(CONFIG_MPC_UT_FRAMEWORK),y)
WMA_OBJS +=	$(WMA_SRC_DIR)/wma_utils_ut.o
endif
ifeq ($(CONFIG_WLAN_FEATURE_11AX), y)
WMA_OBJS+=	$(WMA_SRC_DIR)/wma_he.o
endif

############## PLD ##########
PLD_DIR := core/pld
PLD_INC_DIR := $(PLD_DIR)/inc
PLD_SRC_DIR := $(PLD_DIR)/src

PLD_INC :=	-I$(WLAN_ROOT)/$(PLD_INC_DIR) \
		-I$(WLAN_ROOT)/$(PLD_SRC_DIR)

PLD_OBJS :=	$(PLD_SRC_DIR)/pld_common.o

ifeq ($(CONFIG_PCI), y)
PLD_OBJS +=	$(PLD_SRC_DIR)/pld_pcie.o
endif
ifeq ($(CONFIG_ICNSS),y)
PLD_OBJS +=	$(PLD_SRC_DIR)/pld_snoc.o
endif
ifeq ($(CONFIG_CNSS_SDIO),y)
PLD_OBJS +=	$(PLD_SRC_DIR)/pld_sdio.o
endif
ifeq ($(CONFIG_PLD_USB_CNSS), y)
PLD_OBJS +=	$(PLD_SRC_DIR)/pld_usb.o
endif

TARGET_INC :=	-I$(WLAN_ROOT)/../fw-api/hw/qca6290/v2 \
		-I$(WLAN_ROOT)/../fw-api/fw

LINUX_INC :=	-Iinclude/linux

INCS :=		$(HDD_INC) \
		$(EPPING_INC) \
		$(LINUX_INC) \
		$(MAC_INC) \
		$(SAP_INC) \
		$(SME_INC) \
		$(SYS_INC) \
		$(QDF_INC) \
		$(CDS_INC) \
		$(DFS_INC) \
		$(TARGET_IF_INC) \
		$(OS_IF_INC) \
		$(GLOBAL_LMAC_IF_INC)

INCS +=		$(WMA_INC) \
		$(UAPI_INC) \
		$(COMMON_INC) \
		$(WMI_INC) \
		$(FWLOG_INC) \
		$(TXRX_INC) \
		$(OL_INC) \
		$(CDP_INC) \
		$(PKTLOG_INC) \
		$(HTT_INC) \
		$(INIT_DEINIT_INC) \
		$(SCHEDULER_INC) \
		$(REGULATORY_INC) \
		$(HTC_INC) \
		$(DFS_INC) \
		$(WCFG_INC)

INCS +=		$(HIF_INC) \
		$(BMI_INC)

ifeq ($(CONFIG_LITHIUM), y)
INCS += 	$(HAL_INC) \
		$(DP_INC)
endif

################ WIFI POS ################
INCS +=		$(WIFI_POS_API_INC)
INCS +=		$(WIFI_POS_TGT_INC)
INCS +=		$(WIFI_POS_OS_IF_INC)
##########################################

################ NAN POS ################
INCS +=		$(NAN_CORE_INC)
INCS +=		$(NAN_UCFG_INC)
INCS +=		$(NAN_TGT_INC)
INCS +=		$(NAN_OS_IF_INC)
##########################################
INCS +=		$(UMAC_OBJMGR_INC)
INCS +=		$(UMAC_MGMT_TXRX_INC)
INCS +=		$(PMO_INC)
INCS +=		$(UMAC_P2P_INC)
INCS +=		$(UMAC_POLICY_MGR_INC)
INCS +=		$(TARGET_INC)
INCS +=		$(UMAC_TDLS_INC)
INCS +=		$(UMAC_SER_INC)
INCS +=		$(NLINK_INC) \
		$(PTT_INC) \
		$(WLAN_LOGGING_INC)

INCS +=		$(PLD_INC)

ifeq ($(CONFIG_REMOVE_PKT_LOG), 0)
INCS +=		$(PKTLOG_INC)
endif

ifeq ($(BUILD_DIAG_VERSION), 1)
INCS +=		$(HOST_DIAG_LOG_INC)
endif

INCS +=		$(UMAC_DISP_INC)
INCS +=		$(UMAC_SCAN_INC)
INCS +=		$(UMAC_TARGET_SCAN_INC)
INCS +=		$(UMAC_COMMON_INC)

OBJS :=		$(HDD_OBJS) \
		$(EPPING_OBJS) \
		$(MAC_OBJS) \
		$(SAP_OBJS) \
		$(SME_OBJS) \
		$(SYS_OBJS) \
		$(QDF_OBJS) \
		$(CDS_OBJS) \
		$(DFS_OBJS)

OBJS +=		$(WMA_OBJS) \
		$(TXRX_OBJS) \
		$(WMI_OBJS) \
		$(FWLOG_OBJS) \
		$(HTC_OBJS) \
		$(INIT_DEINIT_OBJS) \
		$(SCHEDULER_OBJS) \
		$(REGULATORY_OBJS) \
		$(DFS_OBJS)

OBJS +=		$(HIF_OBJS) \
		$(BMI_OBJS) \
		$(HTT_OBJS) \
		$(OS_IF_OBJ) \
		$(TARGET_IF_OBJ) \
		$(GLOBAL_LMAC_IF_OBJ)

ifeq ($(CONFIG_LITHIUM), y)
OBJS += 	$(HAL_OBJS)
endif

OBJS +=		$(UMAC_OBJMGR_OBJS)
OBJS +=		$(WIFI_POS_OBJS)
OBJS +=		$(WLAN_NAN_OBJS)
OBJS +=		$(UMAC_MGMT_TXRX_OBJS)
OBJS +=		$(UMAC_TDLS_OBJS)
OBJS +=		$(PMO_OBJS)
OBJS +=		$(UMAC_P2P_OBJS)
OBJS +=		$(UMAC_POLICY_MGR_OBJS)
OBJS +=		$(WLAN_LOGGING_OBJS)
OBJS +=		$(NLINK_OBJS)
OBJS +=		$(PTT_OBJS)
OBJS +=		$(UMAC_SER_OBJS)
OBJS +=		$(PLD_OBJS)

ifeq ($(CONFIG_REMOVE_PKT_LOG), 0)
OBJS +=		$(PKTLOG_OBJS)
endif

ifeq ($(BUILD_DIAG_VERSION), 1)
OBJS +=		$(HOST_DIAG_LOG_OBJS)
endif

OBJS +=		$(UMAC_DISP_OBJS)
OBJS +=		$(UMAC_SCAN_OBJS)
OBJS +=		$(UMAC_COMMON_OBJS)
OBJS +=		$(WCFG_OBJS)

ifeq ($(CONFIG_LITHIUM), y)
OBJS +=		$(DP_OBJS)
endif

EXTRA_CFLAGS += $(INCS)

CDEFINES :=	-DANI_LITTLE_BYTE_ENDIAN \
		-DANI_LITTLE_BIT_ENDIAN \
		-DDOT11F_LITTLE_ENDIAN_HOST \
		-DANI_COMPILER_TYPE_GCC \
		-DANI_OS_TYPE_ANDROID=6 \
		-DPTT_SOCK_SVC_ENABLE \
		-Wall\
		-Werror\
		-D__linux__ \
		-DHAL_SELF_STA_PER_BSS=1 \
		-DFEATURE_WLAN_WAPI \
		-DSOFTAP_CHANNEL_RANGE \
		-DWLAN_AP_STA_CONCURRENCY \
		-DFEATURE_WLAN_SCAN_PNO \
		-DWLAN_FEATURE_PACKET_FILTERING \
		-DWLAN_FEATURE_P2P_DEBUG \
		-DWLAN_ENABLE_AGEIE_ON_SCAN_RESULTS \
		-DWLANTL_DEBUG\
		-DWLAN_NS_OFFLOAD \
		-DWLAN_SOFTAP_VSTA_FEATURE \
		-DWLAN_FEATURE_GTK_OFFLOAD \
		-DWLAN_WAKEUP_EVENTS \
		-DFEATURE_WLAN_RA_FILTERING\
		-DWLAN_NL80211_TESTMODE \
		-DFEATURE_WLAN_LPHB \
		-DQCA_SUPPORT_TX_THROTTLE \
		-DWMI_INTERFACE_EVENT_LOGGING \
		-DATH_SUPPORT_WAPI \
		-DWLAN_FEATURE_LINK_LAYER_STATS \
		-DFEATURE_WLAN_EXTSCAN \
		-DWLAN_FEATURE_MBSSID \
		-DCONFIG_160MHZ_SUPPORT \
		-DCONFIG_MCL \
		-DNAPIER_SCAN \
		-DWLAN_PMO_ENABLE \
		-DCONVERGED_P2P_ENABLE \
		-DWLAN_POLICY_MGR_ENABLE \
		-DSUPPORT_11AX \
		-DCONVERGED_TDLS_ENABLE


############ WIFI POS ##############
ifeq ($(CONFIG_WIFI_POS_CONVERGED), y)
CDEFINES += -DWIFI_POS_CONVERGED
else
CDEFINES += -DFEATURE_OEM_DATA_SUPPORT
endif
####################################

ifneq ($(CONFIG_HIF_USB), 1)
CDEFINES += -DWLAN_LOGGING_SOCK_SVC_ENABLE
endif

ifeq ($(CONFIG_CNSS), y)
ifeq ($(CONFIG_CNSS_SDIO), y)
CDEFINES += -DCONFIG_PLD_SDIO_CNSS
else
CDEFINES += -DCONFIG_PLD_PCIE_CNSS
endif
endif

ifeq ($(CONFIG_CNSS2), y)
CDEFINES += -DCONFIG_PLD_PCIE_CNSS
CDEFINES += -DCONFIG_PLD_PCIE_INIT
endif

# Flag to enable bus auto suspend
ifeq ($(CONFIG_ROME_IF),pci)
ifeq ($(CONFIG_BUS_AUTO_SUSPEND), y)
CDEFINES += -DFEATURE_RUNTIME_PM
endif
endif

ifeq ($(CONFIG_ICNSS), y)
CDEFINES += -DCONFIG_PLD_SNOC_ICNSS
endif

ifeq (y,$(filter y,$(CONFIG_CNSS_EOS) $(CONFIG_ICNSS)))
CDEFINES += -DQCA_WIFI_3_0
CDEFINES += -DQCA_WIFI_3_0_EMU
endif

ifeq (y,$(filter y,$(CONFIG_CNSS_ADRASTEA) $(CONFIG_ICNSS)))
CDEFINES += -DQCA_WIFI_3_0_ADRASTEA
CDEFINES += -DADRASTEA_SHADOW_REGISTERS
CDEFINES += -DADRASTEA_RRI_ON_DDR
endif

ifeq ($(CONFIG_WLAN_FASTPATH), y)
CDEFINES +=	-DWLAN_FEATURE_FASTPATH
endif

ifeq ($(CONFIG_FEATURE_PKTLOG), y)
CDEFINES +=     -DFEATURE_PKTLOG
endif

CDEFINES +=	-DFEATURE_DP_TRACE

ifeq ($(CONFIG_WLAN_NAPI), y)
CDEFINES += -DFEATURE_NAPI
ifeq ($(CONFIG_WLAN_NAPI_DEBUG), y)
CDEFINES += -DFEATURE_NAPI_DEBUG
endif
endif

ifeq (y,$(findstring y,$(CONFIG_ARCH_MSM) $(CONFIG_ARCH_QCOM)))
CDEFINES += -DMSM_PLATFORM
endif

CDEFINES +=	-DQCA_SUPPORT_TXRX_LOCAL_PEER_ID

ifeq ($(CONFIG_WLAN_TX_FLOW_CONTROL_V2), y)
CDEFINES +=	-DQCA_LL_TX_FLOW_CONTROL_V2
CDEFINES +=	-DQCA_LL_TX_FLOW_GLOBAL_MGMT_POOL
else
ifeq ($(CONFIG_ROME_IF),pci)
ifneq ($(CONFIG_LITHIUM),y)
CDEFINES +=	-DQCA_LL_LEGACY_TX_FLOW_CONTROL
endif
endif
endif

ifneq ($(CONFIG_QCA_CLD_WLAN),)
CDEFINES += -DWCN_PRONTO
CDEFINES += -DWCN_PRONTO_V1
endif

ifeq ($(BUILD_DEBUG_VERSION),1)
CDEFINES +=	-DWLAN_DEBUG \
		-DTRACE_RECORD \
		-DLIM_TRACE_RECORD \
		-DSME_TRACE_RECORD \
		-DHDD_TRACE_RECORD \
		-DPE_DEBUG_LOGW \
		-DPE_DEBUG_LOGE
endif

ifeq ($(CONFIG_SLUB_DEBUG_ON),y)
CDEFINES += -DTIMER_MANAGER
CDEFINES += -DMEMORY_DEBUG
CDEFINES += -DCONFIG_HALT_KMEMLEAK
CDEFINES += -DWLAN_SUSPEND_RESUME_TEST
endif

ifeq ($(HAVE_CFG80211),1)
CDEFINES += -DWLAN_FEATURE_P2P
CDEFINES += -DWLAN_FEATURE_WFD
ifeq ($(CONFIG_QCOM_VOWIFI_11R),y)
CDEFINES += -DKERNEL_SUPPORT_11R_CFG80211
CDEFINES += -DUSE_80211_WMMTSPEC_FOR_RIC
endif
endif

ifeq ($(CONFIG_QCOM_ESE),y)
CDEFINES += -DFEATURE_WLAN_ESE
CDEFINES += -DQCA_COMPUTE_TX_DELAY
CDEFINES += -DQCA_COMPUTE_TX_DELAY_PER_TID
endif

#normally, TDLS negative behavior is not needed
ifeq ($(CONFIG_QCOM_TDLS),y)
CDEFINES += -DFEATURE_WLAN_TDLS
endif

ifeq ($(CONFIG_QCACLD_WLAN_LFR3),y)
CDEFINES += -DWLAN_FEATURE_ROAM_OFFLOAD
endif

ifeq ($(CONFIG_CNSS_GENL), y)
CDEFINES += -DCNSS_GENL
endif

ifeq ($(CONFIG_QCACLD_WLAN_LFR2),y)
CDEFINES += -DWLAN_FEATURE_HOST_ROAM
endif

ifeq ($(CONFIG_WLAN_POWER_DEBUGFS), y)
CDEFINES += -DWLAN_POWER_DEBUGFS
endif

# Enable object manager reference count debug infrastructure
CDEFINES += -DWLAN_OBJMGR_DEBUG

ifeq ($(BUILD_DIAG_VERSION),1)
CDEFINES += -DFEATURE_WLAN_DIAG_SUPPORT
CDEFINES += -DFEATURE_WLAN_DIAG_SUPPORT_CSR
CDEFINES += -DFEATURE_WLAN_DIAG_SUPPORT_LIM
ifeq ($(CONFIG_HIF_PCI), 1)
CDEFINES += -DCONFIG_ATH_PROCFS_DIAG_SUPPORT
endif
endif

ifeq ($(CONFIG_HIF_USB), 1)
CDEFINES += -DCONFIG_ATH_PROCFS_DIAG_SUPPORT
CDEFINES += -DQCA_SUPPORT_OL_RX_REORDER_TIMEOUT
CDEFINES += -DCONFIG_ATH_PCIE_MAX_PERF=0 -DCONFIG_ATH_PCIE_AWAKE_WHILE_DRIVER_LOAD=0 -DCONFIG_DISABLE_CDC_MAX_PERF_WAR=0
endif

ifeq ($(CONFIG_WLAN_FEATURE_11W),y)
CDEFINES += -DWLAN_FEATURE_11W
endif

ifeq ($(CONFIG_QCA_TXDESC_SANITY_CHECKS), 1)
CDEFINES += -DQCA_SUPPORT_TXDESC_SANITY_CHECKS
endif

ifeq ($(CONFIG_QCOM_LTE_COEX),y)
CDEFINES += -DFEATURE_WLAN_CH_AVOID
endif

ifeq ($(CONFIG_WLAN_FEATURE_LPSS),y)
CDEFINES += -DWLAN_FEATURE_LPSS
endif

ifneq ($(TARGET_BUILD_VARIANT),user)
CDEFINES += -DDESC_DUP_DETECT_DEBUG
CDEFINES += -DDEBUG_RX_RING_BUFFER
endif

ifeq ($(PANIC_ON_BUG),1)
CDEFINES += -DPANIC_ON_BUG
endif

ifeq ($(WLAN_OPEN_SOURCE), 1)
CDEFINES += -DWLAN_OPEN_SOURCE
endif

ifeq ($(CONFIG_FEATURE_STATS_EXT), 1)
CDEFINES += -DWLAN_FEATURE_STATS_EXT
endif

ifeq ($(CONFIG_QCACLD_FEATURE_NAN),y)
CDEFINES += -DWLAN_FEATURE_NAN
endif

ifeq ($(CONFIG_QCA_IBSS_SUPPORT), 1)
CDEFINES += -DQCA_IBSS_SUPPORT
endif

#Enable OL debug and wmi unified functions
ifeq ($(CONFIG_ATH_PERF_PWR_OFFLOAD), 1)
CDEFINES += -DATH_PERF_PWR_OFFLOAD
endif

#Disable packet log
ifeq ($(CONFIG_REMOVE_PKT_LOG), 1)
CDEFINES += -DREMOVE_PKT_LOG
endif

#Enable 11AC TX
ifeq ($(CONFIG_ATH_11AC_TXCOMPACT), 1)
CDEFINES += -DATH_11AC_TXCOMPACT
endif

#Enable OS specific IRQ abstraction
ifeq ($(CONFIG_ATH_SUPPORT_SHARED_IRQ), 1)
CDEFINES += -DATH_SUPPORT_SHARED_IRQ
endif

#Enable message based HIF instead of RAW access in BMI
ifeq ($(CONFIG_HIF_MESSAGE_BASED), 1)
CDEFINES += -DHIF_MESSAGE_BASED
endif

#Enable PCI specific APIS (dma, etc)
ifeq ($(CONFIG_HIF_PCI), 1)
CDEFINES += -DHIF_PCI
endif

ifeq ($(CONFIG_HIF_SNOC), 1)
CDEFINES += -DHIF_SNOC
endif

#Enable High Latency related Flags
ifeq ($(CONFIG_QCA_WIFI_SDIO), 1)
CDEFINES += -DCONFIG_HL_SUPPORT \
            -DCONFIG_AR6320_SUPPORT \
            -DSDIO_3_0 \
            -DHIF_SDIO \
            -DCONFIG_DISABLE_CDC_MAX_PERF_WAR=0 \
            -DCONFIG_ATH_PROCFS_DIAG_SUPPORT \
            -DFEATURE_HL_GROUP_CREDIT_FLOW_CONTROL \
            -DHIF_MBOX_SLEEP_WAR \
            -DDEBUG_HL_LOGGING \
            -DQCA_BAD_PEER_TX_FLOW_CL \
            -DCONFIG_TX_DESC_HI_PRIO_RESERVE \
            -DCONFIG_PER_VDEV_TX_DESC_POOL \
            -DCONFIG_SDIO \
            -DFEATURE_WLAN_FORCE_SAP_SCC
endif

ifeq ($(CONFIG_WLAN_FEATURE_DSRC), y)
CDEFINES += -DWLAN_FEATURE_DSRC
endif

ifeq ($(CONFIG_ARCH_SDXHEDGEHOG), y)
ifeq ($(CONFIG_QCA_WIFI_SDIO), 1)
ifeq ($(CONFIG_WCNSS_SKB_PRE_ALLOC), y)
CDEFINES += -DFEATURE_SKB_PRE_ALLOC
endif
endif
endif

#Enable USB specific APIS
ifeq ($(CONFIG_HIF_USB), 1)
CDEFINES += -DHIF_USB \
            -DCONFIG_PLD_USB_CNSS \
            -DCONFIG_HL_SUPPORT
endif

#Enable FW logs through ini
CDEFINES += -DCONFIG_FW_LOGS_BASED_ON_INI

#Enable pci read/write config functions
ifeq ($(CONFIG_ATH_PCI), 1)
CDEFINES += -DATH_PCI
endif

#Enable power management suspend/resume functionality
ifeq ($(CONFIG_ATH_BUS_PM), 1)
CDEFINES += -DATH_BUS_PM
endif

#Enable FLOWMAC module support
ifeq ($(CONFIG_ATH_SUPPORT_FLOWMAC_MODULE), 1)
CDEFINES += -DATH_SUPPORT_FLOWMAC_MODULE
endif

#Enable spectral support
ifeq ($(CONFIG_ATH_SUPPORT_SPECTRAL), 1)
CDEFINES += -DATH_SUPPORT_SPECTRAL
endif

#Enable WDI Event support
ifeq ($(CONFIG_WDI_EVENT_ENABLE), 1)
CDEFINES += -DWDI_EVENT_ENABLE
endif

#Endianess selection
ifeq ($(CONFIG_LITTLE_ENDIAN), 1)
AH_LITTLE_ENDIAN=1234
CDEFINES += -DAH_BYTE_ORDER=$(AH_LITTLE_ENDIAN)
else
AH_BIG_ENDIAN=4321
CDEFINES += -DAH_BYTE_ORDER=$(AH_BIG_ENDIAN)
CDEFINES += -DBIG_ENDIAN_HOST
endif

#Enable TX reclaim support
ifeq ($(CONFIG_TX_CREDIT_RECLAIM_SUPPORT), 1)
CDEFINES += -DTX_CREDIT_RECLAIM_SUPPORT
endif

#Enable FTM support
ifeq ($(CONFIG_QCA_WIFI_FTM), 1)
CDEFINES += -DQCA_WIFI_FTM
endif

#Enable Checksum Offload support
ifeq ($(CONFIG_CHECKSUM_OFFLOAD), 1)
CDEFINES += -DCHECKSUM_OFFLOAD
endif

#Enable Checksum Offload support
ifeq ($(CONFIG_IPA_OFFLOAD), 1)
CDEFINES += -DIPA_OFFLOAD
endif

ifeq ($(CONFIG_ARCH_SDXHEDGEHOG), y)
CDEFINES += -DSYNC_IPA_READY
endif

#Enable GTK Offload
ifeq ($(CONFIG_GTK_OFFLOAD), 1)
CDEFINES += -DWLAN_FEATURE_GTK_OFFLOAD
CDEFINES += -DIGTK_OFFLOAD
endif

#Enable GTK Offload
ifeq ($(CONFIG_EXT_WOW), 1)
CDEFINES += -DWLAN_FEATURE_EXTWOW_SUPPORT
endif

#Mark it as SMP Kernel
ifeq ($(CONFIG_SMP),y)
CDEFINES += -DQCA_CONFIG_SMP
endif

#Enable Channel Matrix restriction for all Rome only targets
ifneq (y,$(filter y,$(CONFIG_CNSS_EOS) $(CONFIG_ICNSS)))
CDEFINES += -DWLAN_ENABLE_CHNL_MATRIX_RESTRICTION
endif

#Enable OBSS feature
CDEFINES += -DQCA_HT_2040_COEX

#features specific to mobile router use case
ifeq ($(CONFIG_MOBILE_ROUTER), y)

#enable MCC TO SCC switch
CDEFINES += -DFEATURE_WLAN_MCC_TO_SCC_SWITCH

#enable wlan auto shutdown feature
CDEFINES += -DFEATURE_WLAN_AUTO_SHUTDOWN

#enable for MBSSID
CDEFINES += -DWLAN_FEATURE_MBSSID

#enable AP-AP ACS Optimization
CDEFINES += -DFEATURE_WLAN_AP_AP_ACS_OPTIMIZE

#Enable 4address scheme
CDEFINES += -DFEATURE_WLAN_STA_4ADDR_SCHEME

#Disable STA-AP Mode DFS support
CDEFINES += -DFEATURE_WLAN_STA_AP_MODE_DFS_DISABLE

else #CONFIG_MOBILE_ROUTER

#Open P2P device interface only for non-Mobile router use cases
CDEFINES += -DWLAN_OPEN_P2P_INTERFACE

#Enable 2.4 GHz social channels in 5 GHz only mode for p2p usage
CDEFINES += -DWLAN_ENABLE_SOCIAL_CHANNELS_5G_ONLY

endif #CONFIG_MOBILE_ROUTER

#Green AP feature
ifeq ($(CONFIG_QCACLD_FEATURE_GREEN_AP),y)
CDEFINES += -DFEATURE_GREEN_AP
endif

#Stats & Quota Metering feature
ifeq ($(CONFIG_QCACLD_FEATURE_METERING),y)
CDEFINES += -DFEATURE_METERING
endif

#Enable RX Full re-order OL feature only "LL and NON-MDM9630 platform"
ifneq ($(CONFIG_ARCH_MDM9630), y)
ifeq ($(CONFIG_HIF_PCI), 1)
CDEFINES += -DWLAN_FEATURE_RX_FULL_REORDER_OL
endif
endif

ifeq ($(CONFIG_ARCH_MDM9607), y)
CDEFINES += -DCONFIG_TUFELLO_DUAL_FW_SUPPORT
endif

#Enable Signed firmware support for split binary format
ifeq ($(CONFIG_QCA_SIGNED_SPLIT_BINARY_SUPPORT), 1)
CDEFINES += -DQCA_SIGNED_SPLIT_BINARY_SUPPORT
endif

#Enable single firmware binary format
ifeq ($(CONFIG_QCA_SINGLE_BINARY_SUPPORT), 1)
CDEFINES += -DQCA_SINGLE_BINARY_SUPPORT
endif

#Enable collecting target RAM dump after kernel panic
ifeq ($(CONFIG_TARGET_RAMDUMP_AFTER_KERNEL_PANIC), 1)
CDEFINES += -DTARGET_RAMDUMP_AFTER_KERNEL_PANIC
endif

#Enable/disable secure firmware feature
ifeq ($(CONFIG_FEATURE_SECURE_FIRMWARE), 1)
CDEFINES += -DFEATURE_SECURE_FIRMWARE
endif

ifeq ($(CONFIG_ATH_PCIE_ACCESS_DEBUG), 1)
CDEFINES += -DCONFIG_ATH_PCIE_ACCESS_DEBUG
endif

# Enable feature support fo Linux version QCMBR
ifeq ($(CONFIG_LINUX_QCMBR),y)
CDEFINES += -DLINUX_QCMBR
endif

# Enable featue sync tsf between multi devices
ifeq ($(CONFIG_WLAN_SYNC_TSF), y)
CDEFINES += -DWLAN_FEATURE_TSF
endif

# Enable full rx re-order offload for adrastea
ifeq (y, $(filter y, $(CONFIG_CNSS_ADRASTEA) $(CONFIG_ICNSS)))
CDEFINES += -DWLAN_FEATURE_RX_FULL_REORDER_OL
endif

# Enable athdiag procfs debug support for adrastea
ifeq (y, $(filter y, $(CONFIG_CNSS_ADRASTEA) $(CONFIG_ICNSS)))
CDEFINES += -DCONFIG_ATH_PROCFS_DIAG_SUPPORT
endif

# Enable 11AC TX compact feature for adrastea
ifeq (y, $(filter y, $(CONFIG_CNSS_ADRASTEA) $(CONFIG_ICNSS)))
CDEFINES += -DATH_11AC_TXCOMPACT
endif

# NOTE: CONFIG_64BIT_PADDR requires CONFIG_HELIUMPLUS
ifeq (y,$(filter y,$(CONFIG_CNSS_EOS) $(CONFIG_ICNSS)))
CONFIG_HELIUMPLUS := y
CONFIG_64BIT_PADDR := y
CONFIG_FEATURE_TSO := y
CONFIG_FEATURE_TSO_DEBUG := y
ifneq ($(CONFIG_FORCE_ALLOC_FROM_DMA_ZONE), y)
CONFIG_ENABLE_DEBUG_ADDRESS_MARKING := y
endif
ifeq ($(CONFIG_HELIUMPLUS),y)
CDEFINES += -DHELIUMPLUS
CDEFINES += -DAR900B
ifeq ($(CONFIG_64BIT_PADDR),y)
CDEFINES += -DHTT_PADDR64
endif

ifeq ($(CONFIG_SLUB_DEBUG_ON),y)
CDEFINES += -DOL_RX_INDICATION_RECORD
endif

endif
endif

ifeq ($(CONFIG_ENABLE_DEBUG_ADDRESS_MARKING),y)
CDEFINES += -DENABLE_DEBUG_ADDRESS_MARKING
endif
ifeq ($(CONFIG_FEATURE_TSO),y)
CDEFINES += -DFEATURE_TSO
endif
ifeq ($(CONFIG_FEATURE_TSO_DEBUG),y)
CDEFINES += -DFEATURE_TSO_DEBUG
endif

ifeq ($(CONFIG_WLAN_LRO), y)
CDEFINES += -DFEATURE_LRO
endif

ifeq ($(CONFIG_MOBILE_ROUTER), y)
CDEFINES += -DFEATURE_AP_MCC_CH_AVOIDANCE
endif

ifeq ($(CONFIG_MPC_UT_FRAMEWORK), y)
CDEFINES += -DMPC_UT_FRAMEWORK
endif

ifeq ($(CONFIG_WLAN_OFFLOAD_PACKETS),y)
CDEFINES += -DWLAN_FEATURE_OFFLOAD_PACKETS
endif

ifeq ($(CONFIG_WLAN_FEATURE_MEMDUMP),y)
CDEFINES += -DWLAN_FEATURE_MEMDUMP
endif

ifeq ($(CONFIG_WLAN_FEATURE_DISA),y)
CDEFINES += -DWLAN_FEATURE_DISA
endif

ifeq ($(CONFIG_LFR_SUBNET_DETECTION), y)
CDEFINES += -DFEATURE_LFR_SUBNET_DETECTION
endif

ifeq ($(CONFIG_MCC_TO_SCC_SWITCH), y)
CDEFINES += -DFEATURE_WLAN_MCC_TO_SCC_SWITCH
endif

ifeq ($(CONFIG_WLAN_FEATURE_NAN_DATAPATH), y)
CDEFINES += -DWLAN_FEATURE_NAN_DATAPATH
endif

ifeq ($(CONFIG_NAN_CONVERGENCE), y)
CDEFINES += -DWLAN_FEATURE_NAN_CONVERGENCE
endif

ifeq ($(CONFIG_LITHIUM),y)
CDEFINES += -DCONFIG_SHADOW_V2
CDEFINES += -DQCA6290_HEADERS_DEF
CDEFINES += -DQCA_WIFI_QCA6290
CDEFINES += -DQCA_WIFI_QCA8074
CDEFINES += -DQCA_WIFI_NAPIER_EMULATION
CDEFINES += -DQCA_WIFI_QCA8074_VP
CDEFINES += -DDP_INTR_POLL_BASED
CDEFINES += -DTX_PER_PDEV_DESC_POOL
CDEFINES += -DWLAN_RX_HASH
CDEFINES += -DDP_LFR
endif

ifeq ($(CONFIG_WLAN_FEATURE_11AX),y)
CDEFINES += -DWLAN_FEATURE_11AX
endif

# Dummy flag for WIN/MCL converged data path compilation
CDEFINES += -DDP_PRINT_ENABLE=0
CDEFINES += -DATH_SUPPORT_WRAP=0
CDEFINES += -DQCA_HOST2FW_RXBUF_RING
#endof dummy flags

# DFS component
CDEFINES += -DQCA_MCL_DFS_SUPPORT
CDEFINES += -DDFS_COMPONENT_ENABLE
CDEFINES += -DQCA_DFS_USE_POLICY_MANAGER
CDEFINES += -DQCA_DFS_NOL_PLATFORM_DRV_SUPPORT

ifeq ($(CONFIG_WLAN_DEBUGFS), y)
CDEFINES += -DWLAN_DEBUGFS
endif

KBUILD_CPPFLAGS += $(CDEFINES)

# Currently, for versions of gcc which support it, the kernel Makefile
# is disabling the maybe-uninitialized warning.  Re-enable it for the
# WLAN driver.  Note that we must use EXTRA_CFLAGS here so that it
# will override the kernel settings.
ifeq ($(call cc-option-yn, -Wmaybe-uninitialized),y)
EXTRA_CFLAGS += -Wmaybe-uninitialized
ifneq (y,$(CONFIG_ARCH_MSM))
EXTRA_CFLAGS += -Wframe-larger-than=4096
endif
endif
EXTRA_CFLAGS += -Wmissing-prototypes

ifeq ($(call cc-option-yn, -Wheader-guard),y)
EXTRA_CFLAGS += -Wheader-guard
endif
# If the module name is not "wlan", then the define MULTI_IF_NAME to be the
# same a the QCA CHIP name. The host driver will then append MULTI_IF_NAME to
# any string that must be unique for all instances of the driver on the system.
# This allows multiple instances of the driver with different module names.
# If the module name is wlan, leave MULTI_IF_NAME undefined and the code will
# treat the driver as the primary driver.
ifneq ($(MODNAME), wlan)
CHIP_NAME ?= $(MODNAME)
CDEFINES += -DMULTI_IF_NAME=\"$(CHIP_NAME)\"
endif

# WLAN_HDD_ADAPTER_MAGIC must be unique for all instances of the driver on the
# system. If it is not defined, then the host driver will use the first 4
# characters (including NULL) of MULTI_IF_NAME to construct
# WLAN_HDD_ADAPTER_MAGIC.
ifdef WLAN_HDD_ADAPTER_MAGIC
CDEFINES += -DWLAN_HDD_ADAPTER_MAGIC=$(WLAN_HDD_ADAPTER_MAGIC)
endif

# Module information used by KBuild framework
obj-$(CONFIG_QCA_CLD_WLAN) += $(MODNAME).o
$(MODNAME)-y := $(OBJS)
