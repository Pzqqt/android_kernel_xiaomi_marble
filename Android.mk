# Android makefile for display kernel modules
MY_LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_ADDITIONAL_DEPENDENCIES := $(wildcard $(LOCAL_PATH)/**/*) $(wildcard $(LOCAL_PATH)/*)

UAPI_OUT := $(PRODUCT_OUT)/obj/vendor/qcom/opensource/display-drivers/include

$(shell mkdir -p $(UAPI_OUT)/display;)

include $(MY_LOCAL_PATH)/include/uapi/display/Android.mk
include $(MY_LOCAL_PATH)/msm/Android.mk
