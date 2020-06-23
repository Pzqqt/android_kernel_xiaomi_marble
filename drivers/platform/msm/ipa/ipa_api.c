// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2015-2020, The Linux Foundation. All rights reserved.
 */

#include <linux/ipa.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/ipa_uc_offload.h>
#include <linux/pci.h>
#include "ipa_api.h"
#include "ipa_v3/ipa_i.h"

/*
 * The following for adding code (ie. for EMULATION) not found on x86.
 */
#if defined(CONFIG_IPA_EMULATION)
# include "ipa_v3/ipa_emulation_stubs.h"
#endif

#define DRV_NAME "ipa"

#define IPA_API_DISPATCH_RETURN(api, p...) \
	do { \
		if (!ipa_api_ctrl) { \
			pr_err("%s:%d IPA HW is not supported\n", \
				__func__, __LINE__); \
			ret = -EPERM; \
		} \
		else { \
			if (ipa_api_ctrl->api) { \
				ret = ipa_api_ctrl->api(p); \
			} else { \
				WARN(1, \
					"%s not implemented for IPA ver %d\n", \
						__func__, ipa_api_hw_type); \
				ret = -EPERM; \
			} \
		} \
	} while (0)

#define IPA_API_DISPATCH(api, p...) \
	do { \
		if (!ipa_api_ctrl) \
			pr_err("%s:%d IPA HW is not supported\n", \
				__func__, __LINE__); \
		else { \
			if (ipa_api_ctrl->api) { \
				ipa_api_ctrl->api(p); \
			} else { \
				WARN(1, \
					"%s not implemented for IPA ver %d\n",\
						__func__, ipa_api_hw_type); \
			} \
		} \
	} while (0)

#define IPA_API_DISPATCH_RETURN_PTR(api, p...) \
	do { \
		if (!ipa_api_ctrl) { \
			pr_err("%s:%d IPA HW is not supported\n", \
				__func__, __LINE__); \
			ret = NULL; \
		} \
		else { \
			if (ipa_api_ctrl->api) { \
				ret = ipa_api_ctrl->api(p); \
			} else { \
				WARN(1, "%s not implemented for IPA ver %d\n",\
						__func__, ipa_api_hw_type); \
				ret = NULL; \
			} \
		} \
	} while (0)

#define IPA_API_DISPATCH_RETURN_BOOL(api, p...) \
	do { \
		if (!ipa_api_ctrl) { \
			pr_err("%s:%d IPA HW is not supported\n", \
				__func__, __LINE__); \
			ret = false; \
		} \
		else { \
			if (ipa_api_ctrl->api) { \
				ret = ipa_api_ctrl->api(p); \
			} else { \
				WARN(1, "%s not implemented for IPA ver %d\n",\
						__func__, ipa_api_hw_type); \
				ret = false; \
			} \
		} \
	} while (0)

#if defined(CONFIG_IPA_EMULATION)
static bool running_emulation = true;
#else
static bool running_emulation;
#endif

static enum ipa_hw_type ipa_api_hw_type;
static struct ipa_api_controller *ipa_api_ctrl;

const char *ipa_clients_strings[IPA_CLIENT_MAX] = {
	__stringify(IPA_CLIENT_HSIC1_PROD),
	__stringify(IPA_CLIENT_HSIC1_CONS),
	__stringify(IPA_CLIENT_HSIC2_PROD),
	__stringify(IPA_CLIENT_HSIC2_CONS),
	__stringify(IPA_CLIENT_HSIC3_PROD),
	__stringify(IPA_CLIENT_HSIC3_CONS),
	__stringify(IPA_CLIENT_HSIC4_PROD),
	__stringify(IPA_CLIENT_HSIC4_CONS),
	__stringify(IPA_CLIENT_HSIC5_PROD),
	__stringify(IPA_CLIENT_HSIC5_CONS),
	__stringify(IPA_CLIENT_WLAN1_PROD),
	__stringify(IPA_CLIENT_WLAN1_CONS),
	__stringify(IPA_CLIENT_WLAN2_PROD),
	__stringify(IPA_CLIENT_WLAN2_CONS),
	__stringify(RESERVED_PROD_14),
	__stringify(IPA_CLIENT_WLAN3_CONS),
	__stringify(RESERVED_PROD_16),
	__stringify(IPA_CLIENT_WLAN4_CONS),
	__stringify(IPA_CLIENT_USB_PROD),
	__stringify(IPA_CLIENT_USB_CONS),
	__stringify(IPA_CLIENT_USB2_PROD),
	__stringify(IPA_CLIENT_USB2_CONS),
	__stringify(IPA_CLIENT_USB3_PROD),
	__stringify(IPA_CLIENT_USB3_CONS),
	__stringify(IPA_CLIENT_USB4_PROD),
	__stringify(IPA_CLIENT_USB4_CONS),
	__stringify(IPA_CLIENT_UC_USB_PROD),
	__stringify(IPA_CLIENT_USB_DPL_CONS),
	__stringify(IPA_CLIENT_A2_EMBEDDED_PROD),
	__stringify(IPA_CLIENT_A2_EMBEDDED_CONS),
	__stringify(IPA_CLIENT_A2_TETHERED_PROD),
	__stringify(IPA_CLIENT_A2_TETHERED_CONS),
	__stringify(IPA_CLIENT_APPS_LAN_PROD),
	__stringify(IPA_CLIENT_APPS_LAN_CONS),
	__stringify(IPA_CLIENT_APPS_WAN_PROD),
	__stringify(IPA_CLIENT_APPS_WAN_CONS),
	__stringify(IPA_CLIENT_APPS_CMD_PROD),
	__stringify(IPA_CLIENT_A5_LAN_WAN_CONS),
	__stringify(IPA_CLIENT_ODU_PROD),
	__stringify(IPA_CLIENT_ODU_EMB_CONS),
	__stringify(RESERVED_PROD_40),
	__stringify(IPA_CLIENT_ODU_TETH_CONS),
	__stringify(IPA_CLIENT_MHI_PROD),
	__stringify(IPA_CLIENT_MHI_CONS),
	__stringify(IPA_CLIENT_MEMCPY_DMA_SYNC_PROD),
	__stringify(IPA_CLIENT_MEMCPY_DMA_SYNC_CONS),
	__stringify(IPA_CLIENT_MEMCPY_DMA_ASYNC_PROD),
	__stringify(IPA_CLIENT_MEMCPY_DMA_ASYNC_CONS),
	__stringify(IPA_CLIENT_ETHERNET_PROD),
	__stringify(IPA_CLIENT_ETHERNET_CONS),
	__stringify(IPA_CLIENT_Q6_LAN_PROD),
	__stringify(IPA_CLIENT_Q6_LAN_CONS),
	__stringify(IPA_CLIENT_Q6_WAN_PROD),
	__stringify(IPA_CLIENT_Q6_WAN_CONS),
	__stringify(IPA_CLIENT_Q6_CMD_PROD),
	__stringify(IPA_CLIENT_Q6_DUN_CONS),
	__stringify(IPA_CLIENT_Q6_DECOMP_PROD),
	__stringify(IPA_CLIENT_Q6_DECOMP_CONS),
	__stringify(IPA_CLIENT_Q6_DECOMP2_PROD),
	__stringify(IPA_CLIENT_Q6_DECOMP2_CONS),
	__stringify(RESERVED_PROD_60),
	__stringify(IPA_CLIENT_Q6_LTE_WIFI_AGGR_CONS),
	__stringify(IPA_CLIENT_TEST_PROD),
	__stringify(IPA_CLIENT_TEST_CONS),
	__stringify(IPA_CLIENT_TEST1_PROD),
	__stringify(IPA_CLIENT_TEST1_CONS),
	__stringify(IPA_CLIENT_TEST2_PROD),
	__stringify(IPA_CLIENT_TEST2_CONS),
	__stringify(IPA_CLIENT_TEST3_PROD),
	__stringify(IPA_CLIENT_TEST3_CONS),
	__stringify(IPA_CLIENT_TEST4_PROD),
	__stringify(IPA_CLIENT_TEST4_CONS),
	__stringify(RESERVED_PROD_72),
	__stringify(IPA_CLIENT_DUMMY_CONS),
	__stringify(IPA_CLIENT_Q6_DL_NLO_DATA_PROD),
	__stringify(IPA_CLIENT_Q6_UL_NLO_DATA_CONS),
	__stringify(RESERVED_PROD_76),
	__stringify(IPA_CLIENT_Q6_UL_NLO_ACK_CONS),
	__stringify(RESERVED_PROD_78),
	__stringify(IPA_CLIENT_Q6_QBAP_STATUS_CONS),
	__stringify(RESERVED_PROD_80),
	__stringify(IPA_CLIENT_MHI_DPL_CONS),
	__stringify(RESERVED_PROD_82),
	__stringify(IPA_CLIENT_ODL_DPL_CONS),
	__stringify(IPA_CLIENT_Q6_AUDIO_DMA_MHI_PROD),
	__stringify(IPA_CLIENT_Q6_AUDIO_DMA_MHI_CONS),
	__stringify(IPA_CLIENT_WIGIG_PROD),
	__stringify(IPA_CLIENT_WIGIG1_CONS),
	__stringify(RESERVERD_PROD_88),
	__stringify(IPA_CLIENT_WIGIG2_CONS),
	__stringify(RESERVERD_PROD_90),
	__stringify(IPA_CLIENT_WIGIG3_CONS),
	__stringify(RESERVERD_PROD_92),
	__stringify(IPA_CLIENT_WIGIG4_CONS),
	__stringify(RESERVERD_PROD_94),
	__stringify(IPA_CLIENT_APPS_WAN_COAL_CONS),
	__stringify(IPA_CLIENT_MHI_PRIME_RMNET_PROD),
	__stringify(IPA_CLIENT_MHI_PRIME_RMNET_CONS),
	__stringify(IPA_CLIENT_MHI_PRIME_TETH_PROD),
	__stringify(IPA_CLIENT_MHI_PRIME_TETH_CONS),
	__stringify(IPA_CLIENT_MHI_PRIME_DPL_PROD),
	__stringify(RESERVERD_CONS_101),
	__stringify(IPA_CLIENT_AQC_ETHERNET_PROD),
	__stringify(IPA_CLIENT_AQC_ETHERNET_CONS),
	__stringify(IPA_CLIENT_APPS_WAN_LOW_LAT_PROD),
	__stringify(IPA_CLIENT_APPS_WAN_LOW_LAT_CONS),
};
EXPORT_SYMBOL(ipa_clients_strings);

/**
 * ipa_write_64() - convert 64 bit value to byte array
 * @w: 64 bit integer
 * @dest: byte array
 *
 * Return value: converted value
 */
u8 *ipa_write_64(u64 w, u8 *dest)
{
	if (unlikely(dest == NULL)) {
		pr_err("%s: NULL address\n", __func__);
		return dest;
	}
	*dest++ = (u8)((w) & 0xFF);
	*dest++ = (u8)((w >> 8) & 0xFF);
	*dest++ = (u8)((w >> 16) & 0xFF);
	*dest++ = (u8)((w >> 24) & 0xFF);
	*dest++ = (u8)((w >> 32) & 0xFF);
	*dest++ = (u8)((w >> 40) & 0xFF);
	*dest++ = (u8)((w >> 48) & 0xFF);
	*dest++ = (u8)((w >> 56) & 0xFF);

	return dest;
}

/**
 * ipa_write_32() - convert 32 bit value to byte array
 * @w: 32 bit integer
 * @dest: byte array
 *
 * Return value: converted value
 */
u8 *ipa_write_32(u32 w, u8 *dest)
{
	if (unlikely(dest == NULL)) {
		pr_err("%s: NULL address\n", __func__);
		return dest;
	}
	*dest++ = (u8)((w) & 0xFF);
	*dest++ = (u8)((w >> 8) & 0xFF);
	*dest++ = (u8)((w >> 16) & 0xFF);
	*dest++ = (u8)((w >> 24) & 0xFF);

	return dest;
}

/**
 * ipa_write_16() - convert 16 bit value to byte array
 * @hw: 16 bit integer
 * @dest: byte array
 *
 * Return value: converted value
 */
u8 *ipa_write_16(u16 hw, u8 *dest)
{
	if (unlikely(dest == NULL)) {
		pr_err("%s: NULL address\n", __func__);
		return dest;
	}
	*dest++ = (u8)((hw) & 0xFF);
	*dest++ = (u8)((hw >> 8) & 0xFF);

	return dest;
}

/**
 * ipa_write_8() - convert 8 bit value to byte array
 * @hw: 8 bit integer
 * @dest: byte array
 *
 * Return value: converted value
 */
u8 *ipa_write_8(u8 b, u8 *dest)
{
	if (unlikely(dest == NULL)) {
		WARN(1, "%s: NULL address\n", __func__);
		return dest;
	}
	*dest++ = (b) & 0xFF;

	return dest;
}

/**
 * ipa_pad_to_64() - pad byte array to 64 bit value
 * @dest: byte array
 *
 * Return value: padded value
 */
u8 *ipa_pad_to_64(u8 *dest)
{
	int i;
	int j;

	if (unlikely(dest == NULL)) {
		WARN(1, "%s: NULL address\n", __func__);
		return dest;
	}

	i = (long)dest & 0x7;

	if (i)
		for (j = 0; j < (8 - i); j++)
			*dest++ = 0;

	return dest;
}

/**
 * ipa_pad_to_32() - pad byte array to 32 bit value
 * @dest: byte array
 *
 * Return value: padded value
 */
u8 *ipa_pad_to_32(u8 *dest)
{
	int i;
	int j;

	if (unlikely(dest == NULL)) {
		WARN(1, "%s: NULL address\n", __func__);
		return dest;
	}

	i = (long)dest & 0x7;

	if (i)
		for (j = 0; j < (4 - i); j++)
			*dest++ = 0;

	return dest;
}

int ipa_smmu_store_sgt(struct sg_table **out_ch_ptr,
	struct sg_table *in_sgt_ptr)
{
	unsigned int nents;

	if (in_sgt_ptr != NULL) {
		*out_ch_ptr = kzalloc(sizeof(struct sg_table), GFP_KERNEL);
		if (*out_ch_ptr == NULL)
			return -ENOMEM;

		nents = in_sgt_ptr->nents;

		(*out_ch_ptr)->sgl =
			kcalloc(nents, sizeof(struct scatterlist),
				GFP_KERNEL);
		if ((*out_ch_ptr)->sgl == NULL) {
			kfree(*out_ch_ptr);
			*out_ch_ptr = NULL;
			return -ENOMEM;
		}

		memcpy((*out_ch_ptr)->sgl, in_sgt_ptr->sgl,
			nents*sizeof((*out_ch_ptr)->sgl));
		(*out_ch_ptr)->nents = nents;
		(*out_ch_ptr)->orig_nents = in_sgt_ptr->orig_nents;
	}
	return 0;
}
EXPORT_SYMBOL(ipa_smmu_store_sgt);

int ipa_smmu_free_sgt(struct sg_table **out_sgt_ptr)
{
	if (*out_sgt_ptr != NULL) {
		kfree((*out_sgt_ptr)->sgl);
		(*out_sgt_ptr)->sgl = NULL;
		kfree(*out_sgt_ptr);
		*out_sgt_ptr = NULL;
	}
	return 0;
}
EXPORT_SYMBOL(ipa_smmu_free_sgt);

/**
 * ipa_cfg_ep - IPA end-point configuration
 * @clnt_hdl:	[in] opaque client handle assigned by IPA to client
 * @ipa_ep_cfg:	[in] IPA end-point configuration params
 *
 * This includes nat, header, mode, aggregation and route settings and is a one
 * shot API to configure the IPA end-point fully
 *
 * Returns:	0 on success, negative on failure
 *
 * Note:	Should not be called from atomic context
 */
int ipa_cfg_ep(u32 clnt_hdl, const struct ipa_ep_cfg *ipa_ep_cfg)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_cfg_ep, clnt_hdl, ipa_ep_cfg);

	return ret;
}
EXPORT_SYMBOL(ipa_cfg_ep);

/**
 * ipa_cfg_ep_nat() - IPA end-point NAT configuration
 * @clnt_hdl:	[in] opaque client handle assigned by IPA to client
 * @ep_nat:	[in] IPA NAT end-point configuration params
 *
 * Returns:	0 on success, negative on failure
 *
 * Note:	Should not be called from atomic context
 */
int ipa_cfg_ep_nat(u32 clnt_hdl, const struct ipa_ep_cfg_nat *ep_nat)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_cfg_ep_nat, clnt_hdl, ep_nat);

	return ret;
}
EXPORT_SYMBOL(ipa_cfg_ep_nat);

/**
 * ipa_cfg_ep_conn_track() - IPA end-point IPv6CT configuration
 * @clnt_hdl:		[in] opaque client handle assigned by IPA to client
 * @ep_conn_track:	[in] IPA IPv6CT end-point configuration params
 *
 * Returns:	0 on success, negative on failure
 *
 * Note:	Should not be called from atomic context
 */
int ipa_cfg_ep_conn_track(u32 clnt_hdl,
	const struct ipa_ep_cfg_conn_track *ep_conn_track)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_cfg_ep_conn_track, clnt_hdl,
		ep_conn_track);

	return ret;
}
EXPORT_SYMBOL(ipa_cfg_ep_conn_track);

/**
 * ipa_cfg_ep_hdr() -  IPA end-point header configuration
 * @clnt_hdl:	[in] opaque client handle assigned by IPA to client
 * @ipa_ep_cfg:	[in] IPA end-point configuration params
 *
 * Returns:	0 on success, negative on failure
 *
 * Note:	Should not be called from atomic context
 */
int ipa_cfg_ep_hdr(u32 clnt_hdl, const struct ipa_ep_cfg_hdr *ep_hdr)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_cfg_ep_hdr, clnt_hdl, ep_hdr);

	return ret;
}
EXPORT_SYMBOL(ipa_cfg_ep_hdr);

/**
 * ipa_cfg_ep_hdr_ext() -  IPA end-point extended header configuration
 * @clnt_hdl:	[in] opaque client handle assigned by IPA to client
 * @ep_hdr_ext:	[in] IPA end-point configuration params
 *
 * Returns:	0 on success, negative on failure
 *
 * Note:	Should not be called from atomic context
 */
int ipa_cfg_ep_hdr_ext(u32 clnt_hdl,
		       const struct ipa_ep_cfg_hdr_ext *ep_hdr_ext)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_cfg_ep_hdr_ext, clnt_hdl, ep_hdr_ext);

	return ret;
}
EXPORT_SYMBOL(ipa_cfg_ep_hdr_ext);

/**
 * ipa_cfg_ep_mode() - IPA end-point mode configuration
 * @clnt_hdl:	[in] opaque client handle assigned by IPA to client
 * @ipa_ep_cfg:	[in] IPA end-point configuration params
 *
 * Returns:	0 on success, negative on failure
 *
 * Note:	Should not be called from atomic context
 */
int ipa_cfg_ep_mode(u32 clnt_hdl, const struct ipa_ep_cfg_mode *ep_mode)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_cfg_ep_mode, clnt_hdl, ep_mode);

	return ret;
}
EXPORT_SYMBOL(ipa_cfg_ep_mode);

/**
 * ipa_cfg_ep_aggr() - IPA end-point aggregation configuration
 * @clnt_hdl:	[in] opaque client handle assigned by IPA to client
 * @ipa_ep_cfg:	[in] IPA end-point configuration params
 *
 * Returns:	0 on success, negative on failure
 *
 * Note:	Should not be called from atomic context
 */
int ipa_cfg_ep_aggr(u32 clnt_hdl, const struct ipa_ep_cfg_aggr *ep_aggr)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_cfg_ep_aggr, clnt_hdl, ep_aggr);

	return ret;
}
EXPORT_SYMBOL(ipa_cfg_ep_aggr);

/**
 * ipa_cfg_ep_deaggr() -  IPA end-point deaggregation configuration
 * @clnt_hdl:	[in] opaque client handle assigned by IPA to client
 * @ep_deaggr:	[in] IPA end-point configuration params
 *
 * Returns:	0 on success, negative on failure
 *
 * Note:	Should not be called from atomic context
 */
int ipa_cfg_ep_deaggr(u32 clnt_hdl,
			const struct ipa_ep_cfg_deaggr *ep_deaggr)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_cfg_ep_deaggr, clnt_hdl, ep_deaggr);

	return ret;
}
EXPORT_SYMBOL(ipa_cfg_ep_deaggr);

/**
 * ipa_cfg_ep_route() - IPA end-point routing configuration
 * @clnt_hdl:	[in] opaque client handle assigned by IPA to client
 * @ipa_ep_cfg:	[in] IPA end-point configuration params
 *
 * Returns:	0 on success, negative on failure
 *
 * Note:	Should not be called from atomic context
 */
int ipa_cfg_ep_route(u32 clnt_hdl, const struct ipa_ep_cfg_route *ep_route)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_cfg_ep_route, clnt_hdl, ep_route);

	return ret;
}
EXPORT_SYMBOL(ipa_cfg_ep_route);

/**
 * ipa_cfg_ep_holb() - IPA end-point holb configuration
 *
 * If an IPA producer pipe is full, IPA HW by default will block
 * indefinitely till space opens up. During this time no packets
 * including those from unrelated pipes will be processed. Enabling
 * HOLB means IPA HW will be allowed to drop packets as/when needed
 * and indefinite blocking is avoided.
 *
 * @clnt_hdl:	[in] opaque client handle assigned by IPA to client
 * @ipa_ep_cfg:	[in] IPA end-point configuration params
 *
 * Returns:	0 on success, negative on failure
 */
int ipa_cfg_ep_holb(u32 clnt_hdl, const struct ipa_ep_cfg_holb *ep_holb)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_cfg_ep_holb, clnt_hdl, ep_holb);

	return ret;
}
EXPORT_SYMBOL(ipa_cfg_ep_holb);


/**
 * ipa_cfg_ep_cfg() - IPA end-point cfg configuration
 * @clnt_hdl:	[in] opaque client handle assigned by IPA to client
 * @ipa_ep_cfg:	[in] IPA end-point configuration params
 *
 * Returns:	0 on success, negative on failure
 *
 * Note:	Should not be called from atomic context
 */
int ipa_cfg_ep_cfg(u32 clnt_hdl, const struct ipa_ep_cfg_cfg *cfg)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_cfg_ep_cfg, clnt_hdl, cfg);

	return ret;
}
EXPORT_SYMBOL(ipa_cfg_ep_cfg);

/**
 * ipa_cfg_ep_metadata_mask() - IPA end-point meta-data mask configuration
 * @clnt_hdl:	[in] opaque client handle assigned by IPA to client
 * @ipa_ep_cfg:	[in] IPA end-point configuration params
 *
 * Returns:	0 on success, negative on failure
 *
 * Note:	Should not be called from atomic context
 */
int ipa_cfg_ep_metadata_mask(u32 clnt_hdl, const struct ipa_ep_cfg_metadata_mask
		*metadata_mask)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_cfg_ep_metadata_mask, clnt_hdl,
			metadata_mask);

	return ret;
}
EXPORT_SYMBOL(ipa_cfg_ep_metadata_mask);

/**
 * ipa_cfg_ep_holb_by_client() - IPA end-point holb configuration
 *
 * Wrapper function for ipa_cfg_ep_holb() with client name instead of
 * client handle. This function is used for clients that does not have
 * client handle.
 *
 * @client:	[in] client name
 * @ipa_ep_cfg:	[in] IPA end-point configuration params
 *
 * Returns:	0 on success, negative on failure
 */
int ipa_cfg_ep_holb_by_client(enum ipa_client_type client,
				const struct ipa_ep_cfg_holb *ep_holb)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_cfg_ep_holb_by_client, client, ep_holb);

	return ret;
}
EXPORT_SYMBOL(ipa_cfg_ep_holb_by_client);

/**
 * ipa_add_hdr_usr() - add the specified headers to SW and optionally
 * commit them to IPA HW
 * @hdrs:		[inout] set of headers to add
 * @user_only:	[in] indicate rules installed by userspace
 *
 * Returns:	0 on success, negative on failure
 *
 * Note:	Should not be called from atomic context
 */
int ipa_add_hdr_usr(struct ipa_ioc_add_hdr *hdrs, bool user_only)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_add_hdr_usr, hdrs, user_only);

	return ret;
}
EXPORT_SYMBOL(ipa_add_hdr_usr);

/**
 * ipa_reset_hdr() - reset the current header table in SW (does not commit to
 * HW)
 *
 * @user_only:	[in] indicate delete rules installed by userspace
 * Returns:	0 on success, negative on failure
 *
 * Note:	Should not be called from atomic context
 */
int ipa_reset_hdr(bool user_only)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_reset_hdr, user_only);

	return ret;
}
EXPORT_SYMBOL(ipa_reset_hdr);

/**
 * ipa_add_hdr_proc_ctx() - add the specified headers to SW
 * and optionally commit them to IPA HW
 * @proc_ctxs:	[inout] set of processing context headers to add
 * @user_only:	[in] indicate rules installed by userspace
 *
 * Returns:	0 on success, negative on failure
 *
 * Note:	Should not be called from atomic context
 */
int ipa_add_hdr_proc_ctx(struct ipa_ioc_add_hdr_proc_ctx *proc_ctxs,
							bool user_only)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_add_hdr_proc_ctx, proc_ctxs, user_only);

	return ret;
}
EXPORT_SYMBOL(ipa_add_hdr_proc_ctx);

/**
 * ipa_del_hdr_proc_ctx() -
 * Remove the specified processing context headers from SW and
 * optionally commit them to IPA HW.
 * @hdls:	[inout] set of processing context headers to delete
 *
 * Returns:	0 on success, negative on failure
 *
 * Note:	Should not be called from atomic context
 */
int ipa_del_hdr_proc_ctx(struct ipa_ioc_del_hdr_proc_ctx *hdls)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_del_hdr_proc_ctx, hdls);

	return ret;
}
EXPORT_SYMBOL(ipa_del_hdr_proc_ctx);

/**
 * ipa_add_rt_rule_v2() - Add the specified routing rules to SW
 * and optionally commit to IPA HW
 * @rules:	[inout] set of routing rules to add
 *
 * Returns:	0 on success, negative on failure
 *
 * Note:	Should not be called from atomic context
 */
int ipa_add_rt_rule_v2(struct ipa_ioc_add_rt_rule_v2 *rules)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_add_rt_rule_v2, rules);

	return ret;
}
EXPORT_SYMBOL(ipa_add_rt_rule_v2);

/**
 * ipa_add_rt_rule_usr() - Add the specified routing rules to SW and optionally
 * commit to IPA HW
 * @rules:	[inout] set of routing rules to add
 * @user_only:	[in] indicate rules installed by userspace
 *
 * Returns:	0 on success, negative on failure
 *
 * Note:	Should not be called from atomic context
 */
int ipa_add_rt_rule_usr(struct ipa_ioc_add_rt_rule *rules, bool user_only)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_add_rt_rule_usr, rules, user_only);

	return ret;
}
EXPORT_SYMBOL(ipa_add_rt_rule_usr);

/**
 * ipa_add_rt_rule_usr_v2() - Add the specified routing rules to
 * SW and optionally commit to IPA HW
 * @rules:	[inout] set of routing rules to add
 * @user_only:	[in] indicate rules installed by userspace
 *
 * Returns:	0 on success, negative on failure
 *
 * Note:	Should not be called from atomic context
 */
int ipa_add_rt_rule_usr_v2(struct ipa_ioc_add_rt_rule_v2 *rules, bool user_only)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_add_rt_rule_usr_v2, rules, user_only);

	return ret;
}
EXPORT_SYMBOL(ipa_add_rt_rule_usr_v2);

/**
 * ipa_del_rt_rule() - Remove the specified routing rules to SW and optionally
 * commit to IPA HW
 * @hdls:	[inout] set of routing rules to delete
 *
 * Returns:	0 on success, negative on failure
 *
 * Note:	Should not be called from atomic context
 */
int ipa_del_rt_rule(struct ipa_ioc_del_rt_rule *hdls)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_del_rt_rule, hdls);

	return ret;
}
EXPORT_SYMBOL(ipa_del_rt_rule);

/**
 * ipa_commit_rt_rule() - Commit the current SW routing table of specified type
 * to IPA HW
 * @ip:	The family of routing tables
 *
 * Returns:	0 on success, negative on failure
 *
 * Note:	Should not be called from atomic context
 */
int ipa_commit_rt(enum ipa_ip_type ip)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_commit_rt, ip);

	return ret;
}
EXPORT_SYMBOL(ipa_commit_rt);

/**
 * ipa_reset_rt() - reset the current SW routing table of specified type
 * (does not commit to HW)
 * @ip:	The family of routing tables
 * @user_only:	[in] indicate delete rules installed by userspace
 *
 * Returns:	0 on success, negative on failure
 *
 * Note:	Should not be called from atomic context
 */
int ipa_reset_rt(enum ipa_ip_type ip, bool user_only)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_reset_rt, ip, user_only);

	return ret;
}
EXPORT_SYMBOL(ipa_reset_rt);

/**
 * ipa_get_rt_tbl() - lookup the specified routing table and return handle if it
 * exists, if lookup succeeds the routing table ref cnt is increased
 * @lookup:	[inout] routing table to lookup and its handle
 *
 * Returns:	0 on success, negative on failure
 *
 * Note:	Should not be called from atomic context
 *	Caller should call ipa_put_rt_tbl later if this function succeeds
 */
int ipa_get_rt_tbl(struct ipa_ioc_get_rt_tbl *lookup)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_get_rt_tbl, lookup);

	return ret;
}
EXPORT_SYMBOL(ipa_get_rt_tbl);

/**
 * ipa_query_rt_index() - find the routing table index
 *			which name and ip type are given as parameters
 * @in:	[out] the index of the wanted routing table
 *
 * Returns: the routing table which name is given as parameter, or NULL if it
 * doesn't exist
 */
int ipa_query_rt_index(struct ipa_ioc_get_rt_tbl_indx *in)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_query_rt_index, in);

	return ret;
}
EXPORT_SYMBOL(ipa_query_rt_index);

/**
 * ipa_mdfy_rt_rule() - Modify the specified routing rules in SW and optionally
 * commit to IPA HW
 *
 * Returns:	0 on success, negative on failure
 *
 * Note:	Should not be called from atomic context
 */
int ipa_mdfy_rt_rule(struct ipa_ioc_mdfy_rt_rule *hdls)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_mdfy_rt_rule, hdls);

	return ret;
}
EXPORT_SYMBOL(ipa_mdfy_rt_rule);

/**
 * ipa_mdfy_rt_rule_v2() - Modify the specified routing rules in
 * SW and optionally commit to IPA HW
 *
 * Returns:	0 on success, negative on failure
 *
 * Note:	Should not be called from atomic context
 */
int ipa_mdfy_rt_rule_v2(struct ipa_ioc_mdfy_rt_rule_v2 *hdls)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_mdfy_rt_rule_v2, hdls);

	return ret;
}
EXPORT_SYMBOL(ipa_mdfy_rt_rule_v2);

/**
 * ipa_add_flt_rule() - Add the specified filtering rules to SW and optionally
 * commit to IPA HW
 * @rules:	[inout] set of filtering rules to add
 *
 * Returns:	0 on success, negative on failure
 *
 * Note:	Should not be called from atomic context
 */
int ipa_add_flt_rule(struct ipa_ioc_add_flt_rule *rules)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_add_flt_rule, rules);

	return ret;
}
EXPORT_SYMBOL(ipa_add_flt_rule);

/**
 * ipa_add_flt_rule_v2() - Add the specified filtering rules to
 * SW and optionally commit to IPA HW
 * @rules:	[inout] set of filtering rules to add
 *
 * Returns:	0 on success, negative on failure
 *
 * Note:	Should not be called from atomic context
 */
int ipa_add_flt_rule_v2(struct ipa_ioc_add_flt_rule_v2 *rules)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_add_flt_rule_v2, rules);

	return ret;
}
EXPORT_SYMBOL(ipa_add_flt_rule_v2);

/**
 * ipa_add_flt_rule_usr() - Add the specified filtering rules to
 * SW and optionally commit to IPA HW
 * @rules:		[inout] set of filtering rules to add
 * @user_only:	[in] indicate rules installed by userspace
 *
 * Returns:	0 on success, negative on failure
 *
 * Note:	Should not be called from atomic context
 */
int ipa_add_flt_rule_usr(struct ipa_ioc_add_flt_rule *rules, bool user_only)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_add_flt_rule_usr, rules, user_only);

	return ret;
}
EXPORT_SYMBOL(ipa_add_flt_rule_usr);

/**
 * ipa_add_flt_rule_usr_v2() - Add the specified filtering rules
 * to SW and optionally commit to IPA HW
 * @rules:		[inout] set of filtering rules to add
 * @user_only:	[in] indicate rules installed by userspace
 *
 * Returns:	0 on success, negative on failure
 *
 * Note:	Should not be called from atomic context
 */
int ipa_add_flt_rule_usr_v2(struct ipa_ioc_add_flt_rule_v2 *rules,
	bool user_only)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_add_flt_rule_usr_v2,
		rules, user_only);

	return ret;
}
EXPORT_SYMBOL(ipa_add_flt_rule_usr_v2);

/**
 * ipa_del_flt_rule() - Remove the specified filtering rules from SW and
 * optionally commit to IPA HW
 *
 * Returns:	0 on success, negative on failure
 *
 * Note:	Should not be called from atomic context
 */
int ipa_del_flt_rule(struct ipa_ioc_del_flt_rule *hdls)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_del_flt_rule, hdls);

	return ret;
}
EXPORT_SYMBOL(ipa_del_flt_rule);

/**
 * ipa_mdfy_flt_rule() - Modify the specified filtering rules in SW and
 * optionally commit to IPA HW
 *
 * Returns:	0 on success, negative on failure
 *
 * Note:	Should not be called from atomic context
 */
int ipa_mdfy_flt_rule(struct ipa_ioc_mdfy_flt_rule *hdls)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_mdfy_flt_rule, hdls);

	return ret;
}
EXPORT_SYMBOL(ipa_mdfy_flt_rule);

/**
 * ipa_mdfy_flt_rule_v2() - Modify the specified filtering rules
 * in SW and optionally commit to IPA HW
 *
 * Returns:	0 on success, negative on failure
 *
 * Note:	Should not be called from atomic context
 */
int ipa_mdfy_flt_rule_v2(struct ipa_ioc_mdfy_flt_rule_v2 *hdls)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_mdfy_flt_rule_v2, hdls);

	return ret;
}
EXPORT_SYMBOL(ipa_mdfy_flt_rule_v2);

/**
 * ipa_commit_flt() - Commit the current SW filtering table of specified type to
 * IPA HW
 * @ip:	[in] the family of routing tables
 *
 * Returns:	0 on success, negative on failure
 *
 * Note:	Should not be called from atomic context
 */
int ipa_commit_flt(enum ipa_ip_type ip)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_commit_flt, ip);

	return ret;
}
EXPORT_SYMBOL(ipa_commit_flt);

/**
 * ipa_reset_flt() - Reset the current SW filtering table of specified type
 * (does not commit to HW)
 * @ip:			[in] the family of routing tables
 * @user_only:	[in] indicate delete rules installed by userspace
 *
 * Returns:	0 on success, negative on failure
 *
 * Note:	Should not be called from atomic context
 */
int ipa_reset_flt(enum ipa_ip_type ip, bool user_only)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_reset_flt, ip, user_only);

	return ret;
}
EXPORT_SYMBOL(ipa_reset_flt);

/**
 * ipa_allocate_nat_device() - Allocates memory for the NAT device
 * @mem:	[in/out] memory parameters
 *
 * Called by NAT client driver to allocate memory for the NAT entries. Based on
 * the request size either shared or system memory will be used.
 *
 * Returns:	0 on success, negative on failure
 */
int ipa_allocate_nat_device(struct ipa_ioc_nat_alloc_mem *mem)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_allocate_nat_device, mem);

	return ret;
}
EXPORT_SYMBOL(ipa_allocate_nat_device);

/**
 * ipa_allocate_nat_table() - Allocates memory for the NAT table
 * @table_alloc: [in/out] memory parameters
 *
 * Called by NAT client to allocate memory for the table entries.
 * Based on the request size either shared or system memory will be used.
 *
 * Returns:	0 on success, negative on failure
 */
int ipa_allocate_nat_table(struct ipa_ioc_nat_ipv6ct_table_alloc *table_alloc)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_allocate_nat_table, table_alloc);

	return ret;
}
EXPORT_SYMBOL(ipa_allocate_nat_table);


/**
 * ipa_allocate_ipv6ct_table() - Allocates memory for the IPv6CT table
 * @table_alloc: [in/out] memory parameters
 *
 * Called by IPv6CT client to allocate memory for the table entries.
 * Based on the request size either shared or system memory will be used.
 *
 * Returns:	0 on success, negative on failure
 */
int ipa_allocate_ipv6ct_table(
	struct ipa_ioc_nat_ipv6ct_table_alloc *table_alloc)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_allocate_ipv6ct_table, table_alloc);

	return ret;
}
EXPORT_SYMBOL(ipa_allocate_ipv6ct_table);

/**
 * ipa_nat_init_cmd() - Post IP_V4_NAT_INIT command to IPA HW
 * @init:	[in] initialization command attributes
 *
 * Called by NAT client driver to post IP_V4_NAT_INIT command to IPA HW
 *
 * Returns:	0 on success, negative on failure
 */
int ipa_nat_init_cmd(struct ipa_ioc_v4_nat_init *init)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_nat_init_cmd, init);

	return ret;
}
EXPORT_SYMBOL(ipa_nat_init_cmd);

/**
 * ipa_ipv6ct_init_cmd() - Post IP_V6_CONN_TRACK_INIT command to IPA HW
 * @init:	[in] initialization command attributes
 *
 * Called by IPv6CT client driver to post IP_V6_CONN_TRACK_INIT command
 * to IPA HW.
 *
 * Returns:	0 on success, negative on failure
 */
int ipa_ipv6ct_init_cmd(struct ipa_ioc_ipv6ct_init *init)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_ipv6ct_init_cmd, init);

	return ret;
}
EXPORT_SYMBOL(ipa_ipv6ct_init_cmd);

/**
 * ipa_nat_dma_cmd() - Post NAT_DMA command to IPA HW
 * @dma:	[in] initialization command attributes
 *
 * Called by NAT client driver to post NAT_DMA command to IPA HW
 *
 * Returns:	0 on success, negative on failure
 */
int ipa_nat_dma_cmd(struct ipa_ioc_nat_dma_cmd *dma)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_nat_dma_cmd, dma);

	return ret;
}
EXPORT_SYMBOL(ipa_nat_dma_cmd);

/**
 * ipa_table_dma_cmd() - Post TABLE_DMA command to IPA HW
 * @dma:	[in] initialization command attributes
 *
 * Called by NAT/IPv6CT client to post TABLE_DMA command to IPA HW
 *
 * Returns:	0 on success, negative on failure
 */
int ipa_table_dma_cmd(struct ipa_ioc_nat_dma_cmd *dma)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_table_dma_cmd, dma);

	return ret;
}
EXPORT_SYMBOL(ipa_table_dma_cmd);

/**
 * ipa_nat_del_cmd() - Delete the NAT table
 * @del:	[in] delete NAT table parameters
 *
 * Called by NAT client driver to delete the nat table
 *
 * Returns:	0 on success, negative on failure
 */
int ipa_nat_del_cmd(struct ipa_ioc_v4_nat_del *del)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_nat_del_cmd, del);

	return ret;
}
EXPORT_SYMBOL(ipa_nat_del_cmd);

/**
 * ipa_del_nat_table() - Delete the NAT table
 * @del:	[in] delete table parameters
 *
 * Called by NAT client to delete the table
 *
 * Returns:	0 on success, negative on failure
 */
int ipa_del_nat_table(struct ipa_ioc_nat_ipv6ct_table_del *del)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_del_nat_table, del);

	return ret;
}
EXPORT_SYMBOL(ipa_del_nat_table);

/**
 * ipa_del_ipv6ct_table() - Delete the IPv6CT table
 * @del:	[in] delete table parameters
 *
 * Called by IPv6CT client to delete the table
 *
 * Returns:	0 on success, negative on failure
 */
int ipa_del_ipv6ct_table(struct ipa_ioc_nat_ipv6ct_table_del *del)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_del_ipv6ct_table, del);

	return ret;
}
EXPORT_SYMBOL(ipa_del_ipv6ct_table);

/**
 * ipa3_nat_mdfy_pdn() - Modify a PDN entry in PDN config table in IPA SRAM
 * @mdfy_pdn:	[in] PDN info to be written to SRAM
 *
 * Called by NAT client driver to modify an entry in the PDN config table
 *
 * Returns:	0 on success, negative on failure
 */
int ipa_nat_mdfy_pdn(struct ipa_ioc_nat_pdn_entry *mdfy_pdn)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_nat_mdfy_pdn, mdfy_pdn);

	return ret;
}
EXPORT_SYMBOL(ipa_nat_mdfy_pdn);

int ipa_sys_setup(struct ipa_sys_connect_params *sys_in,
	unsigned long *ipa_bam_or_gsi_hdl,
	u32 *ipa_pipe_num, u32 *clnt_hdl, bool en_status)

{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_sys_setup, sys_in, ipa_bam_or_gsi_hdl,
			ipa_pipe_num, clnt_hdl, en_status);

	return ret;
}
EXPORT_SYMBOL(ipa_sys_setup);

int ipa_sys_teardown(u32 clnt_hdl)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_sys_teardown, clnt_hdl);

	return ret;
}
EXPORT_SYMBOL(ipa_sys_teardown);

int ipa_sys_update_gsi_hdls(u32 clnt_hdl, unsigned long gsi_ch_hdl,
	unsigned long gsi_ev_hdl)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_sys_update_gsi_hdls, clnt_hdl,
		gsi_ch_hdl, gsi_ev_hdl);

	return ret;
}
EXPORT_SYMBOL(ipa_sys_update_gsi_hdls);

/**
 * ipa_set_wlan_tx_info() -set WDI statistics from uc
 * @info:	[inout] set info populated by driver
 *
 * Returns:	0 on success, negative on failure
 *
 * @note Cannot be called from atomic context
 *
 */
int ipa_set_wlan_tx_info(struct ipa_wdi_tx_info *info)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_set_wlan_tx_info, info);

	return ret;
}
EXPORT_SYMBOL(ipa_set_wlan_tx_info);

/**
 * ipa_wigig_internal_init() - get uc db and register uC
 * ready CB if uC not ready, wigig only.
 * @inout:	[in/out] uc ready input/output parameters
 * from/to client
 * @int_notify: [in] wigig misc interrupt handler function
 * @uc_db_pa: [out] uC db physical address
 *
 * Returns:	0 on success, negative on failure
 *
 */
int ipa_wigig_internal_init(
	struct ipa_wdi_uc_ready_params *inout,
	ipa_wigig_misc_int_cb int_notify,
	phys_addr_t *uc_db_pa)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_wigig_internal_init, inout,
		int_notify, uc_db_pa);

	return ret;
}
EXPORT_SYMBOL(ipa_wigig_internal_init);

/**
 * teth_bridge_init() - Initialize the Tethering bridge driver
 * @params - in/out params for USB initialization API (please look at struct
 *  definition for more info)
 *
 * USB driver gets a pointer to a callback function (usb_notify_cb) and an
 * associated data. USB driver installs this callback function in the call to
 * ipa_connect().
 *
 * Builds IPA resource manager dependency graph.
 *
 * Return codes: 0: success,
 *		-EINVAL - Bad parameter
 *		Other negative value - Failure
 */
int teth_bridge_init(struct teth_bridge_init_params *params)
{
	int ret;

	IPA_API_DISPATCH_RETURN(teth_bridge_init, params);

	return ret;
}
EXPORT_SYMBOL(teth_bridge_init);

/**
 * teth_bridge_disconnect() - Disconnect tethering bridge module
 */
int teth_bridge_disconnect(enum ipa_client_type client)
{
	int ret;

	IPA_API_DISPATCH_RETURN(teth_bridge_disconnect, client);

	return ret;
}
EXPORT_SYMBOL(teth_bridge_disconnect);

/**
 * teth_bridge_connect() - Connect bridge for a tethered Rmnet / MBIM call
 * @connect_params:	Connection info
 *
 * Return codes: 0: success
 *		-EINVAL: invalid parameters
 *		-EPERM: Operation not permitted as the bridge is already
 *		connected
 */
int teth_bridge_connect(struct teth_bridge_connect_params *connect_params)
{
	int ret;

	IPA_API_DISPATCH_RETURN(teth_bridge_connect, connect_params);

	return ret;
}
EXPORT_SYMBOL(teth_bridge_connect);

/* ipa_set_client() - provide client mapping
 * @client: client type
 *
 * Return value: none
 */

void ipa_set_client(int index, enum ipacm_client_enum client, bool uplink)
{
	IPA_API_DISPATCH(ipa_set_client, index, client, uplink);
}
EXPORT_SYMBOL(ipa_set_client);

/**
 * ipa_get_client() - provide client mapping
 * @client: client type
 *
 * Return value: none
 */
enum ipacm_client_enum ipa_get_client(int pipe_idx)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_get_client, pipe_idx);

	return ret;
}
EXPORT_SYMBOL(ipa_get_client);

/**
 * ipa_get_client_uplink() - provide client mapping
 * @client: client type
 *
 * Return value: none
 */
bool ipa_get_client_uplink(int pipe_idx)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_get_client_uplink, pipe_idx);

	return ret;
}
EXPORT_SYMBOL(ipa_get_client_uplink);

int ipa_mhi_init_engine(struct ipa_mhi_init_engine *params)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_mhi_init_engine, params);

	return ret;
}
EXPORT_SYMBOL(ipa_mhi_init_engine);

/**
 * ipa_connect_mhi_pipe() - Connect pipe to IPA and start corresponding
 * MHI channel
 * @in: connect parameters
 * @clnt_hdl: [out] client handle for this pipe
 *
 * This function is called by IPA MHI client driver on MHI channel start.
 * This function is called after MHI engine was started.
 * This function is doing the following:
 *	- Send command to uC to start corresponding MHI channel
 *	- Configure IPA EP control
 *
 * Return codes: 0	  : success
 *		 negative : error
 */
int ipa_connect_mhi_pipe(struct ipa_mhi_connect_params_internal *in,
		u32 *clnt_hdl)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_connect_mhi_pipe, in, clnt_hdl);

	return ret;
}
EXPORT_SYMBOL(ipa_connect_mhi_pipe);

/**
 * ipa_disconnect_mhi_pipe() - Disconnect pipe from IPA and reset corresponding
 * MHI channel
 * @in: connect parameters
 * @clnt_hdl: [out] client handle for this pipe
 *
 * This function is called by IPA MHI client driver on MHI channel reset.
 * This function is called after MHI channel was started.
 * This function is doing the following:
 *	- Send command to uC to reset corresponding MHI channel
 *	- Configure IPA EP control
 *
 * Return codes: 0	  : success
 *		 negative : error
 */
int ipa_disconnect_mhi_pipe(u32 clnt_hdl)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_disconnect_mhi_pipe, clnt_hdl);

	return ret;
}
EXPORT_SYMBOL(ipa_disconnect_mhi_pipe);

bool ipa_mhi_stop_gsi_channel(enum ipa_client_type client)
{
	bool ret;

	IPA_API_DISPATCH_RETURN_BOOL(ipa_mhi_stop_gsi_channel, client);

	return ret;
}
EXPORT_SYMBOL(ipa_mhi_stop_gsi_channel);

int ipa_uc_mhi_reset_channel(int channelHandle)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_uc_mhi_reset_channel, channelHandle);

	return ret;
}
EXPORT_SYMBOL(ipa_uc_mhi_reset_channel);

bool ipa_mhi_sps_channel_empty(enum ipa_client_type client)
{
	bool ret;

	IPA_API_DISPATCH_RETURN_BOOL(ipa_mhi_sps_channel_empty, client);

	return ret;
}
EXPORT_SYMBOL(ipa_mhi_sps_channel_empty);

int ipa_qmi_enable_force_clear_datapath_send(
	struct ipa_enable_force_clear_datapath_req_msg_v01 *req)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_qmi_enable_force_clear_datapath_send, req);

	return ret;
}
EXPORT_SYMBOL(ipa_qmi_enable_force_clear_datapath_send);

int ipa_qmi_disable_force_clear_datapath_send(
	struct ipa_disable_force_clear_datapath_req_msg_v01 *req)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_qmi_disable_force_clear_datapath_send, req);

	return ret;
}
EXPORT_SYMBOL(ipa_qmi_disable_force_clear_datapath_send);

int ipa_generate_tag_process(void)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_generate_tag_process);

	return ret;
}
EXPORT_SYMBOL(ipa_generate_tag_process);

int ipa_disable_sps_pipe(enum ipa_client_type client)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_disable_sps_pipe, client);

	return ret;
}
EXPORT_SYMBOL(ipa_disable_sps_pipe);

int ipa_mhi_reset_channel_internal(enum ipa_client_type client)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_mhi_reset_channel_internal, client);

	return ret;
}
EXPORT_SYMBOL(ipa_mhi_reset_channel_internal);

int ipa_mhi_start_channel_internal(enum ipa_client_type client)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_mhi_start_channel_internal, client);

	return ret;
}
EXPORT_SYMBOL(ipa_mhi_start_channel_internal);

void ipa_get_holb(int ep_idx, struct ipa_ep_cfg_holb *holb)
{
	IPA_API_DISPATCH(ipa_get_holb, ep_idx, holb);
}
EXPORT_SYMBOL(ipa_get_holb);

void ipa_set_tag_process_before_gating(bool val)
{
	IPA_API_DISPATCH(ipa_set_tag_process_before_gating, val);
}
EXPORT_SYMBOL(ipa_set_tag_process_before_gating);

int ipa_mhi_query_ch_info(enum ipa_client_type client,
		struct gsi_chan_info *ch_info)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_mhi_query_ch_info, client, ch_info);

	return ret;
}
EXPORT_SYMBOL(ipa_mhi_query_ch_info);

int ipa_uc_mhi_suspend_channel(int channelHandle)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_uc_mhi_suspend_channel, channelHandle);

	return ret;
}
EXPORT_SYMBOL(ipa_uc_mhi_suspend_channel);

int ipa_uc_mhi_stop_event_update_channel(int channelHandle)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_uc_mhi_stop_event_update_channel,
			channelHandle);

	return ret;
}
EXPORT_SYMBOL(ipa_uc_mhi_stop_event_update_channel);

bool ipa_has_open_aggr_frame(enum ipa_client_type client)
{
	bool ret;

	IPA_API_DISPATCH_RETURN_BOOL(ipa_has_open_aggr_frame, client);

	return ret;
}
EXPORT_SYMBOL(ipa_has_open_aggr_frame);

int ipa_mhi_resume_channels_internal(enum ipa_client_type client,
		bool LPTransitionRejected, bool brstmode_enabled,
		union __packed gsi_channel_scratch ch_scratch, u8 index)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_mhi_resume_channels_internal, client,
			LPTransitionRejected, brstmode_enabled, ch_scratch,
			index);

	return ret;
}
EXPORT_SYMBOL(ipa_mhi_resume_channels_internal);

int ipa_uc_mhi_send_dl_ul_sync_info(union IpaHwMhiDlUlSyncCmdData_t *cmd)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_uc_mhi_send_dl_ul_sync_info,
			cmd);

	return ret;
}
EXPORT_SYMBOL(ipa_uc_mhi_send_dl_ul_sync_info);

int ipa_mhi_destroy_channel(enum ipa_client_type client)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_mhi_destroy_channel, client);

	return ret;
}
EXPORT_SYMBOL(ipa_mhi_destroy_channel);

int ipa_uc_mhi_init(void (*ready_cb)(void),
		void (*wakeup_request_cb)(void))
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_uc_mhi_init, ready_cb, wakeup_request_cb);

	return ret;
}
EXPORT_SYMBOL(ipa_uc_mhi_init);

void ipa_uc_mhi_cleanup(void)
{
	IPA_API_DISPATCH(ipa_uc_mhi_cleanup);
}
EXPORT_SYMBOL(ipa_uc_mhi_cleanup);

int ipa_uc_mhi_print_stats(char *dbg_buff, int size)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_uc_mhi_print_stats, dbg_buff, size);

	return ret;
}
EXPORT_SYMBOL(ipa_uc_mhi_print_stats);

/**
 * ipa_uc_state_check() - Check the status of the uC interface
 *
 * Return value: 0 if the uC is loaded, interface is initialized
 *               and there was no recent failure in one of the commands.
 *               A negative value is returned otherwise.
 */
int ipa_uc_state_check(void)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_uc_state_check);

	return ret;
}
EXPORT_SYMBOL(ipa_uc_state_check);

int ipa_write_qmap_id(struct ipa_ioc_write_qmapid *param_in)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_write_qmap_id, param_in);

	return ret;
}
EXPORT_SYMBOL(ipa_write_qmap_id);

/**
 * ipa_remove_interrupt_handler() - Removes handler to an interrupt type
 * @interrupt:		Interrupt type
 *
 * Removes the handler and disable the specific bit in IRQ_EN register
 */
int ipa_remove_interrupt_handler(enum ipa_irq_type interrupt)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_remove_interrupt_handler, interrupt);

	return ret;
}
EXPORT_SYMBOL(ipa_remove_interrupt_handler);

/**
 * ipa_get_hw_type() - Return IPA HW version
 *
 * Return value: enum ipa_hw_type
 */
enum ipa_hw_type ipa_get_hw_type_internal(void)
{
	return ipa_api_hw_type;
}

/**
 * ipa_start_gsi_channel()- Startsa GSI channel in IPA
 *
 * Return value: 0 on success, negative otherwise
 */
int ipa_start_gsi_channel(u32 clnt_hdl)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_start_gsi_channel, clnt_hdl);

	return ret;
}
EXPORT_SYMBOL(ipa_start_gsi_channel);

/**
 * ipa_get_version_string() - Get string representation of IPA version
 * @ver: IPA version
 *
 * Return: Constant string representation
 */
const char *ipa_get_version_string(enum ipa_hw_type ver)
{
	const char *str;

	switch (ver) {
	case IPA_HW_v1_0:
		str = "1.0";
		break;
	case IPA_HW_v1_1:
		str = "1.1";
		break;
	case IPA_HW_v2_0:
		str = "2.0";
		break;
	case IPA_HW_v2_1:
		str = "2.1";
		break;
	case IPA_HW_v2_5:
		str = "2.5/2.6";
		break;
	case IPA_HW_v2_6L:
		str = "2.6L";
		break;
	case IPA_HW_v3_0:
		str = "3.0";
		break;
	case IPA_HW_v3_1:
		str = "3.1";
		break;
	case IPA_HW_v3_5:
		str = "3.5";
		break;
	case IPA_HW_v3_5_1:
		str = "3.5.1";
		break;
	case IPA_HW_v4_0:
		str = "4.0";
		break;
	case IPA_HW_v4_1:
		str = "4.1";
		break;
	case IPA_HW_v4_2:
		str = "4.2";
		break;
	case IPA_HW_v4_5:
		str = "4.5";
		break;
	case IPA_HW_v4_7:
		str = "4.7";
		break;
	case IPA_HW_v4_9:
		str = "4.9";
		break;
	default:
		str = "Invalid version";
		break;
	}

	return str;
}
EXPORT_SYMBOL(ipa_get_version_string);

static const struct of_device_id ipa_plat_drv_match[] = {
	{ .compatible = "qcom,ipa", },
	{ .compatible = "qcom,ipa-smmu-ap-cb", },
	{ .compatible = "qcom,ipa-smmu-wlan-cb", },
	{ .compatible = "qcom,ipa-smmu-uc-cb", },
	{ .compatible = "qcom,ipa-smmu-11ad-cb", },
	{ .compatible = "qcom,smp2p-map-ipa-1-in", },
	{ .compatible = "qcom,smp2p-map-ipa-1-out", },
	{}
};

/*********************************************************/
/*                PCIe Version                           */
/*********************************************************/

static const struct of_device_id ipa_pci_drv_match[] = {
	{ .compatible = "qcom,ipa", },
	{}
};

/*
 * Forward declarations of static functions required for PCI
 * registraion
 *
 * VENDOR and DEVICE should be defined in pci_ids.h
 */
static int ipa_pci_probe(struct pci_dev *pdev, const struct pci_device_id *ent);
static void ipa_pci_remove(struct pci_dev *pdev);
static void ipa_pci_shutdown(struct pci_dev *pdev);
static pci_ers_result_t ipa_pci_io_error_detected(struct pci_dev *dev,
	pci_channel_state_t state);
static pci_ers_result_t ipa_pci_io_slot_reset(struct pci_dev *dev);
static void ipa_pci_io_resume(struct pci_dev *dev);

#define LOCAL_VENDOR 0x17CB
#define LOCAL_DEVICE 0x00ff

static const char ipa_pci_driver_name[] = "qcipav3";

static const struct pci_device_id ipa_pci_tbl[] = {
	{ PCI_DEVICE(LOCAL_VENDOR, LOCAL_DEVICE) },
	{ 0, 0, 0, 0, 0, 0, 0 }
};

MODULE_DEVICE_TABLE(pci, ipa_pci_tbl);

/* PCI Error Recovery */
static const struct pci_error_handlers ipa_pci_err_handler = {
	.error_detected = ipa_pci_io_error_detected,
	.slot_reset = ipa_pci_io_slot_reset,
	.resume = ipa_pci_io_resume,
};

static struct pci_driver ipa_pci_driver = {
	.name     = ipa_pci_driver_name,
	.id_table = ipa_pci_tbl,
	.probe    = ipa_pci_probe,
	.remove   = ipa_pci_remove,
	.shutdown = ipa_pci_shutdown,
	.err_handler = &ipa_pci_err_handler
};

static int ipa_generic_plat_drv_probe(struct platform_device *pdev_p)
{
	int result;

	/*
	 * IPA probe function can be called for multiple times as the same probe
	 * function handles multiple compatibilities
	 */
	pr_debug("ipa: IPA driver probing started for %s\n",
		pdev_p->dev.of_node->name);

	if (!ipa_api_ctrl) {
		ipa_api_ctrl = kzalloc(sizeof(*ipa_api_ctrl), GFP_KERNEL);
		if (!ipa_api_ctrl)
			return -ENOMEM;

		/* Get IPA HW Version */
		result = of_property_read_u32(pdev_p->dev.of_node,
			"qcom,ipa-hw-ver", &ipa_api_hw_type);
		if ((result) || (ipa_api_hw_type == 0)) {
			pr_err("ipa: get resource failed for ipa-hw-ver!\n");
			kfree(ipa_api_ctrl);
			ipa_api_ctrl = 0;
			return -ENODEV;
		}
		pr_debug("ipa: ipa_api_hw_type = %d\n", ipa_api_hw_type);
	}

	/* call probe based on IPA HW version */
	switch (ipa_api_hw_type) {
	case IPA_HW_v3_0:
	case IPA_HW_v3_1:
	case IPA_HW_v3_5:
	case IPA_HW_v3_5_1:
	case IPA_HW_v4_0:
	case IPA_HW_v4_1:
	case IPA_HW_v4_2:
	case IPA_HW_v4_5:
	case IPA_HW_v4_7:
	case IPA_HW_v4_9:
		result = ipa3_plat_drv_probe(pdev_p, ipa_api_ctrl,
			ipa_plat_drv_match);
		break;
	default:
		pr_err("ipa: unsupported version %d\n", ipa_api_hw_type);
		return -EPERM;
	}

	if (result && result != -EPROBE_DEFER)
		pr_err("ipa: ipa_plat_drv_probe failed\n");

	return result;
}

static int ipa_ap_suspend(struct device *dev)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_ap_suspend, dev);

	return ret;
}

static int ipa_ap_resume(struct device *dev)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_ap_resume, dev);

	return ret;
}

/**
 * ipa_inc_client_enable_clks() - Increase active clients counter, and
 * enable ipa clocks if necessary
 *
 * Please do not use this API, use the wrapper macros instead (ipa_i.h)
 * IPA_ACTIVE_CLIENTS_INC_XXX();
 *
 * Return codes:
 * None
 */
void ipa_inc_client_enable_clks(struct ipa_active_client_logging_info *id)
{
	IPA_API_DISPATCH(ipa_inc_client_enable_clks, id);
}
EXPORT_SYMBOL(ipa_inc_client_enable_clks);

/**
 * ipa_dec_client_disable_clks() - Increase active clients counter, and
 * enable ipa clocks if necessary
 *
 * Please do not use this API, use the wrapper macros instead (ipa_i.h)
 * IPA_ACTIVE_CLIENTS_DEC_XXX();
 *
 * Return codes:
 * None
 */
void ipa_dec_client_disable_clks(struct ipa_active_client_logging_info *id)
{
	IPA_API_DISPATCH(ipa_dec_client_disable_clks, id);
}
EXPORT_SYMBOL(ipa_dec_client_disable_clks);

/**
 * ipa_inc_client_enable_clks_no_block() - Only increment the number of active
 * clients if no asynchronous actions should be done.Asynchronous actions are
 * locking a mutex and waking up IPA HW.
 *
 * Please do not use this API, use the wrapper macros instead(ipa_i.h)
 *
 *
 * Return codes : 0 for success
 *		-EPERM if an asynchronous action should have been done
 */
int ipa_inc_client_enable_clks_no_block(
	struct ipa_active_client_logging_info *id)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_inc_client_enable_clks_no_block, id);

	return ret;
}
EXPORT_SYMBOL(ipa_inc_client_enable_clks_no_block);

/**
 * ipa_suspend_resource_no_block() - suspend client endpoints related to the
 * IPA_RM resource and decrement active clients counter. This function is
 * guaranteed to avoid sleeping.
 *
 * @resource: [IN] IPA Resource Manager resource
 *
 * Return codes: 0 on success, negative on failure.
 */
int ipa_suspend_resource_no_block(enum ipa_rm_resource_name resource)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_suspend_resource_no_block, resource);

	return ret;
}
EXPORT_SYMBOL(ipa_suspend_resource_no_block);
/**
 * ipa_resume_resource() - resume client endpoints related to the IPA_RM
 * resource.
 *
 * @resource: [IN] IPA Resource Manager resource
 *
 * Return codes: 0 on success, negative on failure.
 */
int ipa_resume_resource(enum ipa_rm_resource_name resource)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_resume_resource, resource);

	return ret;
}
EXPORT_SYMBOL(ipa_resume_resource);

/**
 * ipa_suspend_resource_sync() - suspend client endpoints related to the IPA_RM
 * resource and decrement active clients counter, which may result in clock
 * gating of IPA clocks.
 *
 * @resource: [IN] IPA Resource Manager resource
 *
 * Return codes: 0 on success, negative on failure.
 */
int ipa_suspend_resource_sync(enum ipa_rm_resource_name resource)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_suspend_resource_sync, resource);

	return ret;
}
EXPORT_SYMBOL(ipa_suspend_resource_sync);

/**
 * ipa_set_required_perf_profile() - set IPA to the specified performance
 *	profile based on the bandwidth, unless minimum voltage required is
 *	higher. In this case the floor_voltage specified will be used.
 * @floor_voltage: minimum voltage to operate
 * @bandwidth_mbps: needed bandwidth from IPA
 *
 * Return codes: 0 on success, negative on failure.
 */
int ipa_set_required_perf_profile(enum ipa_voltage_level floor_voltage,
	u32 bandwidth_mbps)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_set_required_perf_profile, floor_voltage,
		bandwidth_mbps);

	return ret;
}
EXPORT_SYMBOL(ipa_set_required_perf_profile);

/**
 * ipa_get_ipc_logbuf() - return a pointer to IPA driver IPC log
 */
void *ipa_get_ipc_logbuf(void)
{
	void *ret;

	IPA_API_DISPATCH_RETURN_PTR(ipa_get_ipc_logbuf);

	return ret;
}
EXPORT_SYMBOL(ipa_get_ipc_logbuf);

/**
 * ipa_get_ipc_logbuf_low() - return a pointer to IPA driver IPC low prio log
 */
void *ipa_get_ipc_logbuf_low(void)
{
	void *ret;

	IPA_API_DISPATCH_RETURN_PTR(ipa_get_ipc_logbuf_low);

	return ret;
}
EXPORT_SYMBOL(ipa_get_ipc_logbuf_low);

/**
 * ipa_assert() - general function for assertion
 */
void ipa_assert(void)
{
	pr_err("IPA: unrecoverable error has occurred, asserting\n");
	BUG();
}
EXPORT_SYMBOL(ipa_assert);

/**
 * ipa_rx_poll() - Poll the rx packets from IPA HW in the
 * softirq context
 *
 * @budget: number of packets to be polled in single iteration
 *
 * Return codes: >= 0  : Actual number of packets polled
 *
 */
int ipa_rx_poll(u32 clnt_hdl, int budget)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_rx_poll, clnt_hdl, budget);

	return ret;
}
EXPORT_SYMBOL(ipa_rx_poll);

/**
 * ipa_recycle_wan_skb() - Recycle the Wan skb
 *
 * @skb: skb that needs to recycle
 *
 */
void ipa_recycle_wan_skb(struct sk_buff *skb)
{
	IPA_API_DISPATCH(ipa_recycle_wan_skb, skb);
}
EXPORT_SYMBOL(ipa_recycle_wan_skb);

/**
 * ipa_setup_uc_ntn_pipes() - setup uc offload pipes
 */
int ipa_setup_uc_ntn_pipes(struct ipa_ntn_conn_in_params *inp,
		ipa_notify_cb notify, void *priv, u8 hdr_len,
		struct ipa_ntn_conn_out_params *outp)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_setup_uc_ntn_pipes, inp,
		notify, priv, hdr_len, outp);

	return ret;
}
EXPORT_SYMBOL(ipa_setup_uc_ntn_pipes);

/**
 * ipa_tear_down_uc_offload_pipes() - tear down uc offload pipes
 */
int ipa_tear_down_uc_offload_pipes(int ipa_ep_idx_ul,
		int ipa_ep_idx_dl, struct ipa_ntn_conn_in_params *params)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_tear_down_uc_offload_pipes, ipa_ep_idx_ul,
		ipa_ep_idx_dl, params);

	return ret;
}
EXPORT_SYMBOL(ipa_tear_down_uc_offload_pipes);

/**
 * ipa_get_pdev() - return a pointer to IPA dev struct
 *
 * Return value: a pointer to IPA dev struct
 *
 */
struct device *ipa_get_pdev(void)
{
	struct device *ret;

	IPA_API_DISPATCH_RETURN_PTR(ipa_get_pdev);

	return ret;
}
EXPORT_SYMBOL(ipa_get_pdev);

int ipa_ntn_uc_reg_rdyCB(void (*ipauc_ready_cb)(void *user_data),
			      void *user_data)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_ntn_uc_reg_rdyCB,
				ipauc_ready_cb, user_data);

	return ret;
}
EXPORT_SYMBOL(ipa_ntn_uc_reg_rdyCB);

void ipa_ntn_uc_dereg_rdyCB(void)
{
	IPA_API_DISPATCH(ipa_ntn_uc_dereg_rdyCB);
}
EXPORT_SYMBOL(ipa_ntn_uc_dereg_rdyCB);

/**
 * ipa_conn_wdi_pipes() - connect wdi pipes
 */
int ipa_conn_wdi_pipes(struct ipa_wdi_conn_in_params *in,
	struct ipa_wdi_conn_out_params *out,
	ipa_wdi_meter_notifier_cb wdi_notify)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_conn_wdi_pipes, in, out, wdi_notify);

	return ret;
}
EXPORT_SYMBOL(ipa_conn_wdi_pipes);

/**
 * ipa_disconn_wdi_pipes() - disconnect wdi pipes
 */
int ipa_disconn_wdi_pipes(int ipa_ep_idx_tx, int ipa_ep_idx_rx)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_disconn_wdi_pipes, ipa_ep_idx_tx,
		ipa_ep_idx_rx);

	return ret;
}
EXPORT_SYMBOL(ipa_disconn_wdi_pipes);

/**
 * ipa_enable_wdi_pipes() - enable wdi pipes
 */
int ipa_enable_wdi_pipes(int ipa_ep_idx_tx, int ipa_ep_idx_rx)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_enable_wdi_pipes, ipa_ep_idx_tx,
		ipa_ep_idx_rx);

	return ret;
}
EXPORT_SYMBOL(ipa_enable_wdi_pipes);

/**
 * ipa_disable_wdi_pipes() - disable wdi pipes
 */
int ipa_disable_wdi_pipes(int ipa_ep_idx_tx, int ipa_ep_idx_rx)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_disable_wdi_pipes, ipa_ep_idx_tx,
		ipa_ep_idx_rx);

	return ret;
}
EXPORT_SYMBOL(ipa_disable_wdi_pipes);

/**
 * ipa_wigig_uc_msi_init() - smmu map\unmap msi related wigig HW registers
 *	and init\deinit uC msi config
 */
int ipa_wigig_uc_msi_init(bool init,
	phys_addr_t periph_baddr_pa,
	phys_addr_t pseudo_cause_pa,
	phys_addr_t int_gen_tx_pa,
	phys_addr_t int_gen_rx_pa,
	phys_addr_t dma_ep_misc_pa)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_wigig_uc_msi_init, init,
		periph_baddr_pa,
		pseudo_cause_pa,
		int_gen_tx_pa,
		int_gen_rx_pa,
		dma_ep_misc_pa);

	return ret;
}
EXPORT_SYMBOL(ipa_wigig_uc_msi_init);

/**
 * ipa_conn_wigig_rx_pipe_i() - connect wigig rx pipe
 */
int ipa_conn_wigig_rx_pipe_i(void *in, struct ipa_wigig_conn_out_params *out,
	struct dentry **parent)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_conn_wigig_rx_pipe_i, in, out, parent);

	return ret;
}
EXPORT_SYMBOL(ipa_conn_wigig_rx_pipe_i);

/**
 * ipa_conn_wigig_client_i() - connect a wigig client
 */
int ipa_conn_wigig_client_i(void *in,
	struct ipa_wigig_conn_out_params *out,
	ipa_notify_cb tx_notify,
	void *priv)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_conn_wigig_client_i, in, out,
		tx_notify, priv);

	return ret;
}
EXPORT_SYMBOL(ipa_conn_wigig_client_i);

/**
 * ipa_disconn_wigig_pipe_i() - disconnect a wigig pipe
 */
int ipa_disconn_wigig_pipe_i(enum ipa_client_type client,
	struct ipa_wigig_pipe_setup_info_smmu *pipe_smmu,
	void *dbuff)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_disconn_wigig_pipe_i, client,
		pipe_smmu, dbuff);

	return ret;
}
EXPORT_SYMBOL(ipa_disconn_wigig_pipe_i);

/**
 * ipa_enable_wigig_pipe() - enable a wigig pipe
 */
int ipa_enable_wigig_pipe_i(enum ipa_client_type client)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_enable_wigig_pipe_i, client);

	return ret;
}
EXPORT_SYMBOL(ipa_enable_wigig_pipe_i);

/**
 * ipa_disable_wigig_pipe_i() - disable a wigig pipe
 */
int ipa_disable_wigig_pipe_i(enum ipa_client_type client)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_disable_wigig_pipe_i, client);

	return ret;
}
EXPORT_SYMBOL(ipa_disable_wigig_pipe_i);

/**
 * ipa_tz_unlock_reg() - Allow AP access to memory regions controlled by TZ
 */
int ipa_tz_unlock_reg(struct ipa_tz_unlock_reg_info *reg_info, u16 num_regs)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_tz_unlock_reg, reg_info, num_regs);

	return ret;
}

void ipa_register_client_callback(int (*client_cb)(bool is_lock),
				bool (*teth_port_state)(void),
					enum ipa_client_type client)
{
	IPA_API_DISPATCH(ipa_register_client_callback,
		client_cb, teth_port_state, client);
}
EXPORT_SYMBOL(ipa_register_client_callback);

void ipa_deregister_client_callback(enum ipa_client_type client)
{
	IPA_API_DISPATCH(ipa_deregister_client_callback,
		client);
}
EXPORT_SYMBOL(ipa_deregister_client_callback);

int ipa_uc_debug_stats_alloc(
	struct IpaHwOffloadStatsAllocCmdData_t cmdinfo)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_uc_debug_stats_alloc, cmdinfo);

	return ret;
}
EXPORT_SYMBOL(ipa_uc_debug_stats_alloc);

int ipa_uc_debug_stats_dealloc(uint32_t prot_id)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_uc_debug_stats_dealloc, prot_id);

	return ret;
}
EXPORT_SYMBOL(ipa_uc_debug_stats_dealloc);

void ipa_get_gsi_stats(int prot_id,
	struct ipa_uc_dbg_ring_stats *stats)
{
	IPA_API_DISPATCH(ipa_get_gsi_stats,
		prot_id, stats);
}
EXPORT_SYMBOL(ipa_get_gsi_stats);

int ipa_get_prot_id(enum ipa_client_type client)
{
	int ret;

	IPA_API_DISPATCH_RETURN(ipa_get_prot_id,
		client);

	return ret;
}
EXPORT_SYMBOL(ipa_get_prot_id);

static const struct dev_pm_ops ipa_pm_ops = {
	.suspend_noirq = ipa_ap_suspend,
	.resume_noirq = ipa_ap_resume,
};

static struct platform_driver ipa_plat_drv = {
	.probe = ipa_generic_plat_drv_probe,
	.driver = {
		.name = DRV_NAME,
		.pm = &ipa_pm_ops,
		.of_match_table = ipa_plat_drv_match,
	},
};

/*********************************************************/
/*                PCIe Version                           */
/*********************************************************/

static int ipa_pci_probe(
	struct pci_dev             *pci_dev,
	const struct pci_device_id *ent)
{
	int result;

	if (!pci_dev || !ent) {
		pr_err(
		    "Bad arg: pci_dev (%pK) and/or ent (%pK)\n",
		    pci_dev, ent);
		return -EOPNOTSUPP;
	}

	if (!ipa_api_ctrl) {
		ipa_api_ctrl = kzalloc(sizeof(*ipa_api_ctrl), GFP_KERNEL);
		if (ipa_api_ctrl == NULL)
			return -ENOMEM;
		/* Get IPA HW Version */
		result = of_property_read_u32(NULL,
			"qcom,ipa-hw-ver", &ipa_api_hw_type);
		if (result || ipa_api_hw_type == 0) {
			pr_err("ipa: get resource failed for ipa-hw-ver!\n");
			kfree(ipa_api_ctrl);
			ipa_api_ctrl = NULL;
			return -ENODEV;
		}
		pr_debug("ipa: ipa_api_hw_type = %d\n", ipa_api_hw_type);
	}

	/*
	 * Call a reduced version of platform_probe appropriate for PCIe
	 */
	result = ipa3_pci_drv_probe(pci_dev, ipa_api_ctrl, ipa_pci_drv_match);

	if (result && result != -EPROBE_DEFER)
		pr_err("ipa: ipa3_pci_drv_probe failed\n");

	return result;
}

static void ipa_pci_remove(struct pci_dev *pci_dev)
{
}

static void ipa_pci_shutdown(struct pci_dev *pci_dev)
{
}

static pci_ers_result_t ipa_pci_io_error_detected(struct pci_dev *pci_dev,
	pci_channel_state_t state)
{
	return 0;
}

static pci_ers_result_t ipa_pci_io_slot_reset(struct pci_dev *pci_dev)
{
	return 0;
}

static void ipa_pci_io_resume(struct pci_dev *pci_dev)
{
}

static int __init ipa_module_init(void)
{
	pr_debug("IPA module init\n");

	ipa3_ctx = kzalloc(sizeof(*ipa3_ctx), GFP_KERNEL);
	if (!ipa3_ctx) {
		return -ENOMEM;
	}
	mutex_init(&ipa3_ctx->lock);

	if (running_emulation) {
		/* Register as a PCI device driver */
		return pci_register_driver(&ipa_pci_driver);
	}
	/* Register as a platform device driver */
	return platform_driver_register(&ipa_plat_drv);
}
subsys_initcall(ipa_module_init);

static void __exit ipa_module_exit(void)
{
	if (running_emulation)
		pci_unregister_driver(&ipa_pci_driver);
	platform_driver_unregister(&ipa_plat_drv);
	kfree(ipa3_ctx);
	ipa3_ctx = NULL;
}
module_exit(ipa_module_exit);

MODULE_SOFTDEP("pre: subsys-pil-tz");
MODULE_SOFTDEP("pre: qcom-arm-smmu-mod");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("IPA HW device driver");
