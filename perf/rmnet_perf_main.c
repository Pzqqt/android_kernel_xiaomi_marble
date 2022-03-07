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
MODULE_LICENSE("\x47\x50\x4c\x20\x76\x32");static char*verinfo[]={
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
(struct sk_buff*DATARMNET543491eb0f){u8*DATARMNET352416e828,DATARMNETd9bda61912;
struct udphdr*DATARMNETa1abb4897c;DATARMNETa1abb4897c=udp_hdr(
DATARMNET543491eb0f);if(!DATARMNETe1ef3edea2(DATARMNETa1abb4897c))return false;
if(be16_to_cpu(DATARMNETa1abb4897c->len)<sizeof(struct udphdr)+(0xd26+209-0xdf6)
)return false;DATARMNET352416e828=skb_header_pointer(DATARMNET543491eb0f,(int)((
unsigned char*)DATARMNETa1abb4897c-DATARMNET543491eb0f->data)+sizeof(struct 
udphdr),(0xd26+209-0xdf6),&DATARMNETd9bda61912);if(!DATARMNET352416e828)return 
false;return((*DATARMNET352416e828)&(0xbf7+4869-0x1e3c))==(0xbf7+4869-0x1e3c);}
int DATARMNETee9214ce9b(struct sk_buff*DATARMNET543491eb0f){if(
DATARMNET543491eb0f->protocol==htons(ETH_P_IP)){if(ip_hdr(DATARMNET543491eb0f)->
protocol!=IPPROTO_UDP)return-EINVAL;if(DATARMNET77ab42949f(DATARMNET543491eb0f))
goto DATARMNET606558916b;return-EINVAL;}if(DATARMNET543491eb0f->protocol==htons(
ETH_P_IPV6)){if(ipv6_hdr(DATARMNET543491eb0f)->nexthdr!=IPPROTO_UDP)return-
EINVAL;if(DATARMNET77ab42949f(DATARMNET543491eb0f))goto DATARMNET606558916b;}
return-EINVAL;DATARMNET606558916b:DATARMNET543491eb0f->hash=(0xd2d+202-0xdf7);
DATARMNET543491eb0f->sw_hash=(0xd26+209-0xdf6);return(0xd2d+202-0xdf7);}void 
DATARMNET05c4bf8af9(struct sk_buff*DATARMNET543491eb0f){if(DATARMNET543491eb0f->
protocol==htons(ETH_P_IP)){if(ip_hdr(DATARMNET543491eb0f)->protocol!=IPPROTO_UDP
)return;if(DATARMNET77ab42949f(DATARMNET543491eb0f))goto DATARMNETb261f33b7b;}if
(DATARMNET543491eb0f->protocol==htons(ETH_P_IPV6)){if(ipv6_hdr(
DATARMNET543491eb0f)->nexthdr!=IPPROTO_UDP)return;if(DATARMNET77ab42949f(
DATARMNET543491eb0f))goto DATARMNETb261f33b7b;}return;DATARMNETb261f33b7b:
DATARMNET543491eb0f->priority=14286874;}void DATARMNET49c17a32bc(void){
rcu_assign_pointer(rmnet_perf_ingress_hook1,DATARMNETee9214ce9b);
rcu_assign_pointer(rmnet_perf_egress_hook1,DATARMNET05c4bf8af9);}void 
DATARMNET41e8cc085c(void){rcu_assign_pointer(rmnet_perf_ingress_hook1,NULL);
rcu_assign_pointer(rmnet_perf_egress_hook1,NULL);}static int __init 
DATARMNET63abbdc3d3(void){pr_info("%s(): Loading\n",__func__);
DATARMNET49c17a32bc();return(0xd2d+202-0xdf7);}static void __exit 
DATARMNETa343229e33(void){DATARMNET41e8cc085c();pr_info(
"\x25\x73\x28\x29\x3a\x20\x65\x78\x69\x74\x69\x6e\x67" "\n",__func__);}
module_init(DATARMNET63abbdc3d3);module_exit(DATARMNETa343229e33);
