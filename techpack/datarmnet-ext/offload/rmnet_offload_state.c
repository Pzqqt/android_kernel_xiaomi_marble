/* Copyright (c) 2018-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/rcupdate.h>
#include "rmnet_map.h"
#include "rmnet_private.h"
#include "qmi_rmnet.h"
#include "rmnet_offload_state.h"
#include "rmnet_offload_engine.h"
MODULE_LICENSE("\x47\x50\x4c\x20\x76\x32");
#define DATARMNET1f03ff7987 (0xd2d+202-0xdf7)
static struct DATARMNET70f3b87b5d*DATARMNETe05748b000;static void 
DATARMNETac9c8c14b1(void){struct rmnet_port*DATARMNETf0d9de7e2f=
DATARMNETe05748b000->DATARMNET403589239f;struct DATARMNET74bb35b8f2*
DATARMNET87b015f76f;qmi_rmnet_ps_ind_deregister(DATARMNETf0d9de7e2f,&
DATARMNETe05748b000->DATARMNET341393eda2);DATARMNET87b015f76f=&
DATARMNETe05748b000->DATARMNETa9f2b2f677;if(DATARMNET87b015f76f->
DATARMNETc2832b8f85)rmnet_map_dl_ind_deregister(DATARMNETf0d9de7e2f,&
DATARMNET87b015f76f->DATARMNET8633ae8f1e);}static void DATARMNET345f805b34(void)
{LIST_HEAD(DATARMNET6f9bfa17e6);if(!DATARMNETe05748b000)return;
DATARMNETac9c8c14b1();DATARMNETba38823a2f();DATARMNET560e127137();
synchronize_rcu();DATARMNETae70636c90(&DATARMNET6f9bfa17e6);DATARMNETb98b78b8e3(
);kfree(DATARMNETe05748b000);DATARMNETe05748b000=NULL;}static int 
DATARMNETd68e3594a7(void){struct rmnet_port*DATARMNETf0d9de7e2f=
DATARMNETe05748b000->DATARMNET403589239f;struct qmi_rmnet_ps_ind*
DATARMNETbc78d7fd3c;struct DATARMNET74bb35b8f2*DATARMNET87b015f76f;struct 
rmnet_map_dl_ind*DATARMNET8633ae8f1e;int DATARMNETb14e52a504=DATARMNET0529bb9c4e
;DATARMNETbc78d7fd3c=&DATARMNETe05748b000->DATARMNET341393eda2;
DATARMNETbc78d7fd3c->ps_on_handler=DATARMNET9292bebdd3;DATARMNETbc78d7fd3c->
ps_off_handler=DATARMNETb7e47d7254;if(qmi_rmnet_ps_ind_register(
DATARMNETf0d9de7e2f,DATARMNETbc78d7fd3c)){DATARMNETb14e52a504=
DATARMNET8e1c5c09f8;pr_warn(
"\x25\x73\x28\x29\x3a\x20\x50\x53\x20\x43\x42\x20\x72\x65\x67\x69\x73\x74\x72\x61\x74\x69\x6f\x6e\x20\x66\x61\x69\x6c\x65\x64" "\n"
,__func__);}if(DATARMNETf0d9de7e2f->data_format&
RMNET_INGRESS_FORMAT_DL_MARKER_V2){DATARMNET87b015f76f=&DATARMNETe05748b000->
DATARMNETa9f2b2f677;DATARMNET8633ae8f1e=&DATARMNET87b015f76f->
DATARMNET8633ae8f1e;DATARMNET8633ae8f1e->priority=DATARMNET1f03ff7987;
DATARMNET8633ae8f1e->dl_hdr_handler_v2=DATARMNET95e1703026;DATARMNET8633ae8f1e->
dl_trl_handler_v2=DATARMNETc9dd320f49;if(rmnet_map_dl_ind_register(
DATARMNETf0d9de7e2f,DATARMNET8633ae8f1e)){DATARMNETb14e52a504=
DATARMNET8e1c5c09f8;pr_warn(
"\x25\x73\x28\x29\x3a\x20\x44\x4c\x20\x43\x42\x20\x72\x65\x67\x69\x73\x74\x72\x61\x74\x61\x74\x69\x6f\x6e\x20\x66\x61\x69\x6c\x65\x64" "\n"
,__func__);}else{DATARMNET87b015f76f->DATARMNETc2832b8f85=true;}}
DATARMNETd4230b6bfe();return DATARMNETb14e52a504;}static int DATARMNET9f53d54812
(struct rmnet_port*DATARMNETf0d9de7e2f){int DATARMNETb14e52a504;
DATARMNETe05748b000=kzalloc(sizeof(*DATARMNETe05748b000),GFP_KERNEL);if(!
DATARMNETe05748b000){pr_err(
"\x25\x73\x28\x29\x3a\x20\x52\x65\x73\x6f\x75\x72\x63\x65\x20\x61\x6c\x6c\x6f\x63\x61\x74\x69\x6f\x6e\x20\x66\x61\x69\x6c\x65\x64" "\n"
,__func__);return DATARMNETb881657565;}DATARMNETe05748b000->DATARMNET403589239f=
DATARMNETf0d9de7e2f;DATARMNETb14e52a504=DATARMNETdbcaf01255();if(
DATARMNETb14e52a504<(0xd2d+202-0xdf7))goto DATARMNET1d7e728ddf;
DATARMNETb14e52a504=DATARMNETd68e3594a7();if(DATARMNETb14e52a504!=
DATARMNET0529bb9c4e){pr_warn(
"\x25\x73\x28\x29\x3a\x20\x43\x61\x6c\x6c\x62\x61\x63\x6b\x20\x72\x65\x67\x69\x73\x74\x72\x61\x74\x69\x6f\x6e\x20\x66\x61\x69\x6c\x65\x64" "\n"
,__func__);}DATARMNETe05748b000->DATARMNETcb2b3ae8e4=(0xd26+209-0xdf6);
DATARMNET818b960147();return DATARMNETb14e52a504;DATARMNET1d7e728ddf:kfree(
DATARMNETe05748b000);DATARMNETe05748b000=NULL;return DATARMNETb14e52a504;}static
 int DATARMNET7dec24813e(struct notifier_block*DATARMNET272c159b3c,unsigned long
 DATARMNET4abd997295,void*DATARMNET3f254bae11){struct net_device*
DATARMNET00dcb79bc4=netdev_notifier_info_to_dev(DATARMNET3f254bae11);struct 
rmnet_port*DATARMNETf0d9de7e2f;struct rmnet_priv*DATARMNETe823dcf978;int 
DATARMNETb14e52a504;(void)DATARMNET272c159b3c;if(!DATARMNET00dcb79bc4||strncmp(
DATARMNET00dcb79bc4->name,"\x72\x6d\x6e\x65\x74\x5f\x64\x61\x74\x61",
(0xd27+224-0xdfd)))goto DATARMNETaabe3a05f8;switch(DATARMNET4abd997295){case 
NETDEV_REGISTER:if(DATARMNETe05748b000){DATARMNETe05748b000->DATARMNETcb2b3ae8e4
++;goto DATARMNETaabe3a05f8;}DATARMNETe823dcf978=netdev_priv(DATARMNET00dcb79bc4
);DATARMNETf0d9de7e2f=rmnet_get_port(DATARMNETe823dcf978->real_dev);if(!
DATARMNETf0d9de7e2f){pr_err(
"\x25\x73\x28\x29\x3a\x20\x49\x6e\x76\x61\x6c\x69\x64\x20\x72\x6d\x6e\x65\x74\x20\x63\x6f\x6e\x66\x69\x67\x75\x72\x61\x74\x69\x6f\x6e\x20\x6f\x6e\x20\x25\x73" "\n"
,__func__,DATARMNET00dcb79bc4->name);goto DATARMNETaabe3a05f8;}pr_info(
"\x25\x73\x28\x29\x3a\x20\x49\x6e\x69\x74\x69\x61\x6c\x69\x7a\x69\x6e\x67\x20\x6f\x6e\x20\x64\x65\x76\x69\x63\x65\x20\x25\x73" "\n"
,__func__,DATARMNET00dcb79bc4->name);DATARMNETb14e52a504=DATARMNET9f53d54812(
DATARMNETf0d9de7e2f);if(DATARMNETb14e52a504==DATARMNETb881657565){pr_err(
"\x25\x73\x28\x29\x3a\x20\x49\x6e\x69\x74\x69\x61\x6c\x69\x7a\x61\x74\x69\x6f\x6e\x20\x66\x61\x69\x6c\x65\x64" "\n"
,__func__);goto DATARMNETaabe3a05f8;}break;case NETDEV_UNREGISTER:if(!
DATARMNETe05748b000)goto DATARMNETaabe3a05f8;if(--DATARMNETe05748b000->
DATARMNETcb2b3ae8e4)goto DATARMNETaabe3a05f8;pr_info(
"\x25\x73\x28\x29\x3a\x20\x55\x6e\x69\x6e\x69\x74\x69\x61\x6c\x69\x7a\x69\x6e\x67\x20\x6f\x6e\x20\x64\x65\x76\x69\x63\x65\x20\x25\x73" "\n"
,__func__,DATARMNET00dcb79bc4->name);DATARMNET345f805b34();break;}
DATARMNETaabe3a05f8:return NOTIFY_DONE;}static struct notifier_block 
DATARMNETcae9c64c41={.notifier_call=DATARMNET7dec24813e,.priority=
(0xd26+209-0xdf6),};static int __init DATARMNETb1c42acd9c(void){pr_info(
"\x25\x73\x28\x29\x3a\x20\x72\x6d\x6e\x65\x74\x5f\x6f\x66\x66\x6c\x6f\x61\x64\x20\x69\x6e\x69\x74\x69\x61\x6c\x69\x7a\x69\x6e\x67" "\n"
,__func__);return register_netdevice_notifier(&DATARMNETcae9c64c41);}static void
 __exit DATARMNET0f6d85e415(void){pr_info(
"\x25\x73\x28\x29\x3a\x20\x72\x6d\x6e\x65\x74\x5f\x6f\x66\x66\x6c\x6f\x61\x64\x20\x65\x78\x69\x74\x69\x6e\x67" "\n"
,__func__);unregister_netdevice_notifier(&DATARMNETcae9c64c41);}struct 
DATARMNET70f3b87b5d*DATARMNETc2a630b113(void){return DATARMNETe05748b000;}
module_init(DATARMNETb1c42acd9c);module_exit(DATARMNET0f6d85e415);
