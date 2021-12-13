# Build eva kernel driver
ifneq ($(TARGET_BOARD_AUTO),true)
ifeq ($(call is-board-platform-in-list,$(TARGET_BOARD_PLATFORM)),true)
ifneq ($(call is-board-platform-in-list,parrot),true)
BOARD_VENDOR_KERNEL_MODULES += $(KERNEL_MODULES_OUT)/msm-cvp.ko
endif
endif
endif