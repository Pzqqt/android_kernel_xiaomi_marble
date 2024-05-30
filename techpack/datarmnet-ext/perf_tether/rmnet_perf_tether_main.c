// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * RMNET PERF TETHER framework
 *
 */

#include <linux/module.h>
#include <net/tcp.h>
#include "rmnet_descriptor.h"
#include "rmnet_map.h"
MODULE_LICENSE("\x47\x50\x4c\x20\x76\x32");static char*verinfo[]={
"\x37\x34\x31\x35\x39\x32\x31\x63","\x34\x39\x61\x66\x39\x62\x64\x34"};
module_param_array(verinfo,charp,NULL,(0xcb7+5769-0x221c));MODULE_PARM_DESC(
verinfo,
"\x56\x65\x72\x73\x69\x6f\x6e\x20\x6f\x66\x20\x74\x68\x65\x20\x64\x72\x69\x76\x65\x72"
);extern void(*rmnet_perf_tether_ingress_hook)(struct tcphdr*DATARMNET2e7cc88971
,struct rmnet_frag_descriptor*DATARMNET9d1b321642,struct iphdr*
DATARMNET86f1f2cdc9);extern void(*rmnet_perf_tether_egress_hook)(struct sk_buff*
DATARMNET543491eb0f);extern void(*rmnet_perf_tether_cmd_hook)(u8 
DATARMNET07b1a05079,u64 DATARMNETcc6099cb14);struct DATARMNETf9eee4068d{u8 
DATARMNETd0ef85194f;};static struct DATARMNETf9eee4068d*DATARMNET8a1e9920bf;
unsigned int configure_knob1 __read_mostly=(0xd2d+202-0xdf7);module_param(
configure_knob1,uint,(0xdb7+6665-0x261c));unsigned int knob1 __read_mostly=
(0xd2d+202-0xdf7);module_param(knob1,uint,(0xdb7+6665-0x261c));unsigned int 
configure_knob2 __read_mostly=(0xd2d+202-0xdf7);module_param(configure_knob2,
uint,(0xdb7+6665-0x261c));unsigned int knob2 __read_mostly=(0xd2d+202-0xdf7);
module_param(knob2,uint,(0xdb7+6665-0x261c));static DEFINE_SPINLOCK(
DATARMNETcf56327171);
#define DATARMNETd063f2edce ((0xef7+1112-0x131d))
#define DATARMNET4c845df513 \
	(const_ilog2(DATARMNETd063f2edce))
static DEFINE_HASHTABLE(DATARMNET031f18e039,DATARMNET4c845df513);struct 
DATARMNETb477d446e7{struct list_head list;struct hlist_node hlist;u32 hash;
__be32 DATARMNETdfd47b00ff[(0xd20+231-0xdfc)];u32 DATARMNET4b119c4ff3;};struct 
list_head DATARMNETc588a190d4=LIST_HEAD_INIT(DATARMNETc588a190d4);
#define DATARMNETc99cb98651 ((0xd26+209-0xdf6))
#define DATARMNETfc6d4bac8a ((0xd19+238-0xdfb))
#define DATARMNETc152fdcc34 ((0xec7+1129-0x1315))
#define DATARMNET7bd625a8c0 ((0xd26+209-0xdf6))
struct DATARMNETb0eda9d28e{u8 DATARMNETba8c503d04;u8 DATARMNET4f46aa107b;u8 
DATARMNETc59f891d69;u8 DATARMNETecca9147fd;};static u32 DATARMNET233bccd449(
struct sk_buff*DATARMNET543491eb0f,int*DATARMNET76bafbc6ce,int 
DATARMNET32b08042b9,int DATARMNET3a925295d9){__be32 DATARMNETdfd47b00ff[
(0xd20+231-0xdfc)];u32 DATARMNET5ebf661879;if(DATARMNET543491eb0f->protocol==
htons(ETH_P_IP)){if(!tcp_hdr(DATARMNET543491eb0f)->syn)goto DATARMNET1d7e728ddf;
if(DATARMNET32b08042b9){if(!tcp_hdr(DATARMNET543491eb0f)->ack)goto 
DATARMNET1d7e728ddf;}else{if(tcp_hdr(DATARMNET543491eb0f)->ack)goto 
DATARMNET1d7e728ddf;}DATARMNETdfd47b00ff[(0xd2d+202-0xdf7)]=DATARMNET3a925295d9?
ip_hdr(DATARMNET543491eb0f)->daddr:ip_hdr(DATARMNET543491eb0f)->saddr;
DATARMNETdfd47b00ff[(0xd26+209-0xdf6)]=DATARMNET3a925295d9?ip_hdr(
DATARMNET543491eb0f)->saddr:ip_hdr(DATARMNET543491eb0f)->daddr;
DATARMNETdfd47b00ff[(0xd1f+216-0xdf5)]=ip_hdr(DATARMNET543491eb0f)->protocol;
DATARMNETdfd47b00ff[(0xd18+223-0xdf4)]=DATARMNET3a925295d9?tcp_hdr(
DATARMNET543491eb0f)->dest:tcp_hdr(DATARMNET543491eb0f)->source;
DATARMNETdfd47b00ff[(0xd11+230-0xdf3)]=DATARMNET3a925295d9?tcp_hdr(
DATARMNET543491eb0f)->source:tcp_hdr(DATARMNET543491eb0f)->dest;
DATARMNET5ebf661879=(0xd0a+237-0xdf2);}else if(DATARMNET543491eb0f->protocol==
htons(ETH_P_IPV6)){if(!tcp_hdr(DATARMNET543491eb0f)->syn)goto 
DATARMNET1d7e728ddf;if(DATARMNET32b08042b9){if(!tcp_hdr(DATARMNET543491eb0f)->
ack)goto DATARMNET1d7e728ddf;}else{if(tcp_hdr(DATARMNET543491eb0f)->ack)goto 
DATARMNET1d7e728ddf;}memcpy(&DATARMNETdfd47b00ff[(0xd2d+202-0xdf7)],
DATARMNET3a925295d9?ipv6_hdr(DATARMNET543491eb0f)->daddr.s6_addr:ipv6_hdr(
DATARMNET543491eb0f)->saddr.s6_addr,sizeof(struct in6_addr));memcpy(&
DATARMNETdfd47b00ff[(0xd0a+237-0xdf2)],DATARMNET3a925295d9?ipv6_hdr(
DATARMNET543491eb0f)->saddr.s6_addr:ipv6_hdr(DATARMNET543491eb0f)->daddr.s6_addr
,sizeof(struct in6_addr));DATARMNETdfd47b00ff[(0xd35+210-0xdff)]=ipv6_hdr(
DATARMNET543491eb0f)->nexthdr;DATARMNETdfd47b00ff[9]=tcp_hdr(DATARMNET543491eb0f
)->dest;DATARMNETdfd47b00ff[(0xd27+224-0xdfd)]=tcp_hdr(DATARMNET543491eb0f)->
source;DATARMNET5ebf661879=(0xd20+231-0xdfc);}else{goto DATARMNET1d7e728ddf;}*
DATARMNET76bafbc6ce=(0xd26+209-0xdf6);return jhash2(DATARMNETdfd47b00ff,
DATARMNET5ebf661879,(0xd2d+202-0xdf7));DATARMNET1d7e728ddf:*DATARMNET76bafbc6ce=
(0xd2d+202-0xdf7);return(0xd2d+202-0xdf7);}static void DATARMNET9d42c8a184(
struct tcphdr*DATARMNET2e7cc88971){if(DATARMNET2e7cc88971->syn&&
DATARMNET2e7cc88971->ack){if(configure_knob1){if(knob1>65535)knob1=65535;
DATARMNET2e7cc88971->window=cpu_to_be16(knob1);}if(configure_knob2){unsigned 
char*DATARMNET7b34b7b5be;u32 DATARMNET539d2ad3f2;if(knob2>TCP_MAX_WSCALE)knob2=
TCP_MAX_WSCALE;DATARMNET539d2ad3f2=DATARMNET2e7cc88971->doff*(0xd11+230-0xdf3)-
sizeof(struct tcphdr);DATARMNET7b34b7b5be=(unsigned char*)(DATARMNET2e7cc88971+
(0xd26+209-0xdf6));while(DATARMNET539d2ad3f2>(0xd2d+202-0xdf7)){int 
DATARMNET969cfb9094=*DATARMNET7b34b7b5be++;int DATARMNET68a21637e9;switch(
DATARMNET969cfb9094){case TCPOPT_EOL:return;case TCPOPT_NOP:DATARMNET539d2ad3f2
--;continue;default:if(DATARMNET539d2ad3f2<(0xd1f+216-0xdf5))return;
DATARMNET68a21637e9=*DATARMNET7b34b7b5be++;if(DATARMNET68a21637e9<
(0xd1f+216-0xdf5))return;if(DATARMNET68a21637e9>DATARMNET539d2ad3f2)return;if(
DATARMNET969cfb9094==TCPOPT_WINDOW)*DATARMNET7b34b7b5be=knob2;
DATARMNET7b34b7b5be+=DATARMNET68a21637e9-(0xd1f+216-0xdf5);DATARMNET539d2ad3f2-=
DATARMNET68a21637e9;}}}}}static int DATARMNET94d0b7e790(struct 
DATARMNETb477d446e7*DATARMNET63b1a086d5,struct sk_buff*DATARMNET543491eb0f){if(
DATARMNET543491eb0f->protocol==htons(ETH_P_IP)){if((DATARMNET63b1a086d5->
DATARMNETdfd47b00ff[(0xd2d+202-0xdf7)]==ip_hdr(DATARMNET543491eb0f)->saddr)&&(
DATARMNET63b1a086d5->DATARMNETdfd47b00ff[(0xd26+209-0xdf6)]==ip_hdr(
DATARMNET543491eb0f)->daddr)&&(DATARMNET63b1a086d5->DATARMNETdfd47b00ff[
(0xd1f+216-0xdf5)]==ip_hdr(DATARMNET543491eb0f)->protocol)&&(DATARMNET63b1a086d5
->DATARMNETdfd47b00ff[(0xd18+223-0xdf4)]==tcp_hdr(DATARMNET543491eb0f)->source)
&&(DATARMNET63b1a086d5->DATARMNETdfd47b00ff[(0xd11+230-0xdf3)]==tcp_hdr(
DATARMNET543491eb0f)->dest)&&(DATARMNET63b1a086d5->DATARMNET4b119c4ff3==
(0xd0a+237-0xdf2)))return(0xd2d+202-0xdf7);}else if(DATARMNET543491eb0f->
protocol==htons(ETH_P_IPV6)){if((!memcmp(&DATARMNET63b1a086d5->
DATARMNETdfd47b00ff[(0xd2d+202-0xdf7)],ipv6_hdr(DATARMNET543491eb0f)->saddr.
s6_addr,sizeof(struct in6_addr)))&&(!memcmp(&DATARMNET63b1a086d5->
DATARMNETdfd47b00ff[(0xd0a+237-0xdf2)],ipv6_hdr(DATARMNET543491eb0f)->daddr.
s6_addr,sizeof(struct in6_addr)))&&(DATARMNET63b1a086d5->DATARMNETdfd47b00ff[
(0xd35+210-0xdff)]==ipv6_hdr(DATARMNET543491eb0f)->nexthdr)&&(
DATARMNET63b1a086d5->DATARMNETdfd47b00ff[9]==tcp_hdr(DATARMNET543491eb0f)->
source)&&(DATARMNET63b1a086d5->DATARMNETdfd47b00ff[(0xd27+224-0xdfd)]==tcp_hdr(
DATARMNET543491eb0f)->dest)&&(DATARMNET63b1a086d5->DATARMNET4b119c4ff3==
(0xd20+231-0xdfc)))return(0xd2d+202-0xdf7);}return(0xd26+209-0xdf6);}void 
DATARMNETe70d5ec61a(struct tcphdr*DATARMNET2e7cc88971,struct sk_buff*
DATARMNET543491eb0f){int DATARMNET76bafbc6ce=(0xd2d+202-0xdf7);u32 hash;unsigned
 long DATARMNETfb0677cc3c;struct DATARMNETb477d446e7*DATARMNET63b1a086d5,*
DATARMNET0386f6f82a=NULL;if(!configure_knob1&&!configure_knob2)return;hash=
DATARMNET233bccd449(DATARMNET543491eb0f,&DATARMNET76bafbc6ce,(0xd26+209-0xdf6),
(0xd2d+202-0xdf7));if(!DATARMNET76bafbc6ce)return;spin_lock_irqsave(&
DATARMNETcf56327171,DATARMNETfb0677cc3c);hash_for_each_possible(
DATARMNET031f18e039,DATARMNET63b1a086d5,hlist,hash){if(!DATARMNET94d0b7e790(
DATARMNET63b1a086d5,DATARMNET543491eb0f)){DATARMNET0386f6f82a=
DATARMNET63b1a086d5;break;}DATARMNET0386f6f82a=NULL;}if(!DATARMNET0386f6f82a){
spin_unlock_irqrestore(&DATARMNETcf56327171,DATARMNETfb0677cc3c);return;}if(
DATARMNET63b1a086d5){hash_del(&DATARMNET63b1a086d5->hlist);list_add_tail(&
DATARMNET63b1a086d5->list,&DATARMNETc588a190d4);}spin_unlock_irqrestore(&
DATARMNETcf56327171,DATARMNETfb0677cc3c);DATARMNET9d42c8a184(DATARMNET2e7cc88971
);}static void DATARMNET9eb31df630(struct DATARMNETb477d446e7*
DATARMNET63b1a086d5,struct sk_buff*DATARMNET543491eb0f){if(DATARMNET543491eb0f->
protocol==htons(ETH_P_IP)){DATARMNET63b1a086d5->DATARMNETdfd47b00ff[
(0xd2d+202-0xdf7)]=ip_hdr(DATARMNET543491eb0f)->daddr;DATARMNET63b1a086d5->
DATARMNETdfd47b00ff[(0xd26+209-0xdf6)]=ip_hdr(DATARMNET543491eb0f)->saddr;
DATARMNET63b1a086d5->DATARMNETdfd47b00ff[(0xd1f+216-0xdf5)]=ip_hdr(
DATARMNET543491eb0f)->protocol;DATARMNET63b1a086d5->DATARMNETdfd47b00ff[
(0xd18+223-0xdf4)]=tcp_hdr(DATARMNET543491eb0f)->dest;DATARMNET63b1a086d5->
DATARMNETdfd47b00ff[(0xd11+230-0xdf3)]=tcp_hdr(DATARMNET543491eb0f)->source;
DATARMNET63b1a086d5->DATARMNET4b119c4ff3=(0xd0a+237-0xdf2);}else if(
DATARMNET543491eb0f->protocol==htons(ETH_P_IPV6)){memcpy(&DATARMNET63b1a086d5->
DATARMNETdfd47b00ff[(0xd2d+202-0xdf7)],ipv6_hdr(DATARMNET543491eb0f)->daddr.
s6_addr,sizeof(struct in6_addr));memcpy(&DATARMNET63b1a086d5->
DATARMNETdfd47b00ff[(0xd0a+237-0xdf2)],ipv6_hdr(DATARMNET543491eb0f)->saddr.
s6_addr,sizeof(struct in6_addr));DATARMNET63b1a086d5->DATARMNETdfd47b00ff[
(0xd35+210-0xdff)]=ipv6_hdr(DATARMNET543491eb0f)->nexthdr;DATARMNET63b1a086d5->
DATARMNETdfd47b00ff[9]=tcp_hdr(DATARMNET543491eb0f)->dest;DATARMNET63b1a086d5->
DATARMNETdfd47b00ff[(0xd27+224-0xdfd)]=tcp_hdr(DATARMNET543491eb0f)->source;
DATARMNET63b1a086d5->DATARMNET4b119c4ff3=(0xd20+231-0xdfc);}}void 
DATARMNET6ab362e985(struct sk_buff*DATARMNET543491eb0f){int DATARMNET76bafbc6ce=
(0xd2d+202-0xdf7);u32 hash;unsigned long DATARMNETfb0677cc3c;struct 
DATARMNETb477d446e7*DATARMNET63b1a086d5;struct hlist_node*DATARMNET0386f6f82a;
int DATARMNETae0201901a;struct net_device*dev;if(!configure_knob1&&!
configure_knob2)return;if(!DATARMNET543491eb0f->skb_iif)return;dev=
__dev_get_by_index(&init_net,DATARMNET543491eb0f->skb_iif);if(!dev)return;hash=
DATARMNET233bccd449(DATARMNET543491eb0f,&DATARMNET76bafbc6ce,(0xd2d+202-0xdf7),
(0xd26+209-0xdf6));if(!DATARMNET76bafbc6ce)return;spin_lock_irqsave(&
DATARMNETcf56327171,DATARMNETfb0677cc3c);DATARMNET63b1a086d5=
list_first_entry_or_null(&DATARMNETc588a190d4,struct DATARMNETb477d446e7,list);
if(DATARMNET63b1a086d5){list_del(&DATARMNET63b1a086d5->list);DATARMNET63b1a086d5
->hash=hash;DATARMNET9eb31df630(DATARMNET63b1a086d5,DATARMNET543491eb0f);
hash_add(DATARMNET031f18e039,&DATARMNET63b1a086d5->hlist,DATARMNET63b1a086d5->
hash);}else{hash_for_each_safe(DATARMNET031f18e039,DATARMNETae0201901a,
DATARMNET0386f6f82a,DATARMNET63b1a086d5,hlist){hash_del(&DATARMNET63b1a086d5->
hlist);DATARMNET63b1a086d5->hash=hash;DATARMNET9eb31df630(DATARMNET63b1a086d5,
DATARMNET543491eb0f);hash_add(DATARMNET031f18e039,&DATARMNET63b1a086d5->hlist,
DATARMNET63b1a086d5->hash);break;}}spin_unlock_irqrestore(&DATARMNETcf56327171,
DATARMNETfb0677cc3c);}void DATARMNET136e008d70(u8 DATARMNET07b1a05079,u64 
DATARMNETcc6099cb14){struct net_device*dev=dev_get_by_name(&init_net,
"rmnet_ipa0");struct sk_buff*DATARMNET543491eb0f;if(!dev)return;if(
DATARMNET07b1a05079==DATARMNETc99cb98651){struct 
rmnet_map_control_command_header*DATARMNETb87c07b92a;struct DATARMNETb0eda9d28e*
DATARMNETcd32395e82;struct rmnet_map_header*DATARMNETf937c77d8f;
DATARMNET543491eb0f=alloc_skb((0xeb7+1158-0x132d),GFP_ATOMIC);if(!
DATARMNET543491eb0f)goto DATARMNETaabe3a05f8;skb_put(DATARMNET543491eb0f,
(0xeb7+1158-0x132d));memset(DATARMNET543491eb0f->data,(0xd2d+202-0xdf7),
(0xeb7+1158-0x132d));DATARMNETf937c77d8f=(struct rmnet_map_header*)
DATARMNET543491eb0f->data;DATARMNETf937c77d8f->cd_bit=(0xd26+209-0xdf6);
DATARMNETf937c77d8f->pkt_len=htons(DATARMNETfc6d4bac8a);DATARMNETb87c07b92a=(
struct rmnet_map_control_command_header*)(DATARMNET543491eb0f->data+sizeof(*
DATARMNETf937c77d8f));DATARMNETb87c07b92a->command_name=DATARMNETc152fdcc34;
DATARMNETcd32395e82=(struct DATARMNETb0eda9d28e*)(DATARMNET543491eb0f->data+
sizeof(*DATARMNETf937c77d8f)+sizeof(*DATARMNETb87c07b92a));DATARMNETcd32395e82->
DATARMNETba8c503d04=DATARMNET7bd625a8c0;DATARMNETcd32395e82->DATARMNETc59f891d69
=!DATARMNETcc6099cb14;DATARMNET543491eb0f->dev=dev;DATARMNET543491eb0f->protocol
=htons(ETH_P_MAP);dev_queue_xmit(DATARMNET543491eb0f);}DATARMNETaabe3a05f8:
dev_put(dev);}void DATARMNET5fe3ffe2ab(void){rcu_assign_pointer(
rmnet_perf_tether_ingress_hook,DATARMNETe70d5ec61a);rcu_assign_pointer(
rmnet_perf_tether_egress_hook,DATARMNET6ab362e985);rcu_assign_pointer(
rmnet_perf_tether_cmd_hook,DATARMNET136e008d70);}void DATARMNET229327cbd2(void){
rcu_assign_pointer(rmnet_perf_tether_ingress_hook,NULL);rcu_assign_pointer(
rmnet_perf_tether_egress_hook,NULL);rcu_assign_pointer(
rmnet_perf_tether_cmd_hook,NULL);}static int DATARMNETaa549ce89a(void){int 
DATARMNETefc9df3df2;DATARMNET8a1e9920bf=kzalloc(sizeof(*DATARMNET8a1e9920bf),
GFP_KERNEL);if(!DATARMNET8a1e9920bf){pr_err(
"\x25\x73\x28\x29\x3a\x20\x52\x65\x73\x6f\x75\x72\x63\x65\x20\x61\x6c\x6c\x6f\x63\x61\x74\x69\x6f\x6e\x20\x66\x61\x69\x6c\x65\x64" "\n"
,__func__);return-(0xd26+209-0xdf6);}DATARMNET8a1e9920bf->DATARMNETd0ef85194f++;
for(DATARMNETefc9df3df2=(0xd2d+202-0xdf7);DATARMNETefc9df3df2<
DATARMNETd063f2edce;DATARMNETefc9df3df2++){struct DATARMNETb477d446e7*
DATARMNET63b1a086d5;DATARMNET63b1a086d5=kzalloc(sizeof(*DATARMNET63b1a086d5),
GFP_ATOMIC);if(!DATARMNET63b1a086d5)continue;INIT_LIST_HEAD(&DATARMNET63b1a086d5
->list);INIT_HLIST_NODE(&DATARMNET63b1a086d5->hlist);list_add_tail(&
DATARMNET63b1a086d5->list,&DATARMNETc588a190d4);}DATARMNET5fe3ffe2ab();return
(0xd2d+202-0xdf7);}static void DATARMNET2b8220b2f3(void){struct 
DATARMNETb477d446e7*DATARMNET63b1a086d5;struct hlist_node*DATARMNET0386f6f82a;
int DATARMNETae0201901a;hash_for_each_safe(DATARMNET031f18e039,
DATARMNETae0201901a,DATARMNET0386f6f82a,DATARMNET63b1a086d5,hlist){hash_del(&
DATARMNET63b1a086d5->hlist);kfree(DATARMNET63b1a086d5);}}static void 
DATARMNET66219231cc(void){struct DATARMNETb477d446e7*DATARMNET63b1a086d5,*
DATARMNET2133db01db;list_for_each_entry_safe(DATARMNET63b1a086d5,
DATARMNET2133db01db,&DATARMNETc588a190d4,list){list_del(&DATARMNET63b1a086d5->
list);kfree(DATARMNET63b1a086d5);}}static void DATARMNET3ece352201(void){
DATARMNET229327cbd2();DATARMNET66219231cc();DATARMNET2b8220b2f3();kfree(
DATARMNET8a1e9920bf);DATARMNET8a1e9920bf=NULL;}static int DATARMNET6e31e62d2e(
struct notifier_block*DATARMNET272c159b3c,unsigned long DATARMNET4abd997295,void
*DATARMNET3f254bae11){struct net_device*DATARMNET00dcb79bc4=
netdev_notifier_info_to_dev(DATARMNET3f254bae11);int DATARMNETb14e52a504;(void)
DATARMNET272c159b3c;if(!DATARMNET00dcb79bc4||strncmp(DATARMNET00dcb79bc4->name,
"\x72\x6d\x6e\x65\x74\x5f\x64\x61\x74\x61",(0xd27+224-0xdfd)))goto 
DATARMNETaabe3a05f8;switch(DATARMNET4abd997295){case NETDEV_REGISTER:if(
DATARMNET8a1e9920bf){DATARMNET8a1e9920bf->DATARMNETd0ef85194f++;goto 
DATARMNETaabe3a05f8;}pr_info(
"\x25\x73\x28\x29\x3a\x20\x49\x6e\x69\x74\x69\x61\x6c\x69\x7a\x69\x6e\x67\x20\x6f\x6e\x20\x64\x65\x76\x69\x63\x65\x20\x25\x73" "\n"
,__func__,DATARMNET00dcb79bc4->name);DATARMNETb14e52a504=DATARMNETaa549ce89a();
if(DATARMNETb14e52a504){pr_err(
"\x25\x73\x28\x29\x3a\x20\x49\x6e\x69\x74\x69\x61\x6c\x69\x7a\x61\x74\x69\x6f\x6e\x20\x66\x61\x69\x6c\x65\x64" "\n"
,__func__);goto DATARMNETaabe3a05f8;}break;case NETDEV_UNREGISTER:if(!
DATARMNET8a1e9920bf)goto DATARMNETaabe3a05f8;if(--DATARMNET8a1e9920bf->
DATARMNETd0ef85194f)goto DATARMNETaabe3a05f8;pr_info(
"\x25\x73\x28\x29\x3a\x20\x55\x6e\x69\x6e\x69\x74\x69\x61\x6c\x69\x7a\x69\x6e\x67\x20\x6f\x6e\x20\x64\x65\x76\x69\x63\x65\x20\x25\x73" "\n"
,__func__,DATARMNET00dcb79bc4->name);DATARMNET3ece352201();break;}
DATARMNETaabe3a05f8:return NOTIFY_DONE;}static struct notifier_block 
DATARMNETd9d8382fca={.notifier_call=DATARMNET6e31e62d2e,.priority=
(0xd18+223-0xdf4),};static int __init DATARMNET5e4be08a32(void){pr_info(
"\x25\x73\x28\x29\x3a\x20\x4c\x6f\x61\x64\x69\x6e\x67" "\n",__func__);return 
register_netdevice_notifier(&DATARMNETd9d8382fca);}static void __exit 
DATARMNET50e3d76d94(void){pr_info(
"\x25\x73\x28\x29\x3a\x20\x65\x78\x69\x74\x69\x6e\x67" "\n",__func__);
unregister_netdevice_notifier(&DATARMNETd9d8382fca);}module_init(
DATARMNET5e4be08a32);module_exit(DATARMNET50e3d76d94);
