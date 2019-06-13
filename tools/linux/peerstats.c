/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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
 * file: peer rate statitistics application
 * This file provides framework to display peer rate statistics
 */
#include <netinet/in.h>
#include <arpa/inet.h>
#include <qcatools_lib.h>
#include <dp_rate_stats_pub.h>

#ifndef min
#define min(x, y) ((x) < (y) ? (x) : (y))
#else
#error confilicting defs of min
#endif

#define PRINT(fmt, ...) \
	do { \
		printf(fmt, ##__VA_ARGS__); \
		printf("\n"); \
	} while (0)

char interface[IFNAMSIZ];

static void dp_peer_rx_rate_stats_print(uint8_t *peer_mac,
					uint64_t peer_cookie,
					void *buffer,
					uint32_t buffer_len)
{
	int i = 0;
	struct wlan_rx_rate_stats *rx_stats;
	uint8_t is_lithium;
	uint8_t chain, max_chain, bw, max_bw;
	struct wlan_rx_rate_stats *tmp_rx_stats;;

	rx_stats = tmp_rx_stats = (struct wlan_rx_rate_stats *)buffer;
	PRINT("\n......................................");
	PRINT("......................................");
	PRINT("PEER %02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx\n",
	      peer_mac[0],
	      peer_mac[1],
	      peer_mac[2],
	      peer_mac[3],
	      peer_mac[4],
	      peer_mac[5]);
	PRINT("\tpeer cookie: %016llx\n", (peer_cookie & 0xFFFFFFFF00000000)
					>> WLANSTATS_PEER_COOKIE_LSB);
	is_lithium =  (peer_cookie & WLANSTATS_COOKIE_PLATFORM_OFFSET)
					>> WLANSTATS_PEER_COOKIE_LSB;
	if (is_lithium) {
		max_chain = 8;
		max_bw = 8;
	} else {
		max_chain = 4;
		max_bw = 4;
	}
	PRINT("\n..............................................");
	PRINT("................................");
	PRINT("................................................");
	PRINT(".................................\n");
	PRINT("\tRx statistics:");
	PRINT(" %10s | %10s | %10s | %10s | %10s | %10s|",
	      "rate",
	      "rix",
	      "bytes",
	      "msdus",
	      "mpdus",
	      "ppdus");
	PRINT(" %10s | %10s | %10s |",
	      "retries",
	      "sgi",
	      "rssi\n");

	for (i = 0; i < WLANSTATS_CACHE_SIZE; i++) {
		if (rx_stats->rix != INVALID_CACHE_IDX) {
			PRINT(" %10u | %10u | %10u | %10u | %10u | %10u |",
			      rx_stats->rate,
			      rx_stats->rix,
			      rx_stats->num_bytes,
			      rx_stats->num_msdus,
			      rx_stats->num_mpdus,
			      rx_stats->num_ppdus);
			PRINT(" %10u | %10u | %10u |\n",
			      rx_stats->num_retries,
			      rx_stats->num_sgi,
			      rx_stats->avg_rssi);
		}
		rx_stats = rx_stats + 1;
	}
	if (is_lithium) {
	PRINT("\n %10s | %10s | %10s | %10s | %10s |",
	      "rate",
	      "rssi 1 p20",
	      "rssi 1 e20",
	      "rssi 1 e40 low20",
	      "rssi 1 e40 high20");
	PRINT("\n            | %10s | %10s | %10s | %10s |",
	      "rssi 1 ext80 low20",
	      "rssi 1 ext80 low_high20",
	      "rssi 1 ext80 high_low20",
	      "rssi 1 ext80 high20");
	PRINT("\n            | %10s | %10s | %10s | %10s |",
	      "rssi 2 p20",
	      "rssi 2 e20",
	      "rssi 2 e40 low20",
	      "rssi 2 e40 high20");
	PRINT("\n            | %10s | %10s | %10s | %10s |",
	      "rssi 2 ext80 low20",
	      "rssi 2 ext80 low_high20",
	      "rssi 2 ext80 high_low20",
	      "rssi 2 ext80 high20");
	PRINT("\n            | %10s | %10s | %10s | %10s |",
	      "rssi 3 p20",
	      "rssi 3 e20",
	      "rssi 3 e40 low20",
	      "rssi 3 e40 high20");
	PRINT("\n            | %10s | %10s | %10s | %10s |",
	      "rssi 3 ext80 low20",
	      "rssi 3 ext80 low_high20",
	      "rssi 3 ext80 high_low20",
	      "rssi 3 ext80 high20");
	PRINT("\n            | %10s | %10s | %10s | %10s |",
	      "rssi 4 p20",
	      "rssi 4 e20",
	      "rssi 4 e40 low20",
	      "rssi 4 e40 high20");
	PRINT("\n            | %10s | %10s | %10s | %10s |",
	      "rssi 4 ext80 low20",
	      "rssi 4 ext80 low_high20",
	      "rssi 4 ext80 high_low20",
	      "rssi 4 ext80 high20");
	PRINT("\n            | %10s | %10s | %10s | %10s |",
	      "rssi 5 p20",
	      "rssi 5 e20",
	      "rssi 5 e40 low20",
	      "rssi 5 e40 high20");
	PRINT("\n            | %10s | %10s | %10s | %10s |",
	      "rssi 5 ext80 low20",
	      "rssi 5 ext80 low_high20",
	      "rssi 5 ext80 high_low20",
	      "rssi 5 ext80 high20");
	PRINT("\n            | %10s | %10s | %10s | %10s |",
	      "rssi 6 p20",
	      "rssi 6 e20",
	      "rssi 6 e40 low20",
	      "rssi 6 e40 high20");
	PRINT("\n            | %10s | %10s | %10s | %10s |",
	      "rssi 6 ext80 low20",
	      "rssi 6 ext80 low_high20",
	      "rssi 6 ext80 high_low20",
	      "rssi 6 ext80 high20");
	PRINT("\n            | %10s | %10s | %10s | %10s |",
	      "rssi 7 p20",
	      "rssi 7 e20",
	      "rssi 7 e40 low20",
	      "rssi 7 e40 high20");
	PRINT("\n            | %10s | %10s | %10s | %10s |",
	      "rssi 7 ext80 low20",
	      "rssi 7 ext80 low_high20",
	      "rssi 7 ext80 high_low20",
	      "rssi 7 ext80 high20");
	PRINT("\n            | %10s | %10s | %10s | %10s |",
	      "rssi 8 p20",
	      "rssi 8 e20",
	      "rssi 8 e40 low20",
	      "rssi 8 e40 high20");
	PRINT("\n            | %10s | %10s | %10s | %10s |\n\n\n",
	      "rssi 8 ext80 low20",
	      "rssi 8 ext80 low_high20",
	      "rssi 8 ext80 high_low20",
	      "rssi 8 ext80 high20");
	} else {
	PRINT("\n %10s | %10s | %10s | %10s | %10s |",
	      "rate",
	      "rssi 1 p20",
	      "rssi 1 e20",
	      "rssi 1 e40",
	      "rssi 1 e80");
	PRINT("            | %10s | %10s | %10s | %10s |",
	      "rssi 2 p20",
	      "rssi 2 e20",
	      "rssi 2 e40",
	      "rssi 2 e80");
	PRINT("            | %10s | %10s | %10s | %10s |",
	      "rssi 3 p20",
	      "rssi 3 e20",
	      "rssi 3 e40",
	      "rssi 3 e80");
	PRINT("            | %10s | %10s | %10s | %10s |\n\n\n",
	      "rssi 4 p20",
	      "rssi 4 e20",
	      "rssi 4 e40",
	      "rssi 4 e80");
	}
	for (i = 0; i < WLANSTATS_CACHE_SIZE; i++) {
		if (tmp_rx_stats->rix != INVALID_CACHE_IDX) {
			printf(" %10u |", tmp_rx_stats->rate);
			for (chain = 0; chain < max_chain; chain++) {
				for (bw = 0; bw < max_bw; bw++) {
					printf(" %10d |",
					tmp_rx_stats->avg_rssi_ant[chain][bw]);
				}
				printf("            \n\t     ");
			}
			PRINT("");
		}
		tmp_rx_stats = tmp_rx_stats + 1;
	}
}

static void
dp_peer_tx_sojourn_stats_print(uint8_t *peer_mac,
			       uint64_t peer_cookie,
			       struct wlan_tx_sojourn_stats *sojourn_stats)
{
	uint8_t tid;

	PRINT("\n..........................................");
	PRINT("....................................");
	PRINT("....................................");
	PRINT(".........................................\n");
	PRINT("PEER %02hhx:%02hhx:%02hhx:%02hhx%02hhx:%02hhx\n",
	      peer_mac[0],
	      peer_mac[1],
	      peer_mac[2],
	      peer_mac[3],
	      peer_mac[4],
	      peer_mac[5]);
	PRINT("\tPEER Cookie: %016llx\n", peer_cookie);
	PRINT("\n...........................................");
	PRINT("...................................");
	PRINT("..................................");
	PRINT("............................................");
	PRINT("\n\tSojourn statistics:\n");
	PRINT("\t\t%10s %10s %20s %20s\n", "tid", "ave", "sum", "num");

	for (tid = 0; tid < WLAN_DATA_TID_MAX; tid++) {
				/* change sum_sojourn_msdu data type to u64 */
		PRINT("\t\t%10d %10lu %20u %20u\n",
		      tid,
		      sojourn_stats->avg_sojourn_msdu[tid],
		      sojourn_stats->sum_sojourn_msdu[tid],
		      sojourn_stats->num_msdus[tid]);
	}
	PRINT("sizeof(avg): %d", sizeof(sojourn_stats->avg_sojourn_msdu[tid]));
	PRINT("\n...........................................");
	PRINT("...................................");
	PRINT("...................................");
	PRINT("...........................................\n");
}

static void dp_peer_tx_rate_stats_print(uint8_t *peer_mac,
					uint64_t peer_cookie,
					void *buffer,
					uint32_t buffer_len)
{
	int i = 0;
	struct wlan_tx_rate_stats *tx_stats;
	struct wlan_tx_sojourn_stats *sojourn_stats;

	if (buffer_len < (WLANSTATS_CACHE_SIZE *
			  sizeof(struct wlan_tx_rate_stats))
			  + sizeof(struct wlan_tx_sojourn_stats)) {
		PRINT("invalid buffer len, return");
		return;
	}
	tx_stats = (struct wlan_tx_rate_stats *)buffer;
	PRINT("\n...........................................");
	PRINT("...................................");
	PRINT("...................................");
	PRINT("...........................................\n");
	PRINT("PEER %02hhx:%02hhx:%02hhx:%02hhx%02hhx:%02hhx\n\n",
	      peer_mac[0],
	      peer_mac[1],
	      peer_mac[2],
	      peer_mac[3],
	      peer_mac[4],
	      peer_mac[5]);
	PRINT("\tPEER Cookie: %016llx", peer_cookie);
	PRINT("\n...........................................");
	PRINT("...................................");
	PRINT("...................................");
	PRINT("...........................................\n");
	PRINT("\tTx statistics:\n");
	PRINT("\t\t%10s | %10s | %10s | %10s | %10s",
	      "rate",
	      "rix",
	      "attempts",
	      "success",
	      "ppdus");

	for (i = 0; i < WLANSTATS_CACHE_SIZE; i++) {
		if (tx_stats->rix != INVALID_CACHE_IDX) {
			PRINT("\t\t%10u | %10u | %10u | %10u | %10u\n",
			      tx_stats->rate,
			      tx_stats->rix,
			      tx_stats->mpdu_attempts,
			      tx_stats->mpdu_success,
			      tx_stats->num_ppdus);
		}
		tx_stats = tx_stats + 1;
	}

	sojourn_stats = (struct wlan_tx_sojourn_stats *)((uint8_t *)buffer
			 + (WLANSTATS_CACHE_SIZE *
			 sizeof(struct wlan_tx_rate_stats)));
	dp_peer_tx_sojourn_stats_print(peer_mac, peer_cookie, sojourn_stats);

	return;
}

static void dp_peer_stats_handler(uint32_t cache_type,
				 uint8_t *peer_mac,
				 uint64_t peer_cookie,
				 void *buffer,
				 uint32_t buffer_len)
{
	switch (cache_type) {
	case DP_PEER_RX_RATE_STATS:
		dp_peer_rx_rate_stats_print(peer_mac, peer_cookie,
					    buffer, buffer_len);
		break;
	case DP_PEER_TX_RATE_STATS:
		dp_peer_tx_rate_stats_print(peer_mac, peer_cookie,
					    buffer, buffer_len);
		break;
	}
}

static void
dp_peer_stats_event_callback(char *ifname,
			     uint32_t cmdid,
			     uint8_t *data,
			     size_t len)
{
	struct nlattr *tb_array[QCA_WLAN_VENDOR_ATTR_PEER_STATS_CACHE_MAX + 1];
	struct nlattr *tb;
	void *buffer = NULL;
	uint32_t buffer_len = 0;
	uint8_t *peer_mac;
	uint32_t cache_type;
	uint64_t peer_cookie;

	if (cmdid != QCA_NL80211_VENDOR_SUBCMD_PEER_STATS_CACHE_FLUSH) {
		/* ignore anyother events*/
		return;
	}

	if (strncmp(interface, ifname, sizeof(interface)) != 0) {
		/* ignore events for other interfaces*/
		return;
	}

	if (nla_parse(tb_array, QCA_WLAN_VENDOR_ATTR_PEER_STATS_CACHE_MAX,
		      (struct nlattr *)data, len, NULL)) {
		PRINT("Invalid event\n");
		return;
	}

	tb = tb_array[QCA_WLAN_VENDOR_ATTR_PEER_STATS_CACHE_TYPE];
	if (!tb) {
		PRINT("Cache type in NULL, return");
		return;
	}
	cache_type = nla_get_u32(tb);

	tb = tb_array[QCA_WLAN_VENDOR_ATTR_PEER_STATS_CACHE_PEER_MAC];
	if (!tb) {
		PRINT("Peer mac addr is null, return");
		return;
	}
	peer_mac = (uint8_t *)nla_data(tb);

	tb = tb_array[QCA_WLAN_VENDOR_ATTR_PEER_STATS_CACHE_DATA];
	if (tb) {
		buffer = (void *)nla_data(tb);
		buffer_len = nla_len(tb);
	}

	tb = tb_array[QCA_WLAN_VENDOR_ATTR_PEER_STATS_CACHE_PEER_COOKIE];
	if (!tb) {
		PRINT("peer cookie attribute is null, return");
		return;
	}
	peer_cookie = nla_get_u64(tb);
	if (!buffer) {
		PRINT(" stats buffer is null, return");
		return;
	}

	dp_peer_stats_handler(cache_type, peer_mac, peer_cookie,
			      buffer, buffer_len);
}

int main(int argc, char *argv[])
{
	int err = 0;
	wifi_cfg80211_context cfg80211_ctxt;
	char *ifname;
	int num_msecs = 0;
	int status = 0;

	if (argc < 2) {
		fprintf(stderr, "Invalid commands args\n");
		return -EIO;
	}

	/* Reset the cfg80211 context to 0 if the application does not pass
	 * custom private event and command sockets. In this case, the default
	 * port is used for netlink communication.
	 */
	memset(&cfg80211_ctxt, 0, sizeof(wifi_cfg80211_context));

	ifname = argv[1];
	memcpy(interface, ifname, sizeof(interface));
	cfg80211_ctxt.event_callback = dp_peer_stats_event_callback;

	err = wifi_init_nl80211(&cfg80211_ctxt);
	if (err) {
		fprintf(stderr, "unable to create NL socket\n");
		return -EIO;
	}

	/* Starting event thread to listen for responses*/
	if (wifi_nl80211_start_event_thread(&cfg80211_ctxt)) {
		fprintf(stderr, "Unable to setup nl80211 event thread\n");
		status = -EIO;
		goto cleanup;
	}

	while (true) {
		/*sleep for 1 ms*/
		usleep(1000);
		num_msecs++;
	}

	wifi_destroy_nl80211(&cfg80211_ctxt);
	return 0;

cleanup:
	wifi_destroy_nl80211(&cfg80211_ctxt);
	return status;
}
