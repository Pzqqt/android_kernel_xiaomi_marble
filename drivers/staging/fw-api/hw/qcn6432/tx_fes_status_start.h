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

#ifndef _TX_FES_STATUS_START_H_
#define _TX_FES_STATUS_START_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_TX_FES_STATUS_START 4

#define NUM_OF_QWORDS_TX_FES_STATUS_START 2


struct tx_fes_status_start {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t schedule_id                                             : 32; // [31:0]
             uint32_t reserved_1a                                             :  8, // [7:0]
                      transmit_start_reason                                   :  3, // [10:8]
                      disabled_user_bitmap_36_32                              :  5, // [15:11]
                      schedule_cmd_ring_id                                    :  5, // [20:16]
                      fes_control_mode                                        :  2, // [22:21]
                      schedule_try                                            :  4, // [26:23]
                      medium_prot_type                                        :  3, // [29:27]
                      reserved_1b                                             :  2; // [31:30]
             uint32_t optimal_bw_try_count                                    :  4, // [3:0]
                      number_of_users                                         :  7, // [10:4]
                      coex_nack_count                                         :  5, // [15:11]
                      cca_ed0                                                 : 16; // [31:16]
             uint32_t disabled_user_bitmap_31_0                               : 32; // [31:0]
#else
             uint32_t schedule_id                                             : 32; // [31:0]
             uint32_t reserved_1b                                             :  2, // [31:30]
                      medium_prot_type                                        :  3, // [29:27]
                      schedule_try                                            :  4, // [26:23]
                      fes_control_mode                                        :  2, // [22:21]
                      schedule_cmd_ring_id                                    :  5, // [20:16]
                      disabled_user_bitmap_36_32                              :  5, // [15:11]
                      transmit_start_reason                                   :  3, // [10:8]
                      reserved_1a                                             :  8; // [7:0]
             uint32_t cca_ed0                                                 : 16, // [31:16]
                      coex_nack_count                                         :  5, // [15:11]
                      number_of_users                                         :  7, // [10:4]
                      optimal_bw_try_count                                    :  4; // [3:0]
             uint32_t disabled_user_bitmap_31_0                               : 32; // [31:0]
#endif
};


/* Description		SCHEDULE_ID

			A field that SW can use to link this FES status to the schedule
			 command that originated this transmission.
*/

#define TX_FES_STATUS_START_SCHEDULE_ID_OFFSET                                      0x0000000000000000
#define TX_FES_STATUS_START_SCHEDULE_ID_LSB                                         0
#define TX_FES_STATUS_START_SCHEDULE_ID_MSB                                         31
#define TX_FES_STATUS_START_SCHEDULE_ID_MASK                                        0x00000000ffffffff



#define TX_FES_STATUS_START_RESERVED_1A_OFFSET                                      0x0000000000000000
#define TX_FES_STATUS_START_RESERVED_1A_LSB                                         32
#define TX_FES_STATUS_START_RESERVED_1A_MSB                                         39
#define TX_FES_STATUS_START_RESERVED_1A_MASK                                        0x000000ff00000000


/* Description		TRANSMIT_START_REASON

			Indicates what the SCH start reason reason was for initiating
			 this transmission.
			
			<enum 0 BO_based_transmit_start> The transmission of this
			 PPDU got initiated by the scheduler due to Backoff expiration
			
			<enum 1 Trigger_based_transmit_start> The transmission of
			 this PPDU got initiated by the scheduler due to reception
			 (by the SCH) of the TLV RECEIVED_TRIGGER_INFO that RXPCU
			 generated. Note that this can be an OFDMA trigger frame
			 based transmission as well as some legacy trigger (PS-POLL, 
			Qboost, U-APSD, etc.)  based transmission
			<enum 2 Sifs_continuation_in_ongoing_burst> This transmission
			 of this PPDU got initiated as part of SIFS continuation. 
			An earlier PPDU was transmitted due to RBO expiration. Next
			 command is also expected to be transmitted in SIFS burst.
			
			<enum 3 Sifs_continuation_last_command> This transmission
			 of this PPDU got initiated as part of SIFS continuation
			 and this is the last command in the burst. An earlier PPDU
			 was transmitted due to RBO expiration.
			<enum 4 NTBR_response_start> DO NOT USE
			<legal 0-4>
*/

#define TX_FES_STATUS_START_TRANSMIT_START_REASON_OFFSET                            0x0000000000000000
#define TX_FES_STATUS_START_TRANSMIT_START_REASON_LSB                               40
#define TX_FES_STATUS_START_TRANSMIT_START_REASON_MSB                               42
#define TX_FES_STATUS_START_TRANSMIT_START_REASON_MASK                              0x0000070000000000


/* Description		DISABLED_USER_BITMAP_36_32

			Bitmap of users that are disabled for this transmission, 
			MSB 5 bits
			
			TXPCU converts disabled_group_bitmap_* in 'PCU_PPDU_SETUP_START' 
			from groups to users.
			<legal all>
*/

#define TX_FES_STATUS_START_DISABLED_USER_BITMAP_36_32_OFFSET                       0x0000000000000000
#define TX_FES_STATUS_START_DISABLED_USER_BITMAP_36_32_LSB                          43
#define TX_FES_STATUS_START_DISABLED_USER_BITMAP_36_32_MSB                          47
#define TX_FES_STATUS_START_DISABLED_USER_BITMAP_36_32_MASK                         0x0000f80000000000


/* Description		SCHEDULE_CMD_RING_ID

			The schedule command ring  that originated this transmission
			
			<enum 0 sch_cmd_ring_number0>
			<enum 1 sch_cmd_ring_number1>
			<enum 2 sch_cmd_ring_number2>
			<enum 3 sch_cmd_ring_number3>
			<enum 4 sch_cmd_ring_number4>
			<enum 5 sch_cmd_ring_number5>
			<enum 6 sch_cmd_ring_number6>
			<enum 7 sch_cmd_ring_number7>
			<enum 8 sch_cmd_ring_number8>
			<enum 9 sch_cmd_ring_number9>
			<enum 10 sch_cmd_ring_number10>
			<enum 11 sch_cmd_ring_number11>
			<enum 12 sch_cmd_ring_number12>
			<enum 13 sch_cmd_ring_number13>
			<enum 14 sch_cmd_ring_number14>
			<enum 15 sch_cmd_ring_number15>
			<enum 16 sch_cmd_ring_number16>
			<enum 17 sch_cmd_ring_number17>
			<enum 18 sch_cmd_ring_number18>
			<enum 19 sch_cmd_ring_number19>
			<enum 20 sch_cmd_ring_number20>
			
			 <legal 0-20>
*/

#define TX_FES_STATUS_START_SCHEDULE_CMD_RING_ID_OFFSET                             0x0000000000000000
#define TX_FES_STATUS_START_SCHEDULE_CMD_RING_ID_LSB                                48
#define TX_FES_STATUS_START_SCHEDULE_CMD_RING_ID_MSB                                52
#define TX_FES_STATUS_START_SCHEDULE_CMD_RING_ID_MASK                               0x001f000000000000


/* Description		FES_CONTROL_MODE

			<enum 0  SW_transmit_mode>  No HW generated TLVs
			<enum 1 PDG_transmit_mode> PDG  is activated to generate
			 TLVs
			
			
			Note: Final Bandwidth selection is always performed by TX
			 PCU.
			
			<legal 0-1>
*/

#define TX_FES_STATUS_START_FES_CONTROL_MODE_OFFSET                                 0x0000000000000000
#define TX_FES_STATUS_START_FES_CONTROL_MODE_LSB                                    53
#define TX_FES_STATUS_START_FES_CONTROL_MODE_MSB                                    54
#define TX_FES_STATUS_START_FES_CONTROL_MODE_MASK                                   0x0060000000000000


/* Description		SCHEDULE_TRY

			The number of times this scheduler command has been tried
			
			<legal all>
*/

#define TX_FES_STATUS_START_SCHEDULE_TRY_OFFSET                                     0x0000000000000000
#define TX_FES_STATUS_START_SCHEDULE_TRY_LSB                                        55
#define TX_FES_STATUS_START_SCHEDULE_TRY_MSB                                        58
#define TX_FES_STATUS_START_SCHEDULE_TRY_MASK                                       0x0780000000000000


/* Description		MEDIUM_PROT_TYPE

			Self Gen Medium Prot type used
			<enum 0 No_protection>
			<enum 1 RTS_legacy>
			<enum 2 RTS_11ac_static_bw>
			<enum 3 RTS_11ac_dynamic_bw>
			<enum 4 CTS2Self>
			<enum 5 QoS_Null_no_ack_3addr>
			<enum 6 QoS_Null_no_ack_4addr>
			
			<legal 0-6>
			Field only valid for user0 FES status.
*/

#define TX_FES_STATUS_START_MEDIUM_PROT_TYPE_OFFSET                                 0x0000000000000000
#define TX_FES_STATUS_START_MEDIUM_PROT_TYPE_LSB                                    59
#define TX_FES_STATUS_START_MEDIUM_PROT_TYPE_MSB                                    61
#define TX_FES_STATUS_START_MEDIUM_PROT_TYPE_MASK                                   0x3800000000000000


/* Description		RESERVED_1B

			<legal 0>
*/

#define TX_FES_STATUS_START_RESERVED_1B_OFFSET                                      0x0000000000000000
#define TX_FES_STATUS_START_RESERVED_1B_LSB                                         62
#define TX_FES_STATUS_START_RESERVED_1B_MSB                                         63
#define TX_FES_STATUS_START_RESERVED_1B_MASK                                        0xc000000000000000


/* Description		OPTIMAL_BW_TRY_COUNT

			This field indicates how many times this scheduling command
			 has been flushed by TXPCU  as a result of most desired 
			BW not being available.
			<legal all>
*/

#define TX_FES_STATUS_START_OPTIMAL_BW_TRY_COUNT_OFFSET                             0x0000000000000008
#define TX_FES_STATUS_START_OPTIMAL_BW_TRY_COUNT_LSB                                0
#define TX_FES_STATUS_START_OPTIMAL_BW_TRY_COUNT_MSB                                3
#define TX_FES_STATUS_START_OPTIMAL_BW_TRY_COUNT_MASK                               0x000000000000000f


/* Description		NUMBER_OF_USERS

			The number of users in this transmission.
*/

#define TX_FES_STATUS_START_NUMBER_OF_USERS_OFFSET                                  0x0000000000000008
#define TX_FES_STATUS_START_NUMBER_OF_USERS_LSB                                     4
#define TX_FES_STATUS_START_NUMBER_OF_USERS_MSB                                     10
#define TX_FES_STATUS_START_NUMBER_OF_USERS_MASK                                    0x00000000000007f0


/* Description		COEX_NACK_COUNT

			Consumer: SCH
			Producer: TXPCU
			
			The number of times PDG informed the SCHeduler module that
			 for this scheduling command, the WLAN transmission can 
			not be initialized due to getting a NACK response from the
			 Coex engine, or PDG not being able to fit a transmission
			 within the timing constraints given by Coex.
			
			Note that SCH will (re)set this count to 0 at the start 
			of reading a new SCH command.
			This count is maintained on a per ring basis by the SCHeduler
			
			
			
			<legal all>
*/

#define TX_FES_STATUS_START_COEX_NACK_COUNT_OFFSET                                  0x0000000000000008
#define TX_FES_STATUS_START_COEX_NACK_COUNT_LSB                                     11
#define TX_FES_STATUS_START_COEX_NACK_COUNT_MSB                                     15
#define TX_FES_STATUS_START_COEX_NACK_COUNT_MASK                                    0x000000000000f800


/* Description		CCA_ED0

			Used by TXPCU to report CCA status at time of transmit bandwidth
			 selection.  Each bit is a sample of BUSY/IDLE of ED[0] (as
			 provided by SCH to TXPCU) for each 20 MHz sub-band. These
			 stats could potentially be used in future for rate adaptation.
			
			<legal all>
*/

#define TX_FES_STATUS_START_CCA_ED0_OFFSET                                          0x0000000000000008
#define TX_FES_STATUS_START_CCA_ED0_LSB                                             16
#define TX_FES_STATUS_START_CCA_ED0_MSB                                             31
#define TX_FES_STATUS_START_CCA_ED0_MASK                                            0x00000000ffff0000


/* Description		DISABLED_USER_BITMAP_31_0

			Bitmap of users that are disabled for this transmission, 
			LSB 32 bits
			
			TXPCU converts disabled_group_bitmap_* in 'PCU_PPDU_SETUP_START' 
			from groups to users.
			
			<legal all>
*/

#define TX_FES_STATUS_START_DISABLED_USER_BITMAP_31_0_OFFSET                        0x0000000000000008
#define TX_FES_STATUS_START_DISABLED_USER_BITMAP_31_0_LSB                           32
#define TX_FES_STATUS_START_DISABLED_USER_BITMAP_31_0_MSB                           63
#define TX_FES_STATUS_START_DISABLED_USER_BITMAP_31_0_MASK                          0xffffffff00000000



#endif   // TX_FES_STATUS_START
