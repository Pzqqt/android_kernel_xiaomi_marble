ENABLE_EVA_KERNEL := true
ifeq ($(TARGET_USES_QMAA), true)
ifneq ($(TARGET_USES_QMAA_OVERRIDE_CVP), true)
ENABLE_EVA_KERNEL := false
endif
endif

ifeq ($(ENABLE_EVA_KERNEL), true)
ifneq ($(call is-board-platform-in-list,parrot),true)
PRODUCT_PACKAGES += msm-eva.ko
endif
endif