/*
 * Copyright (c) 2011,2017-2018 The Linux Foundation. All rights reserved.
 *
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

#include <osdep.h>
#include <wlan_tgt_def_config.h>
#include <hif.h>
#include <hif_hw_version.h>
#include <wmi_unified_api.h>
#include <target_if_spectral.h>
#include <wlan_lmac_if_def.h>
#include <wlan_osif_priv.h>
#include <reg_services_public_struct.h>

#ifdef SPECTRAL_USE_NETLINK_SOCKETS
struct sock *target_if_spectral_nl_sock;
static atomic_t spectral_nl_users = ATOMIC_INIT(0);

#if (KERNEL_VERSION(2, 6, 31) > LINUX_VERSION_CODE)
void
target_if_spectral_nl_data_ready(struct sock *sk, int len)
{
	spectral_debug("%d", __LINE__);
}

#else
void
target_if_spectral_nl_data_ready(struct sk_buff *skb)
{
	spectral_debug("%d", __LINE__);
}
#endif				/* VERSION */

/**
 * target_if_spectral_init_nl_cfg() - Initialize netlink kernel
 * configuration parameters
 * @spectral : Pointer to netlink_kernel_cfg
 *
 * Initialize netlink kernel configuration parameters required
 * for spectral module
 *
 * Return: None
 */
#if KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE
static void
target_if_spectral_init_nl_cfg(struct netlink_kernel_cfg *cfg)
{
	cfg->groups = 1;
	cfg->input = target_if_spectral_nl_data_ready;
}
#else
static void
target_if_spectral_init_nl_cfg(struct netlink_kernel_cfg *cfg)
{
}
#endif

/**
 * target_if_spectral_create_nl_sock() - Create Netlink socket
 * @spectral : Pointer to netlink_kernel_cfg
 *
 * Create Netlink socket required for spectral module
 *
 * Return: None
 */
#if KERNEL_VERSION(3, 7, 0) <= LINUX_VERSION_CODE
static void
target_if_spectral_create_nl_sock(struct netlink_kernel_cfg *cfg)
{
	target_if_spectral_nl_sock =
	    (struct sock *)netlink_kernel_create(&init_net,
						 NETLINK_ATHEROS, cfg);
}
#elif KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE
static void
target_if_spectral_create_nl_sock(struct netlink_kernel_cfg *cfg)
{
	target_if_spectral_nl_sock =
	    (struct sock *)netlink_kernel_create(&init_net,
						 NETLINK_ATHEROS,
						 THIS_MODULE, cfg);
}
#elif (KERNEL_VERSION(2, 6, 31) > LINUX_VERSION_CODE)
static void
target_if_spectral_create_nl_sock(struct netlink_kernel_cfg *cfg)
{
	target_if_spectral_nl_sock =
	    (struct sock *)netlink_kernel_create(
		NETLINK_ATHEROS, 1,
		&target_if_spectral_nl_data_ready,
		THIS_MODULE);
}
#else
#if (KERNEL_VERSION(3, 10, 0) <= LINUX_VERSION_CODE)
static void
target_if_spectral_create_nl_sock(struct netlink_kernel_cfg *cfg)
{
	memset(cfg, 0, sizeof(*cfg));
	cfg->groups = 1;
	cfg->input = &target_if_spectral_nl_data_ready;
	target_if_spectral_nl_sock =
	    (struct sock *)netlink_kernel_create(&init_net,
						 NETLINK_ATHEROS, cfg);
}
#else
static void
target_if_spectral_create_nl_sock(struct netlink_kernel_cfg *cfg)
{
	target_if_spectral_nl_sock =
	    (struct sock *)netlink_kernel_create(
		&init_net,
		NETLINK_ATHEROS, 1,
		&target_if_spectral_nl_data_ready,
		NULL, THIS_MODULE);
}
#endif
#endif

int
target_if_spectral_init_netlink(struct target_if_spectral *spectral)
{
	struct netlink_kernel_cfg cfg;

	memset(&cfg, 0, sizeof(cfg));

	target_if_spectral_init_nl_cfg(&cfg);

	if (!spectral) {
		spectral_err("sc_spectral is NULL");
		return -EIO;
	}

	spectral->spectral_sent_msg = 0;

	if (!target_if_spectral_nl_sock) {
		target_if_spectral_create_nl_sock(&cfg);

		if (!target_if_spectral_nl_sock) {
			spectral_err("NETLINK_KERNEL_CREATE FAILED");
			return -ENODEV;
		}
	}
	atomic_inc(&spectral_nl_users);
	spectral->spectral_sock = target_if_spectral_nl_sock;

	if ((!spectral) || (!spectral->spectral_sock)) {
		spectral_err("NULL pointers (spectral=%d) (sock=%d)",
			     (!spectral),
			     (!spectral->spectral_sock));
		return -ENODEV;
	}
	if (!spectral->spectral_skb)
		spectral_err("%d NULL SKB", __LINE__);

	return 0;
}

int
target_if_spectral_destroy_netlink(struct target_if_spectral *spectral)
{
	spectral->spectral_sock = NULL;
	if (atomic_dec_and_test(&spectral_nl_users)) {
		sock_release(target_if_spectral_nl_sock->sk_socket);
		target_if_spectral_nl_sock = NULL;
	}
	return 0;
}

#endif				/* SPECTRAL_USE_NETLINK_SOCKETS */

static void
target_if_spectral_process_noise_pwr_report(
	struct target_if_spectral *spectral,
	const struct spectral_samp_msg *spec_samp_msg)
{
	int i, done;

	qdf_spin_lock(&spectral->noise_pwr_reports_lock);

	if (!spectral->noise_pwr_reports_reqd) {
		qdf_spin_unlock(&spectral->noise_pwr_reports_lock);
		return;
	}

	if (spectral->noise_pwr_reports_recv <
	    spectral->noise_pwr_reports_reqd) {
		spectral->noise_pwr_reports_recv++;

		for (i = 0; i < HOST_MAX_ANTENNA; i++) {
			uint32_t index;

			if (spectral->noise_pwr_chain_ctl[i]) {
				index =
				    spectral->noise_pwr_chain_ctl[i]->
				    rptcount++;
				spectral->noise_pwr_chain_ctl[i]->pwr[index] =
				    spec_samp_msg->samp_data.
				    spectral_chain_ctl_rssi[i];
			}
			if (spectral->noise_pwr_chain_ext[i]) {
				index =
				    spectral->noise_pwr_chain_ext[i]->
				    rptcount++;
				spectral->noise_pwr_chain_ext[i]->pwr[index] =
				    spec_samp_msg->samp_data.
				    spectral_chain_ext_rssi[i];
			}
		}
	}

	done = (spectral->noise_pwr_reports_recv >=
		spectral->noise_pwr_reports_reqd);

	qdf_spin_unlock(&spectral->noise_pwr_reports_lock);

	if (done) {
		qdf_spin_lock(&spectral->spectral_lock);
		target_if_stop_spectral_scan(spectral->pdev_obj);
		spectral->sc_spectral_scan = 0;
		qdf_spin_unlock(&spectral->spectral_lock);
	}
}

/*
 * Function     : spectral_create_samp_msg
 * Description  : create SAMP message and send it host
 * Input        :
 * Output       :
 *
 */

#ifdef SPECTRAL_CLASSIFIER_IN_KERNEL
static void
target_if_spectral_init_interf_list(
	struct spectral_samp_data *data,
	struct target_if_samp_msg_params *params)
{
	if (params->interf_list.count)
		OS_MEMCPY(&data->interf_list,
			  &params->interf_list, sizeof(struct interf_src_rsp));
	else
		data->interf_list.count = 0;
}
#else
static void
target_if_spectral_init_interf_list(
	struct spectral_samp_data *data,
	struct target_if_samp_msg_params *params)
{
	data->interf_list.count = 0;
}
#endif

void
target_if_spectral_create_samp_msg(struct target_if_spectral *spectral,
				   struct target_if_samp_msg_params *params)
{
	/*
	 * XXX : Non-Rentrant. Will be an issue with dual concurrent
	 *       operation on multi-processor system
	 */

	int temp_samp_msg_len = 0;

	static struct spectral_samp_msg spec_samp_msg;

	struct spectral_samp_msg *msg = NULL;
	struct spectral_samp_data *data = NULL;
	uint8_t *bin_pwr_data = NULL;
	struct spectral_classifier_params *cp = NULL;
	struct spectral_classifier_params *pcp = NULL;
	struct target_if_spectral_ops *p_sops = NULL;
	struct target_if_spectral_skb_event *sp_skb_event = NULL;

#ifdef SPECTRAL_USE_NETLINK_SOCKETS
	static int samp_msg_index;
#endif

	p_sops = GET_TARGET_IF_SPECTRAL_OPS(spectral);

	temp_samp_msg_len = sizeof(struct spectral_samp_msg) -
	    (MAX_NUM_BINS * sizeof(uint8_t));
	temp_samp_msg_len += (params->pwr_count * sizeof(uint8_t));
	if (spectral->ch_width == CH_WIDTH_160MHZ)
		temp_samp_msg_len +=
		    (params->pwr_count_sec80 * sizeof(uint8_t));
	bin_pwr_data = params->bin_pwr_data;

	memset(&spec_samp_msg, 0, sizeof(struct spectral_samp_msg));

	data = &spec_samp_msg.samp_data;

	spec_samp_msg.signature = SPECTRAL_SIGNATURE;
	spec_samp_msg.freq = params->freq;
	spec_samp_msg.freq_loading = params->freq_loading;
	spec_samp_msg.samp_data.spectral_data_len = params->datalen;
	spec_samp_msg.samp_data.spectral_rssi = params->rssi;
	spec_samp_msg.samp_data.ch_width = spectral->ch_width;

	spec_samp_msg.samp_data.spectral_combined_rssi =
	    (uint8_t)params->rssi;
	spec_samp_msg.samp_data.spectral_upper_rssi = params->upper_rssi;
	spec_samp_msg.samp_data.spectral_lower_rssi = params->lower_rssi;

	OS_MEMCPY(spec_samp_msg.samp_data.spectral_chain_ctl_rssi,
		  params->chain_ctl_rssi, sizeof(params->chain_ctl_rssi));
	OS_MEMCPY(spec_samp_msg.samp_data.spectral_chain_ext_rssi,
		  params->chain_ext_rssi, sizeof(params->chain_ext_rssi));

	spec_samp_msg.samp_data.spectral_bwinfo = params->bwinfo;
	spec_samp_msg.samp_data.spectral_tstamp = params->tstamp;
	spec_samp_msg.samp_data.spectral_max_index = params->max_index;

	/* Classifier in user space needs access to these */
	spec_samp_msg.samp_data.spectral_lower_max_index =
	    params->max_lower_index;
	spec_samp_msg.samp_data.spectral_upper_max_index =
	    params->max_upper_index;
	spec_samp_msg.samp_data.spectral_nb_lower = params->nb_lower;
	spec_samp_msg.samp_data.spectral_nb_upper = params->nb_upper;
	spec_samp_msg.samp_data.spectral_last_tstamp = params->last_tstamp;
	spec_samp_msg.samp_data.spectral_max_mag = params->max_mag;
	spec_samp_msg.samp_data.bin_pwr_count = params->pwr_count;
	spec_samp_msg.samp_data.lb_edge_extrabins = spectral->lb_edge_extrabins;
	spec_samp_msg.samp_data.rb_edge_extrabins = spectral->rb_edge_extrabins;
	spec_samp_msg.samp_data.spectral_combined_rssi = params->rssi;
	spec_samp_msg.samp_data.spectral_max_scale = params->max_exp;

#ifdef SPECTRAL_USE_NETLINK_SOCKETS

	/*
	 * This is a dirty hack to get the Windows build pass.
	 * Currently Windows and Linux builds source spectral_data.h
	 * form two different place. The windows version do not
	 * have noise_floor member in it.
	 *
	 * As a temp workaround this variable is set under the
	 * SPECTRAL_USE_NETLINK_SOCKETS as this is called only
	 * under the linux build and this saves the day
	 *
	 * The plan to sync of header files in under the way
	 *
	 */

	spec_samp_msg.samp_data.noise_floor = params->noise_floor;
#endif				/* SPECTRAL_USE_NETLINK_SOCKETS */

	/* Classifier in user space needs access to these */
	cp = &spec_samp_msg.samp_data.classifier_params;
	pcp = &params->classifier_params;

	OS_MEMCPY(cp, pcp, sizeof(struct spectral_classifier_params));

	SPECTRAL_MESSAGE_COPY_CHAR_ARRAY(&data->bin_pwr[0],
					 bin_pwr_data, params->pwr_count);

#ifdef SPECTRAL_USE_NETLINK_SOCKETS
	spec_samp_msg.vhtop_ch_freq_seg1 = params->vhtop_ch_freq_seg1;
	spec_samp_msg.vhtop_ch_freq_seg2 = params->vhtop_ch_freq_seg2;

	if (spectral->ch_width == CH_WIDTH_160MHZ) {
		spec_samp_msg.samp_data.spectral_rssi_sec80 =
		    params->rssi_sec80;
		spec_samp_msg.samp_data.noise_floor_sec80 =
		    params->noise_floor_sec80;

		spec_samp_msg.samp_data.spectral_data_len_sec80 =
		    params->datalen_sec80;
		spec_samp_msg.samp_data.spectral_max_index_sec80 =
		    params->max_index_sec80;
		spec_samp_msg.samp_data.spectral_max_mag_sec80 =
		    params->max_mag_sec80;
		spec_samp_msg.samp_data.bin_pwr_count_sec80 =
		    params->pwr_count_sec80;
		SPECTRAL_MESSAGE_COPY_CHAR_ARRAY(&data->bin_pwr_sec80[0],
						 (params->bin_pwr_data_sec80),
						 params->pwr_count_sec80);

		/*
		 * Note: REVERSE_ORDER is not a known use case for
		 * secondary 80 data at this point.
		 */
	}
#endif				/* SPECTRAL_USE_NETLINK_SOCKETS */

	target_if_spectral_init_interf_list(data, params);

#ifdef SPECTRAL_USE_NETLINK_SOCKETS
	target_if_spectral_prep_skb(spectral);
	if (spectral->spectral_skb) {
		p_sops->get_mac_address(spectral, spec_samp_msg.macaddr);
		spectral->spectral_nlh =
		    (struct nlmsghdr *)spectral->spectral_skb->data;
		memcpy(NLMSG_DATA(spectral->spectral_nlh),
		       &spec_samp_msg, sizeof(struct spectral_samp_msg));
		msg = (struct spectral_samp_msg *)NLMSG_DATA(
				spectral->spectral_nlh);
		/*
		 * Broadcast spectral data only
		 * if it is a edma supported device
		 */
		if (!spectral->sc_spectral_non_edma)
			target_if_spectral_bcast_msg(spectral);
		samp_msg_index++;
	}

	/*
	 * Check if the device is non-edma and follow the required broadcast
	 * path if true
	 */
	if (spectral->sc_spectral_non_edma) {
		/*
		 * Allocating memory for the queue entity to
		 * hold the spectral socket buffer
		 */
		sp_skb_event = (struct target_if_spectral_skb_event
				*)qdf_mem_malloc(
					sizeof(
					  struct target_if_spectral_skb_event));

		if (sp_skb_event) {
			OS_MEMZERO(sp_skb_event,
				   sizeof(struct target_if_spectral_skb_event));
			sp_skb_event->sp_skb = spectral->spectral_skb;
			sp_skb_event->sp_nlh = spectral->spectral_nlh;
			spectral->spectral_skb = NULL;
			spectral->spectral_nlh = NULL;

			/*
			 * Queue spectral socket buffers to be broadcasted
			 * outside irq lock
			 */
			qdf_spin_lock(&spectral->spectral_skbqlock);
			STAILQ_INSERT_TAIL(&spectral->spectral_skbq,
					   sp_skb_event, spectral_skb_list);
			qdf_spin_unlock(&spectral->spectral_skbqlock);
		}
	}
#else
	/*
	 * call the indicate function to pass the data to the net layer
	 * Windows will pass to a spectral WIN32 service
	 */
	msg = (struct spectral_samp_msg *)qdf_mem_malloc(
			sizeof(struct spectral_samp_msg));
	if (msg) {
		OS_MEMCPY(msg, &spec_samp_msg,
			  sizeof(struct spectral_samp_msg));
		ath_spectral_indicate(params->sc, (void *)msg,
				      sizeof(struct spectral_samp_msg));
		OS_FREE(msg);
		msg = NULL;
	} else {
		spectral_err("No buffer");
	}
#endif				/* SPECTRAL_USE_NETLINK_SOCKETS */

	if (spectral->sc_spectral_noise_pwr_cal)
		target_if_spectral_process_noise_pwr_report(
			spectral, &spec_samp_msg);
}

#ifdef SPECTRAL_USE_NETLINK_SOCKETS

void
target_if_spectral_prep_skb(struct target_if_spectral *spectral)
{
	spectral->spectral_skb = dev_alloc_skb(MAX_SPECTRAL_PAYLOAD);

	if (spectral->spectral_skb) {
		skb_put(spectral->spectral_skb, MAX_SPECTRAL_PAYLOAD);
		spectral->spectral_nlh =
		    (struct nlmsghdr *)spectral->spectral_skb->data;

		OS_MEMZERO(spectral->spectral_nlh,
			   sizeof(*spectral->spectral_nlh));

		/*
		 * Possible bug that size of  struct spectral_samp_msg and
		 * SPECTRAL_MSG differ by 3 bytes  so we miss 3 bytes
		 */

		spectral->spectral_nlh->nlmsg_len =
		    NLMSG_SPACE(sizeof(struct spectral_samp_msg));
		spectral->spectral_nlh->nlmsg_pid = 0;
		spectral->spectral_nlh->nlmsg_flags = 0;
	} else {
		spectral->spectral_skb = NULL;
		spectral->spectral_nlh = NULL;
	}
}

#if (KERNEL_VERSION(2, 6, 31) > LINUX_VERSION_CODE)
static inline void
target_if_init_spectral_skb_dst_pid(
	struct target_if_spectral *spectral)
{
	NETLINK_CB(spectral->spectral_skb).dst_pid =
	    spectral->spectral_pid;
}
#else
static inline void
target_if_init_spectral_skb_dst_pid(
	struct target_if_spectral *spectral)
{
}
#endif			/* VERSION - field depracated by newer kernel */

#if KERNEL_VERSION(3, 7, 0) > LINUX_VERSION_CODE
static inline void
target_if_init_spectral_skb_pid_portid(
	struct target_if_spectral *spectral)
{
	NETLINK_CB(spectral->spectral_skb).pid = 0;  /* from kernel */
}
#else
static inline void
target_if_init_spectral_skb_pid_portid(
	struct target_if_spectral *spectral)
{
	NETLINK_CB(spectral->spectral_skb).portid = 0;  /* from kernel */
}
#endif

void
target_if_spectral_unicast_msg(struct target_if_spectral *spectral)
{
	if (!spectral) {
		spectral_err("Spectral is NULL");
		return;
	}

	if (!spectral->spectral_sock) {
		spectral_err("Spectral Socket is invalid");
		dev_kfree_skb(spectral->spectral_skb);
		spectral->spectral_skb = NULL;
		return;
	}

	if (spectral->spectral_skb) {
		target_if_init_spectral_skb_dst_pid(spectral);

		target_if_init_spectral_skb_pid_portid(spectral);

		/* to mcast group 1<<0 */
		NETLINK_CB(spectral->spectral_skb).dst_group = 0;

		netlink_unicast(spectral->spectral_sock,
				spectral->spectral_skb,
				spectral->spectral_pid, MSG_DONTWAIT);
	}
}

void
target_if_spectral_bcast_msg(struct target_if_spectral *spectral)
{
#if (KERNEL_VERSION(2, 6, 31) >= LINUX_VERSION_CODE)
	fd_set write_set;
#endif
	struct spectral_samp_msg *msg = NULL;
	struct nlmsghdr *nlh = NULL;
	int status;

#if (KERNEL_VERSION(2, 6, 31) >= LINUX_VERSION_CODE)
	FD_ZERO(&write_set);
#endif

	if (!spectral)
		return;

	if (!spectral->spectral_sock) {
		dev_kfree_skb(spectral->spectral_skb);
		spectral->spectral_skb = NULL;
		return;
	}

	if (!spectral->spectral_skb)
		return;

	nlh = (struct nlmsghdr *)spectral->spectral_skb->data;
	msg = (struct spectral_samp_msg *)NLMSG_DATA(spectral->spectral_nlh);
	status = target_if_send_phydata(spectral->pdev_obj,
					spectral->spectral_sock,
					spectral->spectral_skb);
	if (status == 0)
		spectral->spectral_sent_msg++;

	/* netlink will have freed the skb */
	if (spectral->spectral_skb)
		spectral->spectral_skb = NULL;
}

void
target_if_spectral_skb_dequeue(void *data)
{
	struct target_if_spectral *spectral = (struct target_if_spectral *)data;
	struct target_if_spectral_skb_event *sp_skb_event = NULL;

	qdf_spin_lock(&spectral->spectral_skbqlock);
	/* Deque all the spectral socket buffers queued */
	while (!STAILQ_EMPTY(&spectral->spectral_skbq)) {
		sp_skb_event = STAILQ_FIRST(&spectral->spectral_skbq);
		if (sp_skb_event) {
			spectral->spectral_skb = sp_skb_event->sp_skb;
			spectral->spectral_nlh = sp_skb_event->sp_nlh;
			STAILQ_REMOVE_HEAD(&spectral->spectral_skbq,
					   spectral_skb_list);
			qdf_spin_unlock(&spectral->spectral_skbqlock);
			OS_FREE(sp_skb_event);
			/* Broadcast spectral data after dequeing */
			target_if_spectral_bcast_msg(spectral);
			qdf_spin_lock(&spectral->spectral_skbqlock);
		}
	}
	qdf_spin_unlock(&spectral->spectral_skbqlock);
}

#endif				/* SPECTRAL_USE_NETLINK_SOCKETS */
