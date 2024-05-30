/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _TX_FLUSH_REQ_H_
#define _TX_FLUSH_REQ_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_TX_FLUSH_REQ 2

#define NUM_OF_QWORDS_TX_FLUSH_REQ 1


struct tx_flush_req {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t flush_req_reason                                        :  8, // [7:0]
                      phytx_abort_reason                                      :  8, // [15:8]
                      flush_req_user_number_or_link_id                        :  6, // [21:16]
                      mlo_abort_reason                                        :  5, // [26:22]
                      reserved_0a                                             :  5; // [31:27]
             uint32_t tlv64_padding                                           : 32; // [31:0]
#else
             uint32_t reserved_0a                                             :  5, // [31:27]
                      mlo_abort_reason                                        :  5, // [26:22]
                      flush_req_user_number_or_link_id                        :  6, // [21:16]
                      phytx_abort_reason                                      :  8, // [15:8]
                      flush_req_reason                                        :  8; // [7:0]
             uint32_t tlv64_padding                                           : 32; // [31:0]
#endif
};


/* Description		FLUSH_REQ_REASON

			The reason why the flush request was generated.
			
			<enum 0 reserved_code>This is included for clean implementation
			 and verification. This code should NOT be used during a
			 valid FLUSH. It is used as a keeper value when flush logic
			 is idle
			<enum 1 txpcu_flreq_code_txop_exceeded>Flush request issued
			 by TXPCU in case of a WCOEX abort.
			<enum 2 crypt_flreq_rx_int_tx>This is a corner case scenario. 
			A situation where:
			a.A RX is just over and CCA indication is IDLE
			b.Crypt is still busy decrypting
			c.A TX just starts.
			The TX should be tried later. This situation may be rare. 
			Just taking an extra precaution.
			<enum 3 txpcu_flreq_code_rts_pkt_cca_abort>This is the static
			 BW failure happening right after start_tx for either RTS
			 frame or data packet
			<enum 4 txpcu_flreq_code_cts_cca_abort>This is the static
			 BW failure in the protection sequence (CTS).
			<enum 5 pdg_flreq_code_txop_abort>This is PDG signaling 
			not enough TXOP for transmission
			<enum 6 sw_explicit_flush_termination>When SW issues a flush
			 WHICH CAUSES AN ONGOING FES to terminate
			<enum 7 fes_stp_not_enough_txop_rem>Not enough TXOP remaining
			 in either SW or HW mode. This checks if the remaining TXOP
			 < a parameterized minimum time. Currently half SIFS duration
			 (5 us).
			<enum 8 hwsch_sch_tlv_zero_hdr_err>HWSCH flush when Parser
			 engine encounters a header with all zeros in the DWORD
			<enum 9 fes_stp_tlv_time_exceeded_bkof_exp>Issued in case
			 TLV transmission exceeds start_tx time
			<enum 10 fes_stp_sw_fes_time_gt_hw>SW mode abort. When HWSCH
			 determines that none of the SW programmed (upto 3) BW times
			 can fit into the current TXOP remaining
			<enum 11 txpcu_flreq_ppdu_allow_bw_fields_not_set>Flush 
			request issued by TXPCU in case none of the PPDU_ALLOW_BW_* 
			fields are set in PCU_PPDU_SETUP TLV
			<enum 12 txpcu_false_mu_reception>Flush request issued by
			 TXPCU if RXPCU initiates a response generation for a MU
			 reception even though MU reception was not expected
			<enum 13 hwsch_coex_abort>Flush request issued by HWSCH 
			when a coex event caused this transmit to be aborted
			<enum 14 hwsch_svd_rdy_timeout>Flush request issued by HWSCH
			 when the PHY does not return the SVD_READY before a timeout
			 expires
			<enum 15 num_mpdu_count_zero>Flush request issued by TXPCU
			 when the number of MPDU counter for selected BW is zero
			
			<enum 16 unsupported_cbf>Flush request issued by TXPCU if
			 TXPCU receives TX_PKT_END with error_unsupported_cbf during
			 CV transfer.
			<enum 17 txpcu_flreq_pcu_ppdu_setup_init_not_valid>Indicates
			 TXPCU has not received PCU_PPDU_SETUP_INIT from PDG, by
			 the time it received PRE_START_TX from HWSCH.
			<enum 18 txpcu_flreq_pcu_ppdu_setup_start_not_valid>Indicates
			 TXPCU has not received PCU_PPDU_SETUP_START from PDG, by
			 the time it received START_TX from HWSCH.
			<enum 19 txpcu_flreq_tx_phy_descriptor_not_valid>Indicates
			 TXPCU has not received TX_PHY_DESCRIPTOR within REQD_TLVS_WAIT_TIME
			 after receiving START_TX from HWSCH.
			<enum 20 txpcu_req_tlvs_timeout_for_cbf>TXPCU did nor receive
			 the CBF info TLVs from the PHY fast enough which resulted
			 in a timeout.
			<enum 21 txdma_flreq_no_of_mpdu_less_than_limit_status>Indicates
			 the total number of MPDUs that needs to be send out by 
			TXDMA is less than the number indicated by PDG/TXPCU in 
			the MPDU_LIMIT_STATUS
			<enum 22 txole_flreq_frag_en_amsdu_ampdu>Fragmentation is
			 enabled in TX_FES_SETUP for an AMSDU or AMPDU
			<enum 23 txole_flreq_more_frag_set_for_last_seg>more_frag
			 bit in TX_FES_SETUP TLV is set for the last MPDU fragment
			
			<enum 24 txpcu_flreq_start_tx_bw_mismatch>Indicates TXPCU
			 has detected a mismatch between BWs detected at PRE_START_TX
			 and START_TX
			<enum 25 txpcu_flreq_coex_bw_not_allowed>flush request and
			 is asserted by TXPCU when the final negotiated BW from 
			COEX is not allowed by SW
			<enum 26 txole_flreq_frag_en_sw_encrypted>flush request 
			and is asserted by TXPCU when the final negotiated BW from
			 COEX is not allowed by SW
			<enum 27 txole_flreq_frag_en_buffer_chaining>Fragmentation
			 is enabled in raw mode buffer chaining mode.
			<enum 28 txole_flreq_pv1_type3_amsdu_error>A1 and A2 set
			 to MAC addresses for 11ah PV1 short frame which is an AMSDU
			
			<enum 29 txole_flreq_pv1_wrong_key_type>An unsupported key_type
			 is set for a PV1 frames. WEP, TKIP and WAPI are not supported
			 for PV1 frames
			<enum 30 txole_flreq_illegal_frame>Unexpected Tx Mpdu length. 
			Asserted if the MSDU PACKET TLV length is less than the 
			expected WMAC header
			<enum 31 pdg_flreq_coex_reasons>Asserted by PDG when COEX
			 related logic in PDG requires a flush request.
			<enum 32 wifi_txole_no_full_msdu_for_checksum_en>Full MSDU
			 packet was not provided by TXDMA when checksum/TSO/fragmentation
			 was enabled
			<enum 33 wifi_txole_length_mismatch_802_3_eth_frame>The 
			length field in the incoming 802.3 ethernet frame doesn't
			 match with the actual number of bytes in the data TLV.
			<enum 34 wifi_txole_pv0_amsdu_frame_err>Non-QoS frames are
			 queued as part of AMSDU
			<enum 35 wifi_txole_pv0_wrong_key_type>Key type in peer 
			table set to NO_CIPHER for protected frames
			<enum 36 wifi_fes_stp_cca_busy_in_pifs>This flush is initiated
			 by scheduler when (if enabled) CCA goes busy in the middle
			 of a PIFS burst
			<enum 37 prot_frame_data_underrun>This flush is initiated
			 by TXPCU when a protection frame is send, but TXPCU has
			 not received address fields in time.
			<enum 38 pdg_no_length_received>PDG generated this flush
			 request because not one MPDU length info has been received
			 at the required timeout (which is programmable)
			<enum 39 pdg_wrong_preamble_req_order>PDG generated this
			 flush request because PHY issued an unexpected preamble
			 request type
			<enum 40 txpcu_flreq_retry_for_optimal_bw>The most desired
			 BW was not available, and TXPCU would like to try the most
			 optimal transmit BW again after a new BO period.
			<enum 41 wifi_txole_incomplete_llc_frame>LLC received incomplete
			 frame
			<enum 42 pdg_cts_lower_bw_fit_err>PDG received a CTS frame
			 that reduced the BW, As a result the MPDU does not fit 
			in the previous reserved time, the thus this transmission
			 is aborted
			<enum 43 pdg_cts_shorter_dur_fit_err>PDG received a CTS 
			frame that a reduced duration field. As a result the MPDU
			 does not fit in the previous reserved time, the thus this
			 transmission is aborted
			
			Note the duration field in CTS can be reduced as a result
			 of COEX reasons
			<enum 44 hwsch_sch_tlv_len_oor_err>HWSCH flush when Parser
			 engine encounters a header whose length is greater than
			 511 dwords. This excludes DUMMY TLVs.
			<enum 45 hwsch_sch_tlv_taglen_mismatch_err>HWSCH flush when
			 Parser engine encounters a header whose TAG does not match
			 the XML specified length. This check excludes zero length
			 and variable length TLVs
			<enum 46 hwsch_sch_tlv_sfm_tracking_err>HWSCH flush when
			 Parser engine encounters a non contiguous error check code, 
			while reading SFM. This check is primarily to catch data
			 write or read issues within the buffering process of scheduler
			 TLV in SFM
			<enum 47 wifitx_flush_rssi_above_obss_nonsrg_thr>When HWSCH
			 attempts to transmit a packet based on OBSS_PD non-SRG 
			opportunity, a flush with this code is generated if "ReceivedRssi
			 from RXPCU > Scheduler_cmd.RssiAltNonSrg".
			<enum 48 wifitx_flush_rssi_above_obss_srg_thr>When HWSCH
			 attempts to transmit a packet based on OBSS_PD non-SRG 
			opportunity, a flush with this code is generated if "ReceivedRssi
			 from RXPCU > Scheduler_cmd.RssiAltSrg".
			<enum 49 wifitx_flush_rssi_above_srp_pwr_thr>When HWSCH 
			attempts to transmit a packet within an SRP opportunity 
			window, a flush with this code is generated if "Scheduler_cmd.SrpAltPwr
			 > SRP_less_RSSI".
			<enum 50 hwsch_unexpected_sch_tlv_end_err>parse errors
			<enum 51 hwsch_sch_tlv_tag_oor_err>HWSCH flush when Parser
			 engine encounters a header whose TAG is not listed in the
			 XML TAG table
			<enum 52 txpcu_phytx_abort_err>An abort from PHY TX got 
			received
			<enum 53 txpcu_coex_soft_abort_err>A soft from coex got 
			received before even a single MPDU got transmitted. Therefor
			 transmission is terminated.
			<enum 54 pdg_min_user_count_missed>PDG was asked to start
			 an MU transmission, but the number of users with actual
			 data was less then the threshold (Min_users_with_data_count)
			
			<enum 55 pdg_min_byte_count_missed>PDG was asked to start
			 an SU transmission, but the number of bytes that PDG has
			 been informed about that can be transmitted is less then
			 the required threshold (min_ppdu_bytes)
			<enum 56 pdg_min_mpdu_count_missed>PDG was asked to start
			 an SU transmission, but the number of MPDUs that PDG has
			 been informed about that can be transmitted is less then
			 the required threshold (min_mpdus_in_ppdu)
			<enum 57 pdg_cannot_pad_min_ppdu_time>PDG uses this code
			 when the min PPDU time to pad up to (pad_min_ppdu_time) 
			can not be met due to other boundary conditions (e.g. FES
			 time/TXOP time/TBTT)
			<enum 58 ucode_flush_request>Flush request initiated by 
			the ucode (M3)
			<enum 59 txpcu_resp_frame_flushed>TXPCU uses this code on
			 encountering an error condition (e.g. late MACTX_PHY_DESC
			 or CV error) while generating a response.
			<enum 60 hwsch_sifs_burst_svd_ready_timeout>This flush code
			 is used by HWSCH to indicate that during SIFS bursting, 
			an SVD_READY timeout was detected, which resulted in the
			 SIFS burst to be aborted.
			<enum 61 txpcu_phy_data_request_to_early>TXPCU has not been
			 properly initialized when the first data request from the
			 PHY has been seen.
			<enum 62 txpcu_trigger_response_cs_check_fail>TXPCU found
			 that the medium was not idle for the Carries Sense check
			 that PDG indicated was needed for the triggered response
			 frame.
			<enum 63 pdg_ofdma_max_unused_space_violation>PDG found 
			out that when trying to assign the RUs among the available
			 users, the number of unused RUs remained above the allowed
			 threshold 
			<enum 64 crypto_tx_user_capacity_exceeded>This happens when
			 Crypto receives TLVs for more TX users than it can support
			 at that point of time
			<enum 65 crypto_tx_non_mu_key_type_rcvd>This happens when
			 Crypto receives unsupported Key types (WEP, TKIP) for MU
			
			<enum 66 txpcu_cbf_resp_abort>CBF response generation by
			 TXPCU ran into issues due to info not being available from
			 the PHY
			<enum 67 txpcu_phy_nap_received_during_tx>TXPCU received
			 a PHY NAP TLV from rxpcu while a transmission was ongoing. 
			The transmission will be terminated with this abort reason.
			
			<enum 68 rxpcu_trigger_with_fcs_error>RXPCU found out that
			 the trigger frame that was received and for which the TX
			 path has been activated to generate a response, had an 
			FCS error.
			<enum 69 pdg_flreq_coex_bt_higher_priority>Asserted by PDG
			 when COEX indicated to PDG that the transmit request is
			 NOT granted because a higher priority BT activity is ongoing.
			
			<enum 70 txpcu_txrx_conflict_detected>TXPCU detected a conflict
			 between an FES transmission and a self-gen response transmission. 
			This is when the PHY + RXPCU delays cause a self-gen to 
			overlap with the pre-backoff time from HWSCH for the next
			 FES.
			<enum 71 pdg_mu_cts_ru_allocation_corruption>PDG received
			 a MU-RTS trigger for which the CTS RU response setting 
			is not valid
			<enum 72 pdg_trig_for_blocked_ru>PDG received a trigger 
			based transmission request for an RU size that is blocked
			 by SW.
			<enum 73 pdg_trig_response_mode_corruption>Asserted when
			 PDG gets a TX_FES_SETUP with field "Fes_in_11ax_Trigger_response_config" 
			not being in sync with what it was expecting.
			<enum 74 pdg_invalid_trigger_config_received>PDG received
			 OFDMA_TRIGGER_DETAILS and the configuration in there (which
			 RXPCU gets from the trigger frame has invalid field value
			 combinations
			<enum 75 txole_msdu_too_long>This flush request will be 
			asserted if the length of a checksum enabled MSDU is more
			 than 2400 bytes.
			<enum 76 txole_inconsistent_mesh>This flush request will
			 be asserted if mesh_enable is set for an MSDU subframe 
			while its not set for another MSDU subframe in the same 
			AMSDU
			<enum 77 txole_mesh_enable_for_ethernet>This flush request
			 will be asserted if mesh_enable is set for an ethernet 
			frame
			<enum 78 txpcu_trig_response_mode_corruption>Asserted when
			 TXPCU gets a TX_FES_SETUP with field "ofdma_triggered_response" 
			not being in sync with what it was expecting.
			<enum 79 pdg_11ax_invalid_rate_setup>PDG received an 11ax
			 transmit set of parameters that is not allowed or not supported
			
			<enum 80 txpcu_trig_response_info_too_late>TXPCU generates
			 this flush request because trigger response transmission
			 setup info from the SCH was received too late
			<enum 81 wifitx_flush_obss_pd_disabled_for_tx>When HWSCH
			 attempts to transmit a packet having obss_pd disabled within
			 an obss_pd opportunity window this flush code is generated
			
			<enum 82 wifitx_flush_srp_disabled_for_tx>When HWSCH attempts
			 to transmit a packet having SRP disabled within an obss_pd
			 opportunity window this flush code is generated
			<enum 83 pdg_flreq_code_srp_sr_missed>In SRP SR, PDG will
			 generate flush if receiving PDG_TX_REQ in a blocking window
			 around SRP SR limit
			<enum 84 pdg_rbo_user_limit_no_data>PDG generates when no
			 data can be sent for the users specified by TX_FES_SETUP
			 field "RBO_must_have_data_user_limit."
			<enum 85 pdg_no_cbf_response_received>Used by PDG for an
			 MU-MIMO sounding plus steering burst when it did not receive
			 CBF from any recipient STA
			<enum 86 pdg_flreq_unexpected_notify_frame>PDG generates
			 when encountering a 'HARD_NOTIFY' or a 'SEMI_HARD_NOTIFY' 
			frame unless ignore_tx_notify_setting is set in 'PDG_FES_SETUP'
			
			<enum 87 pdg_flush_min_ppdu_time_missed>PDG was asked to
			 start a transmission, but the time required to transmit
			 the PPDU is less than the required threshold (flush_min_ppdu_time)
			
			<enum 88 txpcu_flreq_rxpcu_setup_config_error>Used by TXPCU
			 when Tx is complete and it is about to generate 'EXPECTED_RESPONSE' 
			but it has not got any 'RXPCU_SETUP_COMPLETE' although 'rxpcu_setup_complete_present' 
			was set in 'TX_FES_SETUP'
			<enum 89 txpcu_flreq_late_trig_tlvs>Used by TXPCU when the
			 'RECEIVED_TRIGER_INFO' TLV is sent to SCH after the 'pre_phy_desc' 
			timer has expired, if enabled
			<enum 90 pdg_flreq_notify_mpdu_late>Used by PDG when the
			 first 'MPDU_INFO' is not available when sending 'PCU_PPDU_SETUP_START' 
			so PDG has assumed a regular MPDU ('FW_tx_notify_frame = 
			NO_TX_NOTIFY'), but later the MPDU turned out to be a notify
			 frame, if enabled
			<enum 91 txdma_flreq_sfm_full>TXDMA generates this flush
			 request when it gets 'MPDU_INFO's for a user that it is
			 unable to write into SFM since its SFM allocation is full.
			
			<enum 92 txpcu_flreq_pre_phy_desc_late>Used in TXPCU for
			 generating a flush request when 'PRE_PHY_DESC' is received
			 late (determined by a timer)
			<enum 93 pdg_flreq_cannot_fit_trig_response>This flush request
			 code is used by PDG if the trigger response MPDUs cannot
			 be fit to avoid sending only null delimiters for e.g. unassociated
			 UORA and colliding with another STA with valid data.
			<enum 94 pdg_flreq_unexpected_fes_setup>Flush request used
			 by PDG in case of unexpected 'TX_FES_SETUP'
			<enum 95 pdg_flreq_code_mlo_abort>Flush request used by 
			PDG in case of MLO constraints forcing an abort
			<enum 96 hwsch_bkoff_trunc_seq_abort>Flush request used 
			by HWSCH if an MLO backoff truncation request resulted in
			 a forced abort to avoid windows too close to transmissions
			
			<enum 97 txole_flreq_illegal_frag_settings>Flush request
			 used by TXOLE if fragmentation is requested but the settings
			 are illegal
			<enum 98 txpcu_flreq_mac_flex_overwrite_err>Flush request
			 used by TXPCU when required overwrite TLVs are not received
			 from microcode, or when overwrite TLVs are dropped in MAC
			 due to SFM full condition
			<enum 99 txpcu_lmr_req_timeout>Flush request by TXPCU if
			 PHY does not respond to 'MACRX_LMR_READ_REQUEST' or 'MACRX_LMR_DATA_REQUEST' 
			on time
			<enum 100 txpcu_lmr_phyrx_err_abort>Flush request by TXPCU
			 if PHY sent 'PHYRX_LMR_TRANSFER_ABORT' or 'PHYRX_LMR_READ_REQUEST_ACK' 
			with status anything other than OK
			<enum 101 txpcu_rx_bitmap_ack_mismatch>Flush request by 
			TXPCU on getting a mismatched TLV from RXPCU for 'RX_FRAME_*BITMAP_ACK' (1Kbit
			 instead of 256-bit or vice versa)
			<enum 102 txpcu_rx_incorrect_ba_cnt_for_ampdu>Flush request
			 by TXPCU on getting an 'RX_RESPONSE_REQUIRED_INFO' with
			 A-MPDU set, VHT Ack clear and 'response_ba*_cnt' zero, 
			to avoid a system hang
			<enum 103 txpcu_flreq_cbf_done_delayed>Flush request by 
			TXPCU on not getting a 'MACTX_CBF_DONE' from RXPCU after
			 sending 'RESPONSE_END_STATUS' TLV
			<enum 104 txpcu_flreq_sfm_full>Flush request by TXPCU if
			 SFM indicates 'user_fifo_full'
			<enum 105 pdg_flreq_calc_psdu_length_too_low>PDG was asked
			 to start an MU transmission, but one of the users' RU is
			 such that within the PPDU time the PSDU length that can
			 be fit is too low (based on a threshold in a PDG register)
			
			<enum 106 pdg_flush_min_ppdu_time_obss_sr_missed>PDG was
			 asked to start an OBSS PD SR transmission, but the time
			 required to transmit the PPDU is less than the required
			 threshold (flush_min_ppdu_time_obss_pd_sr)
			<enum 107 pdg_flreq_code_txop_abort_obss_sr>PDG was asked
			 to start an OBSS PD SR transmission, but the time required
			 for the FES is more than the OBSS PPDU duration (max_fes_time_obss_pd_sr)
			
			<enum 108 pdg_flreq_cv_corr_tlv_timeout>PDG timed out waiting
			 for CV correlation TLVs from microcode
			<enum 109 pdg_flreq_pri_user_cbf_fail>Flush request from
			 PDG if CV correlation is enabled and the 'PHYTX_CV_CORR_STATUS' 
			from microcode indicates that the primary user's CBF has
			 failed
			<enum 110 hwsch_sfm_availability_check_fail>HWSCH-issued
			 flush when the SFM availability check fails during a SIFS
			 burst or when fetching part 2 TLVs
			<enum 111 pdg_cannot_pad_response_time>PDG uses this code
			 when the response time to pad up to (required_response_time) 
			cannot be met due to the frame length in 'PDG_RESPONSE' 
			exceeding the calculated padded length
			<enum 112 ul_mu_rx_early_abort>Flush request to terminate
			 an FES when RXPCU aborted an UL MU reception early because
			 at the end of the "early_termination_window," the required
			 number of users with at least one valid MPDU delimiter 
			was not reached.
			
			<enum 113 reserved_flush_code_25>Placeholder for future 
			needs
			<enum 114 reserved_flush_code_26>TXPCU uses this code when
			 more than the configured maximum CTS2SELF are being sent.
			
			<enum 115 reserved_flush_code_27>TXPCU uses this code when
			 at the time of the main PPDU transmission, fewer than the
			 configured minimum CTS2SELF were sent.
			<enum 116 reserved_flush_code_28>TXDMA uses this code when
			 it is about to issue zero-address or zero-length read or
			 when it read a 'TX_MSDU_LINK' but the  Buffer_type field
			 in the uniform descriptor header does not indicate 'Transmit_MSDU_Link_descriptor'
			
			
			<enum 117 reserved_flush_code_29>TXPCU uses this code when
			 it gets a 'pre_start_tx' pulse from SCH but has not yet
			 got the 'TX_FES_SETUP' TLV
			
			<enum 118 reserved_flush_code_30>Placeholder for future 
			needs
			<enum 119 reserved_flush_code_31>Placeholder for future 
			needs
			<enum 120 reserved_flush_code_32>Placeholder for future 
			needs
			<enum 121 reserved_flush_code_33>Placeholder for future 
			needs
			<enum 122 reserved_flush_code_34>Placeholder for future 
			needs
			<enum 123 reserved_flush_code_35>Placeholder for future 
			needs
			<enum 124 reserved_flush_code_36>Placeholder for future 
			needs
			<enum 125 reserved_flush_code_37>Placeholder for future 
			needs
			<enum 126 reserved_flush_code_38>Placeholder for future 
			needs
			<enum 127 unknown_flush_request_code>Used by SCH when it
			 receives an undefined flush request reason code
*/

#define TX_FLUSH_REQ_FLUSH_REQ_REASON_OFFSET                                        0x0000000000000000
#define TX_FLUSH_REQ_FLUSH_REQ_REASON_LSB                                           0
#define TX_FLUSH_REQ_FLUSH_REQ_REASON_MSB                                           7
#define TX_FLUSH_REQ_FLUSH_REQ_REASON_MASK                                          0x00000000000000ff


/* Description		PHYTX_ABORT_REASON

			Field only valid when Flush_req_reason == TXPCU_PHYTX_ABORT_ERR
			
			
			<enum 0 no_phytx_error_reported>This value is the default
			 value the MAC will fill in the status TLV (when not PHY
			 abort was received).
			
			Note that when PHY generates the PHYTX_ABORT_REQUEST, this
			 value shall never be used.
			<enum 1 error_txtd_ifft_underrun>PHY ran out of transmit
			 data due to transmit underrun - this field is user-specific
			 (see user_number field)
			<enum 2 error_tx_invalid_tlv>
			<enum 3 error_tx_unexpected_tlv>
			<enum 4 error_tx_pkt_end_error>
			<enum 5 error_tx_bw_is_gt_dyn_bw>
			<enum 6 error_txtd_pkt_start_error>
			<enum 7 error_txfd_pre_phy_tlv_ooo>
			<enum 8 error_txtd_mu_data_underrun>
			<enum 9 error_tx_legacy_rate_illegal>
			<enum 10 error_tx_fifo_error>
			<enum 11 error_tx_ack_wd_error>
			<enum 12 error_tx_tpc_miss>
			<enum 13 error_mac_tx_abort>
			<enum 14 error_tx_pcss_phy_desc_wdg_timeout>
			<enum 15 error_unsupported_cbf>
			<enum 16 error_cv_static_bandwidth_mismatch>
			<enum 17 error_cv_dynamic_bandwidth_mismatch>
			<enum 18 error_cv_unsupported_nss_total>
			<enum 19 error_nss_bf_params_mismatch>
			<enum 20 error_txbf_fail>
			<enum 21 error_txbf_snd_fail>This used to be called 'error_illegal_nss.'
			
			<enum 22 error_otp_txbf>
			<enum 23 error_tx_inv_chainmask>
			<enum 24 error_cv_index_assign_overload>This error indicates
			 that CV prefetch command indicated a CV index that is not
			 available.
			<enum 25 error_cv_index_delete>This error indicates that
			 CV delete command indicated a CV index that did not contain
			 any valid info
			<enum 26 error_tx_he_rate_illegal>Error found with the HE
			 transmission parameters
			<enum 27 error_tx_pcss_wdg_timeout>
			<enum 28 error_tx_tlv_tag_mismatch>
			<enum 29 error_tx_cck_fifo_flush>
			<enum 30 error_tx_no_mac_pkt_end>
			<enum 31 error_tx_abort_for_mac_war>
			<enum 32 error_tx_stuck>
			<enum 33 error_tx_invalid_uplink_tlv>
			<enum 34 error_txfd_txcck_illegal_tx_rate_error>
			<enum 35 error_txfd_txcck_underrun_error>
			<enum 36 error_txfd_mpi_req_grant_error>
			<enum 37 error_txfd_control_tlv_fifo_ovfl_error>
			<enum 38 error_txfd_tlv_fifo_overflow_error>
			<enum 39 error_txfd_data_fifo_underflow_error>
			<enum 40 error_txfd_data_fifo_overflow_error>
			<enum 41 error_txfd_service_fifo_overflow_error>
			<enum 42 error_txfd_he_sigb_fifo_overflow_error>
			<enum 43 error_txfd_spurious_data_fifo_error>
			<enum 44 error_txfd_he_siga_fifo_ovfl_error>
			<enum 45 error_txfd_unknown_tlv_error>
			<enum 46 error_txfd_mac_response_ordering_error>
			<enum 47 error_txfd_unexpected_mac_pkt_end_error>
			<enum 48 error_txfd_tlv_fifo_rd_hang_error>All FIFO read
			 hang errors use this value.
			<enum 49 error_txfd_tlv_fifo_no_rd_error>All FIFO no read
			 errors use this value.
			<enum 50 error_txfd_ordering_fifo_no_rd_error>
			<enum 51 error_txfd_illegal_cf_tlv_error>
			<enum 52 error_txfd_user_ru_hang_error>
			<enum 53 error_txfd_stream_ru_hang_error>
			<enum 54 error_txfd_num_pad_bits_error>
			<enum 55 error_txfd_phy_abort_ack_wd_to_error>
			<enum 56 error_txfd_pre_pkt_isr_not_done_before_phy_desc_error>
			
			<enum 57 error_txfd_bf_weights_not_ready_error>
			<enum 58 error_txfd_req_timer_breach_error>
			<enum 59 error_txfd_wd_to_error>
			<enum 60 error_txfd_legacy_bf_weights_not_ready_error>
			<enum 61 error_txfd_axi_slave_to_error>
			<enum 62 error_txfd_hw_acc_error>
			<enum 63 error_txfd_txb_req_fifo_underrun_error>
			<enum 64 error_txfd_unknown_ru_alloc_error>
			<enum 65 error_txfd_more_user_desc_per_user_tlvs_error>
			<enum 66 error_txfd_ldpc_param_calc_to_error>
			<enum 69 error_txfd_cbf_start_before_expect_cbf_clear_error>
			
			<enum 70 error_txfd_out_of_range_cbf_user_id_error>
			<enum 71 error_txfd_less_cbf_data_error>
			<enum 72 error_txfd_more_cbf_data_error>
			<enum 73 error_txfd_cbf_done_not_received_error>
			<enum 74 error_txfd_mpi_cbf_valid_to_error>
			<enum 75 error_txfd_cbf_start_missing_error>
			<enum 76 error_txfd_mimo_ctrl_error>
			<enum 77 error_txfd_cbf_buffer_ovfl_error>
			<enum 78 error_txfd_dma0_hang_error>
			<enum 79 error_txfd_dma1_hang_error>
			<enum 80 error_txfd_b2b_cbf_start_error>
			<enum 81 error_txfd_b2b_cbf_done_error>
			<enum 82 error_txfd_unsaved_cv_error>
			<enum 83 error_txfd_wt_mem_wr_conflict_error>
			<enum 84 error_txfd_wt_mem_rd_conflict_error>
			<enum 85 error_txfd_qre_intf_to_error>
			<enum 86 error_txfd_qre_txbf_stomp_rx_error>
			<enum 87 error_txfd_qre_rx_stomp_txbf_error>
			<enum 88 error_txfd_precoding_start_before_bf_param_clr_error>
			
			<enum 89 error_txfd_tone_map_lut_rd_conflict_error>
			<enum 90 error_txfd_precoding_fifo_ovfl_error>
			<enum 91 error_txfd_precoding_fifo_udfl_error>
			<enum 92 error_txfd_txbf_axi_slave_to_error>
			<enum 93 error_txfd_less_prefetch_tlvs_error>
			<enum 94 error_txfd_more_prefetch_tlvs_error>
			<enum 95 error_txfd_prefetch_fifo_ovfl_error>
			<enum 96 error_txfd_prefetch_fifo_udfl_error>
			<enum 97 error_txfd_precoding_error>
			<enum 98 error_txfd_cv_ctrl_state_to_error>
			<enum 99 error_txfd_txbfp_qre_tone_udfl_error>
			<enum 100 error_txfd_less_bf_param_per_user_tlvs_error>
			<enum 101 error_txfd_more_bf_param_per_user_tlvs_error>
			<enum 102 error_txfd_bf_param_common_unexpected_error>
			<enum 103 error_txfd_less_expect_cbf_per_user_tlvs_error>
			
			<enum 104 error_txfd_more_expect_cbf_per_user_tlvs_error>
			
			<enum 105 error_txfd_precoding_stg1_stg2_wait_to_error>
			<enum 106 error_txfd_expect_cbf_per_user_before_common_error>
			
			<enum 107 error_txfd_prefetch_per_user_before_common_error>
			
			<enum 108 error_txfd_bf_param_per_user_before_common_error>
			
			<enum 109 error_txfd_ndp_cbf_bw_mismatch_error>
			<enum 110 error_txtd_tx_pre_desc_error>
			<enum 111 error_txtd_tx_desc_error>
			<enum 112 error_txtd_start_error>
			<enum 113 error_txtd_sym_error>
			<enum 114 error_txtd_multi_sym_error>
			<enum 115 error_txtd_pre_data_error>
			<enum 116 error_txtd_pkt_data_error>
			<enum 117 error_txtd_pkt_end_error>
			<enum 118 error_txtd_tx_frame_unexp>
			<enum 119 error_txtd_start_unexp>
			<enum 120 error_txtd_fft_error_1>
			<enum 121 error_txtd_fft_error_2>
			<enum 122 error_txtd_uld_sym_cp_len_zero>
			<enum 123 error_txtd_start_done>
			<enum 124 error_txtd_start_nonidle>
			<enum 125 error_txtd_tx_abort_nonidle>
			<enum 126 error_txtd_tx_abort_done>
			<enum 127 error_txtd_tx_abort_idle>
			<enum 128 error_txtd_cck_sample_overflow>
			<enum 129 error_txtd_cck_timeout>
			<enum 130 error_txtd_ofdm_sym_mismatch>
			<enum 131 error_txtd_tx_vld_unalign_error>
			<enum 132 error_txtd_fft_cdc_fifo>This is the merged Rx/Tx
			 CDC FIFO empty/full error code
			<enum 133 error_mac_tb_ppdu_abort>All 'error_txtd_chn' codes
			 use this value as well.
			<enum 136 error_abort_req_from_macrx_enum_05>This code is
			 used to abort the Tx when MAC Rx issues an abort request
			 with code 05 "macrx_abort_too_much_bad_data."
			<enum 137 error_tx_extra_sym_mismatch>
			<enum 138 error_tx_vht_length_not_multiple_of_3>
			<enum 139 error_tx_11b_rate_illegal>
			<enum 140 error_tx_ht_rate_illegal>
			<enum 141 error_tx_vht_rate_illegal>
			<enum 142 error_mac_rf_only_abort>
			<enum 255 error_tx_invalid_error_code>
*/

#define TX_FLUSH_REQ_PHYTX_ABORT_REASON_OFFSET                                      0x0000000000000000
#define TX_FLUSH_REQ_PHYTX_ABORT_REASON_LSB                                         8
#define TX_FLUSH_REQ_PHYTX_ABORT_REASON_MSB                                         15
#define TX_FLUSH_REQ_PHYTX_ABORT_REASON_MASK                                        0x000000000000ff00


/* Description		FLUSH_REQ_USER_NUMBER_OR_LINK_ID

			Field only valid when Flush_req_reason == TXPCU_PHYTX_ABORT_ERR
			 or PDG_FLREQ_CODE_{TXOP, MLO}_ABORT
			
			In case of TXPCU_PHYTX_ABORT_ERR, for some errors, the user
			 for which this error was detected can be indicated in this
			 field.
			
			In case of PDG_FLREQ_CODE_*_ABORT due to MLO, this field
			 will carry the partner link ID and validity due to which
			 the abort was initiated.
			Bit [5]: partner link ID valid
			Bits [4:3]: set to 0 
			Bits [2:0]: partner link ID
			<legal 0-39>
*/

#define TX_FLUSH_REQ_FLUSH_REQ_USER_NUMBER_OR_LINK_ID_OFFSET                        0x0000000000000000
#define TX_FLUSH_REQ_FLUSH_REQ_USER_NUMBER_OR_LINK_ID_LSB                           16
#define TX_FLUSH_REQ_FLUSH_REQ_USER_NUMBER_OR_LINK_ID_MSB                           21
#define TX_FLUSH_REQ_FLUSH_REQ_USER_NUMBER_OR_LINK_ID_MASK                          0x00000000003f0000


/* Description		MLO_ABORT_REASON

			Field valid only when Flush_req_reason == PDG_FLREQ_CODE_{TXOP, 
			MLO}_ABORT
			
			<enum 0 sw_blocked_self> SW-specified block of the peer 
			for self-link
			<enum 1 sw_blocked_partner> SW-specified block of the peer
			 from a partner link
			<enum 2 rx_ongoing> Blocked due to RX ongoing in partner
			 link
			<enum 3 cts2self_truncated> MLO truncated CTS2SELF leading
			 to abort
			<enum 4 max_padding_exceeded> Maximum padding exceeded
			<enum 5 max_overlap_exceeded> Maximum overlap duration exceeded
			
			<enum 6 user_collision_threshold_exceeded> User collision
			 threshold for MU exceeded
			<enum 7 sw_blocked_vdev_id> SW-specified block due to VDEV
			 ID collision with a non-MLO broadcast/multicast
			<enum 8 r2r_response_truncated> 
			<enum 10 emlsr_blackout> Blocked due to EMLSR black-out 
			window
			<enum 16 t2_response_changed> T2 response changed in 'MLO_TX_RESP'
			
			<enum 17 ppdu_duration_zero> PPDU duration zero in 'MLO_TX_RESP'
			
			<enum 18 ppdu_duration_bigger_than_allowed> PPDU duration
			 bigger than allowed in non-response mode 'MLO_TX_RESP'
			<enum 19 ppdu_padding_not_allowed> PPDU in non-A-MPDU format
			 cannot be padded
			<enum 20 resp_ppdu_duration_truncated> PPDU duration truncated
			 in response mode 'MLO_TX_RESP'
			<enum 21 ppdu_duration_limit> flush generated due to TXOP
			 abort
			<enum 22 overview_mpdu_cnt_zero> flush generated due to 
			TXOP abort as MPDU count is zero for all users in 'MPDU_QUEUE_OVERVIEW'
			
			<enum 23 overview_not_ready> flush generated due to MLO 
			abort as 'MPDU_QUEUE_OVERVIEW' is not ready for all users
			 at PPDU phase
			<enum 24 trigger_frame_mlo_alignment_fail> Trigger frame
			 end-alignment cannot be met, e.g. due to LDPC extra symbol
			
			<enum 9 mlo_reserved>
*/

#define TX_FLUSH_REQ_MLO_ABORT_REASON_OFFSET                                        0x0000000000000000
#define TX_FLUSH_REQ_MLO_ABORT_REASON_LSB                                           22
#define TX_FLUSH_REQ_MLO_ABORT_REASON_MSB                                           26
#define TX_FLUSH_REQ_MLO_ABORT_REASON_MASK                                          0x0000000007c00000


/* Description		RESERVED_0A

			<legal 0>
*/

#define TX_FLUSH_REQ_RESERVED_0A_OFFSET                                             0x0000000000000000
#define TX_FLUSH_REQ_RESERVED_0A_LSB                                                27
#define TX_FLUSH_REQ_RESERVED_0A_MSB                                                31
#define TX_FLUSH_REQ_RESERVED_0A_MASK                                               0x00000000f8000000


/* Description		TLV64_PADDING

			Automatic DWORD padding inserted while converting TLV32 
			to TLV64 for 64 bit ARCH
			<legal 0>
*/

#define TX_FLUSH_REQ_TLV64_PADDING_OFFSET                                           0x0000000000000000
#define TX_FLUSH_REQ_TLV64_PADDING_LSB                                              32
#define TX_FLUSH_REQ_TLV64_PADDING_MSB                                              63
#define TX_FLUSH_REQ_TLV64_PADDING_MASK                                             0xffffffff00000000



#endif   // TX_FLUSH_REQ
