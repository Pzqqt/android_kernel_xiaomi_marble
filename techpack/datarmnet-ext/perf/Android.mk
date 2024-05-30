ifneq ($(TARGET_BOARD_AUTO),true)
ifneq ($(TARGET_BOARD_PLATFORM),qssi)

RMNET_PERF_DLKM_PLATFORMS_LIST := taro
RMNET_PERF_DLKM_PLATFORMS_LIST += parrot

ifeq ($(call is-board-platform-in-list, $(RMNET_PERF_DLKM_PLATFORMS_LIST)),true)
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_PATH := $(KERNEL_MODULES_OUT)

LOCAL_MODULE := rmnet_perf.ko
LOCAL_SRC_FILES := $(wildcard $(LOCAL_PATH)/**/*) $(wildcard $(LOCAL_PATH)/*)

#path from build top to the core directory
DATARMNET_CORE_PATH := datarmnet/core
RMNET_CORE_PATH := vendor/qcom/opensource/$(DATARMNET_CORE_PATH)
DLKM_DIR := $(TOP)/device/qcom/common/dlkm
#absolute path to the build directory. Can't use $(TOP) here since
#that resolves to ., and we pass this to Kbuild, where . is different
RMNET_CORE_INC_DIR := $(abspath $(RMNET_CORE_PATH))

#pass variables down to Kbuild environment
KBUILD_OPTIONS := RMNET_CORE_INC_DIR=$(RMNET_CORE_INC_DIR)
KBUILD_OPTIONS += RMNET_CORE_PATH=$(RMNET_CORE_PATH)
KBUILD_OPTIONS += DATARMNET_CORE_PATH=$(DATARMNET_CORE_PATH)
KBUILD_OPTIONS_GKI := RMNET_CORE_INC_DIR=$(RMNET_CORE_INC_DIR)
KBUILD_OPTIONS_GKI += RMNET_CORE_PATH=$(RMNET_CORE_PATH)/gki

#rmnet shs path
DATARMNET_SHS_PATH := datarmnet-ext/shs
RMNET_SHS_PATH := vendor/qcom/opensource/$(DATARMNET_SHS_PATH)
RMNET_SHS_INC_DIR := $(abspath $(RMNET_SHS_PATH))

#Must be built after the core rmnet and shs module
KBUILD_OPTIONS += RMNET_SHS_INC_DIR=$(RMNET_SHS_INC_DIR)
KBUILD_OPTIONS += RMNET_SHS_PATH=$(RMNET_SHS_PATH)
KBUILD_OPTIONS += DATARMNET_SHS_PATH=$(DATARMNET_SHS_PATH)
KBUILD_OPTIONS_GKI += RMNET_SHS_INC_DIR=$(RMNET_SHS_INC_DIR)
KBUILD_OPTIONS_GKI += RMNET_SHS_PATH=$(RMNET_SHS_PATH)/gki

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/DLKM_OBJ/$(RMNET_CORE_PATH)/rmnet_core.ko
LOCAL_ADDITIONAL_DEPENDENCIES_GKI := $(TARGET_OUT_INTERMEDIATES)/DLKM_OBJ/$(RMNET_CORE_PATH)/gki/rmnet_core.ko
LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/DLKM_OBJ/$(RMNET_SHS_PATH)/rmnet_shs.ko
LOCAL_ADDITIONAL_DEPENDENCIES_GKI += $(TARGET_OUT_INTERMEDIATES)/DLKM_OBJ/$(RMNET_SHS_PATH)/gki/rmnet_shs.ko

include $(DLKM_DIR)/Build_external_kernelmodule.mk

endif #End of check for target
endif #End of Check for qssi target
endif #End of check for AUTO Target
