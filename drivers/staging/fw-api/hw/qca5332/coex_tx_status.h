
/* Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
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

 
 
 
 
 
 
 


#ifndef _COEX_TX_STATUS_H_
#define _COEX_TX_STATUS_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_COEX_TX_STATUS 4

#define NUM_OF_QWORDS_COEX_TX_STATUS 2


struct coex_tx_status {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t reserved_0a                                             :  7, // [6:0]
                      tx_bw                                                   :  3, // [9:7]
                      tx_status_reason                                        :  3, // [12:10]
                      tx_wait_ack                                             :  1, // [13:13]
                      fes_tx_is_gen_frame                                     :  1, // [14:14]
                      sch_tx_burst_ongoing                                    :  1, // [15:15]
                      current_tx_duration                                     : 16; // [31:16]
             uint32_t next_rx_active_time                                     : 16, // [15:0]
                      remaining_fes_time                                      : 16; // [31:16]
             uint32_t tx_antenna_mask                                         :  8, // [7:0]
                      shared_ant_tx_pwr                                       :  8, // [15:8]
                      other_ant_tx_pwr                                        :  8, // [23:16]
                      reserved_2                                              :  8; // [31:24]
             uint32_t tlv64_padding                                           : 32; // [31:0]
#else
             uint32_t current_tx_duration                                     : 16, // [31:16]
                      sch_tx_burst_ongoing                                    :  1, // [15:15]
                      fes_tx_is_gen_frame                                     :  1, // [14:14]
                      tx_wait_ack                                             :  1, // [13:13]
                      tx_status_reason                                        :  3, // [12:10]
                      tx_bw                                                   :  3, // [9:7]
                      reserved_0a                                             :  7; // [6:0]
             uint32_t remaining_fes_time                                      : 16, // [31:16]
                      next_rx_active_time                                     : 16; // [15:0]
             uint32_t reserved_2                                              :  8, // [31:24]
                      other_ant_tx_pwr                                        :  8, // [23:16]
                      shared_ant_tx_pwr                                       :  8, // [15:8]
                      tx_antenna_mask                                         :  8; // [7:0]
             uint32_t tlv64_padding                                           : 32; // [31:0]
#endif
};


/* Description		RESERVED_0A

			<legal 0>
*/

#define COEX_TX_STATUS_RESERVED_0A_OFFSET                                           0x0000000000000000
#define COEX_TX_STATUS_RESERVED_0A_LSB                                              0
#define COEX_TX_STATUS_RESERVED_0A_MSB                                              6
#define COEX_TX_STATUS_RESERVED_0A_MASK                                             0x000000000000007f


/* Description		TX_BW

			The BW of the upcoming transmission.
			Note: Coex might have changed this from the original request. 
			See coex related fields below
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define COEX_TX_STATUS_TX_BW_OFFSET                                                 0x0000000000000000
#define COEX_TX_STATUS_TX_BW_LSB                                                    7
#define COEX_TX_STATUS_TX_BW_MSB                                                    9
#define COEX_TX_STATUS_TX_BW_MASK                                                   0x0000000000000380


/* Description		TX_STATUS_REASON

			<enum 0     FES_tx_start> TXPCU sends this status at the
			 start of SCH initiated transmission (when the commands 
			are given to the PHY). This includes the transmission of
			 RTS and CTS
			Note that based on field 'Fes_tx_is_gen_frame' COEX can 
			derive if this is a protection frame or regular PPDU.
			
			<enum 1     FES_tx_end> TXPCU sends this status at the end
			 of SCH initiated transmission (when PHY TX has confirmed
			 the transmit over the medium has finished)
			
			<enum 2     FES_end> TXPCU sends this status at the end 
			of of the entire frame exchange sequence. This includes 
			reception (or lack of..) of the ACK/BA/CTS frame
			TXPCU sends this FES after it has sent the TX_FES_STATUS
			 TLV(s). This also sent in case of 11ax basic trigger response
			 transmissions, when an ACK/BA is expected, and that got
			 received.
			<enum 3     Response_tx_start> TXPCU sends this status at
			 the start of Self gen initiated response transmission (when
			 the commands are given to the PHY)
			<enum 4     Response_tx_end> TXPCU sends this status at 
			the end of Self gen initiated response transmission (when
			 PHY TX has confirmed the transmit over the medium has finished)
			
			
			<enum 5     No_tx_ongoing> TXPCU sends this TLV when forced
			 by SW to do so. It is used to be able to get TXPCU and 
			coex synchronized again in case of some error handling scenarios
			
			
			<legal 0-5>
*/

#define COEX_TX_STATUS_TX_STATUS_REASON_OFFSET                                      0x0000000000000000
#define COEX_TX_STATUS_TX_STATUS_REASON_LSB                                         10
#define COEX_TX_STATUS_TX_STATUS_REASON_MSB                                         12
#define COEX_TX_STATUS_TX_STATUS_REASON_MASK                                        0x0000000000001c00


/* Description		TX_WAIT_ACK

			Field can only be set for the 'FES_tx_end' scenario.
			TXPCU sets this bit to 1 when it is waiting for an ACK/BA
			 or CTS Response.
*/

#define COEX_TX_STATUS_TX_WAIT_ACK_OFFSET                                           0x0000000000000000
#define COEX_TX_STATUS_TX_WAIT_ACK_LSB                                              13
#define COEX_TX_STATUS_TX_WAIT_ACK_MSB                                              13
#define COEX_TX_STATUS_TX_WAIT_ACK_MASK                                             0x0000000000002000


/* Description		FES_TX_IS_GEN_FRAME

			Field only valid in case tx_status_reason indicates FES_tx_start
			 or FES_tx_end.
			
			Field is set to 1 if the frame transmitted is a self generated
			 frame like RTS, CTS 2 self or NDP
*/

#define COEX_TX_STATUS_FES_TX_IS_GEN_FRAME_OFFSET                                   0x0000000000000000
#define COEX_TX_STATUS_FES_TX_IS_GEN_FRAME_LSB                                      14
#define COEX_TX_STATUS_FES_TX_IS_GEN_FRAME_MSB                                      14
#define COEX_TX_STATUS_FES_TX_IS_GEN_FRAME_MASK                                     0x0000000000004000


/* Description		SCH_TX_BURST_ONGOING

			The proposed change by HWSCH  requires TXPCU to reflect 
			TX_FES_SETUP.sch_tx_burst_ongoing field intoCOEX_TX_STATUS.sch_tx_burst_ongoing
			 field, when tx_status_reason is FES_end.
			SCH will overwrite this bit (that is set it to 1), when 
			TXPCU set the tx_status_reason to FES_end, and SCH determines
			 that this FES is followed by other SIFS bursting based 
			Scheduler commands.
			<legal all>
*/

#define COEX_TX_STATUS_SCH_TX_BURST_ONGOING_OFFSET                                  0x0000000000000000
#define COEX_TX_STATUS_SCH_TX_BURST_ONGOING_LSB                                     15
#define COEX_TX_STATUS_SCH_TX_BURST_ONGOING_MSB                                     15
#define COEX_TX_STATUS_SCH_TX_BURST_ONGOING_MASK                                    0x0000000000008000


/* Description		CURRENT_TX_DURATION

			In case of FES related transmission:
			TXPCU sends current transmission time at the beginning of
			 transmission. This time covers the entire (PPDU) tx_frame. 
			This field is only valid when 'tx_status_reason' is equal
			 to FES_tx_start or Response_tx_start. In other scenarios
			 it is set to 0
			In us units <legal all>
*/

#define COEX_TX_STATUS_CURRENT_TX_DURATION_OFFSET                                   0x0000000000000000
#define COEX_TX_STATUS_CURRENT_TX_DURATION_LSB                                      16
#define COEX_TX_STATUS_CURRENT_TX_DURATION_MSB                                      31
#define COEX_TX_STATUS_CURRENT_TX_DURATION_MASK                                     0x00000000ffff0000


/* Description		NEXT_RX_ACTIVE_TIME

			In case of FES transmission:
			The expected receive duration for ACK/CTS/BA frame after
			 current transmission has finished. This field should be
			 set at both the start and end of the transmission.  When
			 no frame reception is expected, this field is 0 
			
			In case of Response transmission or Trigger Response transmission:
			
			The expected receive duration for upcoming reception. This
			 field has the same value as the transmitted duration field.
			
			
			Note that for this scenario, there might be an other TX 
			generated during this specified time. It is not known to
			 this device what the transmitter is planning to do in the
			 remainder of the TXOP. In other words, this value represents
			 the best guess, but might not be fully accurate.
			
			In us units 
			<legal all>
*/

#define COEX_TX_STATUS_NEXT_RX_ACTIVE_TIME_OFFSET                                   0x0000000000000000
#define COEX_TX_STATUS_NEXT_RX_ACTIVE_TIME_LSB                                      32
#define COEX_TX_STATUS_NEXT_RX_ACTIVE_TIME_MSB                                      47
#define COEX_TX_STATUS_NEXT_RX_ACTIVE_TIME_MASK                                     0x0000ffff00000000


/* Description		REMAINING_FES_TIME

			In case of FES transmission:
			TXPCU sends the remaining FES time it expects to occupy 
			the media.
			At the 'FES_tx_start', this value is the current_tx_duration
			 + value of inserted duration field.
			At the 'FES_tx_end', this value is equal to the duration
			 field in the just transmitted frame.
			At the 'FES_end', this value is the remaining FES duration
			 value. Note that this value should only be non zero in 
			case of SIFS burting type of transmissions.
			In case of a FES failure, like reponse frame not received, 
			this field is set to 0
			
			In case of Self Gen response transmission (includes Trigger
			 response):
			At the 'Response_tx_start', this field has the same value
			 as the Current_tx_duration  + inserted duration field
			At the 'Response_tx_end', this field has the same value 
			as the inserted duration field
			<legal all>
*/

#define COEX_TX_STATUS_REMAINING_FES_TIME_OFFSET                                    0x0000000000000000
#define COEX_TX_STATUS_REMAINING_FES_TIME_LSB                                       48
#define COEX_TX_STATUS_REMAINING_FES_TIME_MSB                                       63
#define COEX_TX_STATUS_REMAINING_FES_TIME_MASK                                      0xffff000000000000


/* Description		TX_ANTENNA_MASK

			The actual used antennas for this transmission
			
			For debug purpose only. PDG should not have modified the
			 value given by the Coex.
			
			<legal all>
*/

#define COEX_TX_STATUS_TX_ANTENNA_MASK_OFFSET                                       0x0000000000000008
#define COEX_TX_STATUS_TX_ANTENNA_MASK_LSB                                          0
#define COEX_TX_STATUS_TX_ANTENNA_MASK_MSB                                          7
#define COEX_TX_STATUS_TX_ANTENNA_MASK_MASK                                         0x00000000000000ff


/* Description		SHARED_ANT_TX_PWR

			Actual tx power on the shared antenna
			TXPCU sends at the beginning of transmission when tx_frame
			 is on. 
			
			For debug purpose only. PDG should not have modified the
			 value given by the Coex.
			
			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define COEX_TX_STATUS_SHARED_ANT_TX_PWR_OFFSET                                     0x0000000000000008
#define COEX_TX_STATUS_SHARED_ANT_TX_PWR_LSB                                        8
#define COEX_TX_STATUS_SHARED_ANT_TX_PWR_MSB                                        15
#define COEX_TX_STATUS_SHARED_ANT_TX_PWR_MASK                                       0x000000000000ff00


/* Description		OTHER_ANT_TX_PWR

			Actual tx power on the 'unshared' antenna(s)
			TXPCU sends at the beginning of transmission when tx_frame
			 is on.
			
			For debug purpose only. PDG should not have modified the
			 value given by the Coex.
			
			Transmit Power in s6.2 format. 
			In units of 0.25 dBm
			<legal all>
*/

#define COEX_TX_STATUS_OTHER_ANT_TX_PWR_OFFSET                                      0x0000000000000008
#define COEX_TX_STATUS_OTHER_ANT_TX_PWR_LSB                                         16
#define COEX_TX_STATUS_OTHER_ANT_TX_PWR_MSB                                         23
#define COEX_TX_STATUS_OTHER_ANT_TX_PWR_MASK                                        0x0000000000ff0000


/* Description		RESERVED_2

			Generator should set to 0, consumer shall ignore <legal 
			0>
*/

#define COEX_TX_STATUS_RESERVED_2_OFFSET                                            0x0000000000000008
#define COEX_TX_STATUS_RESERVED_2_LSB                                               24
#define COEX_TX_STATUS_RESERVED_2_MSB                                               31
#define COEX_TX_STATUS_RESERVED_2_MASK                                              0x00000000ff000000


/* Description		TLV64_PADDING

			Automatic DWORD padding inserted while converting TLV32 
			to TLV64 for 64 bit ARCH
			<legal 0>
*/

#define COEX_TX_STATUS_TLV64_PADDING_OFFSET                                         0x0000000000000008
#define COEX_TX_STATUS_TLV64_PADDING_LSB                                            32
#define COEX_TX_STATUS_TLV64_PADDING_MSB                                            63
#define COEX_TX_STATUS_TLV64_PADDING_MASK                                           0xffffffff00000000



#endif   // COEX_TX_STATUS
