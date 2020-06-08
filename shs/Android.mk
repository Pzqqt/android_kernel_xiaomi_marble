ifneq ($(TARGET_PRODUCT),qssi)
RMNET_SHS_DLKM_PLATFORMS_LIST := lahaina

ifeq ($(call is-board-platform-in-list, $(RMNET_SHS_DLKM_PLATFORMS_LIST)),true)
#Make file to create RMNET_SHS DLKM
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_CFLAGS := -Wno-macro-redefined -Wno-unused-function -Wall -Werror
LOCAL_CLANG :=true

LOCAL_MODULE_PATH := $(KERNEL_MODULES_OUT)
LOCAL_MODULE := rmnet_shs.ko

LOCAL_SRC_FILES := \
	rmnet_shs_main.c \
	rmnet_shs_config.c \
	rmnet_shs_common.c \
	rmnet_shs_modules.c \
	rmnet_shs_wq.c \
	rmnet_shs_freq.c \
	rmnet_shs_wq_mem.c \
	rmnet_shs_wq_genl.c

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

endif #End of Check for target
endif #End of Check for qssi target
