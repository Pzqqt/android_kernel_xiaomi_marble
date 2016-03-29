# Android makefile for the WLAN Module

# Assume no targets will be supported
WLAN_CHIPSET :=

ifeq ($(BOARD_HAS_QCOM_WLAN), true)

# Build/Package options for 8084/8092/8960/8992/8994 target
ifeq ($(call is-board-platform-in-list, apq8084 mpq8092 msm8960 msm8992 msm8994 msm8996 msm8998),true)
	WLAN_CHIPSET := qca_cld3
	WLAN_SELECT  := CONFIG_QCA_CLD_WLAN=m
endif # platform

# Build/Package only in case of supported target
ifneq ($(WLAN_CHIPSET),)

LOCAL_PATH := $(call my-dir)

# This makefile is only for DLKM
ifneq ($(findstring vendor,$(LOCAL_PATH)),)

ifneq ($(findstring opensource,$(LOCAL_PATH)),)
    WLAN_PROPRIETARY := 0
    WLAN_BLD_DIR := vendor/qcom/opensource/wlan
else
    WLAN_PROPRIETARY := 1
    WLAN_BLD_DIR := vendor/qcom/proprietary/wlan-noship
endif # opensource

# DLKM_DIR was moved for JELLY_BEAN (PLATFORM_SDK 16)
ifeq ($(call is-platform-sdk-version-at-least,16),true)
       DLKM_DIR := $(TOP)/device/qcom/common/dlkm
else
       DLKM_DIR := build/dlkm
endif # platform-sdk-version

# Copy WCNSS_qcom_cfg.ini file from firmware_bin/ folder to target out directory.
ifeq ($(call is-board-platform-in-list, msm8960),true)
$(shell rm -f $(TARGET_OUT_ETC)/firmware/wlan/qca_cld/WCNSS_qcom_cfg.ini)
$(shell cp $(LOCAL_PATH)/firmware_bin/WCNSS_qcom_cfg.ini $(TARGET_OUT_ETC)/firmware/wlan/qca_cld)
endif

# Build wlan.ko as $(WLAN_CHIPSET)_wlan.ko
###########################################################
# This is set once per LOCAL_PATH, not per (kernel) module
ifeq ($(WLAN_PROPRIETARY),1)
	KBUILD_OPTIONS := WLAN_ROOT=../$(WLAN_BLD_DIR)/qcacld-new
	KBUILD_OPTIONS += WLAN_COMMON_ROOT=../../../opensource/wlan/qca-wifi-host-cmn
else
	KBUILD_OPTIONS := WLAN_ROOT=../$(WLAN_BLD_DIR)/qcacld-3.0
	KBUILD_OPTIONS += WLAN_COMMON_ROOT=../../../opensource/wlan/qca-wifi-host-cmn
endif # WLAN_PROPRIETARY
KBUILD_OPTIONS += WLAN_COMMON_INC=../vendor/qcom/opensource/wlan/qca-wifi-host-cmn

# We are actually building wlan.ko here, as per the
# requirement we are specifying <chipset>_wlan.ko as LOCAL_MODULE.
# This means we need to rename the module to <chipset>_wlan.ko
# after wlan.ko is built.
KBUILD_OPTIONS += MODNAME=wlan
KBUILD_OPTIONS += BOARD_PLATFORM=$(TARGET_BOARD_PLATFORM)
KBUILD_OPTIONS += $(WLAN_SELECT)

include $(CLEAR_VARS)
ifeq ($(WLAN_PROPRIETARY),1)
	LOCAL_MODULE              := proprietary_$(WLAN_CHIPSET)_wlan.ko
else
	LOCAL_MODULE              := $(WLAN_CHIPSET)_wlan.ko
endif # WLAN_PROPRIETARY
LOCAL_MODULE_KBUILD_NAME  := wlan.ko
LOCAL_MODULE_TAGS         := debug
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(TARGET_OUT)/lib/modules/$(WLAN_CHIPSET)
include $(DLKM_DIR)/AndroidKernelModule.mk
###########################################################

# Create Symbolic link
ifeq ($(WLAN_PROPRIETARY),1)
$(shell mkdir -p $(TARGET_OUT)/lib/modules; \
        ln -sf /system/lib/modules/$(WLAN_CHIPSET)/$(LOCAL_MODULE) \
               $(TARGET_OUT)/lib/modules/wlan.ko)
endif
$(shell ln -sf /persist/wlan_mac.bin $(TARGET_OUT_ETC)/firmware/wlan/qca_cld/wlan_mac.bin)

ifeq ($(call is-board-platform-in-list, msm8960),true)
$(shell ln -sf /firmware/image/bdwlan20.bin $(TARGET_OUT_ETC)/firmware/fakeboar.bin)
$(shell ln -sf /firmware/image/otp20.bin $(TARGET_OUT_ETC)/firmware/otp.bin)
$(shell ln -sf /firmware/image/utf20.bin $(TARGET_OUT_ETC)/firmware/utf.bin)
$(shell ln -sf /firmware/image/qwlan20.bin $(TARGET_OUT_ETC)/firmware/athwlan.bin)

$(shell ln -sf /firmware/image/bdwlan20.bin $(TARGET_OUT_ETC)/firmware/bdwlan20.bin)
$(shell ln -sf /firmware/image/otp20.bin $(TARGET_OUT_ETC)/firmware/otp20.bin)
$(shell ln -sf /firmware/image/utf20.bin $(TARGET_OUT_ETC)/firmware/utf20.bin)
$(shell ln -sf /firmware/image/qwlan20.bin $(TARGET_OUT_ETC)/firmware/qwlan20.bin)

$(shell ln -sf /firmware/image/bdwlan30.bin $(TARGET_OUT_ETC)/firmware/bdwlan30.bin)
$(shell ln -sf /firmware/image/otp30.bin $(TARGET_OUT_ETC)/firmware/otp30.bin)
$(shell ln -sf /firmware/image/utf30.bin $(TARGET_OUT_ETC)/firmware/utf30.bin)
$(shell ln -sf /firmware/image/qwlan30.bin $(TARGET_OUT_ETC)/firmware/qwlan30.bin)
endif

# Copy config ini files to target
#ifeq ($(call is-board-platform-in-list, msm8992 msm8994),false)
ifeq ($(WLAN_PROPRIETARY),1)
$(shell mkdir -p $(TARGET_OUT)/etc/firmware/wlan/$(WLAN_CHIPSET))
$(shell mkdir -p $(TARGET_OUT)/etc/wifi)
$(shell rm -f $(TARGET_OUT)/etc/wifi/WCNSS_qcom_cfg.ini)
$(shell cp $(LOCAL_PATH)/config/WCNSS_qcom_cfg.ini $(TARGET_OUT)/etc/wifi)
endif
#endif

endif # DLKM check

endif # supported target check
endif # WLAN enabled check
