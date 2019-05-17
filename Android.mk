# Android makefile for the WLAN Module
LOCAL_PATH := $(call my-dir)

# Assume no targets will be supported
WLAN_CHIPSET :=

ifeq ($(BOARD_HAS_QCOM_WLAN), true)

# Check if this driver needs be built for current target
ifneq ($(findstring qca_cld3,$(WIFI_DRIVER_BUILT)),)
	WLAN_CHIPSET := qca_cld3
	WLAN_SELECT  := CONFIG_QCA_CLD_WLAN=m
endif

# Build/Package only in case of supported target
ifneq ($(WLAN_CHIPSET),)

# This makefile is only for DLKM
ifneq ($(findstring vendor,$(LOCAL_PATH)),)

ifneq ($(findstring opensource,$(LOCAL_PATH)),)
	WLAN_BLD_DIR := vendor/qcom/opensource/wlan
endif # opensource

# Multi-ko check
LOCAL_DEV_NAME := $(lastword $(strip \
	$(subst ~, , \
	$(subst /, ,$(LOCAL_PATH)))))

ifeq (1, $(strip $(shell expr $(words $(strip $(TARGET_WLAN_CHIP))) \>= 2)))

ifeq ($(LOCAL_DEV_NAME), qcacld-3.0)
LOCAL_MULTI_KO := true
else
LOCAL_MULTI_KO := false
endif

endif

ifeq ($(LOCAL_MULTI_KO), true)
LOCAL_ANDROID_ROOT := $(shell pwd)
LOCAL_WLAN_BLD_DIR := $(LOCAL_ANDROID_ROOT)/$(WLAN_BLD_DIR)
$(shell rm -rf $(LOCAL_WLAN_BLD_DIR)/qcacld-3.0/~*)

$(foreach chip, $(TARGET_WLAN_CHIP), \
	$($(shell mkdir -p $(LOCAL_WLAN_BLD_DIR)/qcacld-3.0/~$(chip); \
	ln -sf $(LOCAL_WLAN_BLD_DIR)/qca-wifi-host-cmn \
		$(LOCAL_WLAN_BLD_DIR)/qcacld-3.0/~$(chip)/qca-wifi-host-cmn); \
	$(foreach node, \
	$(shell find $(LOCAL_WLAN_BLD_DIR)/qcacld-3.0/ -maxdepth 1 \
		! -name '.*' ! -name '~*' ! -name '*~' \
		! -name '.' ! -name 'qcacld-3.0'), \
	$(shell ln -sf $(node) \
	$(LOCAL_WLAN_BLD_DIR)/qcacld-3.0/~$(chip)/$(lastword $(strip $(subst /, ,$(node)))) \
	))))

include $(foreach chip, $(TARGET_WLAN_CHIP), $(LOCAL_PATH)/~$(chip)/Android.mk)

else # Multi-ok check

ifeq ($(LOCAL_DEV_NAME), qcacld-3.0)

LOCAL_DEV_NAME := wlan
LOCAL_MOD_NAME := wlan
CMN_OFFSET := ..
LOCAL_SRC_DIR :=
WLAN_PROFILE := default
TARGET_FW_DIR := firmware/wlan/qca_cld
TARGET_CFG_PATH := /vendor/etc/wifi
TARGET_MAC_BIN_PATH := /mnt/vendor/persist

else

LOCAL_SRC_DIR := ~$(LOCAL_DEV_NAME)
CMN_OFFSET := .
WLAN_PROFILE := $(LOCAL_DEV_NAME)
TARGET_FW_DIR := firmware/wlan/qca_cld/$(LOCAL_DEV_NAME)
TARGET_CFG_PATH := /vendor/etc/wifi/$(LOCAL_DEV_NAME)
TARGET_MAC_BIN_PATH := /mnt/vendor/persist/$(LOCAL_DEV_NAME)

ifneq ($(TARGET_MULTI_WLAN), true)
LOCAL_MOD_NAME := wlan
DYNAMIC_SINGLE_CHIP := $(LOCAL_DEV_NAME)
else
LOCAL_MOD_NAME := $(LOCAL_DEV_NAME)
endif

endif

# DLKM_DIR was moved for JELLY_BEAN (PLATFORM_SDK 16)
ifeq ($(call is-platform-sdk-version-at-least,16),true)
	DLKM_DIR := $(TOP)/device/qcom/common/dlkm
else
	DLKM_DIR := build/dlkm
endif # platform-sdk-version

# Build wlan.ko as $(WLAN_CHIPSET)_wlan.ko
###########################################################
# This is set once per LOCAL_PATH, not per (kernel) module
KBUILD_OPTIONS := WLAN_ROOT=$(WLAN_BLD_DIR)/qcacld-3.0/$(LOCAL_SRC_DIR)
KBUILD_OPTIONS += WLAN_COMMON_ROOT=$(CMN_OFFSET)/qca-wifi-host-cmn
KBUILD_OPTIONS += WLAN_COMMON_INC=$(WLAN_BLD_DIR)/qca-wifi-host-cmn
KBUILD_OPTIONS += WLAN_FW_API=$(WLAN_BLD_DIR)/fw-api
KBUILD_OPTIONS += WLAN_PROFILE=$(WLAN_PROFILE)
KBUILD_OPTIONS += DYNAMIC_SINGLE_CHIP=$(DYNAMIC_SINGLE_CHIP)

# We are actually building wlan.ko here, as per the
# requirement we are specifying <chipset>_wlan.ko as LOCAL_MODULE.
# This means we need to rename the module to <chipset>_wlan.ko
# after wlan.ko is built.
KBUILD_OPTIONS += MODNAME=$(LOCAL_MOD_NAME)
KBUILD_OPTIONS += BOARD_PLATFORM=$(TARGET_BOARD_PLATFORM)
KBUILD_OPTIONS += $(WLAN_SELECT)

include $(CLEAR_VARS)
LOCAL_MODULE              := $(WLAN_CHIPSET)_$(LOCAL_DEV_NAME).ko
LOCAL_MODULE_KBUILD_NAME  := $(LOCAL_MOD_NAME).ko
LOCAL_MODULE_DEBUG_ENABLE := true
ifeq ($(PRODUCT_VENDOR_MOVE_ENABLED),true)
    ifeq ($(WIFI_DRIVER_INSTALL_TO_KERNEL_OUT),true)
        LOCAL_MODULE_PATH := $(KERNEL_MODULES_OUT)
    else
        LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/lib/modules/$(WLAN_CHIPSET)
    endif
else
    LOCAL_MODULE_PATH := $(TARGET_OUT)/lib/modules/$(WLAN_CHIPSET)
endif

include $(DLKM_DIR)/AndroidKernelModule.mk
###########################################################

# Create Symbolic link
ifneq ($(findstring $(WLAN_CHIPSET),$(WIFI_DRIVER_DEFAULT)),)
ifeq ($(PRODUCT_VENDOR_MOVE_ENABLED),true)
ifneq ($(WIFI_DRIVER_INSTALL_TO_KERNEL_OUT),)
$(shell mkdir -p $(TARGET_OUT_VENDOR)/lib/modules; \
	ln -sf /$(TARGET_COPY_OUT_VENDOR)/lib/modules/$(WLAN_CHIPSET)/$(LOCAL_MODULE) $(TARGET_OUT_VENDOR)/lib/modules/$(LOCAL_MODULE))
endif
else
$(shell mkdir -p $(TARGET_OUT)/lib/modules; \
	ln -sf /system/lib/modules/$(WLAN_CHIPSET)/$(LOCAL_MODULE) $(TARGET_OUT)/lib/modules/$(LOCAL_MODULE))
endif
endif

ifeq ($(PRODUCT_VENDOR_MOVE_ENABLED),true)
TARGET_FW_PATH := $(TARGET_OUT_VENDOR)/$(TARGET_FW_DIR)
else
TARGET_FW_PATH := $(TARGET_OUT_ETC)/$(TARGET_FW_DIR)
endif

$(shell mkdir -p $(TARGET_FW_PATH); \
	ln -sf $(TARGET_MAC_BIN_PATH)/wlan_mac.bin $(TARGET_FW_PATH)/wlan_mac.bin; \
	ln -sf $(TARGET_CFG_PATH)/WCNSS_qcom_cfg.ini $(TARGET_FW_PATH)/WCNSS_qcom_cfg.ini)

endif # Multi-ko check
endif # DLKM check
endif # supported target check
endif # WLAN enabled check
