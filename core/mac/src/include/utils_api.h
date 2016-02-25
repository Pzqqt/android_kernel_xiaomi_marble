/*
 * Copyright (c) 2011-2016 The Linux Foundation. All rights reserved.
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

#ifndef __UTILSAPI_H
#define __UTILSAPI_H

#include <stdarg.h>
#include <sir_common.h>
#include "ani_global.h"
#include "utils_global.h"
#include "sys_wrapper.h"

/* / System role definition on a per BSS */
typedef enum eBssSystemRole {
	eSYSTEM_UNKNOWN_ROLE,
	eSYSTEM_AP_ROLE,
	eSYSTEM_STA_IN_IBSS_ROLE,
	eSYSTEM_STA_ROLE,
	eSYSTEM_BTAMP_STA_ROLE,
	eSYSTEM_BTAMP_AP_ROLE,

	eSYSTEM_LAST_ROLE,
	eSYSTEM_MULTI_BSS_ROLE = eSYSTEM_LAST_ROLE
} tBssSystemRole;

#define LOG_INDEX_FOR_MODULE(modId) ((modId) - LOG_FIRST_MODULE_ID)
#define GET_MIN_VALUE(__val1, __val2) ((__val1 < __val2) ? __val1 : __val2)

/* The caller must check loglevel. This API assumes loglevel is good */
extern void log_debug(tpAniSirGlobal pMac, uint8_t modId, uint32_t debugLevel,
		      const char *pStr, va_list marker);

extern void log_dbg(tpAniSirGlobal pMac, uint8_t modId, uint32_t debugLevel,
		    const char *pStr, ...);

extern uint32_t gPktAllocCnt, gPktFreeCnt;

extern QDF_TRACE_LEVEL get_vos_debug_level(uint32_t debugLevel);

/* / Log initialization */
extern tSirRetStatus log_init(tpAniSirGlobal);

extern void log_deinit(tpAniSirGlobal);

extern tSirRetStatus cfg_init(tpAniSirGlobal);
extern void cfg_de_init(tpAniSirGlobal);

void sir_dump_buf(tpAniSirGlobal pMac, uint8_t modId, uint32_t level,
		  uint8_t *buf, uint32_t size);

/**
 * sir_swap_u16()
 *
 * FUNCTION:
 * This function is called to swap two U8s of an uint16_t value
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 * None.
 *
 * NOTE:
 *
 * @param  val    uint16_t value to be uint8_t swapped
 * @return        Swapped uint16_t value
 */

static inline uint16_t sir_swap_u16(uint16_t val)
{
	return ((val & 0x00FF) << 8) | ((val & 0xFF00) >> 8);
} /*** end sir_swap_u16() ***/

/**
 * sir_swap_u16if_needed()
 *
 * FUNCTION:
 * This function is called to swap two U8s of an uint16_t value depending
 * on endiannes of the target processor/compiler the software is
 * running on
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 * None.
 *
 * NOTE:
 *
 * @param  val    uint16_t value to be uint8_t swapped
 * @return        Swapped uint16_t value
 */

static inline uint16_t sir_swap_u16if_needed(uint16_t val)
{
#ifndef ANI_LITTLE_BYTE_ENDIAN
	return sir_swap_u16(val);
#else
	return val;
#endif
} /*** end sir_swap_u16if_needed() ***/

/**
 * sir_swap_u32()
 *
 * FUNCTION:
 * This function is called to swap four U8s of an uint32_t value
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 * None.
 *
 * NOTE:
 *
 * @param  val    uint32_t value to be uint8_t swapped
 * @return        Swapped uint32_t value
 */

static inline uint32_t sir_swap_u32(uint32_t val)
{
	return (val << 24) |
		(val >> 24) |
		((val & 0x0000FF00) << 8) | ((val & 0x00FF0000) >> 8);
} /*** end sir_swap_u32() ***/

/**
 * sir_swap_u32if_needed()
 *
 * FUNCTION:
 * This function is called to swap U8s of an uint32_t value depending
 * on endiannes of the target processor/compiler the software is
 * running on
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 * None.
 *
 * NOTE:
 *
 * @param  val    uint32_t value to be uint8_t swapped
 * @return        Swapped uint32_t value
 */

static inline uint32_t sir_swap_u32if_needed(uint32_t val)
{
#ifndef ANI_LITTLE_BYTE_ENDIAN
	return sir_swap_u32(val);
#else
	return val;
#endif
} /*** end sir_swap_u32if_needed() ***/

/**
 * sir_swap_u32_buf
 *
 * FUNCTION:
 * It swaps N dwords into the same buffer
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 * None.
 *
 * NOTE:
 *
 * @param  ptr address of uint32_t array
 * @return void
 *
 */

static inline void sir_swap_u32_buf(uint32_t *ptr, uint32_t nWords)
{
	uint32_t i;

	for (i = 0; i < nWords; i++)
		ptr[i] = sir_swap_u32(ptr[i]);
}

/**
 * sir_swap_u32_buf_if_needed()
 *
 * FUNCTION:
 * This function is called to swap U8s of U32s in the buffer depending
 * on endiannes of the target processor/compiler the software is
 * running on
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 * None.
 *
 * NOTE:
 *
 * @param  pBuf   Buffer that will get swapped
 * @param  nWords Number DWORDS will be swapped
 * @return        void
 */

static inline void sir_swap_u32_buf_if_needed(uint32_t *pBuf, uint32_t nWords)
{
#ifdef ANI_LITTLE_BYTE_ENDIAN
	sir_swap_u32_buf(pBuf, nWords);
#endif
} /*** end sir_swap_u32if_needed() ***/

/**
 * sir_swap_bd_if_needed
 *
 * FUNCTION:
 * Byte swap all the dwords in the BD, except the PHY/MAC headers
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 * None.
 *
 * NOTE:
 *
 * @param  pBd    BD that will get swapped
 * @return        void
 */

static inline void sir_swap_bd_if_needed(uint32_t *pBd)
{
	sir_swap_u32_buf_if_needed(pBd, 6);
	sir_swap_u32_buf_if_needed(pBd + 18, 14);
}

/**
 * sir_store_u16_n
 *
 * FUNCTION:
 * It stores a 16 bit number into the byte array in network byte order
 * i.e. the least significant byte first
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 * None.
 *
 * NOTE:
 *
 * @param  ptr address of destination byte array
 * @param  val value to store
 * @return None
 */

static inline void sir_store_u16_n(uint8_t *ptr, uint16_t val)
{
	*ptr++ = (val >> 8) & 0xff;
	*ptr = val & 0xff;
}

/**
 * sir_store_u32_n
 *
 * FUNCTION:
 * It stores a 32 bit number into the byte array in network byte order
 * i.e. the least significant byte first
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 * None.
 *
 * NOTE:
 *
 * @param  ptr address of destination byte array
 * @param  val value to store
 * @return None
 */

static inline void sir_store_u32_n(uint8_t *ptr, uint32_t val)
{
	*ptr++ = (uint8_t) (val >> 24) & 0xff;
	*ptr++ = (uint8_t) (val >> 16) & 0xff;
	*ptr++ = (uint8_t) (val >> 8) & 0xff;
	*ptr = (uint8_t) (val) & 0xff;
}

/**
 * sir_store_u16
 *
 * FUNCTION:
 * It stores a 16 bit number into the byte array in NON-network byte order
 * i.e. the least significant byte first
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 * None.
 *
 * NOTE:
 *
 * @param  ptr address of destination byte array
 * @param  val value to store
 * @return None
 */

static inline void sir_store_u16(uint8_t *ptr, uint16_t val)
{
	*ptr++ = val & 0xff;
	*ptr = (val >> 8) & 0xff;
}

/**
 * sir_store_u32
 *
 * FUNCTION:
 * It stores a 32 bit number into the byte array in NON-network byte order
 * i.e. the least significant byte first
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 * None.
 *
 * NOTE:
 *
 * @param  ptr address of destination byte array
 * @param  val value to store
 * @return None
 */

static inline void sir_store_u32(uint8_t *ptr, uint32_t val)
{
	*ptr++ = (uint8_t) val & 0xff;
	*ptr++ = (uint8_t) (val >> 8) & 0xff;
	*ptr++ = (uint8_t) (val >> 16) & 0xff;
	*ptr = (uint8_t) (val >> 24) & 0xff;
}

/**
 * sir_store_u32BufN
 *
 * FUNCTION:
 * It stores a 32 bit number into the byte array in network byte order
 * i.e. the least significant byte first. It performs the above operation
 * on entire buffer and writes to the dst buffer
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 * Assumes that the pSrc buffer is of all uint32_t data type fields.
 *
 * NOTE:
 * Must be used if all the fields in the buffer must be of uint32_t types.
 *
 * @param  pDst   address of destination byte array
 * @param  pSrc   address of the source DWORD array
 * @param  length number of DWORDs
 * @return None
 */

static inline void
sir_store_buf_n(uint8_t *pDst, uint32_t *pSrc, uint32_t length)
{
	while (length) {
		sir_store_u32_n(pDst, *pSrc);
		pDst += 4;
		pSrc++;
		length--;
	}
}

/**
 * sir_read_u16_n
 *
 * FUNCTION:
 * It reads a 16 bit number from the byte array in network byte order
 * i.e. the least significant byte first
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 * None.
 *
 * NOTE:
 *
 * @param  ptr address of  byte array
 * @return 16 bit value
 */

static inline uint16_t sir_read_u16_n(uint8_t *ptr)
{
	return ((*ptr) << 8) | (*(ptr + 1));
}

/**
 * sir_swap_u32_buf
 *
 * FUNCTION:
 * It swaps N dwords into the same buffer
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 * None.
 *
 * NOTE:
 *
 * @param  ptr address of uint32_t array
 * @return void
 *
 */

static inline void
sir_swap_n_store(uint32_t *src, uint32_t *dst, uint32_t nWords)
{
	uint32_t i;

	for (i = 0; i < nWords; i++)
		dst[i] = sir_swap_u32(src[i]);
}

/**
 * sir_read_u32_n
 *
 * FUNCTION:
 * It reads a 32 bit number from the byte array in network byte order
 * i.e. the least significant byte first
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 * None.
 *
 * NOTE:
 *
 * @param  ptr address of  byte array
 * @return 32 bit value
 */

static inline uint32_t sir_read_u32_n(uint8_t *ptr)
{
	return (*(ptr) << 24) |
		(*(ptr + 1) << 16) | (*(ptr + 2) << 8) | (*(ptr + 3));
}

/**
 * sir_read_u16
 *
 * FUNCTION:
 * It reads a 16 bit number from the byte array in NON-network byte order
 * i.e. the least significant byte first
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 * None.
 *
 * NOTE:
 *
 * @param  ptr address of  byte array
 * @return 16 bit value
 */

static inline uint16_t sir_read_u16(uint8_t *ptr)
{
	return (*ptr) | (*(ptr + 1) << 8);
}

/**
 * sir_read_u32
 *
 * FUNCTION:
 * It reads a 32 bit number from the byte array in NON-network byte order
 * i.e. the least significant byte first
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 * None.
 *
 * NOTE:
 *
 * @param  ptr address of  byte array
 * @return 32 bit value
 */

static inline uint32_t sir_read_u32(uint8_t *ptr)
{
	return (*(ptr)) |
		(*(ptr + 1) << 8) | (*(ptr + 2) << 16) | (*(ptr + 3) << 24);
}

/* / Copy a MAC address from 'from' to 'to' */
static inline void sir_copy_mac_addr(uint8_t to[], uint8_t from[])
{
#if defined(_X86_)
	uint32_t align = (0x3 & ((uint32_t) to | (uint32_t) from));
	if (align == 0) {
		*((uint16_t *) &(to[4])) = *((uint16_t *) &(from[4]));
		*((uint32_t *) to) = *((uint32_t *) from);
	} else if (align == 2) {
		*((uint16_t *) &to[4]) = *((uint16_t *) &from[4]);
		*((uint16_t *) &to[2]) = *((uint16_t *) &from[2]);
		*((uint16_t *) &to[0]) = *((uint16_t *) &from[0]);
	} else {
		to[5] = from[5];
		to[4] = from[4];
		to[3] = from[3];
		to[2] = from[2];
		to[1] = from[1];
		to[0] = from[0];
	}
#else
	to[0] = from[0];
	to[1] = from[1];
	to[2] = from[2];
	to[3] = from[3];
	to[4] = from[4];
	to[5] = from[5];
#endif
}

static inline uint8_t sir_compare_mac_addr(uint8_t addr1[], uint8_t addr2[])
{
#if defined(_X86_)
	uint32_t align = (0x3 & ((uint32_t) addr1 | (uint32_t) addr2));

	if (align == 0) {
		return (*((uint16_t *) &(addr1[4])) ==
			 *((uint16_t *) &(addr2[4])))
			&& (*((uint32_t *) addr1) == *((uint32_t *) addr2));
	} else if (align == 2) {
		return (*((uint16_t *) &addr1[4]) ==
			 *((uint16_t *) &addr2[4]))
			&& (*((uint16_t *) &addr1[2]) ==
			    *((uint16_t *) &addr2[2]))
			&& (*((uint16_t *) &addr1[0]) ==
			    *((uint16_t *) &addr2[0]));
	} else {
		return (addr1[5] == addr2[5]) &&
			(addr1[4] == addr2[4]) &&
			(addr1[3] == addr2[3]) &&
			(addr1[2] == addr2[2]) &&
			(addr1[1] == addr2[1]) && (addr1[0] == addr2[0]);
	}
#else
	return (addr1[0] == addr2[0]) &&
		(addr1[1] == addr2[1]) &&
		(addr1[2] == addr2[2]) &&
		(addr1[3] == addr2[3]) &&
		(addr1[4] == addr2[4]) && (addr1[5] == addr2[5]);
#endif
}

/*
 * converts uint16_t CW value to 4 bit value to be inserted in IE
 */
static inline uint8_t convert_cw(uint16_t cw)
{
	uint8_t val = 0;
	while (cw > 0) {
		val++;
		cw >>= 1;
	}
	if (val > 15)
		return 0xF;
	return val;
}

/* The user priority to AC mapping is such:
 *   UP(1, 2) ---> AC_BK(1)
 *   UP(0, 3) ---> AC_BE(0)
 *   UP(4, 5) ---> AC_VI(2)
 *   UP(6, 7) ---> AC_VO(3)
 */
#define WLAN_UP_TO_AC_MAP            0x33220110
#define upToAc(up)                ((WLAN_UP_TO_AC_MAP >> ((up) << 2)) & 0x03)

/* ------------------------------------------------------------------- */

/* / Parse the next IE in a message */
extern tSirRetStatus sirParseNextIE(tpAniSirGlobal, uint8_t *pPayload,
				    uint16_t payloadLength, int16_t lastType,
				    uint8_t *pType, uint8_t *pLength);

/* / Check if the given channel is 11b channel */
#define SIR_IS_CHANNEL_11B(chId)  (chId <= 14)

/**
 * hal_round_s32
 *
 * FUNCTION:
 * Performs integer rounding like returns 12346 for 123456 or -12346 for -123456
 * Note that a decimal place is lost.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 * None.
 *
 * NOTE:
 *
 * @param  int32_t input
 * @return rounded number
 */
static inline int32_t hal_round_s32(int32_t p)
{
	int32_t k, i, j;

	i = p / 10;
	j = p % 10;
	if (p > 0)
		k = i + (j > 4 ? 1 : 0);
	else if (p < 0)
		k = i + (j < -5 ? -1 : 0);
	else
		k = p;

	return k;
}

/* New functions for endianess conversion */
#ifdef ANI_LITTLE_BYTE_ENDIAN
#define ani_cpu_to_be16(x) sir_swap_u16((x))
#define ani_cpu_to_le16(x) (x)
#define ani_cpu_to_be32(x) sir_swap_u32((x))
#define ani_cpu_to_le32(x) (x)
#else                           /* ANI_LITTLE_BYTE_ENDIAN */
#define ani_cpu_to_be16(x) (x)
#define ani_cpu_to_le16(x) sir_swap_u16((x))
#define ani_cpu_to_be32(x) (x)
#define ani_cpu_to_le32(x) sir_swap_u32((x))
#endif /* ANI_LITTLE_BYTE_ENDIAN */

#define ani_le16_to_cpu(x)  ani_cpu_to_le16(x)
#define ani_le32_to_cpu(x)  ani_cpu_to_le32(x)
#define ani_be16_to_cpu(x)  ani_cpu_to_be16(x)
#define ani_be32_to_cpu(x)  ani_cpu_to_be32(x)

void convertto_big_endian(void *ptr, uint16_t size);
void create_scan_cts_frame(tpAniSirGlobal pMac, tSirMacMgmtHdr *macMgmtHdr,
			   tSirMacAddr selfMac);
void create_scan_data_null_frame(tpAniSirGlobal pMac,
		tSirMacMgmtHdr *macMgmtHdr, uint8_t pwrMgmt, tSirMacAddr bssid,
		tSirMacAddr selfMacAddr);
void create_init_scan_raw_frame(tpAniSirGlobal pMac, tSirMacMgmtHdr *macMgmtHdr,
				tBssSystemRole role);
void create_finish_scan_raw_frame(tpAniSirGlobal pMac,
		 tSirMacMgmtHdr *macMgmtHdr, tBssSystemRole role);

#endif /* __UTILSAPI_H */
