ifneq ($(TARGET_BOARD_AUTO),true)
ifneq ($(TARGET_PRODUCT),qssi)

RMNET_OFFLOAD_DLKM_PLATFORMS_LIST := lahaina

ifeq ($(call is-board-platform-in-list, $(RMNET_OFFLOAD_DLKM_PLATFORMS_LIST)),true)
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_PATH := $(KERNEL_MODULES_OUT)

LOCAL_MODULE := rmnet_offload.ko
LOCAL_SRC_FILES := \
	rmnet_offload_state.c \
	rmnet_offload_main.c \
	rmnet_offload_engine.c \
	rmnet_offload_tcp.c \
	rmnet_offload_udp.c \
	rmnet_offload_stats.c \
	rmnet_offload_knob.c


#path from build top to the core directory
RMNET_CORE_PATH := vendor/qcom/opensource/datarmnet/core
DLKM_DIR := $(TOP)/device/qcom/common/dlkm
#absolute path to the build directory. Can't use $(TOP) here since
#that resolves to ., and we pass this to Kbuild, where . is different
RMNET_CORE_INC_DIR := $(shell pwd)/$(RMNET_CORE_PATH)

#pass variables down to Kbuild environment
KBUILD_OPTIONS := RMNET_CORE_INC_DIR=$(RMNET_CORE_INC_DIR)
KBUILD_OPTIONS += RMNET_CORE_PATH=$(RMNET_CORE_PATH)
KBUILD_OPTIONS_GKI := RMNET_CORE_INC_DIR=$(RMNET_CORE_INC_DIR)
KBUILD_OPTIONS_GKI += RMNET_CORE_PATH=$(RMNET_CORE_PATH)/gki

#Must be built after the core rmnet module
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/$(RMNET_CORE_PATH)/rmnet_core.ko
LOCAL_ADDITIONAL_DEPENDENCIES_GKI := $(TARGET_OUT_INTERMEDIATES)/$(RMNET_CORE_PATH)/gki/rmnet_core.ko

include $(DLKM_DIR)/AndroidKernelModule.mk

endif #End of check for target
endif #End of Check for qssi target
endif #End of check for AUTO Target
