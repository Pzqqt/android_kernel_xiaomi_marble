/* Copyright (c) 2021. Qualcomm Innovation Center, Inc. All rights reserved.
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

#include "rmnet_aps_genl.h"
#include "rmnet_aps.h"
static struct nla_policy DATARMNET157d4c7a8a[DATARMNET3b044438b9+
(0xd26+209-0xdf6)]={[DATARMNET98b93f2e86]=NLA_POLICY_EXACT_LEN(sizeof(struct 
DATARMNET5d4139d9d7)),[DATARMNET818a0b6438]=NLA_POLICY_EXACT_LEN(sizeof(struct 
DATARMNETca79857d4a)),};
#define DATARMNET7dc6237681(DATARMNET5aeb0ef9bc, DATARMNETbd9859b58e)\
                                         \
	{                                                                      \
		.cmd = DATARMNET5aeb0ef9bc, .doit = DATARMNETbd9859b58e, .dumpit = NULL, .\
flags = (0xd2d+202-0xdf7),        \
	}
static const struct genl_ops DATARMNET27a52625e4[]={DATARMNET7dc6237681(
DATARMNET15437be67e,DATARMNET37a9efbbcb),};struct genl_family 
DATARMNETcdba30eca9={.hdrsize=(0xd2d+202-0xdf7),.name=DATARMNETa4a6e34e45,.
version=DATARMNET33d1a3e23a,.maxattr=DATARMNET3b044438b9,.policy=
DATARMNET157d4c7a8a,.ops=DATARMNET27a52625e4,.n_ops=ARRAY_SIZE(
DATARMNET27a52625e4),};int DATARMNETcfa8d492f8(void){return genl_register_family
(&DATARMNETcdba30eca9);}void DATARMNETca7606cfd1(void){genl_unregister_family(&
DATARMNETcdba30eca9);}
