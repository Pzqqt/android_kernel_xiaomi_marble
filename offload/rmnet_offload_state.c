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

#include <linux/module.h>
#include <linux/netdevice.h>
#include "rmnet_map.h"
#include "rmnet_private.h"
#include "qmi_rmnet.h"
#include "rmnet_offload_state.h"
#include "rmnet_offload_engine.h"
MODULE_LICENSE("\x47\x50\x4c\x20\x76\x32");
#define DATARMNET47b52951b8 (0x68d+4280-0x1745)
static struct DATARMNET8357daf015*DATARMNETf7e8f6395c;static void 
DATARMNET39b47bda29(void){struct rmnet_port*DATARMNETe8a5f322f0=
DATARMNETf7e8f6395c->DATARMNET547651077b;struct DATARMNET92f4d48c56*
DATARMNET18063ff0ab;qmi_rmnet_ps_ind_deregister(DATARMNETe8a5f322f0,&
DATARMNETf7e8f6395c->DATARMNETa0db7ef12a);DATARMNET18063ff0ab=&
DATARMNETf7e8f6395c->DATARMNETfe5583a585;if(DATARMNET18063ff0ab->
DATARMNETf82a9c29ed)rmnet_map_dl_ind_deregister(DATARMNETe8a5f322f0,&
DATARMNET18063ff0ab->DATARMNET524c210c13);}static void DATARMNET296eb47249(void)
{if(!DATARMNETf7e8f6395c)return;DATARMNET39b47bda29();DATARMNET848914898b();
DATARMNET28dfa538f2();DATARMNET54fd6bf483();DATARMNETde815547a0();kfree(
DATARMNETf7e8f6395c);DATARMNETf7e8f6395c=NULL;}static int DATARMNETd813fadf76(
void){struct rmnet_port*DATARMNETe8a5f322f0=DATARMNETf7e8f6395c->
DATARMNET547651077b;struct qmi_rmnet_ps_ind*DATARMNET83359ae71d;struct 
DATARMNET92f4d48c56*DATARMNET18063ff0ab;struct rmnet_map_dl_ind*
DATARMNET524c210c13;int DATARMNET094475d445=DATARMNET6e89887168;
DATARMNET83359ae71d=&DATARMNETf7e8f6395c->DATARMNETa0db7ef12a;
DATARMNET83359ae71d->ps_on_handler=DATARMNETfaa320a86a;DATARMNET83359ae71d->
ps_off_handler=DATARMNET6b57fa3471;if(qmi_rmnet_ps_ind_register(
DATARMNETe8a5f322f0,DATARMNET83359ae71d)){DATARMNET094475d445=
DATARMNETee330d5a81;pr_warn(
"\x25\x73\x28\x29\x3a\x20\x50\x53\x20\x43\x42\x20\x72\x65\x67\x69\x73\x74\x72\x61\x74\x69\x6f\x6e\x20\x66\x61\x69\x6c\x65\x64" "\n"
,__func__);}if(DATARMNETe8a5f322f0->data_format&
RMNET_INGRESS_FORMAT_DL_MARKER_V2){DATARMNET18063ff0ab=&DATARMNETf7e8f6395c->
DATARMNETfe5583a585;DATARMNET524c210c13=&DATARMNET18063ff0ab->
DATARMNET524c210c13;DATARMNET524c210c13->priority=DATARMNET47b52951b8;
DATARMNET524c210c13->dl_hdr_handler_v2=DATARMNET4aefca6d3c;DATARMNET524c210c13->
dl_trl_handler_v2=DATARMNET94edfae28d;if(rmnet_map_dl_ind_register(
DATARMNETe8a5f322f0,DATARMNET524c210c13)){DATARMNETafcdd1146f();
DATARMNET094475d445=DATARMNETee330d5a81;pr_warn(
"\x25\x73\x28\x29\x3a\x20\x44\x4c\x20\x43\x42\x20\x72\x65\x67\x69\x73\x74\x72\x61\x74\x61\x74\x69\x6f\x6e\x20\x66\x61\x69\x6c\x65\x64" "\n"
,__func__);}else{DATARMNET18063ff0ab->DATARMNETf82a9c29ed=true;}}else{
DATARMNETafcdd1146f();DATARMNET094475d445=DATARMNETee330d5a81;pr_warn(
"\x25\x73\x28\x29\x3a\x20\x44\x4c\x20\x6e\x6f\x74\x20\x65\x6e\x61\x62\x6c\x65\x64" "\n"
,__func__);}return DATARMNET094475d445;}static int DATARMNETbe48169564(struct 
rmnet_port*DATARMNETe8a5f322f0){int DATARMNET094475d445;DATARMNETf7e8f6395c=
kzalloc(sizeof(*DATARMNETf7e8f6395c),GFP_KERNEL);if(!DATARMNETf7e8f6395c){pr_err
(
"\x25\x73\x28\x29\x3a\x20\x52\x65\x73\x6f\x75\x72\x63\x65\x20\x61\x6c\x6c\x6f\x63\x61\x74\x69\x6f\x6e\x20\x66\x61\x69\x6c\x65\x64" "\n"
,__func__);return DATARMNETc50a12ee59;}DATARMNETf7e8f6395c->DATARMNET547651077b=
DATARMNETe8a5f322f0;DATARMNET094475d445=DATARMNETcce014cb1d();if(
DATARMNET094475d445<(0x15bb+934-0x1961))goto DATARMNET77e7361053;
DATARMNET094475d445=DATARMNETd813fadf76();if(DATARMNET094475d445!=
DATARMNET6e89887168){pr_warn(
"\x25\x73\x28\x29\x3a\x20\x43\x61\x6c\x6c\x62\x61\x63\x6b\x20\x72\x65\x67\x69\x73\x74\x72\x61\x74\x69\x6f\x6e\x20\x66\x61\x69\x6c\x65\x64" "\n"
,__func__);}DATARMNETd9ec5f4e65();return DATARMNET094475d445;DATARMNET77e7361053
:kfree(DATARMNETf7e8f6395c);DATARMNETf7e8f6395c=NULL;return DATARMNET094475d445;
}static int DATARMNETe4f68c17d0(struct notifier_block*DATARMNET9e976b309a,
unsigned long DATARMNET081daa57e7,void*DATARMNET71ffe74cda){struct net_device*
DATARMNET6db6ebb1e7=netdev_notifier_info_to_dev(DATARMNET71ffe74cda);struct 
rmnet_port*DATARMNETe8a5f322f0;struct rmnet_priv*DATARMNET64e0e8cc28;int 
DATARMNET094475d445;(void)DATARMNET9e976b309a;if(!DATARMNET6db6ebb1e7)goto 
DATARMNET02a8afcd23;switch(DATARMNET081daa57e7){case NETDEV_REGISTER:if(
DATARMNETf7e8f6395c||DATARMNET4e490d9d37()||strncmp(DATARMNET6db6ebb1e7->name,
"\x72\x6d\x6e\x65\x74\x5f\x64\x61\x74\x61",(0xeab+5835-0x256c)))goto 
DATARMNET02a8afcd23;DATARMNET64e0e8cc28=netdev_priv(DATARMNET6db6ebb1e7);
DATARMNETe8a5f322f0=rmnet_get_port(DATARMNET64e0e8cc28->real_dev);if(!
DATARMNETe8a5f322f0){pr_err(
"\x25\x73\x28\x29\x3a\x20\x49\x6e\x76\x61\x6c\x69\x64\x20\x72\x6d\x6e\x65\x74\x20\x63\x6f\x6e\x66\x69\x67\x75\x72\x61\x74\x69\x6f\x6e\x20\x6f\x6e\x20\x25\x73" "\n"
,__func__,DATARMNET6db6ebb1e7->name);goto DATARMNET02a8afcd23;}pr_info(
"\x25\x73\x28\x29\x3a\x20\x49\x6e\x69\x74\x69\x61\x6c\x69\x7a\x69\x6e\x67\x20\x6f\x6e\x20\x64\x65\x76\x69\x63\x65\x20\x25\x73" "\n"
,__func__,DATARMNET6db6ebb1e7->name);DATARMNET094475d445=DATARMNETbe48169564(
DATARMNETe8a5f322f0);if(DATARMNET094475d445==DATARMNETc50a12ee59){pr_err(
"\x25\x73\x28\x29\x3a\x20\x49\x6e\x69\x74\x69\x61\x6c\x69\x7a\x61\x74\x69\x6f\x6e\x20\x66\x61\x69\x6c\x65\x64" "\n"
,__func__);goto DATARMNET02a8afcd23;}break;case NETDEV_UNREGISTER:if(!
DATARMNETf7e8f6395c||!DATARMNET4e490d9d37()||strncmp(DATARMNET6db6ebb1e7->name,
DATARMNETf7e8f6395c->DATARMNET547651077b->dev->name,IFNAMSIZ))goto 
DATARMNET02a8afcd23;pr_info(
"\x25\x73\x28\x29\x3a\x20\x55\x6e\x69\x6e\x69\x74\x69\x61\x6c\x69\x7a\x69\x6e\x67\x20\x6f\x6e\x20\x64\x65\x76\x69\x63\x65\x20\x25\x73" "\n"
,__func__,DATARMNET6db6ebb1e7->name);DATARMNET296eb47249();break;}
DATARMNET02a8afcd23:return NOTIFY_DONE;}static struct notifier_block 
DATARMNET4dff0c2c8a={.notifier_call=DATARMNETe4f68c17d0,.priority=
(0xc93+6150-0x2498),};static int __init DATARMNET99216255df(void){pr_info(
"\x25\x73\x28\x29\x3a\x20\x72\x6d\x6e\x65\x74\x5f\x6f\x66\x66\x6c\x6f\x61\x64\x20\x69\x6e\x69\x74\x69\x61\x6c\x69\x7a\x69\x6e\x67" "\n"
,__func__);return register_netdevice_notifier(&DATARMNET4dff0c2c8a);}static void
 __exit DATARMNET16134ceac1(void){pr_info(
"\x25\x73\x28\x29\x3a\x20\x72\x6d\x6e\x65\x74\x5f\x6f\x66\x66\x6c\x6f\x61\x64\x20\x65\x78\x69\x74\x69\x6e\x67" "\n"
,__func__);unregister_netdevice_notifier(&DATARMNET4dff0c2c8a);}struct 
DATARMNET8357daf015*DATARMNETccc25794e0(void){return DATARMNETf7e8f6395c;}
module_init(DATARMNET99216255df);module_exit(DATARMNET16134ceac1);
