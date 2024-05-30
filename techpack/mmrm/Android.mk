ifneq ($(TARGET_BOARD_PLATFORM), monaco)
BOARD_OPENSOURCE_DIR ?= vendor/qcom/opensource
BOARD_COMMON_DIR ?= device/qcom/common

MMRM_BLD_DIR := $(shell pwd)/$(BOARD_OPENSOURCE_DIR)/mmrm-driver

# Build msm-mmrm.ko
###########################################################
# This is set once per LOCAL_PATH, not per (kernel) module
KBUILD_OPTIONS := MMRM_ROOT=$(MMRM_BLD_DIR)
KBUILD_OPTIONS += BOARD_PLATFORM=$(TARGET_BOARD_PLATFORM)
###########################################################

DLKM_DIR   := $(BOARD_COMMON_DIR)/dlkm

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
# For incremental compilation
LOCAL_SRC_FILES           := $(wildcard $(LOCAL_PATH)/**/*) $(wildcard $(LOCAL_PATH)/*)
LOCAL_MODULE              := mmrm-module-symvers
LOCAL_MODULE_STEM         := Module.symvers
LOCAL_MODULE_KBUILD_NAME  := Module.symvers
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
# Include kp_module.ko in the /vendor/lib/modules (vendor.img)
# BOARD_VENDOR_KERNEL_MODULES += $(LOCAL_MODULE_PATH)/$(LOCAL_MODULE)
include $(DLKM_DIR)/Build_external_kernelmodule.mk

include $(CLEAR_VARS)
# For incremental compilation
LOCAL_SRC_FILES           := $(wildcard $(LOCAL_PATH)/**/*) $(wildcard $(LOCAL_PATH)/*)
LOCAL_MODULE              := msm-mmrm.ko
LOCAL_MODULE_KBUILD_NAME  := driver/msm-mmrm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
# Include kp_module.ko in the /vendor/lib/modules (vendor.img)
# BOARD_VENDOR_KERNEL_MODULES += $(LOCAL_MODULE_PATH)/$(LOCAL_MODULE)
include $(DLKM_DIR)/Build_external_kernelmodule.mk

include $(CLEAR_VARS)
# For incremental compilation
LOCAL_SRC_FILES           := $(wildcard $(LOCAL_PATH)/**/*) $(wildcard $(LOCAL_PATH)/*)
LOCAL_MODULE              := mmrm_test_module.ko
LOCAL_MODULE_KBUILD_NAME  := test/mmrm_test_module.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
# Include kp_module.ko in the /vendor/lib/modules (vendor.img)
# BOARD_VENDOR_KERNEL_MODULES += $(LOCAL_MODULE_PATH)/$(LOCAL_MODULE)
include $(DLKM_DIR)/Build_external_kernelmodule.mk
endif
