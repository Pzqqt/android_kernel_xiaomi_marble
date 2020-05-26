/* Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
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

#ifndef DATARMNET8ff259402c
#define DATARMNET8ff259402c
#include <linux/types.h>
#include <asm/byteorder.h>
#include "rmnet_descriptor.h"
#include "rmnet_map.h"
#define DATARMNET70889e5696		(0x11e1+4818-0x24ad)
#define DATARMNETb02d95e937		(0x398+2717-0xe24)
#define DATARMNET75cddb74df	(0x1d0+8549-0x2309)
struct DATARMNETf61a39bce9{
#if defined(__LITTLE_ENDIAN_BITFIELD)
u8 DATARMNET422c25939d:(0xc6f+4384-0x1d8b);u8 DATARMNET8f06b45a68:
(0xe4+813-0x40d);
#elif defined (__BIG_ENDIAN_BITFIELD)
u8 DATARMNET8f06b45a68:(0x297+3591-0x109a);u8 DATARMNET422c25939d:
(0x504+4747-0x178b);
#else
#error "<asm/byteorder.h> error"
#endif
u8 DATARMNET14d3de9c1a;__be16 DATARMNETefd12e7a2b;__be16 DATARMNET4de0560151;
__be16 DATARMNETa803e4f68b;u8 DATARMNET58886f3d9d;u8 DATARMNET065f40367c;__be16 
DATARMNET32d70026ed;__be32 DATARMNET7c6ed51964;__be32 DATARMNET799b2a47a5;};
struct DATARMNETfab66e680d{__be32 DATARMNETd750a8b9e5;__be16 DATARMNET5aeba29b95
;u8 DATARMNETa2be9ad0de;u8 DATARMNET63034bb4af;__be32 DATARMNET0f6f3a8868[
(0xf34+819-0x1263)];__be32 DATARMNET818f335f71[(0xda6+1408-0x1322)];};struct 
DATARMNET1bffb1c388{__be16 DATARMNET1addfaf963;__be16 DATARMNET88b102698d;__be32
 DATARMNET614032cc71;__be32 DATARMNET5e09091b6e;
#if defined(__LITTLE_ENDIAN_BITFIELD)
u8 DATARMNET0f1c1adba0:(0x975+7513-0x26ca);u8 DATARMNET686636af95:
(0x5d8+6880-0x20b4);
#elif defined (__BIG_ENDIAN_BITFIELD)
u8 DATARMNET686636af95:(0x882+5440-0x1dbe);u8 DATARMNET0f1c1adba0:
(0xbc6+6497-0x2523);
#else
#error "<asm/byteorder.h> error"
#endif
u8 DATARMNETf538480d20;__be16 DATARMNETf62e1ed00e;__be16 DATARMNETefa4b043cf;
__be16 DATARMNETd93b885d17;};struct DATARMNET2d87db499d{__be16 
DATARMNETaad9b0a864;__be16 DATARMNETd751c12d13;__be16 DATARMNET69d29eba58;__be16
 DATARMNET7efecd6ba5;};struct DATARMNETadd6ff90d0{struct rmnet_frag_descriptor*
DATARMNET798fafaf36;union{struct DATARMNETf61a39bce9*DATARMNET574a565f78;struct 
DATARMNETfab66e680d*DATARMNET1dab0af728;};union{struct DATARMNET1bffb1c388*
DATARMNET7ff34eacb7;struct DATARMNET2d87db499d*DATARMNETc51cdf696b;};u32 
DATARMNET68ad0738e2;u16 DATARMNET5671455f38;u16 DATARMNETa7eaea0d00;u16 
DATARMNETfeb7c81ac7;u8 DATARMNET9f5b2909c8;u8 DATARMNET072708db10;bool 
DATARMNET1e7926dc4c;bool DATARMNET2f3cb41448;bool DATARMNET6cc77c7b3f;};void 
DATARMNET8f8c3a4cc3(void);void DATARMNET4c69e80b6f(void);void 
DATARMNETd9ec5f4e65(void);void DATARMNET848914898b(void);bool 
DATARMNET4e490d9d37(void);void DATARMNET654b85a739(struct rmnet_frag_descriptor*
DATARMNETa41162aa9b,u32 DATARMNET0eb96739bf);void DATARMNETf9d626d72b(struct 
DATARMNETadd6ff90d0*DATARMNET809d788099);void DATARMNETfaa320a86a(void*
DATARMNETe8a5f322f0);void DATARMNET6b57fa3471(void*DATARMNETe8a5f322f0);void 
DATARMNET4aefca6d3c(struct rmnet_map_dl_ind_hdr*DATARMNETf3449c6d59,struct 
rmnet_map_control_command_header*DATARMNET23a4adcff8);void DATARMNET94edfae28d(
struct rmnet_map_dl_ind_trl*DATARMNET0303d63d24,struct 
rmnet_map_control_command_header*DATARMNET23a4adcff8);
#endif

