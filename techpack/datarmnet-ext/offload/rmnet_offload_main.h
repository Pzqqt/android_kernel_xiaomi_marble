/* Copyright (c) 2018-2021, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef DATARMNET3f65517842
#define DATARMNET3f65517842
#include <linux/types.h>
#include <asm/byteorder.h>
#include "rmnet_descriptor.h"
#include "rmnet_map.h"
#define DATARMNETfd5c3d30e5		(0xd03+244-0xdf1)
#define DATARMNETa656f324b2		(0xeb7+1151-0x1325)
#define DATARMNET5b166167a8	(0xec7+1138-0x130d)
struct DATARMNET308c06fca6{
#if defined(__LITTLE_ENDIAN_BITFIELD)
u8 DATARMNET610d06a887:(0xd11+230-0xdf3);u8 DATARMNET7af6b06548:
(0xd11+230-0xdf3);
#elif defined (__BIG_ENDIAN_BITFIELD)
u8 DATARMNET7af6b06548:(0xd11+230-0xdf3);u8 DATARMNET610d06a887:
(0xd11+230-0xdf3);
#else
#error "<asm/byteorder.h> error"
#endif
u8 DATARMNET03e081f69f;__be16 DATARMNET8e175ac02d;__be16 DATARMNET4acf94d119;
__be16 DATARMNET2c091223da;u8 DATARMNET1eb6c27583;u8 DATARMNET953d32410f;__be16 
DATARMNET9e4950461a;__be32 DATARMNET9bbd2ca483;__be32 DATARMNET11ef55b2bb;};
struct DATARMNET64bb8f616e{__be32 DATARMNET0687f7ecdc;__be16 DATARMNETaa340485c5
;u8 DATARMNET7244bfdc4f;u8 DATARMNET00f98dd445;__be32 DATARMNET31202449fc[
(0xd11+230-0xdf3)];__be32 DATARMNETc657c0a139[(0xd11+230-0xdf3)];};struct 
DATARMNETd2991e8952{__be16 DATARMNET6acbeeebb1;__be16 DATARMNET29b8c05d14;__be32
 DATARMNETb3e70104e0;__be32 DATARMNET9a9c9d060a;
#if defined(__LITTLE_ENDIAN_BITFIELD)
u8 DATARMNET655b34c160:(0xd11+230-0xdf3);u8 DATARMNETcf2e43ad69:
(0xd11+230-0xdf3);
#elif defined (__BIG_ENDIAN_BITFIELD)
u8 DATARMNETcf2e43ad69:(0xd11+230-0xdf3);u8 DATARMNET655b34c160:
(0xd11+230-0xdf3);
#else
#error "<asm/byteorder.h> error"
#endif
u8 DATARMNETd570802f3d;__be16 DATARMNET807cd6af6c;__be16 DATARMNET8891a7f7ce;
__be16 DATARMNET9921296467;};struct DATARMNET0b82084e89{__be16 
DATARMNET1b1c406ae9;__be16 DATARMNETdf0e8545ee;__be16 DATARMNET39a05c5524;__be16
 DATARMNET45a3ebe8e3;};struct DATARMNET4287f07234{__be16 DATARMNETa60d2ae3f6;
__be16 DATARMNET5e7452ec23;union{__be32 DATARMNET1a2be7b6c2;__be32 
DATARMNET949c2754de[(0xd11+230-0xdf3)];};union{__be32 DATARMNET0a28fa80f4;__be32
 DATARMNET45ec858bc5[(0xd11+230-0xdf3)];};union{struct{u8 DATARMNET19b03754c4;u8
 DATARMNET57bf608183;__be16 DATARMNETcf915688f5;};__be32 DATARMNETabd58f7f89;};
u32 DATARMNETbc28a5970f;u16 DATARMNET4ca5ac9de1;u16 DATARMNET0aeee57ceb;u8 
DATARMNET388842c721;u8 DATARMNET7fa8b2acbf;};struct DATARMNETd812bcdbb5{struct 
rmnet_frag_descriptor*DATARMNET719f68fb88;struct DATARMNET4287f07234 
DATARMNET144d119066;u32 DATARMNET645e8912b8;u16 DATARMNET1ef22e4c76;bool 
DATARMNETf1b6b0a6cc;bool DATARMNET3eb91ee54d;bool DATARMNET8f59f5a745;};void 
DATARMNET664568fcd0(void);void DATARMNET6a76048590(void);void 
DATARMNET818b960147(void);void DATARMNETba38823a2f(void);bool 
DATARMNET19eb0de3b2(void);void DATARMNETc70e73c8d4(struct list_head*
DATARMNETb5730f5283);void DATARMNET19d190f2bd(struct DATARMNETd812bcdbb5*
DATARMNET458b70e7e5,struct list_head*DATARMNET6f9bfa17e6);void 
DATARMNET9292bebdd3(void*DATARMNETf0d9de7e2f);void DATARMNETb7e47d7254(void*
DATARMNETf0d9de7e2f);void DATARMNET95e1703026(struct rmnet_map_dl_ind_hdr*
DATARMNET7c7748ef7a,struct rmnet_map_control_command_header*DATARMNET8b07ee3e82)
;void DATARMNETc9dd320f49(struct rmnet_map_dl_ind_trl*DATARMNET2541770fea,struct
 rmnet_map_control_command_header*DATARMNET8b07ee3e82);
#endif

