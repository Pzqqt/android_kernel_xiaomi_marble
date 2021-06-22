ifneq ($(TARGET_BOARD_PLATFORM), qssi)
ifeq ($(call is-board-platform-in-list, $(TARGET_BOARD_PLATFORM)),true)

DLKM_DIR   := device/qcom/common/dlkm

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
# For incremental compilation
LOCAL_SRC_FILES   := $(wildcard $(LOCAL_PATH)/**/*) $(wildcard $(LOCAL_PATH)/*)
LOCAL_MODULE      := msm-eva.ko
LOCAL_MODULE_KBUILD_NAME := msm/msm-eva.ko
LOCAL_MODULE_PATH := $(KERNEL_MODULES_OUT)

LOCAL_ADDITIONAL_DEPENDENCY      := synx-driver.ko

# export to kbuild
KBUILD_OPTIONS += KBUILD_EXTRA_SYMBOLS=$(shell pwd)/$(call intermediates-dir-for,DLKM,mmrm-module-symvers)/Module.symvers

LOCAL_REQUIRED_MODULES    := mmrm-module-symvers
LOCAL_ADDITIONAL_DEPENDENCIES := $(call intermediates-dir-for,DLKM,mmrm-module-symvers)/Module.symvers

# print out variables
$(info KBUILD_OPTIONS = $(KBUILD_OPTIONS))
$(info intermediates mmrm symvers path = $(call intermediates-dir-for,DLKM,mmrm-module-symvers))
$(info LOCAL_ADDITIONAL_DEPENDENCY = $(LOCAL_ADDITIONAL_DEPENDENCY))
$(info LOCAL_ADDITIONAL_DEPENDENCIES = $(LOCAL_ADDITIONAL_DEPENDENCIES))
$(info LOCAL_REQUIRED_MODULES = $(LOCAL_REQUIRED_MODULES))
$(info DLKM_DIR = $(DLKM_DIR))

include $(DLKM_DIR)/Build_external_kernelmodule.mk

endif # End of check for board platform
endif # End of check for target product
