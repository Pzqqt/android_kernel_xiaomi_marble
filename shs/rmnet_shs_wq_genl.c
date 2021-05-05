/* Copyright (c) 2019-2021 The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "rmnet_shs_modules.h"
#include "rmnet_shs_common.h"
#include "rmnet_shs_wq_genl.h"
#include <net/sock.h>
#include <linux/skbuff.h>
MODULE_LICENSE("\x47\x50\x4c\x20\x76\x32");static struct net*DATARMNETb01cbc5ec9
;static u32 DATARMNET373156e169;uint32_t DATARMNET7c4038843f;int 
DATARMNETc252c204a8;
#define DATARMNETe429c5f3dd	(0xbf7+4364-0x1c04)
static struct nla_policy DATARMNET23b45455b1[DATARMNETcecb35ee33+
(0xd26+209-0xdf6)]={[DATARMNET7d289a7bfa]={.type=NLA_S32},[DATARMNET813a742587]=
NLA_POLICY_EXACT_LEN(sizeof(struct DATARMNET6c41b886b2)),[DATARMNET50e1cd26c7]=
NLA_POLICY_EXACT_LEN(sizeof(struct DATARMNET837c876a22)),[DATARMNETaa0fe5a855]={
.type=NLA_NUL_STRING,.len=DATARMNETe429c5f3dd},};
#define DATARMNETcfe22ed4d3(DATARMNET5aeb0ef9bc, DATARMNETbd9859b58e)			\
	{						\
		.cmd	= DATARMNET5aeb0ef9bc,				\
		.doit	= DATARMNETbd9859b58e,			\
		.dumpit	= NULL,				\
		.flags	= (0xd2d+202-0xdf7),				\
	}
static const struct genl_ops DATARMNETf2d168ff8d[]={DATARMNETcfe22ed4d3(
DATARMNETafd55f4acf,DATARMNET740f3b34b3),DATARMNETcfe22ed4d3(DATARMNET8e3adfc5dd
,DATARMNET29175fb5fc),DATARMNETcfe22ed4d3(DATARMNETffb2945689,
DATARMNETd81d2866ba),DATARMNETcfe22ed4d3(DATARMNET51b1ee5a68,DATARMNETc850634243
),};struct genl_family DATARMNETecc643c219={.hdrsize=(0xd2d+202-0xdf7),.name=
DATARMNET0228d9f101,.version=DATARMNET0fa03ac25b,.maxattr=DATARMNETcecb35ee33,.
policy=DATARMNET23b45455b1,.ops=DATARMNETf2d168ff8d,.n_ops=ARRAY_SIZE(
DATARMNETf2d168ff8d),};int DATARMNET5d4ca1da1c(struct genl_info*
DATARMNET54338da2ff,int val){struct sk_buff*skb;void*msg_head;int rc;skb=
genlmsg_new(NLMSG_GOODSIZE,GFP_ATOMIC);if(skb==NULL)goto DATARMNETbf4095f79e;
msg_head=genlmsg_put(skb,(0xd2d+202-0xdf7),DATARMNET54338da2ff->snd_seq+
(0xd26+209-0xdf6),&DATARMNETecc643c219,(0xd2d+202-0xdf7),DATARMNETafd55f4acf);if
(msg_head==NULL){rc=-ENOMEM;goto DATARMNETbf4095f79e;}rc=nla_put_u32(skb,
DATARMNET7d289a7bfa,val);if(rc!=(0xd2d+202-0xdf7))goto DATARMNETbf4095f79e;
genlmsg_end(skb,msg_head);rc=genlmsg_unicast(genl_info_net(DATARMNET54338da2ff),
skb,DATARMNET54338da2ff->snd_portid);if(rc!=(0xd2d+202-0xdf7))goto 
DATARMNETbf4095f79e;rm_err(
"\x53\x48\x53\x5f\x47\x4e\x4c\x3a\x20\x53\x75\x63\x63\x65\x73\x73\x66\x75\x6c\x6c\x79\x20\x73\x65\x6e\x74\x20\x69\x6e\x74\x20\x25\x64" "\n"
,val);return(0xd2d+202-0xdf7);DATARMNETbf4095f79e:rm_err(
"\x53\x48\x53\x5f\x47\x4e\x4c\x3a\x20\x46\x41\x49\x4c\x45\x44\x20\x74\x6f\x20\x73\x65\x6e\x64\x20\x69\x6e\x74\x20\x25\x64" "\n"
,val);return-(0xd26+209-0xdf6);}int DATARMNET5945236cd3(int val){struct sk_buff*
skb;void*msg_head;int rc;if(DATARMNETb01cbc5ec9==NULL){rm_err(
"\x53\x48\x53\x5f\x47\x4e\x4c\x3a\x20\x46\x41\x49\x4c\x45\x44\x20\x74\x6f\x20\x73\x65\x6e\x64\x20\x69\x6e\x74\x20\x25\x64\x20\x2d\x20\x6c\x61\x73\x74\x5f\x6e\x65\x74\x20\x69\x73\x20\x4e\x55\x4c\x4c" "\n"
,val);return-(0xd26+209-0xdf6);}skb=genlmsg_new(NLMSG_GOODSIZE,GFP_ATOMIC);if(
skb==NULL)goto DATARMNETbf4095f79e;msg_head=genlmsg_put(skb,(0xd2d+202-0xdf7),
DATARMNET7c4038843f++,&DATARMNETecc643c219,(0xd2d+202-0xdf7),DATARMNETafd55f4acf
);if(msg_head==NULL){rc=-ENOMEM;goto DATARMNETbf4095f79e;}rc=nla_put_u32(skb,
DATARMNET7d289a7bfa,val);if(rc!=(0xd2d+202-0xdf7))goto DATARMNETbf4095f79e;
genlmsg_end(skb,msg_head);rc=genlmsg_unicast(DATARMNETb01cbc5ec9,skb,
DATARMNET373156e169);if(rc!=(0xd2d+202-0xdf7))goto DATARMNETbf4095f79e;rm_err(
"\x53\x48\x53\x5f\x47\x4e\x4c\x3a\x20\x53\x75\x63\x63\x65\x73\x73\x66\x75\x6c\x6c\x79\x20\x73\x65\x6e\x74\x20\x69\x6e\x74\x20\x25\x64" "\n"
,val);return(0xd2d+202-0xdf7);DATARMNETbf4095f79e:rm_err(
"\x53\x48\x53\x5f\x47\x4e\x4c\x3a\x20\x46\x41\x49\x4c\x45\x44\x20\x74\x6f\x20\x73\x65\x6e\x64\x20\x69\x6e\x74\x20\x25\x64" "\n"
,val);DATARMNETc252c204a8=(0xd2d+202-0xdf7);return-(0xd26+209-0xdf6);}int 
DATARMNETa9a7fa898c(void){struct sk_buff*skb;void*msg_head;int rc;int val=
DATARMNET7c4038843f++;rm_err(
"\x53\x48\x53\x5f\x47\x4e\x4c\x3a\x20\x54\x72\x79\x69\x6e\x67\x20\x74\x6f\x20\x73\x65\x6e\x64\x20\x6d\x73\x67\x20\x25\x64" "\n"
,val);skb=genlmsg_new(NLMSG_GOODSIZE,GFP_ATOMIC);if(skb==NULL)goto 
DATARMNETbf4095f79e;msg_head=genlmsg_put(skb,(0xd2d+202-0xdf7),
DATARMNET7c4038843f++,&DATARMNETecc643c219,(0xd2d+202-0xdf7),DATARMNETafd55f4acf
);if(msg_head==NULL){rc=-ENOMEM;goto DATARMNETbf4095f79e;}rc=nla_put_u32(skb,
DATARMNET7d289a7bfa,val);if(rc!=(0xd2d+202-0xdf7))goto DATARMNETbf4095f79e;
genlmsg_end(skb,msg_head);genlmsg_multicast(&DATARMNETecc643c219,skb,
(0xd2d+202-0xdf7),(0xd2d+202-0xdf7),GFP_ATOMIC);rm_err(
"\x53\x48\x53\x5f\x47\x4e\x4c\x3a\x20\x53\x75\x63\x63\x65\x73\x73\x66\x75\x6c\x6c\x79\x20\x73\x65\x6e\x74\x20\x69\x6e\x74\x20\x25\x64" "\n"
,val);return(0xd2d+202-0xdf7);DATARMNETbf4095f79e:rm_err(
"\x53\x48\x53\x5f\x47\x4e\x4c\x3a\x20\x46\x41\x49\x4c\x45\x44\x20\x74\x6f\x20\x73\x65\x6e\x64\x20\x69\x6e\x74\x20\x25\x64" "\n"
,val);DATARMNETc252c204a8=(0xd2d+202-0xdf7);return-(0xd26+209-0xdf6);}int 
DATARMNET740f3b34b3(struct sk_buff*DATARMNETaafc1d9519,struct genl_info*
DATARMNET54338da2ff){rm_err("\x25\x73",
"\x53\x48\x53\x5f\x47\x4e\x4c\x3a\x20\x72\x6d\x6e\x65\x74\x5f\x73\x68\x73\x5f\x67\x65\x6e\x6c\x5f\x64\x6d\x61\x5f\x69\x6e\x69\x74"
);if(DATARMNET54338da2ff==NULL){rm_err("\x25\x73",
"\x53\x48\x53\x5f\x47\x4e\x4c\x3a\x20\x61\x6e\x20\x65\x72\x72\x6f\x72\x20\x6f\x63\x63\x75\x72\x65\x64\x20\x2d\x20\x69\x6e\x66\x6f\x20\x69\x73\x20\x6e\x75\x6c\x6c"
);return-(0xd26+209-0xdf6);}return(0xd2d+202-0xdf7);}int DATARMNETd81d2866ba(
struct sk_buff*DATARMNETaafc1d9519,struct genl_info*DATARMNET54338da2ff){struct 
nlattr*na;struct DATARMNET837c876a22 DATARMNET1317c6a4a2;int rc=
(0xd2d+202-0xdf7);rm_err("\x25\x73",
"\x53\x48\x53\x5f\x47\x4e\x4c\x3a\x20\x72\x6d\x6e\x65\x74\x5f\x73\x68\x73\x5f\x67\x65\x6e\x6c\x5f\x73\x65\x74\x5f\x66\x6c\x6f\x77\x5f\x73\x65\x67\x6d\x65\x6e\x74\x61\x74\x69\x6f\x6e"
);if(DATARMNET54338da2ff==NULL){rm_err("\x25\x73",
"\x53\x48\x53\x5f\x47\x4e\x4c\x3a\x20\x61\x6e\x20\x65\x72\x72\x6f\x72\x20\x6f\x63\x63\x75\x72\x65\x64\x20\x2d\x20\x69\x6e\x66\x6f\x20\x69\x73\x20\x6e\x75\x6c\x6c"
);return-(0xd26+209-0xdf6);}na=DATARMNET54338da2ff->attrs[DATARMNET50e1cd26c7];
if(na){if(nla_memcpy(&DATARMNET1317c6a4a2,na,sizeof(DATARMNET1317c6a4a2))>
(0xd2d+202-0xdf7)){rm_err(
"\x53\x48\x53\x5f\x47\x4e\x4c\x3a\x20\x72\x65\x63\x76\x20\x73\x65\x67\x6d\x65\x6e\x74\x61\x74\x69\x6f\x6e\x20\x72\x65\x71\x20"
"\x68\x61\x73\x68\x5f\x74\x6f\x5f\x73\x65\x74\x20\x3d\x20\x30\x78\x25\x78\x20\x73\x65\x67\x73\x5f\x70\x65\x72\x5f\x73\x6b\x62\x20\x3d\x20\x25\x75"
,DATARMNET1317c6a4a2.DATARMNET8c11bd9466,DATARMNET1317c6a4a2.DATARMNET87636d0152
);rc=DATARMNETf85599b9d8(DATARMNET1317c6a4a2.DATARMNET8c11bd9466,
DATARMNET1317c6a4a2.DATARMNET87636d0152);if(rc==(0xd26+209-0xdf6)){
DATARMNET5d4ca1da1c(DATARMNET54338da2ff,DATARMNET0cb8735618);
trace_rmnet_shs_wq_high(DATARMNETa0ecb9daac,DATARMNETf814701a94,
DATARMNET1317c6a4a2.DATARMNET8c11bd9466,DATARMNET1317c6a4a2.DATARMNET87636d0152,
(0x16e8+787-0xc0c),(0x16e8+787-0xc0c),NULL,NULL);}else{DATARMNET5d4ca1da1c(
DATARMNET54338da2ff,DATARMNET00895c1601);trace_rmnet_shs_wq_high(
DATARMNETa0ecb9daac,DATARMNET166a43f3aa,DATARMNET1317c6a4a2.DATARMNET8c11bd9466,
DATARMNET1317c6a4a2.DATARMNET87636d0152,(0x16e8+787-0xc0c),(0x16e8+787-0xc0c),
NULL,NULL);return(0xd2d+202-0xdf7);}}else{rm_err(
"\x53\x48\x53\x5f\x47\x4e\x4c\x3a\x20\x6e\x6c\x61\x5f\x6d\x65\x6d\x63\x70\x79\x20\x66\x61\x69\x6c\x65\x64\x20\x25\x64" "\n"
,DATARMNET50e1cd26c7);DATARMNET5d4ca1da1c(DATARMNET54338da2ff,
DATARMNET00895c1601);return(0xd2d+202-0xdf7);}}else{rm_err(
"\x53\x48\x53\x5f\x47\x4e\x4c\x3a\x20\x6e\x6f\x20\x69\x6e\x66\x6f\x2d\x3e\x61\x74\x74\x72\x73\x20\x25\x64" "\n"
,DATARMNET50e1cd26c7);DATARMNET5d4ca1da1c(DATARMNET54338da2ff,
DATARMNET00895c1601);return(0xd2d+202-0xdf7);}return(0xd2d+202-0xdf7);}int 
DATARMNET29175fb5fc(struct sk_buff*DATARMNETaafc1d9519,struct genl_info*
DATARMNET54338da2ff){struct nlattr*na;struct DATARMNET6c41b886b2 
DATARMNET7f0ce2d6ad;int rc=(0xd2d+202-0xdf7);rm_err("\x25\x73",
"\x53\x48\x53\x5f\x47\x4e\x4c\x3a\x20\x72\x6d\x6e\x65\x74\x5f\x73\x68\x73\x5f\x67\x65\x6e\x6c\x5f\x74\x72\x79\x5f\x74\x6f\x5f\x6d\x6f\x76\x65\x5f\x66\x6c\x6f\x77"
);if(DATARMNET54338da2ff==NULL){rm_err("\x25\x73",
"\x53\x48\x53\x5f\x47\x4e\x4c\x3a\x20\x61\x6e\x20\x65\x72\x72\x6f\x72\x20\x6f\x63\x63\x75\x72\x65\x64\x20\x2d\x20\x69\x6e\x66\x6f\x20\x69\x73\x20\x6e\x75\x6c\x6c"
);return-(0xd26+209-0xdf6);}na=DATARMNET54338da2ff->attrs[DATARMNET813a742587];
if(na){if(nla_memcpy(&DATARMNET7f0ce2d6ad,na,sizeof(DATARMNET7f0ce2d6ad))>
(0xd2d+202-0xdf7)){rm_err(
"\x53\x48\x53\x5f\x47\x4e\x4c\x3a\x20\x63\x75\x72\x5f\x63\x70\x75\x20\x3d\x25\x75\x20\x64\x65\x73\x74\x5f\x63\x70\x75\x20\x3d\x20\x25\x75\x20"
"\x68\x61\x73\x68\x5f\x74\x6f\x5f\x6d\x6f\x76\x65\x20\x3d\x20\x30\x78\x25\x78\x20\x73\x75\x67\x67\x5f\x74\x79\x70\x65\x20\x3d\x20\x25\x75"
,DATARMNET7f0ce2d6ad.DATARMNETc790ff30fc,DATARMNET7f0ce2d6ad.DATARMNET208ea67e1d
,DATARMNET7f0ce2d6ad.DATARMNET4da4612f1e,DATARMNET7f0ce2d6ad.DATARMNETa3f89581b5
);if(DATARMNET7f0ce2d6ad.DATARMNETa3f89581b5==DATARMNET5dccc475d4){
DATARMNETb7ddf3c5dd[DATARMNETf13db5ace8]++;if(!(((0xd26+209-0xdf6)<<
DATARMNET7f0ce2d6ad.DATARMNET208ea67e1d)&DATARMNET9273f84bf1)||((
(0xd26+209-0xdf6)<<DATARMNETecc0627c70.DATARMNET5c24e1df05)&DATARMNET9273f84bf1)
){DATARMNET5d4ca1da1c(DATARMNET54338da2ff,DATARMNET2d19c9b1ef);return-
(0xd26+209-0xdf6);}DATARMNETecc0627c70.DATARMNET5c24e1df05=DATARMNET7f0ce2d6ad.
DATARMNET208ea67e1d;rcu_read_lock();RCU_INIT_POINTER(rmnet_shs_switch,
DATARMNET756778f14f);rcu_read_unlock();DATARMNET5d4ca1da1c(DATARMNET54338da2ff,
DATARMNET2d19c9b1ef);return(0xd2d+202-0xdf7);}if(DATARMNET7f0ce2d6ad.
DATARMNETa3f89581b5==DATARMNET37da25c8e8){DATARMNETb7ddf3c5dd[
DATARMNETb6eae1e097]++;if(!(((0xd26+209-0xdf6)<<DATARMNET7f0ce2d6ad.
DATARMNET208ea67e1d)&DATARMNETbc3c416b77)||(((0xd26+209-0xdf6)<<
DATARMNETecc0627c70.DATARMNET5c24e1df05)&DATARMNETbc3c416b77)){
DATARMNET5d4ca1da1c(DATARMNET54338da2ff,DATARMNET2d19c9b1ef);return-
(0xd26+209-0xdf6);}DATARMNETecc0627c70.DATARMNET5c24e1df05=DATARMNET7f0ce2d6ad.
DATARMNET208ea67e1d;rcu_read_lock();RCU_INIT_POINTER(rmnet_shs_switch,
DATARMNET756778f14f);rcu_read_unlock();DATARMNET5d4ca1da1c(DATARMNET54338da2ff,
DATARMNET2d19c9b1ef);return(0xd2d+202-0xdf7);}rc=DATARMNET5f72606f6f(
DATARMNET7f0ce2d6ad.DATARMNETc790ff30fc,DATARMNET7f0ce2d6ad.DATARMNET208ea67e1d,
DATARMNET7f0ce2d6ad.DATARMNET4da4612f1e,DATARMNET7f0ce2d6ad.DATARMNETa3f89581b5)
;if(rc==(0xd26+209-0xdf6)){DATARMNET5d4ca1da1c(DATARMNET54338da2ff,
DATARMNET598eb03fad);trace_rmnet_shs_wq_high(DATARMNETa0ecb9daac,
DATARMNETd7f7ade458,DATARMNET7f0ce2d6ad.DATARMNETc790ff30fc,DATARMNET7f0ce2d6ad.
DATARMNET208ea67e1d,DATARMNET7f0ce2d6ad.DATARMNET4da4612f1e,DATARMNET7f0ce2d6ad.
DATARMNETa3f89581b5,NULL,NULL);}else{DATARMNET5d4ca1da1c(DATARMNET54338da2ff,
DATARMNETe64295b6cb);trace_rmnet_shs_wq_high(DATARMNETa0ecb9daac,
DATARMNET53e4a6b394,DATARMNET7f0ce2d6ad.DATARMNETc790ff30fc,DATARMNET7f0ce2d6ad.
DATARMNET208ea67e1d,DATARMNET7f0ce2d6ad.DATARMNET4da4612f1e,DATARMNET7f0ce2d6ad.
DATARMNETa3f89581b5,NULL,NULL);return(0xd2d+202-0xdf7);}}else{rm_err(
"\x53\x48\x53\x5f\x47\x4e\x4c\x3a\x20\x6e\x6c\x61\x5f\x6d\x65\x6d\x63\x70\x79\x20\x66\x61\x69\x6c\x65\x64\x20\x25\x64" "\n"
,DATARMNET813a742587);DATARMNET5d4ca1da1c(DATARMNET54338da2ff,
DATARMNETe64295b6cb);return(0xd2d+202-0xdf7);}}else{rm_err(
"\x53\x48\x53\x5f\x47\x4e\x4c\x3a\x20\x6e\x6f\x20\x69\x6e\x66\x6f\x2d\x3e\x61\x74\x74\x72\x73\x20\x25\x64" "\n"
,DATARMNET813a742587);DATARMNET5d4ca1da1c(DATARMNET54338da2ff,
DATARMNETe64295b6cb);return(0xd2d+202-0xdf7);}return(0xd2d+202-0xdf7);}int 
DATARMNETc850634243(struct sk_buff*DATARMNETaafc1d9519,struct genl_info*
DATARMNET54338da2ff){rm_err("\x25\x73",
"\x53\x48\x53\x5f\x47\x4e\x4c\x3a\x20\x72\x6d\x6e\x65\x74\x5f\x73\x68\x73\x5f\x67\x65\x6e\x6c\x5f\x6d\x65\x6d\x5f\x73\x79\x6e\x63"
);if(!DATARMNETc252c204a8)DATARMNETc252c204a8=(0xd26+209-0xdf6);
trace_rmnet_shs_wq_high(DATARMNETa0ecb9daac,DATARMNETd1d3902361,
(0x16e8+787-0xc0c),(0x16e8+787-0xc0c),(0x16e8+787-0xc0c),(0x16e8+787-0xc0c),NULL
,NULL);if(DATARMNET54338da2ff==NULL){rm_err("\x25\x73",
"\x53\x48\x53\x5f\x47\x4e\x4c\x3a\x20\x61\x6e\x20\x65\x72\x72\x6f\x72\x20\x6f\x63\x63\x75\x72\x65\x64\x20\x2d\x20\x69\x6e\x66\x6f\x20\x69\x73\x20\x6e\x75\x6c\x6c"
);return-(0xd26+209-0xdf6);}DATARMNETb01cbc5ec9=genl_info_net(
DATARMNET54338da2ff);DATARMNET373156e169=DATARMNET54338da2ff->snd_portid;return
(0xd2d+202-0xdf7);}int DATARMNET0dbc627e8f(void){int ret;DATARMNETc252c204a8=
(0xd2d+202-0xdf7);ret=genl_register_family(&DATARMNETecc643c219);if(ret!=
(0xd2d+202-0xdf7)){rm_err(
"\x53\x48\x53\x5f\x47\x4e\x4c\x3a\x20\x72\x65\x67\x69\x73\x74\x65\x72\x20\x66\x61\x6d\x69\x6c\x79\x20\x66\x61\x69\x6c\x65\x64\x3a\x20\x25\x69"
,ret);genl_unregister_family(&DATARMNETecc643c219);return-(0xd26+209-0xdf6);}
rm_err(
"\x53\x48\x53\x5f\x47\x4e\x4c\x3a\x20\x73\x75\x63\x63\x65\x73\x73\x66\x75\x6c\x6c\x79\x20\x72\x65\x67\x69\x73\x74\x65\x72\x65\x64\x20\x67\x65\x6e\x65\x72\x69\x63\x20\x6e\x65\x74\x6c\x69\x6e\x6b\x20\x66\x61\x6d\x69\x6c\x69\x79\x3a\x20\x25\x73"
,DATARMNET0228d9f101);return(0xd2d+202-0xdf7);}int DATARMNETeabd69d1ab(void){int
 ret;DATARMNET5945236cd3(DATARMNET19092afcc2);ret=genl_unregister_family(&
DATARMNETecc643c219);if(ret!=(0xd2d+202-0xdf7)){rm_err(
"\x53\x48\x53\x5f\x47\x4e\x4c\x3a\x20\x75\x6e\x72\x65\x67\x69\x73\x74\x65\x72\x20\x66\x61\x6d\x69\x6c\x79\x20\x66\x61\x69\x6c\x65\x64\x3a\x20\x25\x69" "\n"
,ret);}DATARMNETc252c204a8=(0xd2d+202-0xdf7);return(0xd2d+202-0xdf7);}
