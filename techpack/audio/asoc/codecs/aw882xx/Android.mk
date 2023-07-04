# Android makefile for audio kernel modules

# Assume no targets will be supported

# Check if this driver needs be built for current target
ifeq ($(call is-board-platform,msmnile),true)
AUDIO_SELECT  := CONFIG_SND_SOC_SM8150=m
endif

ifeq ($(call is-board-platform,$(MSMSTEPPE)),true)
AUDIO_SELECT  := CONFIG_SND_SOC_SM6150=m
endif

AUDIO_CHIPSET := audio
# Build/Package only in case of supported target
ifeq ($(call is-board-platform-in-list,msmnile $(MSMSTEPPE)),true)

LOCAL_PATH := $(call my-dir)

# This makefile is only for DLKM
ifneq ($(findstring vendor,$(LOCAL_PATH)),)

ifneq ($(findstring opensource,$(LOCAL_PATH)),)
	AUDIO_BLD_DIR := $(shell pwd)/vendor/qcom/opensource/audio-kernel
endif # opensource

DLKM_DIR := $(TOP)/device/qcom/common/dlkm

endif # DLKM check
endif # supported target check
