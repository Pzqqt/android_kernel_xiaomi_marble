/*
 * Copyright (c) 2015 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

#ifndef _REGTABLE_CE_H_
#define _REGTABLE_CE_H_

/*
 * @d_DST_WR_INDEX_ADDRESS: Destination ring write index
 *
 * @d_SRC_WATERMARK_ADDRESS: Source ring watermark
 *
 * @d_SRC_WATERMARK_LOW_MASK: Bits indicating low watermark from Source ring
 *			      watermark
 *
 * @d_SRC_WATERMARK_HIGH_MASK: Bits indicating high watermark from Source ring
 *			       watermark
 *
 * @d_DST_WATERMARK_LOW_MASK: Bits indicating low watermark from Destination
 *			      ring watermark
 *
 * @d_DST_WATERMARK_HIGH_MASK: Bits indicating high watermark from Destination
 *			       ring watermark
 *
 * @d_CURRENT_SRRI_ADDRESS: Current source ring read index.The Start Offset
 *			    will be reflected after a CE transfer is completed.
 *
 * @d_CURRENT_DRRI_ADDRESS: Current Destination ring read index. The Start
 *			    Offset will be reflected after a CE transfer
 *			    is completed.
 *
 * @d_HOST_IS_SRC_RING_HIGH_WATERMARK_MASK: Source ring high watermark
 *					    Interrupt Status
 *
 * @d_HOST_IS_SRC_RING_LOW_WATERMARK_MASK: Source ring low watermark
 *					   Interrupt Status
 *
 * @d_HOST_IS_DST_RING_HIGH_WATERMARK_MASK: Destination ring high watermark
 *					    Interrupt Status
 *
 * @d_HOST_IS_DST_RING_LOW_WATERMARK_MASK: Source ring low watermark
 *					   Interrupt Status
 *
 * @d_HOST_IS_ADDRESS: Host Interrupt Status Register
 *
 * @d_MISC_IS_ADDRESS: Miscellaneous Interrupt Status Register
 *
 * @d_HOST_IS_COPY_COMPLETE_MASK: Bits indicating Copy complete interrupt
 *				  status from the Host Interrupt Status
 *				  register
 *
 * @d_CE_WRAPPER_BASE_ADDRESS: Copy Engine Wrapper Base Address
 *
 * @d_CE_WRAPPER_INTERRUPT_SUMMARY_ADDRESS: CE Wrapper summary for interrupts
 *					    to host
 *
 * @d_CE_WRAPPER_INDEX_BASE_LOW: The LSB Base address to which source and
 *				 destination read indices are written
 *
 * @d_CE_WRAPPER_INDEX_BASE_HIGH: The MSB Base address to which source and
 *				  destination read indices are written
 *
 * @d_HOST_IE_ADDRESS: Host Line Interrupt Enable Register
 *
 * @d_HOST_IE_COPY_COMPLETE_MASK: Bits indicating Copy complete interrupt
 * 				  enable from the IE register
 *
 * @d_SR_BA_ADDRESS: LSB of Source Ring Base Address
 *
 * @d_SR_BA_ADDRESS_HIGH: MSB of Source Ring Base Address
 *
 * @d_SR_SIZE_ADDRESS: Source Ring size - number of entries and Start Offset
 *
 * @d_CE_CTRL1_ADDRESS: CE Control register
 *
 * @d_CE_CTRL1_DMAX_LENGTH_MASK: Destination buffer Max Length used for error
 * 				 check
 *
 * @d_DR_BA_ADDRESS: Destination Ring Base Address Low
 *
 * @d_DR_BA_ADDRESS_HIGH: Destination Ring Base Address High
 *
 * @d_DR_SIZE_ADDRESS: Destination Ring size - number of entries Start Offset
 *
 * @d_CE_CMD_REGISTER: Implements commands to all CE Halt Flush
 *
 * @d_CE_MSI_ADDRESS: CE MSI LOW Address register
 *
 * @d_CE_MSI_ADDRESS_HIGH: CE MSI High Address register
 *
 * @d_CE_MSI_DATA: CE MSI Data Register
 *
 * @d_CE_MSI_ENABLE_BIT: Bit in CTRL1 register indication the MSI enable
 *
 * @d_MISC_IE_ADDRESS: Miscellaneous Interrupt Enable Register
 *
 * @d_MISC_IS_AXI_ERR_MASK: Bit in Misc IS indicating AXI Timeout Interrupt
 *			    status
 *
 * @d_MISC_IS_DST_ADDR_ERR_MASK: Bit in Misc IS indicating Destination Address
 * 				 Error
 *
 * @d_MISC_IS_SRC_LEN_ERR_MASK: Bit in Misc IS indicating Source Zero Length
 * 				Error Interrupt status
 *
 * @d_MISC_IS_DST_MAX_LEN_VIO_MASK: Bit in Misc IS indicating Destination Max
 *				    Length Violated Interrupt status
 *
 * @d_MISC_IS_DST_RING_OVERFLOW_MASK: Bit in Misc IS indicating Destination
 * 				      Ring Overflow Interrupt status
 *
 * @d_MISC_IS_SRC_RING_OVERFLOW_MASK: Bit in Misc IS indicating Source Ring
 * 				      Overflow Interrupt status
 *
 * @d_SRC_WATERMARK_LOW_LSB: Source Ring Low Watermark LSB
 *
 * @d_SRC_WATERMARK_HIGH_LSB: Source Ring Low Watermark MSB
 *
 * @d_DST_WATERMARK_LOW_LSB: Destination Ring Low Watermark LSB
 *
 * @d_DST_WATERMARK_HIGH_LSB: Destination Ring High Watermark LSB
 *
 *
 * @d_CE_WRAPPER_INTERRUPT_SUMMARY_HOST_MSI_MASK: Bits in
 * 						  d_CE_WRAPPER_INTERRUPT_SUMMARY_ADDR
 * 						  indicating Copy engine
 * 						  miscellaneous interrupt summary
 *
 * @d_CE_WRAPPER_INTERRUPT_SUMMARY_HOST_MSI_LSB:Bits in
 * 						d_CE_WRAPPER_INTERRUPT_SUMMARY_ADDR
 * 						indicating Host interrupts summary
 *
 * @d_CE_CTRL1_DMAX_LENGTH_LSB: LSB of Destination buffer Max Length used for
 *				error check
 *
 * @d_CE_CTRL1_SRC_RING_BYTE_SWAP_EN_MASK: Bits indicating Source ring Byte Swap
 * 					   enable. Treats source ring memory
 * 					   organisation as big-endian
 *
 * @d_CE_CTRL1_DST_RING_BYTE_SWAP_EN_MASK: Bits indicating Destination ring
 * 					   byte swap enable. Treats destination
 * 					   ring memory organisation as big-endian
 *
 * @d_CE_CTRL1_SRC_RING_BYTE_SWAP_EN_LSB: LSB of Source ring Byte Swap enable
 *
 * @d_CE_CTRL1_DST_RING_BYTE_SWAP_EN_LSB: LSB of Destination ring Byte Swap enable
 *
 * @d_CE_WRAPPER_DEBUG_OFFSET: Offset of CE OBS BUS Select register
 *
 * @d_CE_WRAPPER_DEBUG_SEL_MSB: MSB of Control register selecting inputs for
 *				trace/debug
 *
 * @d_CE_WRAPPER_DEBUG_SEL_LSB: LSB of Control register selecting inputs for
 * 				trace/debug
 *
 * @d_CE_WRAPPER_DEBUG_SEL_MASK: Bits indicating Control register selecting
 * 				 inputs for trace/debug
 *
 * @d_CE_DEBUG_OFFSET: Offset of Copy Engine FSM Debug Status
 *
 * @d_CE_DEBUG_SEL_MSB: MSB of Copy Engine FSM Debug Status
 *
 * @d_CE_DEBUG_SEL_LSB: LSB of Copy Engine FSM Debug Status
 *
 * @d_CE_DEBUG_SEL_MASK: Bits indicating Copy Engine FSM Debug Status
 *
*/

struct ce_reg_def {
	/* copy_engine.c */
	uint32_t d_DST_WR_INDEX_ADDRESS;
	uint32_t d_SRC_WATERMARK_ADDRESS;
	uint32_t d_SRC_WATERMARK_LOW_MASK;
	uint32_t d_SRC_WATERMARK_HIGH_MASK;
	uint32_t d_DST_WATERMARK_LOW_MASK;
	uint32_t d_DST_WATERMARK_HIGH_MASK;
	uint32_t d_CURRENT_SRRI_ADDRESS;
	uint32_t d_CURRENT_DRRI_ADDRESS;
	uint32_t d_HOST_IS_SRC_RING_HIGH_WATERMARK_MASK;
	uint32_t d_HOST_IS_SRC_RING_LOW_WATERMARK_MASK;
	uint32_t d_HOST_IS_DST_RING_HIGH_WATERMARK_MASK;
	uint32_t d_HOST_IS_DST_RING_LOW_WATERMARK_MASK;
	uint32_t d_HOST_IS_ADDRESS;
	uint32_t d_MISC_IS_ADDRESS;
	uint32_t d_HOST_IS_COPY_COMPLETE_MASK;
	uint32_t d_CE_WRAPPER_BASE_ADDRESS;
	uint32_t d_CE_WRAPPER_INTERRUPT_SUMMARY_ADDRESS;
	uint32_t d_CE_DDR_ADDRESS_FOR_RRI_LOW;
	uint32_t d_CE_DDR_ADDRESS_FOR_RRI_HIGH;
	uint32_t d_HOST_IE_ADDRESS;
	uint32_t d_HOST_IE_COPY_COMPLETE_MASK;
	uint32_t d_SR_BA_ADDRESS;
	uint32_t d_SR_BA_ADDRESS_HIGH;
	uint32_t d_SR_SIZE_ADDRESS;
	uint32_t d_CE_CTRL1_ADDRESS;
	uint32_t d_CE_CTRL1_DMAX_LENGTH_MASK;
	uint32_t d_DR_BA_ADDRESS;
	uint32_t d_DR_BA_ADDRESS_HIGH;
	uint32_t d_DR_SIZE_ADDRESS;
	uint32_t d_CE_CMD_REGISTER;
	uint32_t d_CE_MSI_ADDRESS;
	uint32_t d_CE_MSI_ADDRESS_HIGH;
	uint32_t d_CE_MSI_DATA;
	uint32_t d_CE_MSI_ENABLE_BIT;
	uint32_t d_MISC_IE_ADDRESS;
	uint32_t d_MISC_IS_AXI_ERR_MASK;
	uint32_t d_MISC_IS_DST_ADDR_ERR_MASK;
	uint32_t d_MISC_IS_SRC_LEN_ERR_MASK;
	uint32_t d_MISC_IS_DST_MAX_LEN_VIO_MASK;
	uint32_t d_MISC_IS_DST_RING_OVERFLOW_MASK;
	uint32_t d_MISC_IS_SRC_RING_OVERFLOW_MASK;
	uint32_t d_SRC_WATERMARK_LOW_LSB;
	uint32_t d_SRC_WATERMARK_HIGH_LSB;
	uint32_t d_DST_WATERMARK_LOW_LSB;
	uint32_t d_DST_WATERMARK_HIGH_LSB;
	uint32_t d_CE_WRAPPER_INTERRUPT_SUMMARY_HOST_MSI_MASK;
	uint32_t d_CE_WRAPPER_INTERRUPT_SUMMARY_HOST_MSI_LSB;
	uint32_t d_CE_CTRL1_DMAX_LENGTH_LSB;
	uint32_t d_CE_CTRL1_SRC_RING_BYTE_SWAP_EN_MASK;
	uint32_t d_CE_CTRL1_DST_RING_BYTE_SWAP_EN_MASK;
	uint32_t d_CE_CTRL1_SRC_RING_BYTE_SWAP_EN_LSB;
	uint32_t d_CE_CTRL1_DST_RING_BYTE_SWAP_EN_LSB;
	uint32_t d_CE_CTRL1_IDX_UPD_EN_MASK;
	uint32_t d_CE_WRAPPER_DEBUG_OFFSET;
	uint32_t d_CE_WRAPPER_DEBUG_SEL_MSB;
	uint32_t d_CE_WRAPPER_DEBUG_SEL_LSB;
	uint32_t d_CE_WRAPPER_DEBUG_SEL_MASK;
	uint32_t d_CE_DEBUG_OFFSET;
	uint32_t d_CE_DEBUG_SEL_MSB;
	uint32_t d_CE_DEBUG_SEL_LSB;
	uint32_t d_CE_DEBUG_SEL_MASK;
	uint32_t d_CE0_BASE_ADDRESS;
	uint32_t d_CE1_BASE_ADDRESS;
	uint32_t d_A_WIFI_APB_3_A_WCMN_APPS_CE_INTR_ENABLES;
	uint32_t d_A_WIFI_APB_3_A_WCMN_APPS_CE_INTR_STATUS;
};
#endif /* _REGTABLE_CE_H_ */
