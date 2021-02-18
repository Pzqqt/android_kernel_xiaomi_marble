# Android makefile for the WLAN Module

# set WLAN_BUILD_DEBUG=y in your environment to enable debug logging
define wlog
$(if $(WLAN_BUILD_DEBUG),$(info $(1)))
endef

LOCAL_PATH := $(call my-dir)
$(call wlog,LOCAL_PATH=$(LOCAL_PATH))

ENABLE_QCACLD := true
ifeq ($(TARGET_USES_QMAA), true)
ifneq ($(TARGET_USES_QMAA_OVERRIDE_WLAN), true)
ENABLE_QCACLD := false
else
ENABLE_QCACLD := true
endif
endif

ifeq  ($(ENABLE_QCACLD), true)

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
LOCAL_DEV_NAME := $(patsubst .%,%,\
	$(lastword $(strip $(subst /, ,$(LOCAL_PATH)))))

$(call wlog,LOCAL_DEV_NAME=$(LOCAL_DEV_NAME))
$(call wlog,TARGET_WLAN_CHIP=$(TARGET_WLAN_CHIP))

TARGET_WLAN_CHIP ?= wlan
LOCAL_MULTI_KO := false
ifneq ($(TARGET_WLAN_CHIP), wlan)
ifeq ($(LOCAL_DEV_NAME), qcacld-3.0)
LOCAL_MULTI_KO := true
endif
endif

ifeq ($(LOCAL_MULTI_KO), true)
LOCAL_ANDROID_ROOT := $(shell pwd)
LOCAL_WLAN_BLD_DIR := $(LOCAL_ANDROID_ROOT)/$(WLAN_BLD_DIR)
$(shell find $(LOCAL_WLAN_BLD_DIR)/qcacld-3.0/ -maxdepth 1 \
	-name '.*' ! -name '.git' -exec rm -rf {} +)

$(foreach chip, $(TARGET_WLAN_CHIP), \
	$(shell ln -sf . $(LOCAL_WLAN_BLD_DIR)/qcacld-3.0/.$(chip)))
include $(foreach chip, $(TARGET_WLAN_CHIP), $(LOCAL_PATH)/.$(chip)/Android.mk)

else # Multi-ok check

ifeq ($(WLAN_PROFILE),)
WLAN_PROFILE := default
endif

ifeq ($(LOCAL_DEV_NAME), qcacld-3.0)

LOCAL_DEV_NAME := wlan
LOCAL_MOD_NAME := wlan
LOCAL_SRC_DIR :=
TARGET_FW_DIR := firmware/wlan/qca_cld
TARGET_CFG_PATH := /vendor/etc/wifi
TARGET_MAC_BIN_PATH := /mnt/vendor/persist

else

LOCAL_SRC_DIR := .$(LOCAL_DEV_NAME)
# Use default profile if WLAN_CFG_USE_DEFAULT defined.
ifeq ($(WLAN_CFG_USE_DEFAULT),)
WLAN_PROFILE := $(LOCAL_DEV_NAME)
endif
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
KBUILD_OPTIONS += WLAN_COMMON_ROOT=cmn
KBUILD_OPTIONS += WLAN_COMMON_INC=$(WLAN_BLD_DIR)/qcacld-3.0/cmn
KBUILD_OPTIONS += WLAN_FW_API=$(WLAN_BLD_DIR)/fw-api
KBUILD_OPTIONS += WLAN_PROFILE=$(WLAN_PROFILE)
KBUILD_OPTIONS += DYNAMIC_SINGLE_CHIP=$(DYNAMIC_SINGLE_CHIP)

# We are actually building wlan.ko here, as per the
# requirement we are specifying <chipset>_wlan.ko as LOCAL_MODULE.
# This means we need to rename the module to <chipset>_wlan.ko
# after wlan.ko is built.
KBUILD_OPTIONS += MODNAME=$(LOCAL_MOD_NAME)
KBUILD_OPTIONS += DEVNAME=$(LOCAL_DEV_NAME)
KBUILD_OPTIONS += BOARD_PLATFORM=$(TARGET_BOARD_PLATFORM)
KBUILD_OPTIONS += $(WLAN_SELECT)

ifneq ($(WLAN_CFG_OVERRIDE_$(LOCAL_DEV_NAME)),)
KBUILD_OPTIONS += WLAN_CFG_OVERRIDE="$(WLAN_CFG_OVERRIDE_$(LOCAL_DEV_NAME))"
endif

# Pass build options per chip to Kbuild. This will be injected from upper layer
# makefile.
#
# e.g.
#  WLAN_KBUILD_OPTIONS_qca6390 := CONFIG_CNSS_QCA6390=y
ifneq ($(WLAN_KBUILD_OPTIONS_$(LOCAL_DEV_NAME)),)
KBUILD_OPTIONS += "$(WLAN_KBUILD_OPTIONS_$(LOCAL_DEV_NAME))"
endif

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

ifeq ($(TARGET_PRODUCT), taro)
    include $(DLKM_DIR)/Build_external_kernelmodule.mk
else
    include $(DLKM_DIR)/AndroidKernelModule.mk
endif
###########################################################

# Create Symbolic link
ifneq ($(findstring $(WLAN_CHIPSET),$(WIFI_DRIVER_DEFAULT)),)
ifeq ($(PRODUCT_VENDOR_MOVE_ENABLED),true)
ifneq ($(WIFI_DRIVER_INSTALL_TO_KERNEL_OUT),true)
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
	ln -sf $(TARGET_MAC_BIN_PATH)/wlan_mac.bin $(TARGET_FW_PATH)/wlan_mac.bin)
ifeq ($(TARGET_BOARD_AUTO),true)
$(shell ln -sf $(TARGET_CFG_PATH)/WCNSS_qcom_cfg.ini $(TARGET_FW_PATH)/WCNSS_qcom_cfg.ini)
$(call wlog,"generate soft link because TARGET_BOARD_AUTO true")
else
ifneq ($(GENERIC_ODM_IMAGE),true)
$(shell ln -sf $(TARGET_CFG_PATH)/WCNSS_qcom_cfg.ini $(TARGET_FW_PATH)/WCNSS_qcom_cfg.ini)
$(call wlog,"generate soft link because GENERIC_ODM_IMAGE not true")
endif
endif
endif # Multi-ko check
endif # DLKM check
endif # supported target check
endif # WLAN enabled check
endif # ENABLE_QCACLD
