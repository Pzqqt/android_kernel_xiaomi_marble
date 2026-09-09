#!/usr/bin/bash

yellow='\033[0;33m'
white='\033[0m'
red='\033[0;31m'
gre='\e[0;32m'

cd ${0%/*}

KDIR=$(pwd)
DEFCONFIG=marble_defconfig
IMAGE=${KDIR}/out/arch/arm64/boot/Image
OUTPUT_DIR=~/Bouquet_marble_out
KERNELSU_REPO=~/KernelSU
SUSFS_REPO=~/susfs4ksu
DEVICETREE="arch/arm64/boot/dts/vendor/qcom"

mkdir -p $OUTPUT_DIR
mkdir -p ${OUTPUT_DIR}/vendor_boot_modules
mkdir -p ${OUTPUT_DIR}/vendor_dlkm_modules
mkdir -p ${OUTPUT_DIR}/alt_kernel_modules

########## Parsing parameters ##########

use_defconfig=$DEFCONFIG
no_mkclean=false
no_ccache=false
with_ksu=false
with_susfs=false
make_target=

while [ $# != 0 ]; do
	case $1 in
		"--noclean") no_mkclean=true;;
		"--noccache") no_ccache=true;;
		"--ksu") with_ksu=true;;
		"--susfs") {
			with_ksu=true
			with_susfs=true
		};;
		"--defconfig") {
			shift
			use_defconfig=$1
		};;
		"--") {
			shift
			make_target=$*
			break
		};;
		*) {
			cat <<EOF
Usage: $0 <operate>
operate:
    --noclean               : build without run "make mrproper"
    --noccache              : build without ccache
    --ksu                   : build with KernelSU support
    --susfs                 : build with susfs support (also auto enable KernelSU support)
    --defconfig <defconfig> : use the specified defconfig (default: $DEFCONFIG)
    -- <args>               : parameters passed directly to make
EOF
			exit 1
		};;
	esac
	shift
done

########## Preparation Phase ##########

export KBUILD_BUILD_HOST="ubuntu"
export KBUILD_BUILD_USER="github"

echo -e "${gre}Building kernel with Slim LLVM 23.1.1 $white"
CLANG_PATH=~/build_toolchain/llvm-23.1.1-x86_64/bin

export PATH=$(realpath $CLANG_PATH):$(realpath ${KDIR}/build-tools):${PATH}

export LOCALVERSION=-$(git rev-parse --short HEAD)
$with_ksu && {
	while true; do
		kversion_ksu_suffix=$(cat /dev/urandom | tr -dc 'a-zA-Z' | head -c 3)
		echo $kversion_ksu_suffix | grep -qi 'ksu' || break
	done
	export LOCALVERSION="${LOCALVERSION}-${kversion_ksu_suffix}"
	unset kversion_ksu_suffix
}

make_flags="ARCH=arm64 LLVM=1 LLVM_IAS=1 O=out"
make_kcflags="-D__ANDROID_COMMON_KERNEL__ -O3"
make_kbuild_ldflags="-O3 --lto-O3"

$no_ccache && {
	echo -e "${yellow}Warning: ccache is not used! $white"
	make_flags+=" CCACHE="
}

kernel_tree_cleanup() {
	rm ${KDIR}/drivers/kernelsu/.git
	rm -rf ${KDIR}/drivers/kernelsu
	rm -f ${KDIR}/fs/susfs.c ${KDIR}/include/linux/susfs.h ${KDIR}/include/linux/susfs_def.h 2>/dev/null
	git checkout -- fs/ drivers/ include/ kernel/ security/ mm/
}

if ${with_ksu}; then
	trap "kernel_tree_cleanup; exit" 0 SIGINT SIGTERM SIGQUIT SIGHUP
fi

########## Make it ##########

if ${with_ksu}; then
	mkdir "${KDIR}/drivers/kernelsu"
	ln -s "${KERNELSU_REPO}/.git" "${KDIR}/drivers/kernelsu/.git"
	cp -r "${KERNELSU_REPO}/kernel" "${KDIR}/drivers/kernelsu/kernel"
	[ -d "${KERNELSU_REPO}/uapi" ] && cp -r "${KERNELSU_REPO}/uapi" "${KDIR}/drivers/kernelsu/uapi"
	sed -i '/drivers\/most\/Kconfig/asource\ "drivers\/kernelsu\/kernel\/Kconfig"' ${KDIR}/drivers/Kconfig
	echo 'obj-y += kernelsu/kernel/' >> ${KDIR}/drivers/Makefile

	if ${with_susfs}; then
		cp ${SUSFS_REPO}/kernel_patches/fs/*            ${KDIR}/fs/
		cp ${SUSFS_REPO}/kernel_patches/include/linux/* ${KDIR}/include/linux/

		git apply ${SUSFS_REPO}/kernel_patches/50_add_susfs_in_gki-android12-5.10.patch || exit 1
		sed -e 's|a/kernel/|a/drivers/kernelsu/kernel/|g' \
		    -e 's|b/kernel/|b/drivers/kernelsu/kernel/|g' \
		    ${SUSFS_REPO}/kernel_patches/KernelSU/10_enable_susfs_for_ksu.patch | git apply || exit 1
	fi
fi

$no_mkclean || make $make_flags KCFLAGS="$make_kcflags" KBUILD_LDFLAGS="$make_kbuild_ldflags" mrproper
make $make_flags KCFLAGS="$make_kcflags" KBUILD_LDFLAGS="$make_kbuild_ldflags" "$use_defconfig"

if [ -d "${KDIR}/${DEVICETREE}" ] && [ -d "${KDIR}/out/${DEVICETREE}" ]; then
	rm -rf "${KDIR}/out/${DEVICETREE}"
fi

if ${with_susfs}; then
	# 禁用 `UNAME_OVERRIDE` (和 susfs 的 `KSU_SUSFS_SPOOF_UNAME` 冲突)
	${KDIR}/scripts/config --file ${KDIR}/out/.config -d UNAME_OVERRIDE
fi

if [ "$(${KDIR}/scripts/config --file ${KDIR}/out/.config -s CFI_FORCE_SKIP_CHECK)" == "y" ]; then
	echo -e "${yellow}Warning: CFI checks is disabled! $white"
fi

t_start=$(date +"%s")

make $make_flags KCFLAGS="$make_kcflags" KBUILD_LDFLAGS="$make_kbuild_ldflags" -j$(nproc --all) $make_target

if [ $? != 0 ]; then
	echo -e "$red << Failed to compile, fix the errors first >>$white"
	exit 1
fi

########## Processing products ##########

if [ -f "$IMAGE" ]; then
	if ${with_susfs}; then
		image_filename=Image_susfs
	elif ${with_ksu}; then
		image_filename=Image_ksu
	else
		image_filename=Image
	fi
	cp -f "$IMAGE" ${OUTPUT_DIR}/${image_filename}
	cp -f ${KDIR}/out/vmlinux.symvers ${OUTPUT_DIR}/${image_filename}_vmlinux.symvers
	unset image_filename
fi

# vendor_boot 和 vendor_dlkm 共有且都需要替换的内核模块
both_need_modules='
crypto/lzo-rle.ko
crypto/lzo.ko
drivers/block/zram/zram.ko
drivers/bluetooth/bt_fm_slim.ko
drivers/bluetooth/btpower.ko
drivers/bus/mhi/controllers/mhi_cntrl_qcom.ko
drivers/bus/mhi/devices/mhi_dev_dtr.ko
drivers/bus/mhi/devices/mhi_dev_netdev.ko
drivers/bus/mhi/devices/mhi_dev_uci.ko
drivers/bus/mhi/host/mhi.ko
drivers/char/frpc-adsprpc.ko
drivers/char/hw_random/msm_rng.ko
drivers/clk/qcom/camcc-diwali.ko
drivers/clk/qcom/camcc-waipio.ko
drivers/clk/qcom/debugcc-diwali.ko
drivers/clk/qcom/debugcc-waipio.ko
drivers/clk/qcom/gpucc-diwali.ko
drivers/clk/qcom/gpucc-waipio.ko
drivers/clk/qcom/videocc-diwali.ko
drivers/clk/qcom/videocc-waipio.ko
drivers/cpuidle/governors/qcom_lpm.ko
drivers/crypto/msm/qce50.ko
drivers/crypto/msm/qcedev-mod.ko
drivers/dma/qcom/bam_dma.ko
drivers/dma/qcom/gpi.ko
drivers/dma/qcom/pci-edma.ko
drivers/firmware/arm_scmi/gplaf_vendor.ko
drivers/firmware/arm_scmi/plh_vendor.ko
drivers/firmware/arm_scmi/shared_rail_vendor.ko
drivers/firmware/qcom/tz_log.ko
drivers/gpu/msm/msm_kgsl.ko
drivers/hwmon/hwmon.ko
drivers/hwmon/qti_amoled_ecm.ko
drivers/i2c/busses/i2c-msm-geni.ko
drivers/i3c/master/i3c-master-msm-geni.ko
drivers/iio/adc/qcom-spmi-adc5.ko
drivers/iio/adc/qcom-vadc-common.ko
drivers/input/fingerprint/fpc_1540/fpc1540.ko
drivers/input/fingerprint/goodix_3626/goodix_3626.ko
drivers/input/fingerprint/xiaomi_fp/xiaomifp.ko
drivers/input/touchscreen/goodix_berlin_driver/goodix_core.ko
drivers/input/touchscreen/xiaomi_los/xiaomi_touch.ko
drivers/input/misc/pm8941-pwrkey.ko
drivers/input/misc/qcom-hv-haptics.ko
drivers/interconnect/icc-test.ko
drivers/interconnect/qcom/qnoc-parrot.ko
drivers/irqchip/msm_show_resume_irq.ko
drivers/leds/leds-qpnp-vibrator-ldo.ko
drivers/leds/leds-qti-flash.ko
drivers/leds/leds-qti-tri-led.ko
drivers/media/platform/msm/synx/qcom_ipc_lite.ko
drivers/media/platform/msm/synx/synx-driver.ko
drivers/media/radio/rtc6226/radio-i2c-rtc6226-qca.ko
drivers/media/rc/ir-spi.ko
drivers/mfd/qcom-i2c-pmic.ko
drivers/misc/hdcp/hdcp.ko
drivers/misc/qseecom-mod.ko
drivers/mmc/host/sdhci-msm.ko
drivers/mtd/chips/chipreg.ko
drivers/mtd/devices/block2mtd.ko
drivers/mtd/mtd.ko
drivers/mtd/mtd_blkdevs.ko
drivers/mtd/mtdblock.ko
drivers/mtd/mtdoops.ko
drivers/mtd/parsers/ofpart.ko
drivers/net/wireless/cnss2/cnss2.ko
drivers/net/wireless/cnss_genl/cnss_nl.ko
drivers/net/wireless/cnss_prealloc/cnss_prealloc.ko
drivers/net/wireless/cnss_utils/cnss_plat_ipc_qmi_svc.ko
drivers/net/wireless/cnss_utils/cnss_utils.ko
drivers/net/wireless/cnss_utils/wlan_firmware_service.ko
drivers/nvmem/nvmem_qfprom.ko
drivers/pci/controller/pci-msm-drv.ko
drivers/phy/qualcomm/phy-qcom-ufs-qmp-14nm.ko
drivers/phy/qualcomm/phy-qcom-ufs-qmp-v3.ko
drivers/phy/qualcomm/phy-qcom-ufs-qmp-v4-anarok.ko
drivers/phy/qualcomm/phy-qcom-ufs-qmp-v4-lahaina.ko
drivers/phy/qualcomm/phy-qcom-ufs-qmp-v4-parrot.ko
drivers/pinctrl/qcom/pinctrl-spmi-gpio.ko
drivers/pinctrl/qcom/pinctrl-spmi-mpp.ko
drivers/platform/msm/ep_pcie/ep_pcie_drv.ko
drivers/platform/msm/ipa_fmwk/ipa_fmwk.ko
drivers/platform/msm/mhi_dev/mhi_dev_drv.ko
drivers/platform/msm/mhi_dev/mhi_dev_net.ko
drivers/platform/msm/sps/sps_drv.ko
drivers/platform/msm/usb_bam.ko
drivers/power/reset/qcom-pon.ko
drivers/power/supply/qti_battery_charger_main.ko
drivers/pwm/pwm-qti-lpg.ko
drivers/regulator/qcom_pm8008-regulator.ko
drivers/regulator/qpnp-amoled-regulator.ko
drivers/remoteproc/qcom_esoc.ko
drivers/remoteproc/qcom_pil_info.ko
drivers/remoteproc/qcom_q6v5.ko
drivers/remoteproc/qcom_q6v5_pas.ko
drivers/remoteproc/qcom_sysmon.ko
drivers/remoteproc/rproc_qcom_common.ko
drivers/rpmsg/qcom_glink.ko
drivers/rpmsg/qcom_glink_smem.ko
drivers/rpmsg/qcom_smd.ko
drivers/scsi/sg.ko
drivers/slimbus/slim-qcom-ngd-ctrl.ko
drivers/slimbus/slimbus.ko
drivers/soc/qcom/adsp_sleepmon.ko
drivers/soc/qcom/altmode-glink.ko
drivers/soc/qcom/boot_stats.ko
drivers/soc/qcom/cdsprm.ko
drivers/soc/qcom/charger-ulog-glink.ko
drivers/soc/qcom/core_hang_detect.ko
drivers/soc/qcom/dcc_v2.ko
drivers/soc/qcom/dcvs/memlat.ko
drivers/soc/qcom/eud.ko
drivers/soc/qcom/fsa4480-i2c.ko
drivers/soc/qcom/glink_pkt.ko
drivers/soc/qcom/glink_probe.ko
drivers/soc/qcom/gplaf_scmi.ko
drivers/soc/qcom/guestvm_loader.ko
drivers/soc/qcom/hung_task_enh.ko
drivers/soc/qcom/hyp_core_ctl.ko
drivers/soc/qcom/llcc_perfmon.ko
drivers/soc/qcom/mdt_loader.ko
drivers/soc/qcom/memshare/heap_mem_ext_v01.ko
drivers/soc/qcom/memshare/msm_memshare.ko
drivers/soc/qcom/microdump_collector.ko
drivers/soc/qcom/msm_ext_display.ko
drivers/soc/qcom/msm_performance.ko
drivers/soc/qcom/msm_show_epoch.ko
drivers/soc/qcom/panel_event_notifier.ko
drivers/soc/qcom/pdr_interface.ko
drivers/soc/qcom/plh_scmi.ko
drivers/soc/qcom/pmic-pon-log.ko
drivers/soc/qcom/pmic_glink.ko
drivers/soc/qcom/qbt_handler.ko
drivers/soc/qcom/qcom_cpuss_sleep_stats.ko
drivers/soc/qcom/qcom_logbuf_vendor_hooks.ko
drivers/soc/qcom/qcom_ramdump.ko
drivers/soc/qcom/qcom_soc_wdt.ko
drivers/soc/qcom/qdsp6v2/cdsp-loader.ko
drivers/soc/qcom/qdss_bridge.ko
drivers/soc/qcom/qfprom-sys.ko
drivers/soc/qcom/qmi_helpers.ko
drivers/soc/qcom/qpnp-pbs.ko
drivers/soc/qcom/qsee_ipc_irq_bridge.ko
drivers/soc/qcom/rimps_log.ko
drivers/soc/qcom/rq_stats.ko
drivers/soc/qcom/sensors_ssc.ko
drivers/soc/qcom/shared_rail_scmi.ko
drivers/soc/qcom/smcinvoke_mod.ko
drivers/soc/qcom/smp2p.ko
drivers/soc/qcom/smp2p_sleepstate.ko
drivers/soc/qcom/soc_sleep_stats.ko
drivers/soc/qcom/subsystem_sleep_stats.ko
drivers/soc/qcom/sync_fence/qcom_sync_file.ko
drivers/soc/qcom/sys_pm_vx.ko
drivers/soc/qcom/sysmon_subsystem_stats.ko
drivers/spi/spi-msm-geni.ko
drivers/thermal/mi_thermal_interface.ko
drivers/thermal/qcom/bcl_soc.ko
drivers/thermal/qcom/cpu_voltage_cooling.ko
drivers/thermal/qcom/ddr_cdev.ko
drivers/thermal/qcom/msm_lmh_dcvs.ko
drivers/thermal/qcom/policy_engine.ko
drivers/thermal/qcom/qcom-spmi-temp-alarm.ko
drivers/thermal/qcom/qti-adc-tm.ko
drivers/thermal/qcom/qti_cpufreq_cdev.ko
drivers/thermal/qcom/qti_devfreq_cdev.ko
drivers/thermal/qcom/qti_qmi_cdev.ko
drivers/thermal/qcom/qti_qmi_sensor_v2.ko
drivers/thermal/qcom/qti_userspace_cdev.ko
drivers/thermal/qcom/sdpm_clk.ko
drivers/tty/hvc/hvc_gunyah.ko
drivers/uio/msm_sharedmem/msm_sharedmem.ko
drivers/usb/dwc3/dwc3-msm.ko
drivers/usb/gadget/function/usb_f_ccid.ko
drivers/usb/gadget/function/usb_f_cdev.ko
drivers/usb/gadget/function/usb_f_diag.ko
drivers/usb/gadget/function/usb_f_gsi.ko
drivers/usb/gadget/function/usb_f_qdss.ko
drivers/usb/phy/phy-msm-snps-eusb2.ko
drivers/usb/phy/phy-msm-snps-hs.ko
drivers/usb/phy/phy-msm-ssusb-qmp.ko
drivers/usb/phy/phy-qcom-emu.ko
drivers/usb/redriver/nb7vpq904m.ko
drivers/usb/redriver/redriver.ko
drivers/usb/repeater/repeater-i2c-eusb2.ko
drivers/usb/repeater/repeater.ko
drivers/usb/typec/ucsi/ucsi_glink.ko
drivers/virt/gunyah/gh_irq_lend.ko
drivers/virt/gunyah/gh_mem_notifier.ko
drivers/virt/gunyah/gh_virtio_backend.ko
kernel/msm_sysstats.ko
mm/zsmalloc.ko
net/qrtr/qrtr-gunyah.ko
net/qrtr/qrtr-mhi.ko
net/qrtr/qrtr-smd.ko
sound/usb/snd-usb-audio-qmi.ko
techpack/display/msm/msm_drm.ko
techpack/mmrm/driver/msm-mmrm.ko
'

# 额外需要在 vendor_boot 分区替换的内核模块
vendor_boot_need_modules='
arch/arm64/gunyah/gh_arm_drv.ko
drivers/base/regmap/qti-regmap-debugfs.ko
drivers/base/regmap/regmap-spmi.ko
drivers/clk/qcom/clk-dummy.ko
drivers/clk/qcom/clk-qcom.ko
drivers/clk/qcom/clk-rpmh.ko
drivers/clk/qcom/dispcc-diwali.ko
drivers/clk/qcom/dispcc-waipio.ko
drivers/clk/qcom/gcc-diwali.ko
drivers/clk/qcom/gcc-waipio.ko
drivers/clk/qcom/gdsc-regulator.ko
drivers/cpufreq/qcom-cpufreq-hw.ko
drivers/dma-buf/heaps/qcom_dma_heaps.ko
drivers/firmware/arm_scmi/c1dcvs_vendor.ko
drivers/firmware/arm_scmi/pmu_vendor.ko
drivers/firmware/qcom-scm.ko
drivers/hwspinlock/qcom_hwspinlock.ko
drivers/interconnect/qcom/icc-bcm-voter.ko
drivers/interconnect/qcom/icc-debug.ko
drivers/interconnect/qcom/icc-rpmh.ko
drivers/interconnect/qcom/qnoc-diwali.ko
drivers/interconnect/qcom/qnoc-qos.ko
drivers/interconnect/qcom/qnoc-waipio.ko
drivers/iommu/arm/arm-smmu/arm_smmu.ko
drivers/iommu/iommu-logger.ko
drivers/iommu/msm_dma_iommu_mapping.ko
drivers/iommu/qcom_iommu_util.ko
drivers/irqchip/qcom-pdc.ko
drivers/mailbox/msm_qmp.ko
drivers/mfd/qcom-spmi-pmic.ko
drivers/misc/hwid/hwid.ko
drivers/mmc/host/cqhci.ko
drivers/nfc/qti/nfc_i2c.ko
drivers/nvmem/nvmem_qcom-spmi-sdam.ko
drivers/perf/qcom_llcc_pmu.ko
drivers/phy/qualcomm/phy-qcom-ufs-qmp-v4-cape.ko
drivers/phy/qualcomm/phy-qcom-ufs-qmp-v4-diwali.ko
drivers/phy/qualcomm/phy-qcom-ufs-qmp-v4-waipio.ko
drivers/phy/qualcomm/phy-qcom-ufs.ko
drivers/pinctrl/qcom/pinctrl-cape.ko
drivers/pinctrl/qcom/pinctrl-diwali.ko
drivers/pinctrl/qcom/pinctrl-msm.ko
drivers/pinctrl/qcom/pinctrl-waipio.ko
drivers/platform/msm/msm-geni-se.ko
drivers/power/reset/qcom-dload-mode.ko
drivers/power/reset/qcom-reboot-reason.ko
drivers/power/reset/reboot-mode.ko
drivers/regulator/debug-regulator.ko
drivers/regulator/proxy-consumer.ko
drivers/regulator/qti-fixed-regulator.ko
drivers/regulator/rpmh-regulator.ko
drivers/regulator/stub-regulator.ko
drivers/rtc/rtc-pm8xxx.ko
drivers/scsi/ufs/ufs_qcom.ko
drivers/scsi/ufs/ufshcd-crypto-qti.ko
drivers/soc/qcom/cmd-db.ko
drivers/soc/qcom/crypto-qti-common.ko
drivers/soc/qcom/crypto-qti-hwkm.ko
drivers/soc/qcom/dcvs/bwmon.ko
drivers/soc/qcom/dcvs/c1dcvs_scmi.ko
drivers/soc/qcom/dcvs/dcvs_fp.ko
drivers/soc/qcom/dcvs/pmu_scmi.ko
drivers/soc/qcom/dcvs/qcom-dcvs.ko
drivers/soc/qcom/dcvs/qcom-pmu-lib.ko
drivers/soc/qcom/hwkm.ko
drivers/soc/qcom/llcc-qcom.ko
drivers/soc/qcom/mem-hooks.ko
drivers/soc/qcom/mem_buf/mem_buf.ko
drivers/soc/qcom/mem_buf/mem_buf_dev.ko
drivers/soc/qcom/memory_dump_v2.ko
drivers/soc/qcom/minidump.ko
drivers/soc/qcom/qcom_aoss.ko
drivers/soc/qcom/qcom_cpu_vendor_hooks.ko
drivers/soc/qcom/qcom_gic_intr_routing.ko
drivers/soc/qcom/qcom_ipcc.ko
drivers/soc/qcom/qcom_rimps.ko
drivers/soc/qcom/qcom_rpmh.ko
drivers/soc/qcom/qcom_wdt_core.ko
drivers/soc/qcom/secure_buffer.ko
drivers/soc/qcom/smem.ko
drivers/soc/qcom/socinfo.ko
drivers/soc/qcom/tmecom/tmecom-intf.ko
drivers/spmi/spmi-pmic-arb.ko
drivers/staging/kshrink_slabd/kshrink_slabd.ko
drivers/staging/kshrink_lruvecd/kshrink_lruvecd.ko
drivers/thermal/qcom/bcl_pmic5.ko
drivers/thermal/qcom/cpu_hotplug.ko
drivers/thermal/qcom/qcom_tsens.ko
drivers/thermal/qcom/thermal_pause.ko
drivers/tty/serial/msm_geni_serial.ko
drivers/usb/phy/phy-generic.ko
drivers/virt/gunyah/gh_ctrl.ko
drivers/virt/gunyah/gh_dbl.ko
drivers/virt/gunyah/gh_msgq.ko
drivers/virt/gunyah/gh_rm_drv.ko
drivers/virt/gunyah/gh_virt_wdt.ko
kernel/sched/walt/sched-walt.ko
kernel/trace/qcom_ipc_logging.ko
net/qrtr/qrtr.ko
techpack/bootinfo/bootinfo.ko
'

# 额外需要在 vendor_dlkm 分区替换的内核模块
vendor_dlkm_need_modules='
drivers/soc/qcom/vh_fs/vh_fs.ko
drivers/staging/qcacld-3.0/qca6490.ko
net/mac80211/mac80211.ko
net/wireless/cfg80211.ko
techpack/audio/asoc/codecs/aw882xx/aw882xx_dlkm.ko
techpack/audio/asoc/codecs/hdmi_dlkm.ko
techpack/audio/asoc/codecs/lpass-cdc/lpass_cdc_dlkm.ko
techpack/audio/asoc/codecs/lpass-cdc/lpass_cdc_rx_macro_dlkm.ko
techpack/audio/asoc/codecs/lpass-cdc/lpass_cdc_tx_macro_dlkm.ko
techpack/audio/asoc/codecs/lpass-cdc/lpass_cdc_va_macro_dlkm.ko
techpack/audio/asoc/codecs/lpass-cdc/lpass_cdc_wsa2_macro_dlkm.ko
techpack/audio/asoc/codecs/lpass-cdc/lpass_cdc_wsa_macro_dlkm.ko
techpack/audio/asoc/codecs/mbhc_dlkm.ko
techpack/audio/asoc/codecs/stub_dlkm.ko
techpack/audio/asoc/codecs/swr_dmic_dlkm.ko
techpack/audio/asoc/codecs/swr_haptics_dlkm.ko
techpack/audio/asoc/codecs/wcd937x/wcd937x_dlkm.ko
techpack/audio/asoc/codecs/wcd937x/wcd937x_slave_dlkm.ko
techpack/audio/asoc/codecs/wcd938x/wcd938x_dlkm.ko
techpack/audio/asoc/codecs/wcd938x/wcd938x_slave_dlkm.ko
techpack/audio/asoc/codecs/wcd9xxx_dlkm.ko
techpack/audio/asoc/codecs/wcd_core_dlkm.ko
techpack/audio/asoc/codecs/wsa881x_dlkm.ko
techpack/audio/asoc/codecs/wsa883x/wsa883x_dlkm.ko
techpack/audio/asoc/machine_dlkm.ko
techpack/audio/dsp/adsp_loader_dlkm.ko
techpack/audio/dsp/audio_prm_dlkm.ko
techpack/audio/dsp/audpkt_ion_dlkm.ko
techpack/audio/dsp/q6_dlkm.ko
techpack/audio/dsp/q6_notifier_dlkm.ko
techpack/audio/dsp/q6_pdr_dlkm.ko
techpack/audio/dsp/spf_core_dlkm.ko
techpack/audio/ipc/audio_pkt_dlkm.ko
techpack/audio/ipc/gpr_dlkm.ko
techpack/audio/soc/pinctrl_lpi_dlkm.ko
techpack/audio/soc/snd_event_dlkm.ko
techpack/audio/soc/swr_ctrl_dlkm.ko
techpack/audio/soc/swr_dlkm.ko
techpack/camera/camera.ko
techpack/cvp/msm/msm-cvp.ko
techpack/dataipa/drivers/platform/msm/gsi/gsim.ko
techpack/dataipa/drivers/platform/msm/ipa/ipa_clients/ipa_clientsm.ko
techpack/dataipa/drivers/platform/msm/ipa/ipa_clients/rndisipam.ko
techpack/dataipa/drivers/platform/msm/ipa/ipam.ko
techpack/dataipa/drivers/platform/msm/ipa/ipanetm.ko
techpack/datarmnet-ext/aps/rmnet_aps.ko
techpack/datarmnet-ext/offload/rmnet_offload.ko
techpack/datarmnet-ext/perf/rmnet_perf.ko
techpack/datarmnet-ext/perf_tether/rmnet_perf_tether.ko
techpack/datarmnet-ext/sch/rmnet_sch.ko
techpack/datarmnet-ext/shs/rmnet_shs.ko
techpack/datarmnet-ext/wlan/rmnet_wlan.ko
techpack/datarmnet/core/rmnet_core.ko
techpack/datarmnet/core/rmnet_ctl.ko
techpack/eva/msm/msm-eva.ko
techpack/video/msm_video.ko
'

# 备用
alt_need_modules='
drivers/misc/ntsync.ko
'

rm ${OUTPUT_DIR}/*.ko 2>/dev/null
rm ${OUTPUT_DIR}/vendor_boot_modules/*.ko 2>/dev/null
rm ${OUTPUT_DIR}/vendor_dlkm_modules/*.ko 2>/dev/null
rm ${OUTPUT_DIR}/alt_kernel_modules/*.ko 2>/dev/null

strip_kmod() {
	local module=$1
	local output_dir=$2
	local module_file_name

	[ -n "$module" ] || return 1
	if [ ! -f "$module" ]; then
		echo -e "${yellow}! ${module} not found! ${white}"
		return 1
	fi

	module_file_name=$(basename $module)
	case "$module_file_name" in
		"qca6490.ko") module_file_name="qca_cld3_qca6490.ko";;
	esac

	echo "- Striping $module_file_name ..."
	llvm-strip -S "$module" -o ${output_dir}/${module_file_name}
}

for module in $both_need_modules; do
	strip_kmod "${KDIR}/out/$module" "${OUTPUT_DIR}/vendor_boot_modules"
done


cp ${OUTPUT_DIR}/vendor_boot_modules/* ${OUTPUT_DIR}/vendor_dlkm_modules

for module in $vendor_boot_need_modules; do
	strip_kmod "${KDIR}/out/$module" "${OUTPUT_DIR}/vendor_boot_modules"
done
for module in $vendor_dlkm_need_modules; do
	strip_kmod "${KDIR}/out/$module" "${OUTPUT_DIR}/vendor_dlkm_modules"
done
for module in $alt_need_modules; do
	strip_kmod "${KDIR}/out/$module" "${OUTPUT_DIR}/alt_kernel_modules"
done

t_end=$(date +"%s")
t_diff=$(($t_end - $t_start))

echo -e "$gre << Build completed in $(($t_diff / 60)) minutes and $(($t_diff % 60)) seconds >> \n $white"

if [ -d ${KDIR}/${DEVICETREE} ] && [ -d ${KDIR}/out/${DEVICETREE} ]; then
	mkdir -p /tmp/devicetree_base
	mkdir -p /tmp/devicetree_techpack
	mkdir -p ${OUTPUT_DIR}/devicetree
	rm ${OUTPUT_DIR}/devicetree/* 2>/dev/null

	# Only keep marble's
	cp ${KDIR}/out/${DEVICETREE}/ukee.dtb /tmp/devicetree_base/
	cp ${KDIR}/out/${DEVICETREE}/marble-sm7475-pm8008-overlay.dtbo /tmp/devicetree_base/

	for d in \
	    ${KDIR}/out/${DEVICETREE}/audio \
	    ${KDIR}/out/${DEVICETREE}/camera \
	    ${KDIR}/out/${DEVICETREE}/cvp \
	    ${KDIR}/out/${DEVICETREE}/display/display \
	    ${KDIR}/out/${DEVICETREE}/eva \
	    ${KDIR}/out/${DEVICETREE}/mmrm \
	    ${KDIR}/out/${DEVICETREE}/video; do
		mkdir -p /tmp/devicetree_techpack/$(basename $d)
		cp ${d}/*.dtbo /tmp/devicetree_techpack/$(basename $d)/
	done

	echo ""
	echo "Merging dtbs & dtbos..."
	merge_dtbs.py -b /tmp/devicetree_base -t /tmp/devicetree_techpack -o ${OUTPUT_DIR}/devicetree

	echo ""
	echo "- Making dtbo.img ..."
	mkdtboimg.py create ${OUTPUT_DIR}/devicetree/dtbo.img ${OUTPUT_DIR}/devicetree/marble-sm7475-pm8008-overlay.dtbo
	avbtool add_hash_footer --partition_name dtbo --partition_size $((24 * 1024 * 1024)) --image ${OUTPUT_DIR}/devicetree/dtbo.img

	rm -rf /tmp/devicetree_base
	rm -rf /tmp/devicetree_techpack
fi

echo ""
echo "- Done!"
