/* Copyright (c) 2020-2021 The Linux Foundation. All rights reserved.
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
#include <linux/pm_qos.h>
#define DATARMNET81fe789d21 INT_MAX
#define MIN_FREQ (0xd2d+202-0xdf7)
#define DATARMNET59b491fbc9 DATARMNET81fe789d21
#define DATARMNET03d51cb126 (0xd1f+216-0xdf5)
struct cpu_freq{unsigned int DATARMNET103c8d34fe;unsigned int 
DATARMNET1159aa2cb6;};unsigned int DATARMNET666c9ff35e __read_mostly=
(0xd26+209-0xdf6);module_param(DATARMNET666c9ff35e,uint,(0xdb7+6665-0x261c));
MODULE_PARM_DESC(DATARMNET666c9ff35e,
"\x45\x6e\x61\x62\x6c\x65\x2f\x64\x69\x73\x61\x62\x6c\x65\x20\x66\x72\x65\x71\x20\x62\x6f\x6f\x73\x74\x20\x66\x65\x61\x74\x75\x72\x65"
);struct workqueue_struct*DATARMNETde8f350999;struct DATARMNET4e6b0cd2b8{struct 
cpu_freq __percpu*DATARMNET9dd9bc4abb;};static struct DATARMNET4e6b0cd2b8 
DATARMNETc4b1be7898;static struct work_struct DATARMNETbfcbb4b8ac;static 
DEFINE_PER_CPU(struct freq_qos_request,DATARMNET17d6a9530a);static void 
DATARMNETb90d2272b4(struct work_struct*DATARMNET33110a3ff5){struct cpu_freq*
DATARMNETe24d518157;unsigned int i;int ret;struct freq_qos_request*
DATARMNETddcafd8b91;get_online_cpus();for_each_online_cpu(i){DATARMNETe24d518157
=per_cpu_ptr(DATARMNETc4b1be7898.DATARMNET9dd9bc4abb,i);DATARMNETddcafd8b91=&
per_cpu(DATARMNET17d6a9530a,i);ret=freq_qos_update_request(DATARMNETddcafd8b91,
DATARMNETe24d518157->DATARMNET103c8d34fe);}put_online_cpus();}void 
DATARMNET82d7f4ffa2(void){struct cpu_freq*DATARMNETe24d518157;int i;
for_each_possible_cpu(i){DATARMNETe24d518157=per_cpu_ptr(DATARMNETc4b1be7898.
DATARMNET9dd9bc4abb,i);DATARMNETe24d518157->DATARMNET103c8d34fe=MIN_FREQ;
DATARMNETe24d518157->DATARMNET1159aa2cb6=DATARMNET81fe789d21;}}void 
DATARMNET5e4aeef593(int cpu){struct cpu_freq*DATARMNETe24d518157;int i=cpu;if((
(0xd26+209-0xdf6)<<i)&DATARMNET9273f84bf1)return;DATARMNETe24d518157=per_cpu_ptr
(DATARMNETc4b1be7898.DATARMNET9dd9bc4abb,i);DATARMNETe24d518157->
DATARMNET103c8d34fe=DATARMNET59b491fbc9;DATARMNETe24d518157->DATARMNET1159aa2cb6
=DATARMNET81fe789d21;trace_rmnet_freq_boost(i,DATARMNET59b491fbc9);}void 
DATARMNETfb7007f025(void){struct cpu_freq*DATARMNETe24d518157;int i;
for_each_possible_cpu(i){if(((0xd26+209-0xdf6)<<i)&DATARMNETbc3c416b77)continue;
DATARMNETe24d518157=per_cpu_ptr(DATARMNETc4b1be7898.DATARMNET9dd9bc4abb,i);
DATARMNETe24d518157->DATARMNET103c8d34fe=DATARMNET59b491fbc9;DATARMNETe24d518157
->DATARMNET1159aa2cb6=DATARMNET81fe789d21;trace_rmnet_freq_boost(i,
DATARMNET59b491fbc9);}if(work_pending(&DATARMNETbfcbb4b8ac))return;if(
DATARMNETde8f350999){queue_work_on(DATARMNET03d51cb126,DATARMNETde8f350999,&
DATARMNETbfcbb4b8ac);}}void DATARMNET371703c28d(void){struct cpu_freq*
DATARMNETe24d518157;int i;for_each_possible_cpu(i){DATARMNETe24d518157=
per_cpu_ptr(DATARMNETc4b1be7898.DATARMNET9dd9bc4abb,i);DATARMNETe24d518157->
DATARMNET103c8d34fe=MIN_FREQ;DATARMNETe24d518157->DATARMNET1159aa2cb6=
DATARMNET81fe789d21;trace_rmnet_freq_reset(i,MIN_FREQ);}if(work_pending(&
DATARMNETbfcbb4b8ac))return;if(DATARMNETde8f350999)queue_work_on(
DATARMNET03d51cb126,DATARMNETde8f350999,&DATARMNETbfcbb4b8ac);}static void 
DATARMNET009d37d173(void){struct freq_qos_request*DATARMNETddcafd8b91;int i;
for_each_possible_cpu(i){DATARMNETddcafd8b91=&per_cpu(DATARMNET17d6a9530a,i);if(
DATARMNETddcafd8b91&&freq_qos_request_active(DATARMNETddcafd8b91)){
freq_qos_remove_request(DATARMNETddcafd8b91);}}}int DATARMNETe6e8431304(void){
struct cpu_freq*DATARMNETe24d518157;int i;int ret=(0xd2d+202-0xdf7);struct 
freq_qos_request*DATARMNETddcafd8b91;struct cpufreq_policy*policy;
DATARMNETc4b1be7898.DATARMNET9dd9bc4abb=alloc_percpu(struct cpu_freq);if(!
DATARMNETc4b1be7898.DATARMNET9dd9bc4abb)return-ENOMEM;if(!DATARMNETde8f350999)
DATARMNETde8f350999=alloc_workqueue(
"\x73\x68\x73\x5f\x62\x6f\x6f\x73\x74\x5f\x77\x71",WQ_HIGHPRI,(0xd2d+202-0xdf7))
;if(!DATARMNETde8f350999){ret=-ENOMEM;goto err;}for_each_possible_cpu(i){
DATARMNETe24d518157=per_cpu_ptr(DATARMNETc4b1be7898.DATARMNET9dd9bc4abb,i);
DATARMNETddcafd8b91=&per_cpu(DATARMNET17d6a9530a,i);policy=cpufreq_cpu_get(i);if
(!policy){pr_err(
"\x25\x73\x3a\x20\x63\x70\x75\x66\x72\x65\x71\x20\x70\x6f\x6c\x69\x63\x79\x20\x6e\x6f\x74\x20\x66\x6f\x75\x6e\x64\x20\x66\x6f\x72\x20\x63\x70\x75\x25\x64" "\n"
,__func__,i);return-ESRCH;}ret=freq_qos_add_request(&policy->constraints,
DATARMNETddcafd8b91,FREQ_QOS_MIN,MIN_FREQ);if(ret<(0xd2d+202-0xdf7)){pr_err(
"\x25\x73\x3a\x20\x46\x61\x69\x6c\x65\x64\x20\x74\x6f\x20\x61\x64\x64\x20\x66\x72\x65\x71\x20\x63\x6f\x6e\x73\x74\x72\x61\x69\x6e\x74\x20\x28\x25\x64\x29" "\n"
,__func__,ret);return ret;}}INIT_WORK(&DATARMNETbfcbb4b8ac,DATARMNETb90d2272b4);
DATARMNET82d7f4ffa2();return(0xd2d+202-0xdf7);err:DATARMNET82d7f4ffa2();
free_percpu(DATARMNETc4b1be7898.DATARMNET9dd9bc4abb);if(DATARMNETde8f350999){
destroy_workqueue(DATARMNETde8f350999);DATARMNETde8f350999=NULL;}return ret;}int
 DATARMNETdf74db7e38(void){DATARMNET009d37d173();if(DATARMNETde8f350999){
destroy_workqueue(DATARMNETde8f350999);DATARMNETde8f350999=NULL;}free_percpu(
DATARMNETc4b1be7898.DATARMNET9dd9bc4abb);return(0xd2d+202-0xdf7);}
