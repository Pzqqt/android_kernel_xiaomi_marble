ifneq ($(TARGET_BOARD_PLATFORM),qssi)
GSI_DLKM_PLATFORMS_LIST := taro parrot

ifeq ($(call is-board-platform-in-list, $(GSI_DLKM_PLATFORMS_LIST)),true)
#Make file to create GSI DLKM
DLKM_DIR := $(TOP)/device/qcom/common/dlkm
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_CFLAGS := -Wno-macro-redefined -Wno-unused-function -Wall -Werror
LOCAL_CLANG :=true


KBUILD_OPTIONS += MODNAME=gsim
LOCAL_SRC_FILES   := $(wildcard $(LOCAL_PATH)/**/*) $(wildcard $(LOCAL_PATH)/*)
LOCAL_MODULE              := gsim.ko
LOCAL_MODULE_KBUILD_NAME  := gsi/gsim.ko
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH := $(KERNEL_MODULES_OUT)
$(warning $(DLKM_DIR))
include $(DLKM_DIR)/Build_external_kernelmodule.mk


include $(CLEAR_VARS)
KBUILD_OPTIONS += MODNAME=ipam
LOCAL_SRC_FILES   := $(wildcard $(LOCAL_PATH)/**/*) $(wildcard $(LOCAL_PATH)/*)
LOCAL_MODULE              := ipam.ko
LOCAL_MODULE_KBUILD_NAME  := ipa/ipam.ko
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH := $(KERNEL_MODULES_OUT)
$(warning $(DLKM_DIR))
include $(DLKM_DIR)/Build_external_kernelmodule.mk

include $(CLEAR_VARS)
KBUILD_OPTIONS += MODNAME=ipanetm
LOCAL_SRC_FILES   := $(wildcard $(LOCAL_PATH)/**/*) $(wildcard $(LOCAL_PATH)/*)
LOCAL_MODULE              := ipanetm.ko
LOCAL_MODULE_KBUILD_NAME  := ipa/ipanetm.ko
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH := $(KERNEL_MODULES_OUT)
$(warning $(DLKM_DIR))
include $(DLKM_DIR)/Build_external_kernelmodule.mk

include $(CLEAR_VARS)
KBUILD_OPTIONS += MODNAME=rndisipam
LOCAL_SRC_FILES   := $(wildcard $(LOCAL_PATH)/**/*) $(wildcard $(LOCAL_PATH)/*)
LOCAL_MODULE              := rndisipam.ko
LOCAL_MODULE_KBUILD_NAME  := ipa/ipa_clients/rndisipam.ko
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH := $(KERNEL_MODULES_OUT)
$(warning $(DLKM_DIR))
include $(DLKM_DIR)/Build_external_kernelmodule.mk

include $(CLEAR_VARS)
KBUILD_OPTIONS += MODNAME=ipaclientsm
LOCAL_SRC_FILES   := $(wildcard $(LOCAL_PATH)/**/*) $(wildcard $(LOCAL_PATH)/*)
LOCAL_MODULE              := ipa_clientsm.ko
LOCAL_MODULE_KBUILD_NAME  := ipa/ipa_clients/ipa_clientsm.ko
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH := $(KERNEL_MODULES_OUT)
$(warning $(DLKM_DIR))
include $(DLKM_DIR)/Build_external_kernelmodule.mk

endif #End of Check for target
endif #End of Check for qssi target
