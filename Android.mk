# Android makefile for the WLAN Module

# Assume no targets will be supported
WLAN_CHIPSET :=

ifeq ($(BOARD_HAS_QCOM_WLAN), true)

# Build/Package options for 8084/8092/8960/8992/8994 target
ifeq ($(call is-board-platform-in-list, msm8994 msm8996 msm8998 msmcobalt),true)
	WLAN_CHIPSET := qca_cld3
	WLAN_SELECT  := CONFIG_QCA_CLD_WLAN=m
endif # platform

# Build/Package only in case of supported target
ifneq ($(WLAN_CHIPSET),)

# If TARGET_KERNEL_VERSION is not defined, using default kernel path,
# otherwise kernel path should come from top level Android makefiles.
ifeq ($(TARGET_KERNEL_VERSION),)
$(info "WLAN: TARGET_KERNEL_VERSION not defined, assuming default")
TARGET_KERNEL_SOURCE := kernel
KERNEL_TO_BUILD_ROOT_OFFSET := ../
endif

LOCAL_PATH := $(call my-dir)

# This makefile is only for DLKM
ifneq ($(findstring vendor,$(LOCAL_PATH)),)

ifneq ($(findstring opensource,$(LOCAL_PATH)),)
    WLAN_BLD_DIR := vendor/qcom/opensource/wlan
endif # opensource

# DLKM_DIR was moved for JELLY_BEAN (PLATFORM_SDK 16)
ifeq ($(call is-platform-sdk-version-at-least,16),true)
       DLKM_DIR := $(TOP)/device/qcom/common/dlkm
else
       DLKM_DIR := build/dlkm
endif # platform-sdk-version

# Build wlan.ko as $(WLAN_CHIPSET)_wlan.ko
###########################################################
# This is set once per LOCAL_PATH, not per (kernel) module
KBUILD_OPTIONS := WLAN_ROOT=$(KERNEL_TO_BUILD_ROOT_OFFSET)$(WLAN_BLD_DIR)/qcacld-3.0
KBUILD_OPTIONS += WLAN_COMMON_ROOT=../qca-wifi-host-cmn
KBUILD_OPTIONS += WLAN_COMMON_INC=$(KERNEL_TO_BUILD_ROOT_OFFSET)$(WLAN_BLD_DIR)/qca-wifi-host-cmn

# We are actually building wlan.ko here, as per the
# requirement we are specifying <chipset>_wlan.ko as LOCAL_MODULE.
# This means we need to rename the module to <chipset>_wlan.ko
# after wlan.ko is built.
KBUILD_OPTIONS += MODNAME=wlan
KBUILD_OPTIONS += BOARD_PLATFORM=$(TARGET_BOARD_PLATFORM)
KBUILD_OPTIONS += $(WLAN_SELECT)

include $(CLEAR_VARS)
LOCAL_MODULE              := $(WLAN_CHIPSET)_wlan.ko
LOCAL_MODULE_KBUILD_NAME  := wlan.ko
LOCAL_MODULE_TAGS         := debug
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(TARGET_OUT)/lib/modules/$(WLAN_CHIPSET)
include $(DLKM_DIR)/AndroidKernelModule.mk
###########################################################

# Create Symbolic link
$(shell ln -sf /persist/wlan_mac.bin $(TARGET_OUT_ETC)/firmware/wlan/qca_cld/wlan_mac.bin)

endif # DLKM check

endif # supported target check
endif # WLAN enabled check
