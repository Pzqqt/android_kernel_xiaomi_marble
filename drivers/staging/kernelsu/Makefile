obj-y += ksu.o
obj-y += allowlist.o
kernelsu-objs := apk_sign.o
obj-y += kernelsu.o
obj-y += module_api.o
obj-y += sucompat.o
obj-y += uid_observer.o
obj-y += manager.o
obj-y += core_hook.o
obj-y += ksud.o
obj-y += embed_ksud.o
obj-y += kernel_compat.o

obj-y += selinux/

KSU_GIT_VERSION := 1038
# ksu_version: major * 10000 + git version + 200 for historical reasons
$(eval KSU_VERSION=$(shell expr 10000 + $(KSU_GIT_VERSION) + 200))
$(info -- KernelSU version: $(KSU_VERSION))
ccflags-y += -DKSU_VERSION=$(KSU_VERSION)

ifndef KSU_EXPECTED_SIZE
KSU_EXPECTED_SIZE := 0x033b
endif

ifndef KSU_EXPECTED_HASH
KSU_EXPECTED_HASH := 0xb0b91415
endif

$(info -- KernelSU Manager signature size: $(KSU_EXPECTED_SIZE))
$(info -- KernelSU Manager signature hash: $(KSU_EXPECTED_HASH))

ccflags-y += -DEXPECTED_SIZE=$(KSU_EXPECTED_SIZE)
ccflags-y += -DEXPECTED_HASH=$(KSU_EXPECTED_HASH)
ccflags-y += -Wno-implicit-function-declaration -Wno-strict-prototypes -Wno-int-conversion -Wno-gcc-compat
ccflags-y += -Wno-declaration-after-statement
