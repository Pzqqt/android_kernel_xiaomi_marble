# Build eva kernel driver

ENABLE_EVA_KERNEL := true
ifeq ($(TARGET_USES_QMAA), true)
ifneq ($(TARGET_USES_QMAA_OVERRIDE_CVP), true)
ENABLE_EVA_KERNEL := false
endif
endif

ifeq ($(ENABLE_EVA_KERNEL), true)
ifneq ($(TARGET_BOARD_AUTO),true)
ifeq ($(call is-board-platform-in-list,$(TARGET_BOARD_PLATFORM)),true)
BOARD_VENDOR_KERNEL_MODULES += $(KERNEL_MODULES_OUT)/msm-eva.ko
endif
endif
endif