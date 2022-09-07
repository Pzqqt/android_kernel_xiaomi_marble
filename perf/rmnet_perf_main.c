// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * RMNET PERF framework
 *
 */
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/in.h>
#include <linux/udp.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <net/ipv6.h>
#include <net/ip.h>
MODULE_LICENSE("\x47\x50\x4c\x20\x76\x32");static char*verinfo[]={
"\x65\x35\x36\x63\x62\x35\x35\x64","\x33\x39\x63\x62\x64\x37\x64\x35",
"\x37\x34\x31\x35\x39\x32\x31\x63","\x34\x64\x65\x34\x39\x64\x62\x35",
"\x62\x31\x63\x34\x34\x62\x34\x63"};module_param_array(verinfo,charp,NULL,
(0xcb7+5769-0x221c));MODULE_PARM_DESC(verinfo,
"\x56\x65\x72\x73\x69\x6f\x6e\x20\x6f\x66\x20\x74\x68\x65\x20\x64\x72\x69\x76\x65\x72"
);extern int(*rmnet_perf_ingress_hook1)(struct sk_buff*DATARMNET543491eb0f);
extern void(*rmnet_perf_egress_hook1)(struct sk_buff*DATARMNET543491eb0f);
#define DATARMNETf241a4b20f (0xe07+6616-0x2624)
static inline bool DATARMNETe1ef3edea2(struct udphdr*DATARMNETa1abb4897c){return
 be16_to_cpu(DATARMNETa1abb4897c->source)==DATARMNETf241a4b20f||be16_to_cpu(
DATARMNETa1abb4897c->dest)==DATARMNETf241a4b20f;}static bool DATARMNET77ab42949f
(struct sk_buff*DATARMNET543491eb0f,int DATARMNET611d08d671){u8*
DATARMNET352416e828,DATARMNETd9bda61912;struct udphdr*DATARMNETa1abb4897c,
DATARMNET7b1af10647;DATARMNETa1abb4897c=skb_header_pointer(DATARMNET543491eb0f,
DATARMNET611d08d671,sizeof(*DATARMNETa1abb4897c),&DATARMNET7b1af10647);if(!
DATARMNETa1abb4897c||!DATARMNETe1ef3edea2(DATARMNETa1abb4897c))return false;if(
be16_to_cpu(DATARMNETa1abb4897c->len)<sizeof(struct udphdr)+(0xd26+209-0xdf6))
return false;DATARMNET352416e828=skb_header_pointer(DATARMNET543491eb0f,
DATARMNET611d08d671+sizeof(struct udphdr),(0xd26+209-0xdf6),&DATARMNETd9bda61912
);if(!DATARMNET352416e828)return false;return((*DATARMNET352416e828)&
(0xbf7+4869-0x1e3c))==(0xbf7+4869-0x1e3c);}int DATARMNETee9214ce9b(struct 
sk_buff*DATARMNET543491eb0f){if(DATARMNET543491eb0f->protocol==htons(ETH_P_IP)){
struct iphdr*DATARMNET86f1f2cdc9,DATARMNETbf6548198e;DATARMNET86f1f2cdc9=
skb_header_pointer(DATARMNET543491eb0f,(0xd2d+202-0xdf7),sizeof(*
DATARMNET86f1f2cdc9),&DATARMNETbf6548198e);if(!DATARMNET86f1f2cdc9||
ip_is_fragment(DATARMNET86f1f2cdc9)||DATARMNET86f1f2cdc9->protocol!=IPPROTO_UDP)
return-EINVAL;if(DATARMNET77ab42949f(DATARMNET543491eb0f,(DATARMNET86f1f2cdc9->
ihl)*(0xd11+230-0xdf3)))goto DATARMNET606558916b;return-EINVAL;}if(
DATARMNET543491eb0f->protocol==htons(ETH_P_IPV6)){struct ipv6hdr*
DATARMNETbf55123e5b,DATARMNETcf1d9e2c1e;u8 protocol;int DATARMNET611d08d671=
(0xd2d+202-0xdf7);__be16 DATARMNET98fed39916=(0xd2d+202-0xdf7);
DATARMNETbf55123e5b=skb_header_pointer(DATARMNET543491eb0f,(0xd2d+202-0xdf7),
sizeof(*DATARMNETbf55123e5b),&DATARMNETcf1d9e2c1e);if(!DATARMNETbf55123e5b)
return-EINVAL;protocol=DATARMNETbf55123e5b->nexthdr;DATARMNET611d08d671=
ipv6_skip_exthdr(DATARMNET543491eb0f,sizeof(struct ipv6hdr),&protocol,&
DATARMNET98fed39916);if(DATARMNET98fed39916||protocol!=IPPROTO_UDP||
DATARMNET611d08d671==-(0xd26+209-0xdf6))return-EINVAL;if(DATARMNET77ab42949f(
DATARMNET543491eb0f,DATARMNET611d08d671))goto DATARMNET606558916b;}return-EINVAL
;DATARMNET606558916b:DATARMNET543491eb0f->hash=(0xd2d+202-0xdf7);
DATARMNET543491eb0f->sw_hash=(0xd26+209-0xdf6);return(0xd2d+202-0xdf7);}void 
DATARMNET05c4bf8af9(struct sk_buff*DATARMNET543491eb0f){if(DATARMNET543491eb0f->
protocol==htons(ETH_P_IP)){struct iphdr*DATARMNET86f1f2cdc9,DATARMNETbf6548198e;
DATARMNET86f1f2cdc9=skb_header_pointer(DATARMNET543491eb0f,(0xd2d+202-0xdf7),
sizeof(*DATARMNET86f1f2cdc9),&DATARMNETbf6548198e);if(!DATARMNET86f1f2cdc9||
ip_is_fragment(DATARMNET86f1f2cdc9)||DATARMNET86f1f2cdc9->protocol!=IPPROTO_UDP)
return;if(DATARMNET77ab42949f(DATARMNET543491eb0f,(DATARMNET86f1f2cdc9->ihl)*
(0xd11+230-0xdf3)))goto DATARMNETb261f33b7b;return;}if(DATARMNET543491eb0f->
protocol==htons(ETH_P_IPV6)){struct ipv6hdr*DATARMNETbf55123e5b,
DATARMNETcf1d9e2c1e;u8 protocol;int DATARMNET611d08d671=(0xd2d+202-0xdf7);__be16
 DATARMNET98fed39916=(0xd2d+202-0xdf7);DATARMNETbf55123e5b=skb_header_pointer(
DATARMNET543491eb0f,(0xd2d+202-0xdf7),sizeof(*DATARMNETbf55123e5b),&
DATARMNETcf1d9e2c1e);if(!DATARMNETbf55123e5b)return;protocol=DATARMNETbf55123e5b
->nexthdr;DATARMNET611d08d671=ipv6_skip_exthdr(DATARMNET543491eb0f,sizeof(struct
 ipv6hdr),&protocol,&DATARMNET98fed39916);if(DATARMNET98fed39916||protocol!=
IPPROTO_UDP||DATARMNET611d08d671==-(0xd26+209-0xdf6))return;if(
DATARMNET77ab42949f(DATARMNET543491eb0f,DATARMNET611d08d671))goto 
DATARMNETb261f33b7b;}return;DATARMNETb261f33b7b:DATARMNET543491eb0f->priority=
14286874;}void DATARMNET49c17a32bc(void){rcu_assign_pointer(
rmnet_perf_ingress_hook1,DATARMNETee9214ce9b);rcu_assign_pointer(
rmnet_perf_egress_hook1,DATARMNET05c4bf8af9);}void DATARMNET41e8cc085c(void){
rcu_assign_pointer(rmnet_perf_ingress_hook1,NULL);rcu_assign_pointer(
rmnet_perf_egress_hook1,NULL);synchronize_rcu();}static int __init 
DATARMNET63abbdc3d3(void){pr_info("%s(): Loading\n",__func__);
DATARMNET49c17a32bc();return(0xd2d+202-0xdf7);}static void __exit 
DATARMNETa343229e33(void){DATARMNET41e8cc085c();pr_info(
"\x25\x73\x28\x29\x3a\x20\x65\x78\x69\x74\x69\x6e\x67" "\n",__func__);}
module_init(DATARMNET63abbdc3d3);module_exit(DATARMNETa343229e33);
