ifneq ($(TARGET_PRODUCT), qssi)
ifeq ($(call is-board-platform-in-list, $(TARGET_BOARD_PLATFORM)),true)

# Test dlkm
DLKM_DIR   := device/qcom/common/dlkm

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
# For incremental compilation
LOCAL_SRC_FILES   := $(wildcard $(LOCAL_PATH)/**/*) $(wildcard $(LOCAL_PATH)/*)
# LOCAL_SRC_FILES   := $(wildcard $(LOCAL_PATH)/msm/eva/*)
LOCAL_MODULE      := msm-eva.ko
LOCAL_MODULE_KBUILD_NAME := msm/msm-eva.ko
LOCAL_MODULE_PATH := $(KERNEL_MODULES_OUT)
# Include msm-eva.ko in the /vendor/lib/modules
# BOARD_VENDOR_KERNEL_MODULES += $(LOCAL_MODULE_PATH)/$(LOCAL_MODULE)
LOCAL_ADDITIONAL_DEPENDENCY      := synx-driver.ko
include $(DLKM_DIR)/Build_external_kernelmodule.mk

endif # End of check for board platform
endif # End of check for target product