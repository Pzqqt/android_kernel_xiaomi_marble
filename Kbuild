# SPDX-License-Identifier: GPL-2.0-only

KBUILD_CPPFLAGS += -DCONFIG_MSM_MMRM=1

ifeq ($(CONFIG_ARCH_WAIPIO), y)
include $(VIDEO_ROOT)/config/waipio_video.conf
LINUXINCLUDE    += -include $(VIDEO_ROOT)/config/waipio_video.h
endif

LINUXINCLUDE    += -I$(VIDEO_ROOT)/driver/vidc/inc \
                   -I$(VIDEO_ROOT)/include/uapi/vidc \
                   -I$(VIDEO_ROOT)/driver/platform/waipio/inc \
                   -I$(VIDEO_ROOT)/driver/variant/iris2/inc

USERINCLUDE     += -I$(VIDEO_ROOT)/include/uapi/vidc/media \
                   -I$(VIDEO_ROOT)/include/uapi/vidc

obj-m += msm_video.o

ifeq ($(CONFIG_MSM_VIDC_WAIPIO), y)
msm_video-objs += driver/platform/waipio/src/msm_vidc_waipio.o
endif

ifeq ($(CONFIG_MSM_VIDC_IRIS2), y)
msm_video-objs += driver/variant/iris2/src/msm_vidc_buffer_iris2.o \
                  driver/variant/iris2/src/msm_vidc_power_iris2.o \
                  driver/variant/iris2/src/msm_vidc_iris2.o
endif

msm_video-objs += driver/vidc/src/msm_vidc_v4l2.o \
                  driver/vidc/src/msm_vidc_vb2.o \
                  driver/vidc/src/msm_vidc.o \
                  driver/vidc/src/msm_vdec.o \
                  driver/vidc/src/msm_venc.o \
                  driver/vidc/src/msm_vidc_driver.o \
                  driver/vidc/src/msm_vidc_control.o \
                  driver/vidc/src/msm_vidc_buffer.o \
                  driver/vidc/src/msm_vidc_power.o \
                  driver/vidc/src/msm_vidc_probe.o \
                  driver/vidc/src/msm_vidc_dt.o \
                  driver/vidc/src/msm_vidc_platform.o \
                  driver/vidc/src/msm_vidc_debug.o \
                  driver/vidc/src/msm_vidc_memory.o \
                  driver/vidc/src/venus_hfi.o \
                  driver/vidc/src/hfi_packet.o \
                  driver/vidc/src/venus_hfi_response.o
