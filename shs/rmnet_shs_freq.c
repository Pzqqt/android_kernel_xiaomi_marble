/* Copyright (c) 2020 The Linux Foundation. All rights reserved.
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

#include <linux/module.h>
#include "rmnet_shs.h"
#include "rmnet_shs_freq.h"
#include <linux/cpufreq.h>
#include <linux/cpu.h>
#define DATARMNET81fe789d21 INT_MAX
#define DATARMNET3e06a1e794 (0xd2d+202-0xdf7)
#define DATARMNET59b491fbc9 DATARMNET81fe789d21
struct DATARMNET66fa8c4c30{unsigned int DATARMNET103c8d34fe;unsigned int 
DATARMNET1159aa2cb6;};unsigned int DATARMNET666c9ff35e __read_mostly=
(0xd26+209-0xdf6);module_param(DATARMNET666c9ff35e,uint,(0xdb7+6665-0x261c));
MODULE_PARM_DESC(DATARMNET666c9ff35e,
"\x45\x6e\x61\x62\x6c\x65\x2f\x64\x69\x73\x61\x62\x6c\x65\x20\x66\x72\x65\x71\x20\x62\x6f\x6f\x73\x74\x20\x66\x65\x61\x74\x75\x72\x65"
);struct workqueue_struct*DATARMNETde8f350999;struct DATARMNET4e6b0cd2b8{struct 
DATARMNET66fa8c4c30 __percpu*DATARMNET9dd9bc4abb;};static struct 
DATARMNET4e6b0cd2b8 DATARMNETc4b1be7898;static struct work_struct 
DATARMNETbfcbb4b8ac;static int DATARMNETb1e8f00240(struct notifier_block*
DATARMNET272c159b3c,unsigned long val,void*data){return NOTIFY_OK;}static struct
 notifier_block DATARMNET50781dba43={.notifier_call=DATARMNETb1e8f00240,};static
 void DATARMNET13c1a83eb3(struct work_struct*DATARMNET33110a3ff5){unsigned int i
;get_online_cpus();for_each_online_cpu(i){cpufreq_update_policy(i);}
put_online_cpus();}void DATARMNET82d7f4ffa2(void){struct DATARMNET66fa8c4c30*
DATARMNETe24d518157;int i;for_each_possible_cpu(i){DATARMNETe24d518157=
per_cpu_ptr(DATARMNETc4b1be7898.DATARMNET9dd9bc4abb,i);DATARMNETe24d518157->
DATARMNET103c8d34fe=DATARMNET3e06a1e794;DATARMNETe24d518157->DATARMNET1159aa2cb6
=DATARMNET81fe789d21;}}void DATARMNETfb7007f025(void){struct DATARMNET66fa8c4c30
*DATARMNETe24d518157;int i;for_each_possible_cpu(i){if(((0xd26+209-0xdf6)<<i)&
DATARMNETbc3c416b77)continue;DATARMNETe24d518157=per_cpu_ptr(DATARMNETc4b1be7898
.DATARMNET9dd9bc4abb,i);DATARMNETe24d518157->DATARMNET103c8d34fe=
DATARMNET59b491fbc9;DATARMNETe24d518157->DATARMNET1159aa2cb6=DATARMNET81fe789d21
;trace_rmnet_freq_boost(i,DATARMNETe24d518157->DATARMNET103c8d34fe);}if(
work_pending(&DATARMNETbfcbb4b8ac))return;if(DATARMNETde8f350999)queue_work(
DATARMNETde8f350999,&DATARMNETbfcbb4b8ac);}void DATARMNET371703c28d(void){struct
 DATARMNET66fa8c4c30*DATARMNETe24d518157;int i;for_each_possible_cpu(i){if((
(0xd26+209-0xdf6)<<i)&DATARMNETbc3c416b77)continue;DATARMNETe24d518157=
per_cpu_ptr(DATARMNETc4b1be7898.DATARMNET9dd9bc4abb,i);DATARMNETe24d518157->
DATARMNET103c8d34fe=DATARMNET3e06a1e794;DATARMNETe24d518157->DATARMNET1159aa2cb6
=DATARMNET81fe789d21;trace_rmnet_freq_reset(i,DATARMNETe24d518157->
DATARMNET103c8d34fe);}if(work_pending(&DATARMNETbfcbb4b8ac))return;if(
DATARMNETde8f350999)queue_work(DATARMNETde8f350999,&DATARMNETbfcbb4b8ac);}int 
DATARMNETe6e8431304(void){DATARMNETc4b1be7898.DATARMNET9dd9bc4abb=alloc_percpu(
struct DATARMNET66fa8c4c30);if(!DATARMNETc4b1be7898.DATARMNET9dd9bc4abb)return-
ENOMEM;if(!DATARMNETde8f350999)DATARMNETde8f350999=alloc_workqueue(
"\x73\x68\x73\x5f\x62\x6f\x6f\x73\x74\x5f\x77\x71",WQ_HIGHPRI,(0xd2d+202-0xdf7))
;if(!DATARMNETde8f350999){free_percpu(DATARMNETc4b1be7898.DATARMNET9dd9bc4abb);
return-EFAULT;}INIT_WORK(&DATARMNETbfcbb4b8ac,DATARMNET13c1a83eb3);if(
DATARMNET666c9ff35e)cpufreq_register_notifier(&DATARMNET50781dba43,
CPUFREQ_POLICY_NOTIFIER);DATARMNET82d7f4ffa2();return(0xd2d+202-0xdf7);}int 
DATARMNETdf74db7e38(void){DATARMNET82d7f4ffa2();cancel_work_sync(&
DATARMNETbfcbb4b8ac);if(DATARMNETde8f350999){destroy_workqueue(
DATARMNETde8f350999);DATARMNETde8f350999=NULL;}if(DATARMNET666c9ff35e)
cpufreq_unregister_notifier(&DATARMNET50781dba43,CPUFREQ_POLICY_NOTIFIER);
free_percpu(DATARMNETc4b1be7898.DATARMNET9dd9bc4abb);return(0xd2d+202-0xdf7);}
