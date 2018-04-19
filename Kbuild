# We can build either as part of a standalone Kernel build or as
# an external module.  Determine which mechanism is being used
ifeq ($(MODNAME),)
	KERNEL_BUILD := y
else
	KERNEL_BUILD := n
endif

ifeq ($(CONFIG_CNSS_QCA6290), y)
	CONFIG_LITHIUM := y
	CONFIG_WLAN_FEATURE_11AX := y
	CONFIG_WLAN_FEATURE_DFS_OFFLOAD := y
	CONFIG_IPA3 := n
endif

ifeq ($(CONFIG_CLD_HL_SDIO_CORE), y)
	CONFIG_QCA_WIFI_SDIO := y
endif

ifeq ($(CONFIG_QCA_WIFI_SDIO), y)
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

ifeq ($(KERNEL_BUILD), y)
	# These are provided in external module based builds
	# Need to explicitly define for Kernel-based builds
	MODNAME := wlan
	WLAN_ROOT := drivers/staging/qcacld-3.0
	WLAN_COMMON_ROOT := ../qca-wifi-host-cmn
	WLAN_COMMON_INC := $(WLAN_ROOT)/$(WLAN_COMMON_ROOT)
endif

# Make WLAN as open-source driver by default
WLAN_OPEN_SOURCE := y

ifeq ($(KERNEL_BUILD), n)
	# These are configurable via Kconfig for kernel-based builds
	# Need to explicitly configure for Android-based builds

	ifeq ($(CONFIG_ICNSS), y)
		CONFIG_HELIUMPLUS := y
		CONFIG_64BIT_PADDR := y
		CONFIG_FEATURE_TSO := y
		CONFIG_FEATURE_TSO_DEBUG := y
		ifeq ($(CONFIG_INET_LRO), y)
			CONFIG_WLAN_LRO := y
		else
			CONFIG_WLAN_LRO := n
		endif
	endif

	ifneq ($(DEVELOPER_DISABLE_BUILD_TIMESTAMP), y)
	ifneq ($(WLAN_DISABLE_BUILD_TAG), y)
	CONFIG_BUILD_TAG := y
	endif
	endif

	ifeq ($(CONFIG_ARCH_MDM9630), y)
	CONFIG_MOBILE_ROUTER := y
	endif

	ifeq ($(CONFIG_ARCH_MDM9640), y)
	CONFIG_MOBILE_ROUTER := y
	endif

	ifeq ($(CONFIG_ARCH_SDX20), y)
	CONFIG_MOBILE_ROUTER := y
	endif

	ifeq ($(CONFIG_ARCH_MSM8917), y)
		ifeq ($(CONFIG_ROME_IF), sdio)
			CONFIG_WLAN_SYNC_TSF_PLUS := y
		endif
	endif

	#Flag to enable Legacy Fast Roaming2(LFR2)
	CONFIG_QCACLD_WLAN_LFR2 := y
	#Flag to enable Legacy Fast Roaming3(LFR3)
	ifneq ($(CONFIG_ARCH_SDX20), y)
	CONFIG_QCACLD_WLAN_LFR3 := y
	endif

	ifneq ($(CONFIG_MOBILE_ROUTER), y)
	#Flag to enable TDLS feature
	CONFIG_QCOM_TDLS := y
	endif

	CONFIG_QCACLD_FEATURE_GREEN_AP := y

	ifeq ($(CONFIG_ARCH_MSM8998), y)
	CONFIG_QCACLD_FEATURE_METERING := y
	endif

	ifeq ($(CONFIG_ARCH_SDM660), y)
	CONFIG_QCACLD_FEATURE_METERING := y
	endif

	ifeq ($(CONFIG_ARCH_SDM630), y)
	CONFIG_QCACLD_FEATURE_METERING := y
	endif

	ifeq ($(CONFIG_ARCH_SDM845), y)
	CONFIG_QCACLD_FEATURE_METERING := y
	endif

	ifeq ($(CONFIG_ARCH_SDM670), y)
	CONFIG_QCACLD_FEATURE_METERING := y
	endif

	#Flag to enable Fast Transition (11r) feature
	CONFIG_QCOM_VOWIFI_11R := y

	#Flag to enable FILS Feature (11ai)
	CONFIG_WLAN_FEATURE_FILS := y
	ifneq ($(CONFIG_QCA_CLD_WLAN),)
		ifeq (y,$(findstring y,$(CONFIG_CNSS) $(CONFIG_CNSS2) $(CONFIG_ICNSS)))
		#Flag to enable Protected Management Frames (11w) feature
		CONFIG_WLAN_FEATURE_11W := y
		#Flag to enable LTE CoEx feature
		CONFIG_QCOM_LTE_COEX := y
			ifneq ($(CONFIG_MOBILE_ROUTER), y)
			#Flag to enable LPSS feature
			CONFIG_WLAN_FEATURE_LPSS := y
			endif
		endif
	endif

	#Flag to enable Protected Management Frames (11w) feature
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

	CONFIG_FEATURE_EPPING := y

	#Flag to enable offload packets feature
	CONFIG_WLAN_OFFLOAD_PACKETS := y

	#enable TSF get feature
	CONFIG_WLAN_SYNC_TSF := y
	#Enable DSRC feature

	ifeq ($(CONFIG_QCA_WIFI_SDIO), y)
	CONFIG_WLAN_FEATURE_DSRC := y
	endif

ifneq ($(CONFIG_ROME_IF),usb)
ifneq ($(CONFIG_ROME_IF),sdio)
	#Flag to enable DISA
	CONFIG_WLAN_FEATURE_DISA := y

	#Flag to enable FIPS
	CONFIG_WLAN_FEATURE_FIPS := y

	#Flag to enable SAE
	CONFIG_WLAN_FEATURE_SAE := y

	#Flag to enable Fast Path feature
	CONFIG_WLAN_FASTPATH := y

	# Flag to enable NAPI
	CONFIG_WLAN_NAPI := y
	CONFIG_WLAN_NAPI_DEBUG := n

	# Flag to enable FW based TX Flow control
	ifeq ($(CONFIG_LITHIUM), y)
		CONFIG_WLAN_TX_FLOW_CONTROL_V2 := y
	else
		CONFIG_WLAN_TX_FLOW_CONTROL_V2 := n
	endif

endif
endif

CONFIG_POWER_MANAGEMENT_OFFLOAD := y

ifeq ($(CONFIG_ROME_IF), snoc)
	CONFIG_WLAN_TX_FLOW_CONTROL_V2 := y
endif

	# Flag to enable LFR Subnet Detection
	CONFIG_LFR_SUBNET_DETECTION := y

	# Flag to enable MCC to SCC switch feature
	CONFIG_MCC_TO_SCC_SWITCH := y

ifeq ($(CONFIG_SLUB_DEBUG), y)
	# Enable Obj Mgr Degug services if slub build
	CONFIG_WLAN_OBJMGR_DEBUG:= y
endif
endif

ifeq ($(CONFIG_HIF_PCI), y)
ifneq ($(CONFIG_WLAN_TX_FLOW_CONTROL_V2), y)
ifneq ($(CONFIG_LITHIUM), y)
CONFIG_WLAN_TX_FLOW_CONTROL_LEGACY := y
endif
endif
endif

#Whether have QMI support
CONFIG_QMI_SUPPORT := y

ifeq ($(CONFIG_ICNSS), y)
CONFIG_WIFI_3_0_ADRASTEA := y
CONFIG_ADRASTEA_RRI_ON_DDR := y
# Enable athdiag procfs debug support for adrastea
CONFIG_ATH_PROCFS_DIAG_SUPPORT := y
# Enable 11AC TX compact feature for adrastea
CONFIG_11AC_TXCOMPACT := y
ifeq ($(CONFIG_QMI_SUPPORT), y)
CONFIG_ADRASTEA_SHADOW_REGISTERS := y
endif
endif

# NOTE: CONFIG_64BIT_PADDR requires CONFIG_HELIUMPLUS
ifeq ($(CONFIG_HELIUMPLUS), y)
CONFIG_AR900B := y

ifeq ($(CONFIG_64BIT_PADDR), y)
CONFIG_HTT_PADDR64 := y
endif

ifeq ($(CONFIG_SLUB_DEBUG_ON), y)
CONFIG_OL_RX_INDICATION_RECORD := y
CONFIG_TSOSEG_DEBUG := y
endif

endif #CONFIG_HELIUMPLUS

ifeq ($(CONFIG_LITHIUM), y)
CONFIG_SHADOW_V2 := y
CONFIG_QCA6290_HEADERS_DEF := y
CONFIG_QCA_WIFI_QCA6290 := y
CONFIG_QCA_WIFI_QCA8074 := y
CONFIG_QCA_WIFI_QCA8074_VP := y
CONFIG_DP_INTR_POLL_BASED := y
CONFIG_TX_PER_PDEV_DESC_POOL := y
CONFIG_DP_TRACE := y
CONFIG_FEATURE_TSO := y
CONFIG_TSO_DEBUG_LOG_ENABLE := y
CONFIG_DP_LFR := y
CONFIG_HTT_PADDR64 := y
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
	CONFIG_WLAN_DEBUG_CRASH_INJECT := y
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

# Feature flags which are not (currently) configurable via Kconfig

#Whether to build debug version
BUILD_DEBUG_VERSION := y

#Enable this flag to build driver in diag version
BUILD_DIAG_VERSION := y

ifeq ($(CONFIG_SLUB_DEBUG), y)
	PANIC_ON_BUG := y
	WLAN_WARN_ON_ASSERT := y
else ifeq ($(CONFIG_PERF_DEBUG), y)
	PANIC_ON_BUG := y
	WLAN_WARN_ON_ASSERT := y
else
	PANIC_ON_BUG := n
	WLAN_WARN_ON_ASSERT := n
endif

# Compile all log levels by default
CONFIG_WLAN_LOG_FATAL := y
CONFIG_WLAN_LOG_ERROR := y
CONFIG_WLAN_LOG_WARN := y
CONFIG_WLAN_LOG_INFO := y
CONFIG_WLAN_LOG_DEBUG := y

#Enable OL debug and wmi unified functions
CONFIG_ATH_PERF_PWR_OFFLOAD := y

#Disable packet log
CONFIG_REMOVE_PKT_LOG := n

#Enable 11AC TX
ifeq ($(CONFIG_ROME_IF),pci)
	CONFIG_ATH_11AC_TXCOMPACT := y
endif
ifeq ($(CONFIG_ROME_IF),usb)
	CONFIG_ATH_11AC_TXCOMPACT := n
endif

#Enable PCI specific APIS (dma, etc)
ifeq ($(CONFIG_ROME_IF),pci)
	CONFIG_HIF_PCI := y
endif

#Enable USB specific APIS
ifeq ($(CONFIG_ROME_IF),usb)
	CONFIG_HIF_USB := y
	CONFIG_PLD_USB_CNSS := y
endif

#Enable SDIO specific APIS
ifeq ($(CONFIG_ROME_IF),sdio)
	CONFIG_HIF_SDIO := y
endif

#Enable pci read/write config functions
ifeq ($(CONFIG_ROME_IF),pci)
	CONFIG_ATH_PCI := y
endif

ifeq ($(CONFIG_ROME_IF),snoc)
	CONFIG_HIF_SNOC:= y
endif

# enable/disable feature flags based upon mobile router profile
ifeq ($(CONFIG_MOBILE_ROUTER), y)
CONFIG_FEATURE_WLAN_MCC_TO_SCC_SWITCH := y
CONFIG_FEATURE_WLAN_AUTO_SHUTDOWN := y
CONFIG_FEATURE_WLAN_AP_AP_ACS_OPTIMIZE := y
CONFIG_FEATURE_WLAN_STA_4ADDR_SCHEME := y
CONFIG_MDM_PLATFORM := y
CONFIG_FEATURE_WLAN_STA_AP_MODE_DFS_DISABLE := y
CONFIG_FEATURE_AP_MCC_CH_AVOIDANCE := y
else
CONFIG_QCOM_ESE := y
CONFIG_QCA_IBSS_SUPPORT := y
CONFIG_WLAN_OPEN_P2P_INTERFACE := y
CONFIG_WLAN_ENABLE_SOCIAL_CHANNELS_5G_ONLY := y
endif

#Enable power management suspend/resume functionality to PCI
CONFIG_ATH_BUS_PM := y

#Enable FLOWMAC module support
CONFIG_ATH_SUPPORT_FLOWMAC_MODULE := n

#Enable spectral support
CONFIG_ATH_SUPPORT_SPECTRAL := n

#Enable WDI Event support
CONFIG_WDI_EVENT_ENABLE := y

#Endianness selection
CONFIG_LITTLE_ENDIAN := y

#Enable TX reclaim support
CONFIG_TX_CREDIT_RECLAIM_SUPPORT := n

#Enable FTM support
CONFIG_QCA_WIFI_FTM := y

#Enable Checksum Offload
CONFIG_CHECKSUM_OFFLOAD := y

#Enable GTK offload
CONFIG_GTK_OFFLOAD := y

#Enable EXT WOW
ifeq ($(CONFIG_HIF_PCI), y)
	CONFIG_EXT_WOW := y
endif

#Set this to 1 to catch erroneous Target accesses during debug.
CONFIG_ATH_PCIE_ACCESS_DEBUG := n

#Enable IPA offload
ifeq ($(CONFIG_IPA), y)
CONFIG_IPA_OFFLOAD := y
endif
ifeq ($(CONFIG_IPA3), y)
CONFIG_IPA_OFFLOAD := y
endif

#Flag to enable SMMU S1 support
ifeq ($(CONFIG_ARCH_SDM845), y)
ifeq ($(CONFIG_IPA_OFFLOAD), y)
CONFIG_ENABLE_SMMU_S1_TRANSLATION := y
endif
endif

ifeq ($(CONFIG_ARCH_SDX20), y)
ifeq ($(CONFIG_QCA_WIFI_SDIO), y)
ifeq ($(CONFIG_WCNSS_SKB_PRE_ALLOC), y)
CONFIG_FEATURE_SKB_PRE_ALLOC := y
endif
endif
endif

#Enable Signed firmware support for split binary format
CONFIG_QCA_SIGNED_SPLIT_BINARY_SUPPORT := n

#Enable single firmware binary format
CONFIG_QCA_SINGLE_BINARY_SUPPORT := n

#Enable collecting target RAM dump after kernel panic
CONFIG_TARGET_RAMDUMP_AFTER_KERNEL_PANIC := y

#Flag to enable/disable secure firmware feature
CONFIG_FEATURE_SECURE_FIRMWARE := n

#Flag to enable Stats Ext implementation
CONFIG_FEATURE_STATS_EXT := y

#Flag to enable HTC credit history feature
CONFIG_FEATURE_HTC_CREDIT_HISTORY := y

#Flag to enable MTRACE feature
CONFIG_TRACE_RECORD_FEATURE := y

#Flag to enable p2p debug feature
CONFIG_WLAN_FEATURE_P2P_DEBUG := y

#Flag to enable DFS Master feature
CONFIG_WLAN_DFS_MASTER_ENABLE := y

#Flag to enable nud tracking feature
CONFIG_WLAN_NUD_TRACKING := y

CONFIG_WIFI_POS_CONVERGED := y
ifneq ($(CONFIG_WIFI_POS_CONVERGED), y)
CONFIG_WIFI_POS_LEGACY := y
endif

CONFIG_CP_STATS := y

CONFIG_FEATURE_WLAN_WAPI := y

CONFIG_AGEIE_ON_SCAN_RESULTS := y

CONFIG_PTT_SOCK_SVC_ENABLE := y
CONFIG_SOFTAP_CHANNEL_RANGE := y
CONFIG_FEATURE_WLAN_SCAN_PNO := y
CONFIG_WLAN_FEATURE_PACKET_FILTERING := y
CONFIG_WLAN_NS_OFFLOAD := y
CONFIG_FEATURE_WLAN_RA_FILTERING:= y
CONFIG_FEATURE_WLAN_LPHB := y
CONFIG_QCA_SUPPORT_TX_THROTTLE := y
CONFIG_WMI_INTERFACE_EVENT_LOGGING := y
CONFIG_WLAN_FEATURE_LINK_LAYER_STATS := y
CONFIG_FEATURE_WLAN_EXTSCAN := y
CONFIG_160MHZ_SUPPORT := y
CONFIG_MCL := y
CONFIG_MCL_REGDB := y
CONFIG_LEGACY_CHAN_ENUM := y
CONFIG_NAPIER_SCAN := y
CONFIG_WLAN_PMO_ENABLE := y
CONFIG_CONVERGED_P2P_ENABLE := y
CONFIG_WLAN_POLICY_MGR_ENABLE := y
CONFIG_SUPPORT_11AX := y
CONFIG_HDD_INIT_WITH_RTNL_LOCK := y
CONFIG_CONVERGED_TDLS_ENABLE := y
CONFIG_WLAN_CONV_SPECTRAL_ENABLE := y
CONFIG_WLAN_SPECTRAL_ENABLE := y
CONFIG_WMI_CMD_STRINGS := y

ifeq ($(CONFIG_HELIUMPLUS), y)
ifneq ($(CONFIG_FORCE_ALLOC_FROM_DMA_ZONE), y)
CONFIG_ENABLE_DEBUG_ADDRESS_MARKING := y
endif
endif

ifeq ($(CONFIG_SLUB_DEBUG_ON), y)
	CONFIG_FEATURE_UNIT_TEST_SUSPEND := y
	CONFIG_LEAK_DETECTION := y
endif

# enable unit-test suspend for napier builds
ifeq ($(CONFIG_LITHIUM), y)
	CONFIG_FEATURE_UNIT_TEST_SUSPEND := y
endif

#Flag to enable/disable WLAN D0-WOW
ifeq ($(CONFIG_PCI_MSM), y)
ifeq ($(CONFIG_HIF_PCI), y)
CONFIG_FEATURE_WLAN_D0WOW := y
endif
endif

#Enable Channel Matrix restriction for all Rome only targets
ifneq ($(CONFIG_ICNSS), y)
CONFIG_CHNL_MATRIX_RESTRICTION := y
endif

ifeq ($(CONFIG_ARCH_MDM9607), y)
CONFIG_TUFELLO_DUAL_FW_SUPPORT := y
endif

ifeq ($(CONFIG_ARCH_MSM8996), y)
CONFIG_CHANNEL_HOPPING_ALL_BANDS := y
endif

ifneq ($(CONFIG_HIF_USB), y)
CONFIG_WLAN_LOGGING_SOCK_SVC := y
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
		$(HDD_SRC_DIR)/wlan_hdd_cfg80211.o \
		$(HDD_SRC_DIR)/wlan_hdd_data_stall_detection.o \
		$(HDD_SRC_DIR)/wlan_hdd_driver_ops.o \
		$(HDD_SRC_DIR)/wlan_hdd_ext_scan.o \
		$(HDD_SRC_DIR)/wlan_hdd_ftm.o \
		$(HDD_SRC_DIR)/wlan_hdd_hostapd.o \
		$(HDD_SRC_DIR)/wlan_hdd_ioctl.o \
		$(HDD_SRC_DIR)/wlan_hdd_main.o \
		$(HDD_SRC_DIR)/wlan_hdd_memdump.o \
		$(HDD_SRC_DIR)/wlan_hdd_object_manager.o \
		$(HDD_SRC_DIR)/wlan_hdd_oemdata.o \
		$(HDD_SRC_DIR)/wlan_hdd_p2p.o \
		$(HDD_SRC_DIR)/wlan_hdd_packet_filter.o \
		$(HDD_SRC_DIR)/wlan_hdd_power.o \
		$(HDD_SRC_DIR)/wlan_hdd_regulatory.o \
		$(HDD_SRC_DIR)/wlan_hdd_request_manager.o \
		$(HDD_SRC_DIR)/wlan_hdd_scan.o \
		$(HDD_SRC_DIR)/wlan_hdd_softap_tx_rx.o \
		$(HDD_SRC_DIR)/wlan_hdd_spectralscan.o \
		$(HDD_SRC_DIR)/wlan_hdd_stats.o \
		$(HDD_SRC_DIR)/wlan_hdd_sysfs.o \
		$(HDD_SRC_DIR)/wlan_hdd_trace.o \
		$(HDD_SRC_DIR)/wlan_hdd_tx_rx.o \
		$(HDD_SRC_DIR)/wlan_hdd_wext.o \
		$(HDD_SRC_DIR)/wlan_hdd_wmm.o \
		$(HDD_SRC_DIR)/wlan_hdd_wowl.o

ifeq ($(CONFIG_WLAN_DEBUGFS), y)
HDD_OBJS += $(HDD_SRC_DIR)/wlan_hdd_debugfs.o
HDD_OBJS += $(HDD_SRC_DIR)/wlan_hdd_debugfs_llstat.o
endif

ifeq ($(CONFIG_WLAN_FEATURE_DSRC), y)
HDD_OBJS+=	$(HDD_SRC_DIR)/wlan_hdd_ocb.o
endif

ifeq ($(CONFIG_WLAN_FEATURE_FIPS), y)
HDD_OBJS+=	$(HDD_SRC_DIR)/wlan_hdd_fips.o
endif

ifeq ($(CONFIG_QCACLD_FEATURE_GREEN_AP), y)
HDD_OBJS+=	$(HDD_SRC_DIR)/wlan_hdd_green_ap.o
endif

ifeq ($(CONFIG_WLAN_FEATURE_LPSS), y)
HDD_OBJS +=	$(HDD_SRC_DIR)/wlan_hdd_lpass.o
endif

ifeq ($(CONFIG_WLAN_LRO), y)
HDD_OBJS +=     $(HDD_SRC_DIR)/wlan_hdd_lro.o
endif

ifeq ($(CONFIG_WLAN_NAPI), y)
HDD_OBJS +=     $(HDD_SRC_DIR)/wlan_hdd_napi.o
endif

ifeq ($(CONFIG_IPA_OFFLOAD), y)
HDD_OBJS +=	$(HDD_SRC_DIR)/wlan_hdd_ipa.o
endif

ifeq ($(CONFIG_QCACLD_FEATURE_NAN), y)
HDD_OBJS +=	$(HDD_SRC_DIR)/wlan_hdd_nan.o
endif

ifeq ($(CONFIG_QCOM_TDLS), y)
HDD_OBJS +=	$(HDD_SRC_DIR)/wlan_hdd_tdls.o
endif

ifeq ($(CONFIG_WLAN_SYNC_TSF_PLUS), y)
CONFIG_WLAN_SYNC_TSF := y
endif

ifeq ($(CONFIG_WLAN_SYNC_TSF), y)
HDD_OBJS +=	$(HDD_SRC_DIR)/wlan_hdd_tsf.o
endif

ifeq ($(CONFIG_MPC_UT_FRAMEWORK), y)
HDD_OBJS +=	$(HDD_SRC_DIR)/wlan_hdd_conc_ut.o
endif

ifeq ($(CONFIG_WLAN_FEATURE_DISA), y)
HDD_OBJS += $(HDD_SRC_DIR)/wlan_hdd_disa.o
endif

ifeq ($(CONFIG_LFR_SUBNET_DETECTION), y)
HDD_OBJS +=	$(HDD_SRC_DIR)/wlan_hdd_subnet_detect.o
endif

ifeq ($(CONFIG_WLAN_FEATURE_NAN_DATAPATH), y)
HDD_OBJS += $(HDD_SRC_DIR)/wlan_hdd_nan_datapath.o
endif

ifeq ($(CONFIG_WLAN_FEATURE_11AX), y)
HDD_OBJS += $(HDD_SRC_DIR)/wlan_hdd_he.o
endif

ifeq ($(CONFIG_LITHIUM), y)
HDD_OBJS += $(HDD_SRC_DIR)/wlan_hdd_rx_monitor.o
endif

ifeq ($(CONFIG_WLAN_NUD_TRACKING), y)
HDD_OBJS += $(HDD_SRC_DIR)/wlan_hdd_nud_tracking.o
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

ifeq ($(CONFIG_QCOM_TDLS), y)
MAC_LIM_OBJS += $(MAC_SRC_DIR)/pe/lim/lim_process_tdls.o
endif

ifeq ($(CONFIG_WLAN_FEATURE_FILS), y)
MAC_LIM_OBJS += $(MAC_SRC_DIR)/pe/lim/lim_process_fils.o
endif

ifeq ($(CONFIG_WLAN_FEATURE_NAN_DATAPATH), y)
MAC_NDP_OBJS += $(MAC_SRC_DIR)/pe/nan/nan_datapath.o
endif

ifeq ($(CONFIG_QCACLD_WLAN_LFR2), y)
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

############ DFS ############
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

DFS_OBJS :=	$(DFS_CORE_SRC_DIR)/misc/dfs.o \
		$(DFS_CORE_SRC_DIR)/misc/dfs_cac.o \
		$(DFS_CORE_SRC_DIR)/misc/dfs_nol.o \
		$(DFS_CORE_SRC_DIR)/misc/dfs_random_chan_sel.o \
		$(DFS_CORE_SRC_DIR)/misc/dfs_process_radar_found_ind.o \
		$(DFS_DISP_SRC_DIR)/wlan_dfs_init_deinit_api.o \
		$(DFS_DISP_SRC_DIR)/wlan_dfs_lmac_api.o \
		$(DFS_DISP_SRC_DIR)/wlan_dfs_mlme_api.o \
		$(DFS_DISP_SRC_DIR)/wlan_dfs_tgt_api.o \
		$(DFS_DISP_SRC_DIR)/wlan_dfs_ucfg_api.o \
		$(DFS_DISP_SRC_DIR)/wlan_dfs_tgt_api.o \
		$(DFS_DISP_SRC_DIR)/wlan_dfs_utils_api.o \
		$(WLAN_COMMON_ROOT)/target_if/dfs/src/target_if_dfs.o

ifeq ($(CONFIG_WLAN_FEATURE_DFS_OFFLOAD), y)
DFS_OBJS +=	$(WLAN_COMMON_ROOT)/target_if/dfs/src/target_if_dfs_full_offload.o \
		$(DFS_CORE_SRC_DIR)/misc/dfs_full_offload.o
else
DFS_OBJS +=	$(WLAN_COMMON_ROOT)/target_if/dfs/src/target_if_dfs_partial_offload.o \
		$(DFS_CORE_SRC_DIR)/filtering/dfs_fcc_bin5.o \
		$(DFS_CORE_SRC_DIR)/filtering/dfs_bindetects.o \
		$(DFS_CORE_SRC_DIR)/filtering/dfs_debug.o \
		$(DFS_CORE_SRC_DIR)/filtering/dfs_init.o \
		$(DFS_CORE_SRC_DIR)/filtering/dfs_misc.o \
		$(DFS_CORE_SRC_DIR)/filtering/dfs_phyerr_tlv.o \
		$(DFS_CORE_SRC_DIR)/filtering/dfs_process_phyerr.o \
		$(DFS_CORE_SRC_DIR)/filtering/dfs_process_radarevent.o \
		$(DFS_CORE_SRC_DIR)/filtering/dfs_staggered.o \
		$(DFS_CORE_SRC_DIR)/filtering/dfs_radar.o \
		$(DFS_CORE_SRC_DIR)/filtering/dfs_ar.o \
		$(DFS_CORE_SRC_DIR)/filtering/dfs_partial_offload_radar.o \
		$(DFS_CORE_SRC_DIR)/misc/dfs_filter_init.o \
		$(DFS_CORE_SRC_DIR)/misc/dfs_zero_cac.o
endif

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


ifeq ($(CONFIG_QCACLD_WLAN_LFR2), y)
SME_CSR_OBJS += $(SME_SRC_DIR)/csr/csr_roam_preauth.o \
		$(SME_SRC_DIR)/csr/csr_host_scan_roam.o
endif

SME_QOS_OBJS := $(SME_SRC_DIR)/qos/sme_qos.o

SME_CMN_OBJS := $(SME_SRC_DIR)/common/sme_api.o \
		$(SME_SRC_DIR)/common/sme_ft_api.o \
		$(SME_SRC_DIR)/common/sme_power_save.o \
		$(SME_SRC_DIR)/common/sme_trace.o

SME_RRM_OBJS := $(SME_SRC_DIR)/rrm/sme_rrm.o

ifeq ($(CONFIG_QCACLD_FEATURE_NAN), y)
SME_NAN_OBJS = $(SME_SRC_DIR)/nan/nan_api.o
endif

ifeq ($(CONFIG_WLAN_FEATURE_NAN_DATAPATH), y)
SME_NDP_OBJS += $(SME_SRC_DIR)/nan/nan_datapath_api.o
endif

SME_OBJS :=	$(SME_CMN_OBJS) \
		$(SME_CSR_OBJS) \
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
WLAN_LOGGING_OBJS    := $(WLAN_LOGGING_SRC_DIR)/wlan_logging_sock_svc.o \
		$(WLAN_LOGGING_SRC_DIR)/wlan_roam_debug.o

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
QDF_OS_SRC_DIR := $(QDF_OS_DIR)/src
QDF_OS_LINUX_SRC_DIR := $(QDF_OS_DIR)/linux/src
QDF_OBJ_DIR := $(WLAN_COMMON_ROOT)/$(QDF_OS_SRC_DIR)
QDF_LINUX_OBJ_DIR := $(WLAN_COMMON_ROOT)/$(QDF_OS_LINUX_SRC_DIR)

QDF_INC :=	-I$(WLAN_COMMON_INC)/$(QDF_OS_INC_DIR) \
		-I$(WLAN_COMMON_INC)/$(QDF_OS_LINUX_SRC_DIR)

QDF_OBJS := 	$(QDF_LINUX_OBJ_DIR)/qdf_defer.o \
		$(QDF_LINUX_OBJ_DIR)/qdf_event.o \
		$(QDF_LINUX_OBJ_DIR)/qdf_file.o \
		$(QDF_LINUX_OBJ_DIR)/qdf_list.o \
		$(QDF_LINUX_OBJ_DIR)/qdf_lock.o \
		$(QDF_LINUX_OBJ_DIR)/qdf_mc_timer.o \
		$(QDF_LINUX_OBJ_DIR)/qdf_mem.o \
		$(QDF_LINUX_OBJ_DIR)/qdf_nbuf.o \
		$(QDF_LINUX_OBJ_DIR)/qdf_threads.o \
		$(QDF_LINUX_OBJ_DIR)/qdf_crypto.o \
		$(QDF_LINUX_OBJ_DIR)/qdf_trace.o \
		$(QDF_OBJ_DIR)/qdf_flex_mem.o \
		$(QDF_OBJ_DIR)/qdf_parse.o \
		$(QDF_OBJ_DIR)/qdf_platform.o \
		$(QDF_OBJ_DIR)/qdf_str.o \
		$(QDF_OBJ_DIR)/qdf_types.o \

ifeq ($(CONFIG_WLAN_DEBUGFS), y)
QDF_OBJS += $(QDF_LINUX_OBJ_DIR)/qdf_debugfs.o
endif

ifeq ($(CONFIG_IPA_OFFLOAD), y)
QDF_OBJS += $(QDF_LINUX_OBJ_DIR)/qdf_ipa.o
endif

# enable CPU hotplug support if SMP is enabled
ifeq ($(CONFIG_SMP), y)
	QDF_OBJS += $(QDF_OBJ_DIR)/qdf_cpuhp.o
	QDF_OBJS += $(QDF_LINUX_OBJ_DIR)/qdf_cpuhp.o
endif

ifeq ($(CONFIG_LEAK_DETECTION), y)
	QDF_OBJS += $(QDF_OBJ_DIR)/qdf_debug_domain.o
endif

##########OS_IF #######
OS_IF_DIR := $(WLAN_COMMON_ROOT)/os_if

OS_IF_INC := -I$(WLAN_COMMON_INC)/os_if/linux \
            -I$(WLAN_COMMON_INC)/os_if/linux/scan/inc \
            -I$(WLAN_COMMON_INC)/os_if/linux/p2p/inc \
            -I$(WLAN_COMMON_INC)/os_if/linux/spectral/inc \
            -I$(WLAN_COMMON_INC)/os_if/linux/tdls/inc

OS_IF_OBJ := $(OS_IF_DIR)/linux/p2p/src/wlan_cfg80211_p2p.o \
	     $(OS_IF_DIR)/linux/wlan_osif_request_manager.o

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
		$(UMAC_SCAN_CORE_DIR)/wlan_scan_11d.o \
		$(UMAC_SCAN_CORE_DIR)/wlan_scan_bss_score.o \
		$(UMAC_SCAN_CORE_DIR)/wlan_scan_filter.o \
		$(UMAC_SCAN_CORE_DIR)/wlan_scan_main.o \
		$(UMAC_SCAN_CORE_DIR)/wlan_scan_manager.o \
		$(UMAC_SCAN_DISP_DIR)/wlan_scan_tgt_api.o \
		$(UMAC_SCAN_DISP_DIR)/wlan_scan_ucfg_api.o \
		$(UMAC_SCAN_DISP_DIR)/wlan_scan_utils_api.o \
		$(WLAN_COMMON_ROOT)/os_if/linux/scan/src/wlan_cfg80211_scan.o \
		$(WLAN_COMMON_ROOT)/os_if/linux/wlan_cfg80211.o \
		$(WLAN_COMMON_ROOT)/target_if/scan/src/target_if_scan.o

############# UMAC_SPECTRAL_SCAN ############
UMAC_SPECTRAL_DIR := spectral
UMAC_SPECTRAL_DISP_INC_DIR := $(UMAC_SPECTRAL_DIR)/dispatcher/inc
UMAC_SPECTRAL_CORE_INC_DIR := $(UMAC_SPECTRAL_DIR)/core
UMAC_SPECTRAL_CORE_DIR := $(WLAN_COMMON_ROOT)/$(UMAC_SPECTRAL_DIR)/core
UMAC_SPECTRAL_DISP_DIR := $(WLAN_COMMON_ROOT)/$(UMAC_SPECTRAL_DIR)/dispatcher/src
UMAC_TARGET_SPECTRAL_INC := -I$(WLAN_COMMON_INC)/target_if/spectral

UMAC_SPECTRAL_INC := -I$(WLAN_COMMON_INC)/$(UMAC_SPECTRAL_DISP_INC_DIR) \
			-I$(WLAN_COMMON_INC)/$(UMAC_SPECTRAL_CORE_INC_DIR) \
			-I$(WLAN_COMMON_INC)/target_if/direct_buf_rx/inc
UMAC_SPECTRAL_OBJS := $(UMAC_SPECTRAL_CORE_DIR)/spectral_offload.o \
		$(UMAC_SPECTRAL_CORE_DIR)/spectral_common.o \
		$(UMAC_SPECTRAL_DISP_DIR)/wlan_spectral_ucfg_api.o \
		$(UMAC_SPECTRAL_DISP_DIR)/wlan_spectral_utils_api.o \
		$(UMAC_SPECTRAL_DISP_DIR)/wlan_spectral_tgt_api.o \
		$(WLAN_COMMON_ROOT)/os_if/linux/spectral/src/wlan_cfg80211_spectral.o \
		$(WLAN_COMMON_ROOT)/os_if/linux/spectral/src/os_if_spectral_netlink.o \
		$(WLAN_COMMON_ROOT)/target_if/spectral/target_if_spectral_netlink.o \
		$(WLAN_COMMON_ROOT)/target_if/spectral/target_if_spectral_phyerr.o \
		$(WLAN_COMMON_ROOT)/target_if/spectral/target_if_spectral.o \
		$(WLAN_COMMON_ROOT)/target_if/spectral/target_if_spectral_sim.o

############# UMAC_GREEN_AP ############
UMAC_GREEN_AP_DIR := umac/green_ap
UMAC_GREEN_AP_DISP_INC_DIR := $(UMAC_GREEN_AP_DIR)/dispatcher/inc
UMAC_GREEN_AP_CORE_DIR := $(WLAN_COMMON_ROOT)/$(UMAC_GREEN_AP_DIR)/core/src
UMAC_GREEN_AP_DISP_DIR := $(WLAN_COMMON_ROOT)/$(UMAC_GREEN_AP_DIR)/dispatcher/src
UMAC_TARGET_GREEN_AP_INC := -I$(WLAN_COMMON_INC)/target_if/green_ap/inc

UMAC_GREEN_AP_INC := -I$(WLAN_COMMON_INC)/$(UMAC_GREEN_AP_DISP_INC_DIR)
UMAC_GREEN_AP_OBJS := $(UMAC_GREEN_AP_CORE_DIR)/wlan_green_ap_main.o \
		$(UMAC_GREEN_AP_DISP_DIR)/wlan_green_ap_api.o \
                $(UMAC_GREEN_AP_DISP_DIR)/wlan_green_ap_ucfg_api.o \
                $(WLAN_COMMON_ROOT)/target_if/green_ap/src/target_if_green_ap.o

############# FTM CORE ############
FTM_CORE_DIR := ftm
TARGET_IF_FTM_DIR := target_if/ftm
OS_IF_LINUX_FTM_DIR := os_if/linux/ftm

FTM_CORE_SRC := $(WLAN_COMMON_ROOT)/$(FTM_CORE_DIR)/core/src
FTM_DISP_SRC := $(WLAN_COMMON_ROOT)/$(FTM_CORE_DIR)/dispatcher/src
TARGET_IF_FTM_SRC := $(WLAN_COMMON_ROOT)/$(TARGET_IF_FTM_DIR)/src
OS_IF_FTM_SRC := $(WLAN_COMMON_ROOT)/$(OS_IF_LINUX_FTM_DIR)/src

FTM_CORE_INC := $(WLAN_COMMON_INC)/$(FTM_CORE_DIR)/core/src
FTM_DISP_INC := $(WLAN_COMMON_INC)/$(FTM_CORE_DIR)/dispatcher/inc
TARGET_IF_FTM_INC := $(WLAN_COMMON_INC)/$(TARGET_IF_FTM_DIR)/inc
OS_IF_FTM_INC := $(WLAN_COMMON_INC)/$(OS_IF_LINUX_FTM_DIR)/inc

FTM_INC := -I$(FTM_DISP_INC)	\
	   -I$(FTM_CORE_INC)	\
	   -I$(OS_IF_FTM_INC)	\
	   -I$(TARGET_IF_FTM_INC)

FTM_OBJS := $(FTM_DISP_SRC)/wlan_ftm_init_deinit.o \
	    $(FTM_DISP_SRC)/wlan_ftm_ucfg_api.o \
	    $(FTM_CORE_SRC)/wlan_ftm_svc.o \
	    $(OS_IF_FTM_SRC)/wlan_cfg80211_ftm.o \
	    $(TARGET_IF_FTM_SRC)/target_if_ftm.o

ifeq ($(CONFIG_LINUX_QCMBR), y)
FTM_OBJS += $(OS_IF_FTM_SRC)/wlan_ioctl_ftm.o
endif

############# UMAC_CMN_SERVICES ############
UMAC_COMMON_INC := -I$(WLAN_COMMON_INC)/umac/cmn_services/cmn_defs/inc \
		-I$(WLAN_COMMON_INC)/umac/cmn_services/utils/inc
UMAC_COMMON_OBJS := $(WLAN_COMMON_ROOT)/umac/cmn_services/utils/src/wlan_utility.o

ifeq ($(CONFIG_WLAN_LRO), y)
QDF_OBJS +=     $(QDF_LINUX_OBJ_DIR)/qdf_lro.o
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
		$(UMAC_OBJMGR_DIR)/src/wlan_objmgr_vdev_obj.o

ifeq ($(CONFIG_WLAN_OBJMGR_DEBUG), y)
UMAC_OBJMGR_OBJS += $(UMAC_OBJMGR_DIR)/src/wlan_objmgr_debug.o
endif

###########  UMAC MGMT TXRX ##########
UMAC_MGMT_TXRX_DIR := $(WLAN_COMMON_ROOT)/umac/cmn_services/mgmt_txrx

UMAC_MGMT_TXRX_INC := -I$(WLAN_COMMON_INC)/umac/cmn_services/mgmt_txrx/dispatcher/inc \

UMAC_MGMT_TXRX_OBJS := $(UMAC_MGMT_TXRX_DIR)/core/src/wlan_mgmt_txrx_main.o \
	$(UMAC_MGMT_TXRX_DIR)/dispatcher/src/wlan_mgmt_txrx_utils_api.o \
	$(UMAC_MGMT_TXRX_DIR)/dispatcher/src/wlan_mgmt_txrx_tgt_api.o

########## POWER MANAGEMENT OFFLOADS (PMO) ##########
PMO_DIR :=	components/pmo
PMO_INC :=	-I$(WLAN_ROOT)/$(PMO_DIR)/core/inc \
			-I$(WLAN_ROOT)/$(PMO_DIR)/core/src \
			-I$(WLAN_ROOT)/$(PMO_DIR)/dispatcher/inc \
			-I$(WLAN_ROOT)/$(PMO_DIR)/dispatcher/src \

ifeq ($(CONFIG_POWER_MANAGEMENT_OFFLOAD), y)
PMO_OBJS :=     $(PMO_DIR)/core/src/wlan_pmo_main.o \
		$(PMO_DIR)/core/src/wlan_pmo_apf.o \
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
endif

########## DISA (ENCRYPTION TEST) ##########

DISA_DIR :=	components/disa
DISA_INC :=	-I$(WLAN_ROOT)/$(DISA_DIR)/core/inc \
		-I$(WLAN_ROOT)/$(DISA_DIR)/dispatcher/inc

ifeq ($(CONFIG_WLAN_FEATURE_DISA), y)
DISA_OBJS :=	$(DISA_DIR)/core/src/wlan_disa_main.o \
		$(DISA_DIR)/dispatcher/src/wlan_disa_obj_mgmt_api.o \
		$(DISA_DIR)/dispatcher/src/wlan_disa_tgt_api.o \
		$(DISA_DIR)/dispatcher/src/wlan_disa_ucfg_api.o
endif

######## OCB ##############
OCB_DIR := components/ocb
OCB_INC := -I$(WLAN_ROOT)/$(OCB_DIR)/core/inc \
		-I$(WLAN_ROOT)/$(OCB_DIR)/dispatcher/inc

ifeq ($(CONFIG_WLAN_FEATURE_DSRC), y)
OCB_OBJS :=	$(OCB_DIR)/dispatcher/src/wlan_ocb_ucfg_api.o \
		$(OCB_DIR)/dispatcher/src/wlan_ocb_tgt_api.o \
		$(OCB_DIR)/core/src/wlan_ocb_main.o
endif

######## IPA ##############
IPA_DIR := components/ipa
IPA_INC := -I$(WLAN_ROOT)/$(IPA_DIR)/core/inc \
		-I$(WLAN_ROOT)/$(IPA_DIR)/dispatcher/inc

ifeq ($(CONFIG_IPA_OFFLOAD), y)
IPA_OBJS :=	$(IPA_DIR)/dispatcher/src/wlan_ipa_ucfg_api.o \
		$(IPA_DIR)/dispatcher/src/wlan_ipa_obj_mgmt_api.o \
		$(IPA_DIR)/dispatcher/src/wlan_ipa_tgt_api.o \
		$(IPA_DIR)/core/src/wlan_ipa_main.o \
		$(IPA_DIR)/core/src/wlan_ipa_core.o \
		$(IPA_DIR)/core/src/wlan_ipa_stats.o \
		$(IPA_DIR)/core/src/wlan_ipa_rm.o
endif

########## CLD TARGET_IF #######
CLD_TARGET_IF_DIR := components/target_if

CLD_TARGET_IF_INC := -I$(WLAN_ROOT)/$(CLD_TARGET_IF_DIR)/pmo/inc \
	 -I$(WLAN_ROOT)/$(CLD_TARGET_IF_DIR)/pmo/src

CLD_TARGET_IF_OBJ := $(CLD_TARGET_IF_DIR)/pmo/src/target_if_pmo_arp.o \
		$(CLD_TARGET_IF_DIR)/pmo/src/target_if_pmo_gtk.o \
		$(CLD_TARGET_IF_DIR)/pmo/src/target_if_pmo_hw_filter.o \
		$(CLD_TARGET_IF_DIR)/pmo/src/target_if_pmo_lphb.o \
		$(CLD_TARGET_IF_DIR)/pmo/src/target_if_pmo_main.o \
		$(CLD_TARGET_IF_DIR)/pmo/src/target_if_pmo_mc_addr_filtering.o \
		$(CLD_TARGET_IF_DIR)/pmo/src/target_if_pmo_ns.o \
		$(CLD_TARGET_IF_DIR)/pmo/src/target_if_pmo_pkt_filter.o \
		$(CLD_TARGET_IF_DIR)/pmo/src/target_if_pmo_static_config.o \
		$(CLD_TARGET_IF_DIR)/pmo/src/target_if_pmo_suspend_resume.o \
		$(CLD_TARGET_IF_DIR)/pmo/src/target_if_pmo_wow.o

ifeq ($(CONFIG_WLAN_FEATURE_DSRC), y)
CLD_TARGET_IF_INC += -I$(WLAN_ROOT)/$(CLD_TARGET_IF_DIR)/ocb/inc
CLD_TARGET_IF_OBJ += $(CLD_TARGET_IF_DIR)/ocb/src/target_if_ocb.o
endif

ifeq ($(CONFIG_WLAN_FEATURE_DISA), y)
CLD_TARGET_IF_INC += -I$(WLAN_ROOT)/$(CLD_TARGET_IF_DIR)/disa/inc
CLD_TARGET_IF_OBJ += $(CLD_TARGET_IF_DIR)/disa/src/target_if_disa.o
endif

ifeq ($(CONFIG_IPA_OFFLOAD), y)
CLD_TARGET_IF_INC += -I$(WLAN_ROOT)/$(CLD_TARGET_IF_DIR)/ipa/inc
CLD_TARGET_IF_OBJ += $(CLD_TARGET_IF_DIR)/ipa/src/target_if_ipa.o
endif

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
		 -I$(WLAN_COMMON_INC)/target_if/p2p/inc \
		 -I$(WLAN_COMMON_INC)/target_if/regulatory/inc \
		 -I$(WLAN_COMMON_INC)/target_if/tdls/inc

TARGET_IF_OBJ := $(TARGET_IF_DIR)/core/src/target_if_main.o \
		$(TARGET_IF_DIR)/p2p/src/target_if_p2p.o \
		$(TARGET_IF_DIR)/regulatory/src/target_if_reg.o \
		$(TARGET_IF_DIR)/tdls/src/target_if_tdls.o \
		$(TARGET_IF_DIR)/init_deinit/src/init_cmd_api.o \
		$(TARGET_IF_DIR)/init_deinit/src/init_deinit_ucfg.o \
		$(TARGET_IF_DIR)/init_deinit/src/init_event_handler.o \
		$(TARGET_IF_DIR)/init_deinit/src/service_ready_util.o \

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
	    $(WMI_OBJ_DIR)/wmi_unified_reg_api.o

ifeq ($(CONFIG_WLAN_FEATURE_DSRC), y)
ifeq ($(CONFIG_OCB_UT_FRAMEWORK), y)
WMI_OBJS += $(WMI_OBJ_DIR)/wmi_unified_ocb_ut.o
endif
endif

ifeq ($(CONFIG_WLAN_DFS_MASTER_ENABLE), y)
WMI_OBJS += $(WMI_OBJ_DIR)/wmi_unified_dfs_api.o
endif

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

ifeq ($(CONFIG_IPA_OFFLOAD), y)
TXRX_OBJS +=     $(TXRX_DIR)/ol_txrx_ipa.o
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
ifeq ($(CONFIG_WLAN_TX_FLOW_CONTROL_V2), y)
DP_OBJS += $(DP_SRC)/dp_tx_flow_control.o
endif
endif

ifeq ($(CONFIG_IPA_OFFLOAD), y)
DP_OBJS +=     $(DP_SRC)/dp_ipa.o
endif

ifeq ($(CONFIG_WDI_EVENT_ENABLE), y)
DP_OBJS +=     $(DP_SRC)/dp_wdi_event.o
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

###### CP STATS ########
CP_STATS_OS_IF_SRC      := $(WLAN_COMMON_ROOT)/os_if/linux/cp_stats/src
CP_STATS_TGT_SRC        := $(WLAN_COMMON_ROOT)/target_if/cp_stats/src
CP_STATS_CORE_SRC       := $(WLAN_COMMON_ROOT)/umac/cp_stats/core/src
CP_STATS_DISPATCHER_SRC := $(WLAN_COMMON_ROOT)/umac/cp_stats/dispatcher/src

CP_STATS_OS_IF_INC      := -I$(WLAN_COMMON_INC)/os_if/linux/cp_stats/inc
CP_STATS_TGT_INC        := -I$(WLAN_COMMON_INC)/target_if/cp_stats/inc
CP_STATS_DISPATCHER_INC := -I$(WLAN_COMMON_INC)/umac/cp_stats/dispatcher/inc

ifeq ($(CONFIG_CP_STATS), y)
CP_STATS_OBJS := $(CP_STATS_TGT_SRC)/target_if_mc_cp_stats.o                 \
		 $(CP_STATS_CORE_SRC)/wlan_cp_stats_comp_handler.o           \
		 $(CP_STATS_CORE_SRC)/wlan_cp_stats_obj_mgr_handler.o        \
		 $(CP_STATS_CORE_SRC)/wlan_cp_stats_da_api.o                 \
		 $(CP_STATS_CORE_SRC)/wlan_cp_stats_ol_api.o                 \
		 $(CP_STATS_OS_IF_SRC)/wlan_cfg80211_mc_cp_stats.o           \
		 $(CP_STATS_DISPATCHER_SRC)/wlan_cp_stats_utils_api.o        \
		 $(CP_STATS_DISPATCHER_SRC)/wlan_cp_stats_mc_tgt_api.o       \
		 $(CP_STATS_DISPATCHER_SRC)/wlan_cp_stats_mc_ucfg_api.o
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

ifeq ($(CONFIG_FEATURE_HTC_CREDIT_HISTORY), y)
HTC_OBJS += $(WLAN_COMMON_ROOT)/$(HTC_DIR)/htc_credit_history.o
endif

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
	   -I$(WLAN_COMMON_INC)/$(HIF_DIR)/src

ifeq ($(CONFIG_HIF_PCI), y)
HIF_INC += -I$(WLAN_COMMON_INC)/$(HIF_DISPATCHER_DIR)
HIF_INC += -I$(WLAN_COMMON_INC)/$(HIF_PCIE_DIR)
HIF_INC += -I$(WLAN_COMMON_INC)/$(HIF_CE_DIR)
endif

ifeq ($(CONFIG_HIF_SNOC), y)
HIF_INC += -I$(WLAN_COMMON_INC)/$(HIF_DISPATCHER_DIR)
HIF_INC += -I$(WLAN_COMMON_INC)/$(HIF_SNOC_DIR)
HIF_INC += -I$(WLAN_COMMON_INC)/$(HIF_CE_DIR)
endif

ifeq ($(CONFIG_HIF_USB), y)
HIF_INC += -I$(WLAN_COMMON_INC)/$(HIF_DISPATCHER_DIR)
HIF_INC += -I$(WLAN_COMMON_INC)/$(HIF_USB_DIR)
endif

ifeq ($(CONFIG_HIF_SDIO), y)
HIF_INC += -I$(WLAN_COMMON_INC)/$(HIF_DISPATCHER_DIR)
HIF_INC += -I$(WLAN_COMMON_INC)/$(HIF_SDIO_DIR)
HIF_INC += -I$(WLAN_COMMON_INC)/$(HIF_SDIO_NATIVE_INC_DIR)
endif

HIF_COMMON_OBJS := $(WLAN_COMMON_ROOT)/$(HIF_DIR)/src/ath_procfs.o \
                $(WLAN_COMMON_ROOT)/$(HIF_DIR)/src/hif_main.o \
                $(WLAN_COMMON_ROOT)/$(HIF_DIR)/src/mp_dev.o

ifeq ($(CONFIG_WLAN_NAPI), y)
HIF_COMMON_OBJS += $(WLAN_COMMON_ROOT)/$(HIF_DIR)/src/hif_exec.o
HIF_COMMON_OBJS += $(WLAN_COMMON_ROOT)/$(HIF_DIR)/src/hif_irq_affinity.o
endif



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

ifeq ($(CONFIG_FEATURE_UNIT_TEST_SUSPEND), y)
	HIF_OBJS += $(WLAN_COMMON_ROOT)/$(HIF_DIR)/src/hif_unit_test_suspend.o
endif

HIF_PCIE_OBJS := $(WLAN_COMMON_ROOT)/$(HIF_PCIE_DIR)/if_pci.o
HIF_SNOC_OBJS := $(WLAN_COMMON_ROOT)/$(HIF_SNOC_DIR)/if_snoc.o
HIF_SDIO_OBJS += $(WLAN_COMMON_ROOT)/$(HIF_SDIO_DIR)/if_sdio.o

HIF_OBJS += $(WLAN_COMMON_ROOT)/$(HIF_DISPATCHER_DIR)/multibus.o
HIF_OBJS += $(WLAN_COMMON_ROOT)/$(HIF_DISPATCHER_DIR)/dummy.o
HIF_OBJS += $(HIF_COMMON_OBJS)

ifeq ($(CONFIG_HIF_PCI), y)
HIF_OBJS += $(HIF_PCIE_OBJS)
HIF_OBJS += $(HIF_CE_OBJS)
HIF_OBJS += $(WLAN_COMMON_ROOT)/$(HIF_DISPATCHER_DIR)/multibus_pci.o
endif

ifeq ($(CONFIG_HIF_SNOC), y)
HIF_OBJS += $(HIF_SNOC_OBJS)
HIF_OBJS += $(HIF_CE_OBJS)
HIF_OBJS += $(WLAN_COMMON_ROOT)/$(HIF_DISPATCHER_DIR)/multibus_snoc.o
endif

ifeq ($(CONFIG_HIF_SDIO), y)
HIF_OBJS += $(HIF_SDIO_OBJS)
HIF_OBJS += $(HIF_SDIO_NATIVE_OBJS)
HIF_OBJS += $(WLAN_COMMON_ROOT)/$(HIF_DISPATCHER_DIR)/multibus_sdio.o
endif

ifeq ($(CONFIG_HIF_USB), y)
HIF_OBJS += $(HIF_USB_OBJS)
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
ifeq ($(CONFIG_WLAN_FEATURE_FIPS), y)
WMA_OBJS+=	$(WMA_SRC_DIR)/wma_fips_api.o
endif
ifeq ($(CONFIG_MPC_UT_FRAMEWORK), y)
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

ifeq ($(CONFIG_HIF_PCI), y)
PLD_OBJS +=	$(PLD_SRC_DIR)/pld_pcie.o
endif
ifeq ($(CONFIG_ICNSS), y)
PLD_OBJS +=	$(PLD_SRC_DIR)/pld_snoc.o
endif
ifeq ($(CONFIG_QCA_WIFI_SDIO), y)
PLD_OBJS +=	$(PLD_SRC_DIR)/pld_sdio.o
endif
ifeq ($(CONFIG_PLD_USB_CNSS), y)
PLD_OBJS +=	$(PLD_SRC_DIR)/pld_usb.o
endif

ifeq ($(CONFIG_QCA6290_11AX), y)
TARGET_INC :=	-I$(WLAN_ROOT)/../fw-api/hw/qca6290/11ax/v2 \
		-I$(WLAN_ROOT)/../fw-api/fw
else
TARGET_INC :=	-I$(WLAN_ROOT)/../fw-api/hw/qca6290/v2 \
		-I$(WLAN_ROOT)/../fw-api/fw
endif

LINUX_INC :=	-Iinclude

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
		$(CLD_TARGET_IF_INC) \
		$(OS_IF_INC) \
		$(GLOBAL_LMAC_IF_INC) \
		$(FTM_INC)

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
################ CP STATS ################
INCS +=		$(CP_STATS_OS_IF_INC)
INCS +=		$(CP_STATS_TGT_INC)
INCS +=		$(CP_STATS_DISPATCHER_INC)
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
INCS +=		$(OCB_INC)

INCS +=		$(IPA_INC)

ifeq ($(CONFIG_REMOVE_PKT_LOG), n)
INCS +=		$(PKTLOG_INC)
endif

ifeq ($(BUILD_DIAG_VERSION), y)
INCS +=		$(HOST_DIAG_LOG_INC)
endif

INCS +=		$(DISA_INC)

INCS +=		$(UMAC_DISP_INC)
INCS +=		$(UMAC_SCAN_INC)
INCS +=		$(UMAC_TARGET_SCAN_INC)
INCS +=		$(UMAC_GREEN_AP_INC)
INCS +=		$(UMAC_TARGET_GREEN_AP_INC)
INCS +=		$(UMAC_COMMON_INC)
INCS +=		$(UMAC_SPECTRAL_INC)
INCS +=		$(UMAC_TARGET_SPECTRAL_INC)

OBJS :=		$(HDD_OBJS) \
		$(MAC_OBJS) \
		$(SAP_OBJS) \
		$(SME_OBJS) \
		$(SYS_OBJS) \
		$(QDF_OBJS) \
		$(CDS_OBJS) \
		$(FTM_OBJS)

OBJS +=		$(WMA_OBJS) \
		$(TXRX_OBJS) \
		$(WMI_OBJS) \
		$(FWLOG_OBJS) \
		$(HTC_OBJS) \
		$(INIT_DEINIT_OBJS) \
		$(SCHEDULER_OBJS) \
		$(REGULATORY_OBJS)

OBJS +=		$(HIF_OBJS) \
		$(BMI_OBJS) \
		$(HTT_OBJS) \
		$(OS_IF_OBJ) \
		$(TARGET_IF_OBJ) \
		$(CLD_TARGET_IF_OBJ) \
		$(GLOBAL_LMAC_IF_OBJ)

ifeq ($(CONFIG_LITHIUM), y)
OBJS += 	$(HAL_OBJS)
endif

ifeq ($(CONFIG_FEATURE_EPPING), y)
OBJS += 	$(EPPING_OBJS)
endif

ifeq ($(CONFIG_WLAN_DFS_MASTER_ENABLE), y)
OBJS +=		$(DFS_OBJS)
endif

OBJS +=		$(UMAC_OBJMGR_OBJS)
OBJS +=		$(WIFI_POS_OBJS)
OBJS +=		$(CP_STATS_OBJS)
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

ifeq ($(CONFIG_WLAN_FEATURE_DSRC), y)
OBJS +=		$(OCB_OBJS)
endif

ifeq ($(CONFIG_IPA_OFFLOAD), y)
OBJS +=		$(IPA_OBJS)
endif

ifeq ($(CONFIG_REMOVE_PKT_LOG), n)
OBJS +=		$(PKTLOG_OBJS)
endif

ifeq ($(BUILD_DIAG_VERSION), y)
OBJS +=		$(HOST_DIAG_LOG_OBJS)
endif

ifeq ($(CONFIG_WLAN_FEATURE_DISA), y)
OBJS +=		$(DISA_OBJS)
endif

OBJS +=		$(UMAC_DISP_OBJS)
OBJS +=		$(UMAC_SCAN_OBJS)
OBJS +=		$(UMAC_COMMON_OBJS)
OBJS +=		$(WCFG_OBJS)
OBJS +=		$(UMAC_SPECTRAL_OBJS)

ifeq ($(CONFIG_QCACLD_FEATURE_GREEN_AP), y)
OBJS +=		$(UMAC_GREEN_AP_OBJS)
endif

ifeq ($(CONFIG_LITHIUM), y)
OBJS +=		$(DP_OBJS)
endif

ccflags-y += $(INCS)

cppflags-y +=	-DANI_OS_TYPE_ANDROID=6 \
		-Wall\
		-Werror\
		-D__linux__

cppflags-$(CONFIG_PTT_SOCK_SVC_ENABLE) += -DPTT_SOCK_SVC_ENABLE
cppflags-$(CONFIG_FEATURE_WLAN_WAPI) += -DFEATURE_WLAN_WAPI
cppflags-$(CONFIG_FEATURE_WLAN_WAPI) += -DATH_SUPPORT_WAPI
cppflags-$(CONFIG_AGEIE_ON_SCAN_RESULTS) += -DWLAN_ENABLE_AGEIE_ON_SCAN_RESULTS
cppflags-$(CONFIG_SOFTAP_CHANNEL_RANGE) += -DSOFTAP_CHANNEL_RANGE
cppflags-$(CONFIG_FEATURE_WLAN_SCAN_PNO) += -DFEATURE_WLAN_SCAN_PNO
cppflags-$(CONFIG_WLAN_FEATURE_PACKET_FILTERING) += -DWLAN_FEATURE_PACKET_FILTERING
cppflags-$(CONFIG_WLAN_NS_OFFLOAD) += -DWLAN_NS_OFFLOAD
cppflags-$(CONFIG_FEATURE_WLAN_RA_FILTERING) += -DFEATURE_WLAN_RA_FILTERING
cppflags-$(CONFIG_FEATURE_WLAN_LPHB) += -DFEATURE_WLAN_LPHB
cppflags-$(CONFIG_QCA_SUPPORT_TX_THROTTLE) += -DQCA_SUPPORT_TX_THROTTLE
cppflags-$(CONFIG_WMI_INTERFACE_EVENT_LOGGING) += -DWMI_INTERFACE_EVENT_LOGGING
cppflags-$(CONFIG_WLAN_FEATURE_LINK_LAYER_STATS) += -DWLAN_FEATURE_LINK_LAYER_STATS
cppflags-$(CONFIG_FEATURE_WLAN_EXTSCAN) += -DFEATURE_WLAN_EXTSCAN
cppflags-$(CONFIG_160MHZ_SUPPORT) += -DCONFIG_160MHZ_SUPPORT
cppflags-$(CONFIG_MCL) += -DCONFIG_MCL
cppflags-$(CONFIG_MCL_REGDB) += -DCONFIG_MCL_REGDB
cppflags-$(CONFIG_LEGACY_CHAN_ENUM) += -DCONFIG_LEGACY_CHAN_ENUM
cppflags-$(CONFIG_NAPIER_SCAN) += -DNAPIER_SCAN
cppflags-$(CONFIG_WLAN_PMO_ENABLE) += -DWLAN_PMO_ENABLE
cppflags-$(CONFIG_CONVERGED_P2P_ENABLE) += -DCONVERGED_P2P_ENABLE
cppflags-$(CONFIG_WLAN_POLICY_MGR_ENABLE) += -DWLAN_POLICY_MGR_ENABLE
cppflags-$(CONFIG_SUPPORT_11AX) += -DSUPPORT_11AX
cppflags-$(CONFIG_HDD_INIT_WITH_RTNL_LOCK) += -DCONFIG_HDD_INIT_WITH_RTNL_LOCK
cppflags-$(CONFIG_CONVERGED_TDLS_ENABLE) += -DCONVERGED_TDLS_ENABLE
cppflags-$(CONFIG_WLAN_CONV_SPECTRAL_ENABLE) += -DWLAN_CONV_SPECTRAL_ENABLE
cppflags-$(CONFIG_WLAN_SPECTRAL_ENABLE) += -DWLAN_SPECTRAL_ENABLE
cppflags-$(CONFIG_WMI_CMD_STRINGS) += -DWMI_CMD_STRINGS

cppflags-$(CONFIG_WLAN_DISABLE_EXPORT_SYMBOL) += -DWLAN_DISABLE_EXPORT_SYMBOL
cppflags-$(CONFIG_WIFI_POS_CONVERGED) += -DWIFI_POS_CONVERGED
cppflags-$(CONFIG_WIFI_POS_LEGACY) += -DFEATURE_OEM_DATA_SUPPORT
cppflags-$(CONFIG_FEATURE_HTC_CREDIT_HISTORY) += -DFEATURE_HTC_CREDIT_HISTORY
cppflags-$(CONFIG_WLAN_FEATURE_P2P_DEBUG) += -DWLAN_FEATURE_P2P_DEBUG
cppflags-$(CONFIG_WLAN_LOGGING_SOCK_SVC) += -DWLAN_LOGGING_SOCK_SVC_ENABLE
cppflags-$(CONFIG_WLAN_FEATURE_FILS) += -DWLAN_FEATURE_FILS_SK
cppflags-$(CONFIG_CP_STATS) += -DQCA_SUPPORT_CP_STATS

ifeq ($(CONFIG_CNSS), y)
ifeq ($(CONFIG_CNSS_SDIO), y)
cppflags-y += -DCONFIG_PLD_SDIO_CNSS
else
cppflags-y += -DCONFIG_PLD_PCIE_CNSS
endif
endif

ifeq ($(CONFIG_CNSS2), y)
cppflags-y += -DCONFIG_PLD_PCIE_CNSS
cppflags-y += -DCONFIG_PLD_PCIE_INIT
endif

#Enable NL80211 test mode
cppflags-$(CONFIG_NL80211_TESTMODE) += -DWLAN_NL80211_TESTMODE

# Flag to enable bus auto suspend
ifeq ($(CONFIG_HIF_PCI), y)
ifeq ($(CONFIG_BUS_AUTO_SUSPEND), y)
cppflags-y += -DFEATURE_RUNTIME_PM
endif
endif

cppflags-$(CONFIG_ICNSS) += -DCONFIG_PLD_SNOC_ICNSS

cppflags-$(CONFIG_ICNSS) += -DQCA_WIFI_3_0

cppflags-$(CONFIG_WIFI_3_0_ADRASTEA) += -DQCA_WIFI_3_0_ADRASTEA
cppflags-$(CONFIG_ADRASTEA_SHADOW_REGISTERS) += -DADRASTEA_SHADOW_REGISTERS
cppflags-$(CONFIG_ADRASTEA_RRI_ON_DDR) += -DADRASTEA_RRI_ON_DDR

ifeq ($(CONFIG_QMI_SUPPORT), n)
cppflags-y += -DCONFIG_BYPASS_QMI
endif

cppflags-$(CONFIG_WLAN_FASTPATH) +=	-DWLAN_FEATURE_FASTPATH

cppflags-$(CONFIG_FEATURE_PKTLOG) +=     -DFEATURE_PKTLOG

cppflags-y +=	-DCONFIG_DP_TRACE

ifeq ($(CONFIG_WLAN_NAPI), y)
cppflags-y += -DFEATURE_NAPI
cppflags-y += -DHIF_IRQ_AFFINITY
ifeq ($(CONFIG_WLAN_NAPI_DEBUG), y)
cppflags-y += -DFEATURE_NAPI_DEBUG
endif
endif

ifeq (y,$(findstring y,$(CONFIG_ARCH_MSM) $(CONFIG_ARCH_QCOM)))
cppflags-y += -DMSM_PLATFORM
endif

cppflags-y +=	-DQCA_SUPPORT_TXRX_LOCAL_PEER_ID

cppflags-$(CONFIG_WLAN_TX_FLOW_CONTROL_V2) += -DQCA_LL_TX_FLOW_CONTROL_V2
cppflags-$(CONFIG_WLAN_TX_FLOW_CONTROL_V2) += -DQCA_LL_TX_FLOW_GLOBAL_MGMT_POOL
cppflags-$(CONFIG_WLAN_TX_FLOW_CONTROL_LEGACY) += -DQCA_LL_LEGACY_TX_FLOW_CONTROL

ifeq ($(BUILD_DEBUG_VERSION), y)
cppflags-y +=	-DWLAN_DEBUG
ifeq ($(CONFIG_TRACE_RECORD_FEATURE), y)
cppflags-y +=	-DTRACE_RECORD \
		-DLIM_TRACE_RECORD \
		-DSME_TRACE_RECORD \
		-DHDD_TRACE_RECORD
endif
endif
cppflags-$(CONFIG_WLAN_DEBUG_CRASH_INJECT) += -DCONFIG_WLAN_DEBUG_CRASH_INJECT
cppflags-$(CONFIG_FEATURE_UNIT_TEST_SUSPEND) += -DWLAN_SUSPEND_RESUME_TEST

ifeq ($(CONFIG_LEAK_DETECTION), y)
cppflags-y += \
	-DCONFIG_HALT_KMEMLEAK \
	-DCONFIG_LEAK_DETECTION \
	-DMEMORY_DEBUG \
	-DNBUF_MEMORY_DEBUG \
	-DTIMER_MANAGER
endif

cppflags-y += -DWLAN_FEATURE_P2P
cppflags-y += -DWLAN_FEATURE_WFD
ifeq ($(CONFIG_QCOM_VOWIFI_11R), y)
cppflags-y += -DKERNEL_SUPPORT_11R_CFG80211
cppflags-y += -DUSE_80211_WMMTSPEC_FOR_RIC
endif

ifeq ($(CONFIG_QCOM_ESE), y)
cppflags-y += -DFEATURE_WLAN_ESE
cppflags-y += -DQCA_COMPUTE_TX_DELAY
cppflags-y += -DQCA_COMPUTE_TX_DELAY_PER_TID
endif

#normally, TDLS negative behavior is not needed
cppflags-$(CONFIG_QCOM_TDLS) += -DFEATURE_WLAN_TDLS

cppflags-$(CONFIG_QCACLD_WLAN_LFR3) += -DWLAN_FEATURE_ROAM_OFFLOAD

cppflags-$(CONFIG_CNSS_GENL) += -DCNSS_GENL

cppflags-$(CONFIG_QCACLD_WLAN_LFR2) += -DWLAN_FEATURE_HOST_ROAM

cppflags-$(CONFIG_WLAN_POWER_DEBUGFS) += -DWLAN_POWER_DEBUGFS

# Enable object manager reference count debug infrastructure
cppflags-$(CONFIG_WLAN_OBJMGR_DEBUG) += -DWLAN_OBJMGR_DEBUG

cppflags-$(CONFIG_WLAN_FEATURE_SAE) += -DWLAN_FEATURE_SAE

ifeq ($(BUILD_DIAG_VERSION), y)
cppflags-y += -DFEATURE_WLAN_DIAG_SUPPORT
cppflags-y += -DFEATURE_WLAN_DIAG_SUPPORT_CSR
cppflags-y += -DFEATURE_WLAN_DIAG_SUPPORT_LIM
ifeq ($(CONFIG_HIF_PCI), y)
cppflags-y += -DCONFIG_ATH_PROCFS_DIAG_SUPPORT
endif
endif

ifeq ($(CONFIG_HIF_USB), y)
cppflags-y += -DCONFIG_ATH_PROCFS_DIAG_SUPPORT
cppflags-y += -DQCA_SUPPORT_OL_RX_REORDER_TIMEOUT
cppflags-y += -DCONFIG_ATH_PCIE_MAX_PERF=0 -DCONFIG_ATH_PCIE_AWAKE_WHILE_DRIVER_LOAD=0 -DCONFIG_DISABLE_CDC_MAX_PERF_WAR=0
endif

cppflags-$(CONFIG_WLAN_FEATURE_11W) += -DWLAN_FEATURE_11W

cppflags-$(CONFIG_QCA_TXDESC_SANITY_CHECKS) += -DQCA_SUPPORT_TXDESC_SANITY_CHECKS

cppflags-$(CONFIG_QCOM_LTE_COEX) += -DFEATURE_WLAN_CH_AVOID

cppflags-$(CONFIG_WLAN_FEATURE_LPSS) += -DWLAN_FEATURE_LPSS

ifneq ($(TARGET_BUILD_VARIANT),user)
cppflags-y += -DDESC_DUP_DETECT_DEBUG
cppflags-y += -DDEBUG_RX_RING_BUFFER
endif

cppflags-$(PANIC_ON_BUG) += -DPANIC_ON_BUG

cppflags-$(WLAN_WARN_ON_ASSERT) += -DWLAN_WARN_ON_ASSERT

ccflags-$(CONFIG_POWER_MANAGEMENT_OFFLOAD) += -DWLAN_POWER_MANAGEMENT_OFFLOAD

cppflags-$(CONFIG_WLAN_LOG_FATAL) += -DWLAN_LOG_FATAL
cppflags-$(CONFIG_WLAN_LOG_ERROR) += -DWLAN_LOG_ERROR
cppflags-$(CONFIG_WLAN_LOG_WARN) += -DWLAN_LOG_WARN
cppflags-$(CONFIG_WLAN_LOG_INFO) += -DWLAN_LOG_INFO
cppflags-$(CONFIG_WLAN_LOG_DEBUG) += -DWLAN_LOG_DEBUG

cppflags-$(WLAN_OPEN_SOURCE) += -DWLAN_OPEN_SOURCE

cppflags-$(CONFIG_FEATURE_STATS_EXT) += -DWLAN_FEATURE_STATS_EXT

cppflags-$(CONFIG_QCACLD_FEATURE_NAN) += -DWLAN_FEATURE_NAN

cppflags-$(CONFIG_QCA_IBSS_SUPPORT) += -DQCA_IBSS_SUPPORT

#Enable OL debug and wmi unified functions
cppflags-$(CONFIG_ATH_PERF_PWR_OFFLOAD) += -DATH_PERF_PWR_OFFLOAD

#Disable packet log
cppflags-$(CONFIG_REMOVE_PKT_LOG) += -DREMOVE_PKT_LOG

#Enable 11AC TX
cppflags-$(CONFIG_ATH_11AC_TXCOMPACT) += -DATH_11AC_TXCOMPACT

#Enable PCI specific APIS (dma, etc)
cppflags-$(CONFIG_HIF_PCI) += -DHIF_PCI

cppflags-$(CONFIG_HIF_SNOC) += -DHIF_SNOC

#Enable High Latency related Flags
ifeq ($(CONFIG_QCA_WIFI_SDIO), y)
cppflags-y += -DCONFIG_HL_SUPPORT \
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
cppflags-y += -DWLAN_FEATURE_DSRC

ifeq ($(CONFIG_OCB_UT_FRAMEWORK), y)
cppflags-y += -DWLAN_OCB_UT
endif

endif

cppflags-$(CONFIG_FEATURE_SKB_PRE_ALLOC) += -DFEATURE_SKB_PRE_ALLOC

#Enable USB specific APIS
ifeq ($(CONFIG_HIF_USB), y)
cppflags-y += -DHIF_USB \
            -DCONFIG_PLD_USB_CNSS \
            -DDEBUG_HL_LOGGING \
            -DCONFIG_HL_SUPPORT
endif

#Enable FW logs through ini
cppflags-y += -DCONFIG_FW_LOGS_BASED_ON_INI

#Enable pci read/write config functions
cppflags-$(CONFIG_ATH_PCI) += -DATH_PCI

#Enable power management suspend/resume functionality
cppflags-$(CONFIG_ATH_BUS_PM) += -DATH_BUS_PM

#Enable FLOWMAC module support
cppflags-$(CONFIG_ATH_SUPPORT_FLOWMAC_MODULE) += -DATH_SUPPORT_FLOWMAC_MODULE

#Enable spectral support
cppflags-$(CONFIG_ATH_SUPPORT_SPECTRAL) += -DATH_SUPPORT_SPECTRAL

#Enable WDI Event support
cppflags-$(CONFIG_WDI_EVENT_ENABLE) += -DWDI_EVENT_ENABLE

#Endianness selection
ifeq ($(CONFIG_LITTLE_ENDIAN), y)
cppflags-y += -DANI_LITTLE_BYTE_ENDIAN
cppflags-y += -DANI_LITTLE_BIT_ENDIAN
cppflags-y += -DDOT11F_LITTLE_ENDIAN_HOST
else
cppflags-y += -DANI_BIG_BYTE_ENDIAN
cppflags-y += -DBIG_ENDIAN_HOST
endif

#Enable TX reclaim support
cppflags-$(CONFIG_TX_CREDIT_RECLAIM_SUPPORT) += -DTX_CREDIT_RECLAIM_SUPPORT

#Enable FTM support
cppflags-$(CONFIG_QCA_WIFI_FTM) += -DQCA_WIFI_FTM

#Enable Checksum Offload support
cppflags-$(CONFIG_CHECKSUM_OFFLOAD) += -DCHECKSUM_OFFLOAD

#Enable Checksum Offload support
cppflags-$(CONFIG_IPA_OFFLOAD) += -DIPA_OFFLOAD

ifeq ($(CONFIG_ARCH_SDX20), y)
cppflags-y += -DSYNC_IPA_READY
endif

#Enable GTK Offload
cppflags-$(CONFIG_GTK_OFFLOAD) += -DWLAN_FEATURE_GTK_OFFLOAD

#Enable External WoW
cppflags-$(CONFIG_EXT_WOW) += -DWLAN_FEATURE_EXTWOW_SUPPORT

#Mark it as SMP Kernel
cppflags-$(CONFIG_SMP) += -DQCA_CONFIG_SMP

cppflags-$(CONFIG_CHNL_MATRIX_RESTRICTION) += -DWLAN_ENABLE_CHNL_MATRIX_RESTRICTION

#Enable ICMP packet disable powersave feature
cppflags-$(CONFIG_ICMP_DISABLE_PS) += -DWLAN_ICMP_DISABLE_PS

#Enable OBSS feature
cppflags-y += -DQCA_HT_2040_COEX

#enable MCC TO SCC switch
cppflags-$(CONFIG_FEATURE_WLAN_MCC_TO_SCC_SWITCH) += -DFEATURE_WLAN_MCC_TO_SCC_SWITCH

#enable wlan auto shutdown feature
cppflags-$(CONFIG_FEATURE_WLAN_AUTO_SHUTDOWN) += -DFEATURE_WLAN_AUTO_SHUTDOWN

#enable AP-AP ACS Optimization
cppflags-$(CONFIG_FEATURE_WLAN_AP_AP_ACS_OPTIMIZE) += -DFEATURE_WLAN_AP_AP_ACS_OPTIMIZE

#Enable 4address scheme
cppflags-$(CONFIG_FEATURE_WLAN_STA_4ADDR_SCHEME) += -DFEATURE_WLAN_STA_4ADDR_SCHEME

#enable MDM/SDX special config
cppflags-$(CONFIG_MDM_PLATFORM) += -DMDM_PLATFORM

#Disable STA-AP Mode DFS support
cppflags-$(CONFIG_FEATURE_WLAN_STA_AP_MODE_DFS_DISABLE) += -DFEATURE_WLAN_STA_AP_MODE_DFS_DISABLE

#Open P2P device interface only for non-Mobile router use cases
cppflags-$(CONFIG_WLAN_OPEN_P2P_INTERFACE) += -DWLAN_OPEN_P2P_INTERFACE

#Enable 2.4 GHz social channels in 5 GHz only mode for p2p usage
cppflags-$(CONFIG_WLAN_ENABLE_SOCIAL_CHANNELS_5G_ONLY) += -DWLAN_ENABLE_SOCIAL_CHANNELS_5G_ONLY

#Green AP feature
cppflags-$(CONFIG_QCACLD_FEATURE_GREEN_AP) += -DWLAN_SUPPORT_GREEN_AP

#Stats & Quota Metering feature
ifeq ($(CONFIG_IPA_OFFLOAD), y)
ifeq ($(CONFIG_QCACLD_FEATURE_METERING), y)
cppflags-y += -DFEATURE_METERING
endif
endif

cppflags-$(CONFIG_TUFELLO_DUAL_FW_SUPPORT) += -DCONFIG_TUFELLO_DUAL_FW_SUPPORT
cppflags-$(CONFIG_CHANNEL_HOPPING_ALL_BANDS) += -DCHANNEL_HOPPING_ALL_BANDS

#Enable Signed firmware support for split binary format
cppflags-$(CONFIG_QCA_SIGNED_SPLIT_BINARY_SUPPORT) += -DQCA_SIGNED_SPLIT_BINARY_SUPPORT

#Enable single firmware binary format
cppflags-$(CONFIG_QCA_SINGLE_BINARY_SUPPORT) += -DQCA_SINGLE_BINARY_SUPPORT

#Enable collecting target RAM dump after kernel panic
cppflags-$(CONFIG_TARGET_RAMDUMP_AFTER_KERNEL_PANIC) += -DTARGET_RAMDUMP_AFTER_KERNEL_PANIC

#Enable/disable secure firmware feature
cppflags-$(CONFIG_FEATURE_SECURE_FIRMWARE) += -DFEATURE_SECURE_FIRMWARE

cppflags-$(CONFIG_ATH_PCIE_ACCESS_DEBUG) += -DCONFIG_ATH_PCIE_ACCESS_DEBUG

# Enable feature support for Linux version QCMBR
cppflags-$(CONFIG_LINUX_QCMBR) += -DLINUX_QCMBR

# Enable featue sync tsf between multi devices
cppflags-$(CONFIG_WLAN_SYNC_TSF) += -DWLAN_FEATURE_TSF
cppflags-$(CONFIG_WLAN_SYNC_TSF_PLUS) += -DWLAN_FEATURE_TSF_PLUS

cppflags-$(CONFIG_ATH_PROCFS_DIAG_SUPPORT) += -DCONFIG_ATH_PROCFS_DIAG_SUPPORT
cppflags-$(CONFIG_11AC_TXCOMPACT) += -DATH_11AC_TXCOMPACT

cppflags-$(CONFIG_HELIUMPLUS) += -DHELIUMPLUS
cppflags-$(CONFIG_AR900B) += -DAR900B
cppflags-$(CONFIG_HTT_PADDR64) += -DHTT_PADDR64
cppflags-$(CONFIG_OL_RX_INDICATION_RECORD) += -DOL_RX_INDICATION_RECORD
cppflags-$(CONFIG_TSOSEG_DEBUG) += -DTSOSEG_DEBUG

cppflags-$(CONFIG_ENABLE_DEBUG_ADDRESS_MARKING) += -DENABLE_DEBUG_ADDRESS_MARKING
cppflags-$(CONFIG_FEATURE_TSO) += -DFEATURE_TSO
cppflags-$(CONFIG_FEATURE_TSO_DEBUG) += -DFEATURE_TSO_DEBUG

cppflags-$(CONFIG_WLAN_LRO) += -DFEATURE_LRO

cppflags-$(CONFIG_FEATURE_AP_MCC_CH_AVOIDANCE) += -DFEATURE_AP_MCC_CH_AVOIDANCE

cppflags-$(CONFIG_MPC_UT_FRAMEWORK) += -DMPC_UT_FRAMEWORK

cppflags-$(CONFIG_FEATURE_EPPING) += -DWLAN_FEATURE_EPPING

cppflags-$(CONFIG_WLAN_OFFLOAD_PACKETS) += -DWLAN_FEATURE_OFFLOAD_PACKETS

cppflags-$(CONFIG_WLAN_FEATURE_DISA) += -DWLAN_FEATURE_DISA

cppflags-$(CONFIG_WLAN_FEATURE_FIPS) += -DWLAN_FEATURE_FIPS

cppflags-$(CONFIG_LFR_SUBNET_DETECTION) += -DFEATURE_LFR_SUBNET_DETECTION

cppflags-$(CONFIG_MCC_TO_SCC_SWITCH) += -DFEATURE_WLAN_MCC_TO_SCC_SWITCH

cppflags-$(CONFIG_WLAN_FEATURE_NAN_DATAPATH) += -DWLAN_FEATURE_NAN_DATAPATH

cppflags-$(CONFIG_NAN_CONVERGENCE) += -DWLAN_FEATURE_NAN_CONVERGENCE

cppflags-$(CONFIG_FEATURE_WLAN_D0WOW) += -DFEATURE_WLAN_D0WOW

cppflags-$(CONFIG_SHADOW_V2) += -DCONFIG_SHADOW_V2
cppflags-$(CONFIG_QCA6290_HEADERS_DEF) += -DQCA6290_HEADERS_DEF
cppflags-$(CONFIG_QCA_WIFI_QCA6290) += -DQCA_WIFI_QCA6290
cppflags-$(CONFIG_QCA_WIFI_QCA8074) += -DQCA_WIFI_QCA8074
cppflags-$(CONFIG_QCA_WIFI_QCA8074_VP) += -DQCA_WIFI_QCA8074_VP
cppflags-$(CONFIG_DP_INTR_POLL_BASED) += -DDP_INTR_POLL_BASED
cppflags-$(CONFIG_TX_PER_PDEV_DESC_POOL) += -DTX_PER_PDEV_DESC_POOL
cppflags-$(CONFIG_DP_TRACE) += -DCONFIG_DP_TRACE
cppflags-$(CONFIG_FEATURE_TSO) += -DFEATURE_TSO
cppflags-$(CONFIG_TSO_DEBUG_LOG_ENABLE) += -DTSO_DEBUG_LOG_ENABLE
cppflags-$(CONFIG_DP_LFR) += -DDP_LFR
cppflags-$(CONFIG_HTT_PADDR64) += -DHTT_PADDR64

ifeq ($(CONFIG_QCA6290_11AX), y)
cppflags-y += -DQCA_WIFI_QCA6290_11AX
endif

cppflags-$(CONFIG_WLAN_FEATURE_11AX) += -DWLAN_FEATURE_11AX
cppflags-$(CONFIG_WLAN_FEATURE_11AX) += -DWLAN_FEATURE_11AX_BSS_COLOR

# Dummy flag for WIN/MCL converged data path compilation
cppflags-y += -DDP_PRINT_ENABLE=0
cppflags-y += -DATH_SUPPORT_WRAP=0
cppflags-y += -DQCA_HOST2FW_RXBUF_RING
#endof dummy flags

# DFS component
cppflags-$(CONFIG_WLAN_DFS_MASTER_ENABLE) += -DQCA_MCL_DFS_SUPPORT
ifeq ($(CONFIG_WLAN_FEATURE_DFS_OFFLOAD), y)
cppflags-$(CONFIG_WLAN_DFS_MASTER_ENABLE) += -DWLAN_DFS_FULL_OFFLOAD
else
cppflags-$(CONFIG_WLAN_DFS_MASTER_ENABLE) += -DWLAN_DFS_PARTIAL_OFFLOAD
endif
cppflags-$(CONFIG_WLAN_DFS_MASTER_ENABLE) += -DDFS_COMPONENT_ENABLE
cppflags-$(CONFIG_WLAN_DFS_MASTER_ENABLE) += -DQCA_DFS_USE_POLICY_MANAGER
cppflags-$(CONFIG_WLAN_DFS_MASTER_ENABLE) += -DQCA_DFS_NOL_PLATFORM_DRV_SUPPORT

cppflags-$(CONFIG_WLAN_DEBUGFS) += -DWLAN_DEBUGFS

cppflags-$(CONFIG_DYNAMIC_DEBUG) += -DFEATURE_MULTICAST_HOST_FW_MSGS

cppflags-$(CONFIG_ENABLE_SMMU_S1_TRANSLATION) += -DENABLE_SMMU_S1_TRANSLATION

#Flag to enable NUD tracking
cppflags-$(CONFIG_WLAN_NUD_TRACKING) += -DWLAN_NUD_TRACKING

KBUILD_CPPFLAGS += $(cppflags-y)

# Currently, for versions of gcc which support it, the kernel Makefile
# is disabling the maybe-uninitialized warning.  Re-enable it for the
# WLAN driver.  Note that we must use ccflags-y here so that it
# will override the kernel settings.
ifeq ($(call cc-option-yn, -Wmaybe-uninitialized), y)
ccflags-y += -Wmaybe-uninitialized
ifneq (y,$(CONFIG_ARCH_MSM))
ccflags-y += -Wframe-larger-than=4096
endif
endif
ccflags-y += -Wmissing-prototypes

ifeq ($(call cc-option-yn, -Wheader-guard), y)
ccflags-y += -Wheader-guard
endif
# If the module name is not "wlan", then the define MULTI_IF_NAME to be the
# same a the QCA CHIP name. The host driver will then append MULTI_IF_NAME to
# any string that must be unique for all instances of the driver on the system.
# This allows multiple instances of the driver with different module names.
# If the module name is wlan, leave MULTI_IF_NAME undefined and the code will
# treat the driver as the primary driver.
ifneq ($(MODNAME), wlan)
CHIP_NAME ?= $(MODNAME)
ccflags-y += -DMULTI_IF_NAME=\"$(CHIP_NAME)\"
endif

# WLAN_HDD_ADAPTER_MAGIC must be unique for all instances of the driver on the
# system. If it is not defined, then the host driver will use the first 4
# characters (including NULL) of MULTI_IF_NAME to construct
# WLAN_HDD_ADAPTER_MAGIC.
ifdef WLAN_HDD_ADAPTER_MAGIC
ccflags-y += -DWLAN_HDD_ADAPTER_MAGIC=$(WLAN_HDD_ADAPTER_MAGIC)
endif

# Determine if we are building against an arm architecture host
ifeq ($(findstring arm, $(ARCH)),)
	ccflags-y += -DWLAN_HOST_ARCH_ARM=0
else
	ccflags-y += -DWLAN_HOST_ARCH_ARM=1
endif

# inject some build related information
ifeq ($(CONFIG_BUILD_TAG), y)
CLD_CHECKOUT = $(shell cd "$(WLAN_ROOT)" && \
	git reflog | grep -vm1 cherry-pick | grep -oE ^[0-f]+)
CLD_IDS = $(shell cd "$(WLAN_ROOT)" && \
	git log $(CLD_CHECKOUT)~..HEAD | \
		sed -nE 's/^\s*Change-Id: (I[0-f]{10})[0-f]{30}\s*$$/\1/p' | \
		paste -sd "," -)

CMN_CHECKOUT = $(shell cd "$(WLAN_COMMON_INC)" && \
	git reflog | grep -vm1 cherry-pick | grep -oE ^[0-f]+)
CMN_IDS = $(shell cd "$(WLAN_COMMON_INC)" && \
	git log $(CMN_CHECKOUT)~..HEAD | \
		sed -nE 's/^\s*Change-Id: (I[0-f]{10})[0-f]{30}\s*$$/\1/p' | \
		paste -sd "," -)

TIMESTAMP = $(shell date -u +'%Y-%m-%dT%H:%M:%SZ')
BUILD_TAG = "$(TIMESTAMP); cld:$(CLD_IDS); cmn:$(CMN_IDS);"
# It's assumed that BUILD_TAG is used only in wlan_hdd_main.c
CFLAGS_wlan_hdd_main.o += -DBUILD_TAG=\"$(BUILD_TAG)\"
endif

# Module information used by KBuild framework
obj-$(CONFIG_QCA_CLD_WLAN) += $(MODNAME).o
$(MODNAME)-y := $(OBJS)
OBJS_DIRS := $(dir $(OBJS)) \
	     $(WLAN_COMMON_ROOT)/$(HIF_CE_DIR)/ \
	     $(QDF_OBJ_DIR)/ \
	     $(WLAN_COMMON_ROOT)/$(HIF_PCIE_DIR)/ \
	     $(WLAN_COMMON_ROOT)/$(HIF_SNOC_DIR)/ \
	     $(WLAN_COMMON_ROOT)/$(HIF_SDIO_DIR)/
CLEAN_DIRS := $(addsuffix *.o,$(sort $(OBJS_DIRS))) \
	      $(addsuffix .*.o.cmd,$(sort $(OBJS_DIRS)))
clean-files := $(CLEAN_DIRS)
