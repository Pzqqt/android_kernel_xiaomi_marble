# SPDX-License-Identifier: GPL-2.0-only

VIDEO_BLD_DIR := $(shell pwd)/vendor/qcom/opensource/video-driver
VIDEO_SELECT := CONFIG_MSM_VIDC_V4L2=m

# Build msm_video.ko
###########################################################
# This is set once per LOCAL_PATH, not per (kernel) module
KBUILD_OPTIONS := VIDEO_ROOT=$(VIDEO_BLD_DIR)

KBUILD_OPTIONS += MODNAME=msm_video
KBUILD_OPTIONS += BOARD_PLATFORM=$(TARGET_BOARD_PLATFORM)
KBUILD_OPTIONS += $(VIDEO_SELECT)

KBUILD_OPTIONS += KBUILD_EXTRA_SYMBOLS=$(shell pwd)/$(call intermediates-dir-for,DLKM,mmrm-module-symvers)/Module.symvers
###########################################################

DLKM_DIR   := device/qcom/common/dlkm

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
# For incremental compilation
LOCAL_SRC_FILES           := $(wildcard $(LOCAL_PATH)/**/*) $(wildcard $(LOCAL_PATH)/*)
LOCAL_MODULE              := msm_video.ko
LOCAL_MODULE_KBUILD_NAME  := msm_video.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)

LOCAL_REQUIRED_MODULES    := mmrm-module-symvers
LOCAL_ADDITIONAL_DEPENDENCIES := $(call intermediates-dir-for,DLKM,mmrm-module-symvers)/Module.symvers

# Include kp_module.ko in the /vendor/lib/modules (vendor.img)
# BOARD_VENDOR_KERNEL_MODULES += $(LOCAL_MODULE_PATH)/$(LOCAL_MODULE)
include $(DLKM_DIR)/Build_external_kernelmodule.mk
