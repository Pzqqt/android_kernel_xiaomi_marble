/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * RMNET WLAN handler framework
 *
 */

#ifndef DATARMNETc7bc459123
#define DATARMNETc7bc459123
#include <linux/types.h>
#include <net/genetlink.h>
#include <linux/netdevice.h>
#include <linux/in6.h>
struct DATARMNETb89ecedefc{union{__be16 DATARMNETf0d9de7e2f;__be32 
DATARMNET906b2ee561;};u8 DATARMNET0d956cc77a;u8 DATARMNET4924e79411;};struct 
DATARMNET8d3c2559ca{struct net_device*DATARMNET57656f6f2f;union{__be32 
DATARMNET0dc14167a1;struct in6_addr DATARMNET5700daac01;};u8 DATARMNET0d956cc77a
;u8 DATARMNET9954a624ac;};struct DATARMNET41b426061d{struct hlist_node 
DATARMNETe8608dd267;struct rcu_head DATARMNET28bfe9e6ad;struct notifier_block 
DATARMNET272c159b3c;char DATARMNET852d4d00e2[IFNAMSIZ];struct 
DATARMNET8d3c2559ca DATARMNET7ed5754a5c;};enum{DATARMNET356f2a237e,
DATARMNET45ee632553,DATARMNET9bfbc31cd6,DATARMNETdcafdec32a,};void 
DATARMNET7ca470d54b(struct sk_buff*DATARMNET543491eb0f,u32 DATARMNET1464100e7a);
int DATARMNET4899053671(struct sk_buff*DATARMNET543491eb0f,struct 
DATARMNET8d3c2559ca*DATARMNET2d4b4cfc9e);bool DATARMNET4eafcdee07(struct 
DATARMNETb89ecedefc*DATARMNET3396919a68);int DATARMNET53f12a0f7d(struct 
DATARMNETb89ecedefc*DATARMNETcadc2ef9aa,u32 DATARMNETef77661260,struct genl_info
*DATARMNET54338da2ff);int DATARMNET07f6485c9b(struct DATARMNETb89ecedefc*
DATARMNETcadc2ef9aa,u32 DATARMNETef77661260,struct genl_info*DATARMNET54338da2ff
);int DATARMNETe14c875532(struct sk_buff**DATARMNET89946cec52,struct genl_family
*DATARMNET4a4aca7220,struct genl_info*DATARMNET54338da2ff);int 
DATARMNET97b2388e63(char*DATARMNET852d4d00e2,struct genl_info*
DATARMNET54338da2ff);int DATARMNETa903cd5994(char*DATARMNET852d4d00e2,struct 
genl_info*DATARMNET54338da2ff);int DATARMNET92e5468fc5(char*DATARMNET852d4d00e2,
struct genl_info*DATARMNET54338da2ff);int DATARMNET9d7bb9a63d(char*
DATARMNET852d4d00e2,struct genl_info*DATARMNET54338da2ff);int 
DATARMNET947eb110d2(struct DATARMNET8d3c2559ca*DATARMNET2d4b4cfc9e,struct 
genl_info*DATARMNET54338da2ff);int DATARMNETb8b35fdc18(struct 
DATARMNET8d3c2559ca*DATARMNET2d4b4cfc9e,struct genl_info*DATARMNET54338da2ff);
int DATARMNET8d5a5a7047(__be16 DATARMNETf0d9de7e2f,struct genl_info*
DATARMNET54338da2ff);int DATARMNETc97c6a4265(__be16 DATARMNETf0d9de7e2f,struct 
genl_info*DATARMNET54338da2ff);bool DATARMNETa8b2566e6a(struct sk_buff*
DATARMNET543491eb0f,struct DATARMNETb89ecedefc*DATARMNET3396919a68,int 
DATARMNET611d08d671);int DATARMNET59b8376224(__be16 DATARMNETf0d9de7e2f,struct 
genl_info*DATARMNET54338da2ff);int DATARMNET0b12e969c5(__be16 
DATARMNETf0d9de7e2f,struct genl_info*DATARMNET54338da2ff);bool 
DATARMNET0a4704e5e0(struct DATARMNETb89ecedefc*DATARMNET3396919a68);int 
DATARMNET078f6bd384(void);void DATARMNETfae36afa03(void);char*
DATARMNET934406764d(void);char*DATARMNETe447822105(void);struct 
DATARMNET41b426061d*DATARMNETcc0a01df2a(struct DATARMNET8d3c2559ca*
DATARMNET54338da2ff);
#endif

