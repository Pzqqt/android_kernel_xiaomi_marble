LOCAL_PATH := $(call my-dir)
MYLOCAL_PATH := $(LOCAL_PATH)

UAPI_OUT := $(PRODUCT_OUT)/obj/vendor/qcom/opensource/display-drivers/include/display

DISPLAY_DRIVER_HEADERS := $(call all-named-files-under,*.h,drm) $(call all-named-files-under,*.h,media)

HEADER_INSTALL_DIR := kernel/msm-$(TARGET_KERNEL_VERSION)/scripts

BUILD_ROOT_RELATIVE := ../../../../../../

include $(CLEAR_VARS)
LOCAL_MODULE                  := display_driver_headers

GEN := $(addprefix $(UAPI_OUT)/,$(DISPLAY_DRIVER_HEADERS))
$(GEN): $(KERNEL_USR)
$(GEN): PRIVATE_PATH := $(MYLOCAL_PATH)
$(GEN): PRIVATE_CUSTOM_TOOL = $(shell cd $(PRODUCT_OUT)/obj/KERNEL_OBJ; $(BUILD_ROOT_RELATIVE)$(HEADER_INSTALL_DIR)/headers_install.sh $(BUILD_ROOT_RELATIVE)$(dir $@) $(BUILD_ROOT_RELATIVE)$(subst $(UAPI_OUT),$(MYLOCAL_PATH),$(dir $@)) $(notdir $@))
$(GEN): $(addprefix $(MYLOCAL_PATH)/,$(DISPLAY_DRIVER_HEADERS))
	$(transform-generated-source)

LOCAL_GENERATED_SOURCES := $(GEN)
LOCAL_EXPORT_C_INCLUDE_DIRS := $(UAPI_OUT)

include $(BUILD_HEADER_LIBRARY)
