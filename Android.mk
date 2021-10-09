# Android makefile for audio kernel modules

LOCAL_PATH := $(call my-dir)

ifeq ($(call is-board-platform, taro),true)
AUDIO_SELECT  := CONFIG_SND_SOC_WAIPIO=m
endif

# Build/Package only in case of supported target
ifeq ($(call is-board-platform-in-list,taro kalama), true)

LOCAL_PATH := $(call my-dir)

# This makefile is only for DLKM
ifneq ($(findstring vendor,$(LOCAL_PATH)),)

ifneq ($(findstring opensource,$(LOCAL_PATH)),)
	AUDIO_BLD_DIR := $(abspath .)/vendor/qcom/opensource/audio-kernel
endif # opensource

DLKM_DIR := $(TOP)/device/qcom/common/dlkm


###########################################################
# This is set once per LOCAL_PATH, not per (kernel) module
KBUILD_OPTIONS := AUDIO_ROOT=$(AUDIO_BLD_DIR)

# We are actually building audio.ko here, as per the
# requirement we are specifying <chipset>_audio.ko as LOCAL_MODULE.
# This means we need to rename the module to <chipset>_audio.ko
# after audio.ko is built.
KBUILD_OPTIONS += MODNAME=audio_dlkm
KBUILD_OPTIONS += BOARD_PLATFORM=$(TARGET_BOARD_PLATFORM)
KBUILD_OPTIONS += $(AUDIO_SELECT)

AUDIO_SRC_FILES := \
	$(wildcard $(LOCAL_PATH)/*) \
	$(wildcard $(LOCAL_PATH)/*/*) \
	$(wildcard $(LOCAL_PATH)/*/*/*) \
	$(wildcard $(LOCAL_PATH)/*/*/*/*)

########################### dsp ################################

include $(CLEAR_VARS)
LOCAL_SRC_FILES           := $(AUDIO_SRC_FILES)
LOCAL_MODULE              := q6_notifier_dlkm.ko
LOCAL_MODULE_KBUILD_NAME  := dsp/q6_notifier_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/Build_external_kernelmodule.mk
###########################################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES           := $(AUDIO_SRC_FILES)
LOCAL_MODULE              := spf_core_dlkm.ko
LOCAL_MODULE_KBUILD_NAME  := dsp/spf_core_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/Build_external_kernelmodule.mk
###########################################################

include $(CLEAR_VARS)
LOCAL_SRC_FILES           := $(AUDIO_SRC_FILES)
LOCAL_MODULE              := audpkt_ion_dlkm.ko
LOCAL_MODULE_KBUILD_NAME  := dsp/audpkt_ion_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/Build_external_kernelmodule.mk
###########################################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES           := $(AUDIO_SRC_FILES)
LOCAL_MODULE              := gpr_dlkm.ko
LOCAL_MODULE_KBUILD_NAME  := ipc/gpr_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/Build_external_kernelmodule.mk
###########################################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES           := $(AUDIO_SRC_FILES)
LOCAL_MODULE              := audio_pkt_dlkm.ko
LOCAL_MODULE_KBUILD_NAME  := ipc/audio_pkt_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/Build_external_kernelmodule.mk
###########################################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES           := $(AUDIO_SRC_FILES)
LOCAL_MODULE              := q6_dlkm.ko
LOCAL_MODULE_KBUILD_NAME  := dsp/q6_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/Build_external_kernelmodule.mk
###########################################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES           := $(AUDIO_SRC_FILES)
LOCAL_MODULE              := adsp_loader_dlkm.ko
LOCAL_MODULE_KBUILD_NAME  := dsp/adsp_loader_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/Build_external_kernelmodule.mk

########################### ipc  ################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES           := $(AUDIO_SRC_FILES)
LOCAL_MODULE              := audio_prm_dlkm.ko
LOCAL_MODULE_KBUILD_NAME  := dsp/audio_prm_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/Build_external_kernelmodule.mk
###########################################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES           := $(AUDIO_SRC_FILES)
LOCAL_MODULE              := q6_pdr_dlkm.ko
LOCAL_MODULE_KBUILD_NAME  := dsp/q6_pdr_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/Build_external_kernelmodule.mk

############################ soc ###############################
include $(CLEAR_VARS)
LOCAL_SRC_FILES           := $(AUDIO_SRC_FILES)
LOCAL_MODULE              := pinctrl_lpi_dlkm.ko
LOCAL_MODULE_KBUILD_NAME  := soc/pinctrl_lpi_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/Build_external_kernelmodule.mk
###########################################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES           := $(AUDIO_SRC_FILES)
LOCAL_MODULE              := swr_dlkm.ko
LOCAL_MODULE_KBUILD_NAME  := soc/swr_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/Build_external_kernelmodule.mk
###########################################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES           := $(AUDIO_SRC_FILES)
LOCAL_MODULE              := swr_ctrl_dlkm.ko
LOCAL_MODULE_KBUILD_NAME  := soc/swr_ctrl_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/Build_external_kernelmodule.mk
###########################################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES           := $(AUDIO_SRC_FILES)
LOCAL_MODULE              := snd_event_dlkm.ko
LOCAL_MODULE_KBUILD_NAME  := soc/snd_event_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/Build_external_kernelmodule.mk
###########################  ASOC CODEC ################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES           := $(AUDIO_SRC_FILES)
LOCAL_MODULE              := wcd_core_dlkm.ko
LOCAL_MODULE_KBUILD_NAME  := asoc/codecs/wcd_core_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/Build_external_kernelmodule.mk
###########################################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES           := $(AUDIO_SRC_FILES)
LOCAL_MODULE              := mbhc_dlkm.ko
LOCAL_MODULE_KBUILD_NAME  := asoc/codecs/mbhc_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/Build_external_kernelmodule.mk
###########################################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES           := $(AUDIO_SRC_FILES)
LOCAL_MODULE              := swr_dmic_dlkm.ko
LOCAL_MODULE_KBUILD_NAME  := asoc/codecs/swr_dmic_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/Build_external_kernelmodule.mk
###########################################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES           := $(AUDIO_SRC_FILES)
LOCAL_MODULE              := wcd9xxx_dlkm.ko
LOCAL_MODULE_KBUILD_NAME  := asoc/codecs/wcd9xxx_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/Build_external_kernelmodule.mk
###########################################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES           := $(AUDIO_SRC_FILES)
LOCAL_MODULE              := swr_haptics_dlkm.ko
LOCAL_MODULE_KBUILD_NAME  := asoc/codecs/swr_haptics_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/Build_external_kernelmodule.mk
###########################################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES           := $(AUDIO_SRC_FILES)
LOCAL_MODULE              := stub_dlkm.ko
LOCAL_MODULE_KBUILD_NAME  := asoc/codecs/stub_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/Build_external_kernelmodule.mk
########################### ASOC MACHINE ################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES           := $(AUDIO_SRC_FILES)
LOCAL_MODULE              := machine_dlkm.ko
LOCAL_MODULE_KBUILD_NAME  := asoc/machine_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/Build_external_kernelmodule.mk
########################### LPASS-CDC CODEC  ###########################
include $(CLEAR_VARS)
LOCAL_SRC_FILES           := $(AUDIO_SRC_FILES)
LOCAL_MODULE              := lpass_cdc_dlkm.ko
LOCAL_MODULE_KBUILD_NAME  := asoc/codecs/lpass-cdc/lpass_cdc_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/Build_external_kernelmodule.mk
###########################################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES           := $(AUDIO_SRC_FILES)
LOCAL_MODULE              := lpass_cdc_wsa2_macro_dlkm.ko
LOCAL_MODULE_KBUILD_NAME  := asoc/codecs/lpass-cdc/lpass_cdc_wsa2_macro_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/Build_external_kernelmodule.mk
###########################################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES           := $(AUDIO_SRC_FILES)
LOCAL_MODULE              := lpass_cdc_wsa_macro_dlkm.ko
LOCAL_MODULE_KBUILD_NAME  := asoc/codecs/lpass-cdc/lpass_cdc_wsa_macro_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/Build_external_kernelmodule.mk
###########################################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES           := $(AUDIO_SRC_FILES)
LOCAL_MODULE              := lpass_cdc_va_macro_dlkm.ko
LOCAL_MODULE_KBUILD_NAME  := asoc/codecs/lpass-cdc/lpass_cdc_va_macro_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/Build_external_kernelmodule.mk
###########################################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES           := $(AUDIO_SRC_FILES)
LOCAL_MODULE              := lpass_cdc_tx_macro_dlkm.ko
LOCAL_MODULE_KBUILD_NAME  := asoc/codecs/lpass-cdc/lpass_cdc_tx_macro_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/Build_external_kernelmodule.mk
###########################################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES           := $(AUDIO_SRC_FILES)
LOCAL_MODULE              := lpass_cdc_rx_macro_dlkm.ko
LOCAL_MODULE_KBUILD_NAME  := asoc/codecs/lpass-cdc/lpass_cdc_rx_macro_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/Build_external_kernelmodule.mk

########################### WSA883x CODEC  ###########################
include $(CLEAR_VARS)
LOCAL_SRC_FILES           := $(AUDIO_SRC_FILES)
LOCAL_MODULE              := wsa883x_dlkm.ko
LOCAL_MODULE_KBUILD_NAME  := asoc/codecs/wsa883x/wsa883x_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/Build_external_kernelmodule.mk


########################### WCD938x CODEC  ################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES           := $(AUDIO_SRC_FILES)
LOCAL_MODULE              := wcd938x_dlkm.ko
LOCAL_MODULE_KBUILD_NAME  := asoc/codecs/wcd938x/wcd938x_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/Build_external_kernelmodule.mk
###########################################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES           := $(AUDIO_SRC_FILES)
LOCAL_MODULE              := wcd938x_slave_dlkm.ko
LOCAL_MODULE_KBUILD_NAME  := asoc/codecs/wcd938x/wcd938x_slave_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/Build_external_kernelmodule.mk

########################### WCD937x CODEC  ################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES           := $(AUDIO_SRC_FILES)
LOCAL_MODULE              := wcd937x_dlkm.ko
LOCAL_MODULE_KBUILD_NAME  := asoc/codecs/wcd937x/wcd937x_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/Build_external_kernelmodule.mk
###########################################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES           := $(AUDIO_SRC_FILES)
LOCAL_MODULE              := wcd937x_slave_dlkm.ko
LOCAL_MODULE_KBUILD_NAME  := asoc/codecs/wcd937x/wcd937x_slave_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/Build_external_kernelmodule.mk

###########################################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES           := $(AUDIO_SRC_FILES)
LOCAL_MODULE              := hdmi_dlkm.ko
LOCAL_MODULE_KBUILD_NAME  := asoc/codecs/hdmi_dlkm.ko
LOCAL_MODULE_TAGS         := optional
LOCAL_MODULE_DEBUG_ENABLE := true
LOCAL_MODULE_PATH         := $(KERNEL_MODULES_OUT)
include $(DLKM_DIR)/Build_external_kernelmodule.mk
###########################################################

endif # DLKM check
endif # supported target check
