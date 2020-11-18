# SPDX-License-Identifier: GPL-2.0-only

include $(VIDEO_ROOT)/config/waipio_video.conf

LINUXINCLUDE    += -include $(VIDEO_ROOT)/config/waipio_video.h

LINUXINCLUDE    += -I$(VIDEO_ROOT)/driver/vidc/inc \
                   -I$(VIDEO_ROOT)/include/uapi/vidc \
                   -I$(VIDEO_ROOT)/driver/platform/waipio/inc \
                   -I$(VIDEO_ROOT)/driver/variant/iris2/inc

USERINCLUDE     += -I$(VIDEO_ROOT)/include/uapi/vidc/media \
                   -I$(VIDEO_ROOT)/include/uapi/vidc

obj-m += msm_video.o

msm_video-objs += driver/vidc/src/msm_vidc_v4l2.o \
                  driver/vidc/src/msm_vidc_vb2.o \
                  driver/vidc/src/msm_vidc.o \
                  driver/vidc/src/msm_vdec.o \
                  driver/vidc/src/msm_venc.o \
                  driver/vidc/src/msm_vidc_driver.o \
                  driver/vidc/src/msm_vidc_control.o \
                  driver/vidc/src/msm_vidc_buffer.o \
                  driver/vidc/src/msm_vidc_probe.o \
                  driver/vidc/src/msm_vidc_dt.o \
                  driver/vidc/src/msm_vidc_platform.o \
                  driver/vidc/src/msm_vidc_debug.o \
                  driver/vidc/src/msm_vidc_memory.o \
                  driver/vidc/src/venus_hfi.o \
                  driver/vidc/src/hfi_packet.o \
                  driver/vidc/src/venus_hfi_response.o \
                  driver/variant/iris2/src/msm_vidc_buffer_iris2.o \
                  driver/variant/iris2/src/msm_vidc_iris2.o \
                  driver/platform/waipio/src/msm_vidc_waipio.o
