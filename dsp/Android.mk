# Android makefile for audio kernel modules

# Assume no targets will be supported

# Check if this driver needs be built for current target
ifeq ($(call is-board-platform,msmnile),true)
ifeq ($(TARGET_BOARD_AUTO),true)
AUDIO_SELECT  := CONFIG_SND_SOC_SA8155=m
else
AUDIO_SELECT  := CONFIG_SND_SOC_SM8150=m
endif
endif

ifeq ($(call is-board-platform-in-list,$(MSMSTEPPE) $(TRINKET)),true)
ifeq ($(TARGET_BOARD_AUTO),true)
AUDIO_SELECT  := CONFIG_SND_SOC_SA6155=m
else
AUDIO_SELECT  := CONFIG_SND_SOC_SM6150=m
endif
endif

ifeq ($(call is-board-platform, kona),true)
AUDIO_SELECT  := CONFIG_SND_SOC_KONA=m
endif

ifeq ($(call is-board-platform, lito),true)
AUDIO_SELECT  := CONFIG_SND_SOC_LITO=m
endif

AUDIO_CHIPSET := audio
# Build/Package only in case of supported target
ifeq ($(call is-board-platform-in-list,msmnile $(MSMSTEPPE) $(TRINKET) kona lito bengal),true)

LOCAL_PATH := $(call my-dir)

# This makefile is only for DLKM
ifneq ($(findstring vendor,$(LOCAL_PATH)),)

ifneq ($(findstring opensource,$(LOCAL_PATH)),)
	AUDIO_BLD_DIR := $(shell pwd)/vendor/qcom/opensource/audio-kernel
endif # opensource

DLKM_DIR := $(TOP)/device/qcom/common/dlkm

# Build audio.ko as $(AUDIO_CHIPSET)_audio.ko
###########################################################
# This is set once per LOCAL_PATH, not per (kernel) module
KBUILD_OPTIONS := AUDIO_ROOT=$(AUDIO_BLD_DIR)

# We are actually building audio.ko here, as per the
# requirement we are specifying <chipset>_audio.ko as LOCAL_MODULE.
# This means we need to rename the module to <chipset>_audio.ko
# after audio.ko is built.
KBUILD_OPTIONS += MODNAME=q6_dlkm
KBUILD_OPTIONS += BOARD_PLATFORM=$(TARGET_BOARD_PLATFORM)
KBUILD_OPTIONS += $(AUDIO_SELECT)

###########################################################
include $(CLEAR_VARS)
LOCAL_MODULE              := $(AUDIO_CHIPSET)_q6.ko
LOCAL_MODULE_KBUILD_NAME  := q6_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/AndroidKernelModule.mk
###########################################################
include $(CLEAR_VARS)
LOCAL_MODULE              := $(AUDIO_CHIPSET)_adsp_loader.ko
LOCAL_MODULE_KBUILD_NAME  := adsp_loader_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/AndroidKernelModule.mk
###########################################################
include $(CLEAR_VARS)
LOCAL_MODULE              := $(AUDIO_CHIPSET)_q6_notifier.ko
LOCAL_MODULE_KBUILD_NAME  := q6_notifier_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/AndroidKernelModule.mk
###########################################################
# target specific build
ifneq ($(TARGET_BOARD_AUTO),true)
include $(CLEAR_VARS)
LOCAL_MODULE              := $(AUDIO_CHIPSET)_usf.ko
LOCAL_MODULE_KBUILD_NAME  := usf_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/AndroidKernelModule.mk
###########################################################
include $(CLEAR_VARS)
LOCAL_MODULE              := $(AUDIO_CHIPSET)_q6_pdr.ko
LOCAL_MODULE_KBUILD_NAME  := q6_pdr_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/AndroidKernelModule.mk
###########################################################
endif # target specific build
endif # DLKM check
endif # supported target check
