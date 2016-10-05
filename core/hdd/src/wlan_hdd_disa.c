/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC : wlan_hdd_disa.c
 *
 * WLAN Host Device Driver file for DISA certification
 *
 */

#include "wlan_hdd_disa.h"
#include "sme_api.h"

#define ENCRYPT_DECRYPT_CONTEXT_MAGIC 0x4475354
#define WLAN_WAIT_TIME_ENCRYPT_DECRYPT 1000


/**
 * hdd_encrypt_decrypt_msg_context - hdd encrypt/decrypt message context
 *
 * @magic: magic number
 * @completion: Completion variable for encrypt/decrypt message
 * @response_event: encrypt/decrypt message request wait event
 */
struct hdd_encrypt_decrypt_msg_context {
	unsigned int magic;
	struct completion completion;
	struct sir_encrypt_decrypt_rsp_params response;
};
static struct hdd_encrypt_decrypt_msg_context encrypt_decrypt_msg_context;

/**
 * hdd_encrypt_decrypt_msg_cb () - sends encrypt/decrypt data to user space
 * @encrypt_decrypt_rsp_params: encrypt/decrypt response parameters
 *
 * Return: none
 */
static void hdd_encrypt_decrypt_msg_cb(void *hdd_context,
	struct sir_encrypt_decrypt_rsp_params *encrypt_decrypt_rsp_params)
{
	hdd_context_t *hdd_ctx = hdd_context;
	int ret;
	struct hdd_encrypt_decrypt_msg_context *context;

	ENTER();

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return;

	if (!encrypt_decrypt_rsp_params) {
		hdd_err("rsp params is NULL");
		return;
	}

	print_hex_dump(KERN_INFO, "Data in hdd_encrypt_decrypt_msg_cb: ",
		DUMP_PREFIX_NONE, 16, 1,
		encrypt_decrypt_rsp_params->data,
		encrypt_decrypt_rsp_params->data_length, 0);

	hdd_err("vdev_id %d,status %d data_length %d",
		encrypt_decrypt_rsp_params->vdev_id,
		encrypt_decrypt_rsp_params->status,
		encrypt_decrypt_rsp_params->data_length);

	spin_lock(&hdd_context_lock);

	context = &encrypt_decrypt_msg_context;
	/* The caller presumably timed out so there is nothing we can do */
	if (context->magic != ENCRYPT_DECRYPT_CONTEXT_MAGIC) {
		spin_unlock(&hdd_context_lock);
		return;
	}

	/* context is valid so caller is still waiting */
	context->response = *encrypt_decrypt_rsp_params;

	if (encrypt_decrypt_rsp_params->data_length) {
		context->response.data =
			qdf_mem_malloc(sizeof(uint8_t) *
				encrypt_decrypt_rsp_params->data_length);
		if (context->response.data == NULL) {
			hdd_err("cdf_mem_alloc failed for data");
			spin_unlock(&hdd_context_lock);
			return;
		}
		qdf_mem_copy(context->response.data,
			encrypt_decrypt_rsp_params->data,
			encrypt_decrypt_rsp_params->data_length);
	}

	/*
	 * Indicate to calling thread that
	 * response data is available
	 */
	context->magic = 0;

	complete(&context->completion);

	spin_unlock(&hdd_context_lock);


	EXIT();
}


/**
 * hdd_encrypt_decrypt_msg_cb () - sends encrypt/decrypt data to user space
 * @encrypt_decrypt_rsp_params: encrypt/decrypt response parameters
 *
 * Return: none
 */
static int hdd_post_encrypt_decrypt_msg_rsp(hdd_context_t *hdd_ctx,
	struct sir_encrypt_decrypt_rsp_params *encrypt_decrypt_rsp_params)
{
	struct sk_buff *skb;
	uint32_t nl_buf_len;

	ENTER();

	nl_buf_len = encrypt_decrypt_rsp_params->data_length + NLA_HDRLEN;

	skb = cfg80211_vendor_cmd_alloc_reply_skb(hdd_ctx->wiphy, nl_buf_len);
	if (!skb) {
		hdd_err(FL("cfg80211_vendor_cmd_alloc_reply_skb failed"));
		return -ENOMEM;
	}

	if (encrypt_decrypt_rsp_params->data_length) {
		if (nla_put(skb, QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_DATA,
				encrypt_decrypt_rsp_params->data_length,
				encrypt_decrypt_rsp_params->data)) {
			hdd_err(FL("put fail"));
			goto nla_put_failure;
		}
	}

	cfg80211_vendor_cmd_reply(skb);
	EXIT();
	return 0;

nla_put_failure:
	kfree_skb(skb);
	return -EINVAL;
}

/**
 * hdd_fill_encrypt_decrypt_params () - parses data from user space
 * and fills encrypt/decrypt parameters
 * @encrypt_decrypt_params: encrypt/decrypt request parameters
 * @adapter : adapter context
 * @data: Pointer to data
 * @data_len: Data length
 *
 Return: 0 on success, negative errno on failure
 */
static int hdd_fill_encrypt_decrypt_params(struct encrypt_decrypt_req_params
						*encrypt_decrypt_params,
						hdd_adapter_t *adapter,
						const void *data,
						int data_len)
{
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_MAX + 1];
	uint8_t len, mac_hdr_len;
	uint8_t *tmp;
	uint8_t fc[2];

	if (nla_parse(tb, QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_MAX,
		      data, data_len, NULL)) {
		hdd_err("Invalid ATTR");
		return -EINVAL;
	}

	encrypt_decrypt_params->vdev_id = adapter->sessionId;
	hdd_err("vdev_id : %d", encrypt_decrypt_params->vdev_id);

	if (!tb[QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_NEEDS_DECRYPTION]) {
		hdd_err("attr flag NEEDS_DECRYPTION not present");
		encrypt_decrypt_params->key_flag = WMI_ENCRYPT;
	} else {
		hdd_err("attr flag NEEDS_DECRYPTION present");
		encrypt_decrypt_params->key_flag = WMI_DECRYPT;
	}
	hdd_err("Key flag : %d", encrypt_decrypt_params->key_flag);

	if (!tb[QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_KEYID]) {
		hdd_err("attr key id failed");
		return -EINVAL;
	}
	encrypt_decrypt_params->key_idx = nla_get_u8(tb
		    [QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_KEYID]);
	hdd_err("Key Idx : %d", encrypt_decrypt_params->key_idx);

	if (!tb[QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_CIPHER]) {
		hdd_err("attr Cipher failed");
		return -EINVAL;
	}
	encrypt_decrypt_params->key_cipher = nla_get_u32(tb
		    [QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_CIPHER]);
	hdd_err("key_cipher : %d", encrypt_decrypt_params->key_cipher);

	if (!tb[QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_TK]) {
		hdd_err("attr TK failed");
		return -EINVAL;
	}
	encrypt_decrypt_params->key_len =
		nla_len(tb[QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_TK]);
	if (!encrypt_decrypt_params->key_len) {
		hdd_err("Invalid TK length");
		return -EINVAL;
	}
	hdd_err("Key len : %d", encrypt_decrypt_params->key_len);

	if (encrypt_decrypt_params->key_len > SIR_MAC_MAX_KEY_LENGTH)
		encrypt_decrypt_params->key_len = SIR_MAC_MAX_KEY_LENGTH;

	tmp = nla_data(tb[QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_TK]);

	qdf_mem_copy(encrypt_decrypt_params->key_data, tmp,
			encrypt_decrypt_params->key_len);

	print_hex_dump(KERN_INFO, "Key : ", DUMP_PREFIX_NONE, 16, 1,
			&encrypt_decrypt_params->key_data,
			encrypt_decrypt_params->key_len, 0);

	if (!tb[QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_PN]) {
		hdd_err("attr PN failed");
		return -EINVAL;
	}
	len = nla_len(tb[QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_PN]);
	if (!len) {
		hdd_err("Invalid PN length");
		return -EINVAL;
	}

	tmp = nla_data(tb[QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_PN]);

	qdf_mem_copy(encrypt_decrypt_params->pn, tmp, len);

	print_hex_dump(KERN_INFO, "PN received : ", DUMP_PREFIX_NONE, 16, 1,
			&encrypt_decrypt_params->pn, len, 0);

	if (!tb[QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_DATA]) {
		hdd_err("attr header failed");
		return -EINVAL;
	}
	len = nla_len(tb[QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_DATA]);
	if (!len) {
		hdd_err("Invalid header and payload length");
		return -EINVAL;
	}

	hdd_err("Header and Payload length %d ", len);

	tmp = nla_data(tb[QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_DATA]);

	print_hex_dump(KERN_INFO, "Header and Payload received: ",
			DUMP_PREFIX_NONE, 16, 1,
			tmp, len, 0);

	mac_hdr_len = MIN_MAC_HEADER_LEN;

	/*
	 * Check to find out address 4. Address 4 is present if ToDS and FromDS
	 * are 1 and data representation is little endian.
	 */
	fc[1] = *tmp;
	fc[0] = *(tmp + 1);
	if ((fc[0] & 0x03) == 0x03) {
		hdd_err("Address 4 is present");
		mac_hdr_len += IEEE80211_ADDR_LEN;
	}

	/*
	 * Check to find out Qos control field. Qos control field is present
	 * if msb of subtype field is 1 and data representation is
	 * little endian.
	 */
	if (fc[1] & 0x80) {
		hdd_err("Qos control is present");
		mac_hdr_len += QOS_CONTROL_LEN;
	}

	hdd_err("mac_hdr_len %d", mac_hdr_len);

	qdf_mem_copy(encrypt_decrypt_params->mac_header,
			tmp, mac_hdr_len);

	print_hex_dump(KERN_INFO, "Header received in request: ",
			DUMP_PREFIX_NONE, 16, 1,
			encrypt_decrypt_params->mac_header,
			mac_hdr_len, 0);

	encrypt_decrypt_params->data_len =
			len - mac_hdr_len;

	hdd_err("Payload length : %d", encrypt_decrypt_params->data_len);

	if (encrypt_decrypt_params->data_len) {
		encrypt_decrypt_params->data =
			qdf_mem_malloc(sizeof(uint8_t) *
				encrypt_decrypt_params->data_len);

		if (encrypt_decrypt_params->data == NULL) {
			hdd_err("cdf_mem_alloc failed for data");
			return -ENOMEM;
		}

		qdf_mem_copy(encrypt_decrypt_params->data,
			tmp + mac_hdr_len,
			encrypt_decrypt_params->data_len);

		print_hex_dump(KERN_INFO, "Data received in request: ",
			DUMP_PREFIX_NONE, 16, 1,
			encrypt_decrypt_params->data,
			encrypt_decrypt_params->data_len, 0);
	}

	return 0;
}

/**
 * hdd_encrypt_decrypt_msg () - process encrypt/decrypt message
 * @adapter : adapter context
 * @hdd_ctx: hdd context
 * @data: Pointer to data
 * @data_len: Data length
 *
 Return: 0 on success, negative errno on failure
 */
static int hdd_encrypt_decrypt_msg(hdd_adapter_t *adapter,
						hdd_context_t *hdd_ctx,
						const void *data,
						int data_len)
{
	struct encrypt_decrypt_req_params encrypt_decrypt_params = {0};
	QDF_STATUS qdf_status;
	int ret;
	struct hdd_encrypt_decrypt_msg_context *context;
	unsigned long rc;

	ret = hdd_fill_encrypt_decrypt_params(&encrypt_decrypt_params,
				adapter, data, data_len);
	if (ret)
		return ret;

	spin_lock(&hdd_context_lock);
	context = &encrypt_decrypt_msg_context;
	context->magic = ENCRYPT_DECRYPT_CONTEXT_MAGIC;
	INIT_COMPLETION(context->completion);
	spin_unlock(&hdd_context_lock);

	qdf_status = sme_encrypt_decrypt_msg(hdd_ctx->hHal,
					&encrypt_decrypt_params);

	qdf_mem_free(encrypt_decrypt_params.data);

	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		hdd_err("Unable to post encrypt/decrypt message");
		return -EINVAL;
	}

	rc = wait_for_completion_timeout(&context->completion,
			msecs_to_jiffies(WLAN_WAIT_TIME_ENCRYPT_DECRYPT));

	spin_lock(&hdd_context_lock);
	if (!rc && (context->magic ==
			ENCRYPT_DECRYPT_CONTEXT_MAGIC)) {
		hdd_err("Target response timed out");
		context->magic = 0;
		spin_unlock(&hdd_context_lock);
		return -ETIMEDOUT;
	}

	spin_unlock(&hdd_context_lock);
	ret = hdd_post_encrypt_decrypt_msg_rsp(hdd_ctx,
				&encrypt_decrypt_msg_context.response);
	if (ret)
		hdd_err("Failed to post encrypt/decrypt message response");

	qdf_mem_free(encrypt_decrypt_msg_context.response.data);

	EXIT();
	return ret;
}

/**
 * hdd_encrypt_decrypt_init () - exposes encrypt/decrypt initialization
 * functionality
 * @hdd_ctx: hdd context
 *
 Return: 0 on success, negative errno on failure
 */
int hdd_encrypt_decrypt_init(hdd_context_t *hdd_ctx)
{
	QDF_STATUS status;

	init_completion(&encrypt_decrypt_msg_context.completion);

	status = sme_encrypt_decrypt_msg_register_callback(hdd_ctx->hHal,
					hdd_encrypt_decrypt_msg_cb);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("encrypt/decrypt callback failed %d", status);
		return -EINVAL;
	}
	return 0;
}

/**
 * hdd_encrypt_decrypt_deinit () - exposes encrypt/decrypt deinitialization
 * functionality
 * @hdd_ctx: hdd context
 *
 Return: 0 on success, negative errno on failure
 */
int hdd_encrypt_decrypt_deinit(hdd_context_t *hdd_ctx)
{
	QDF_STATUS status;

	status = sme_encrypt_decrypt_msg_deregister_callback(hdd_ctx->hHal);
	if (!QDF_IS_STATUS_SUCCESS(status))
		hdd_err("De-register encrypt/decrypt callback failed: %d",
			status);
	return 0;
}

/**
 * __wlan_hdd_cfg80211_encrypt_decrypt_msg () - Encrypt/Decrypt msg
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: 0 on success, negative errno on failure
 */
static int __wlan_hdd_cfg80211_encrypt_decrypt_msg(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len)
{
	hdd_context_t *hdd_ctx = wiphy_priv(wiphy);
	struct net_device *dev = wdev->netdev;
	hdd_adapter_t *adapter = NULL;
	int ret;

	ENTER_DEV(dev);

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	adapter = WLAN_HDD_GET_PRIV_PTR(dev);

	ret = hdd_encrypt_decrypt_msg(adapter, hdd_ctx, data, data_len);

	return ret;
}

/**
 * wlan_hdd_cfg80211_encrypt_decrypt_msg () - Encrypt/Decrypt msg
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: 0 on success, negative errno on failure
 */
int wlan_hdd_cfg80211_encrypt_decrypt_msg(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data,
						int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_encrypt_decrypt_msg(wiphy, wdev,
						data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}
