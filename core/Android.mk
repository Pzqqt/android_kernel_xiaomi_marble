ifneq ($(TARGET_BOARD_PLATFORM),qssi)
RMNET_CORE_DLKM_PLATFORMS_LIST := lahaina
RMNET_CORE_DLKM_PLATFORMS_LIST += taro
RMNET_CORE_DLKM_PLATFORMS_LIST += sa2150p

ifeq ($(call is-board-platform-in-list, $(RMNET_CORE_DLKM_PLATFORMS_LIST)),true)
#Make file to create RMNET_CORE DLKM
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_CFLAGS := -Wno-macro-redefined -Wno-unused-function -Wall -Werror
LOCAL_CLANG :=true
LOCAL_MODULE_PATH := $(KERNEL_MODULES_OUT)
LOCAL_MODULE := rmnet_core.ko
LOCAL_SRC_FILES   := $(wildcard $(LOCAL_PATH)/**/*) $(wildcard $(LOCAL_PATH)/*)
DLKM_DIR := $(TOP)/device/qcom/common/dlkm
$(warning $(DLKM_DIR))
include $(DLKM_DIR)/Build_external_kernelmodule.mk

######## Create RMNET_CTL DLKM ########
include $(CLEAR_VARS)

LOCAL_CFLAGS := -Wno-macro-redefined -Wno-unused-function -Wall -Werror
LOCAL_CLANG :=true
LOCAL_MODULE_PATH := $(KERNEL_MODULES_OUT)
LOCAL_MODULE := rmnet_ctl.ko
LOCAL_SRC_FILES   := $(wildcard $(LOCAL_PATH)/**/*) $(wildcard $(LOCAL_PATH)/*)
DLKM_DIR := $(TOP)/device/qcom/common/dlkm
$(warning $(DLKM_DIR))
include $(DLKM_DIR)/Build_external_kernelmodule.mk

endif #End of Check for target
endif #End of Check for qssi target
