/*
 * Copyright (c) 2017,2019 The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *  * Neither the name of The Linux Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef CONSTANTS_H_
#define CONSTANTS_H_


#include <stdlib.h>
#include <stdio.h>

#define PRE_PHASE_ZERO_TEST_CONFIGURATION 0
/*----------------------------------------------------------------------
 *Configuration 1 (see configure_system_1 )
 *-----USB_PROD--->[IPA DMA]----USB_CONS--->
 *----------------------------------------------------------------------
 */
#define PHASE_ZERO_TEST_CONFIGURATION	  1
/*----------------------------------------------------------------------
 *Configuration 2 (see configure_system_2 )
 *					[IPA]----USB2_CONS-->
 *-----USB_PROD--->[IPA]----USB3_CONS--->
 *					[IPA]----USB4_CONS--->
 *----------------------------------------------------------------------
 */
#define PHASE_TWO_TEST_CONFIGURATION      2
/*----------------------------------------------------------------------
 *Configuration 3 (see configure_system_3 )
 *					[IPA]----USB_CONS------>
 *-----USB2_PROD--->[IPA]----USB2_CONS--->
 *					[IPA]----USB4_CONS---->
 *----------------------------------------------------------------------
 */
#define PHASE_THREE_TEST_CONFIGURATION    3
/*Configuration 5 (see configure_system_5 )
 *					[IPA]----USB2_CONS + Header Insertion (6Bytes) -->
 *-----USB_PROD--->[IPA]----USB3_CONS +
 *Header Insertion (22Bytes) + Length offset (11Bytes) -->
 *					[IPA]----USB4_CONS +
 *Header Insertion (22Bytes) + Length offset (11Bytes) + Const(1Byte)-->
 */

/* This Cfg is only for USB Integration Phase I*/
#define PHASE_FIVE_TEST_CONFIGURATION	  5
#define PHASE_SIX_TEST_CONFIGURATION	  6
/*Configuration 7 (see configure_system_7 )
 *                  [IPA]----USB2_CONS-->
 *-----USB_PROD--->[IPA]----USB3_CONS--->
 *                  [IPA]----USB4_CONS--->
 *                  [IPA]----A5 - Exception Pipe--->
 */
#define PHASE_SEVEN_TEST_CONFIGURATION    7
/*----------------------------------------------------------------------
 *Configuration 8 (see configure_system_8 )
 *-----USB3_CONS--->[IPA DMA]----USB_CONS (TLP aggregation byte limit)--->
 *-----USB_PROD (TLP deaggregation)--->[IPA DMA]----USB3_CONS--->
 *-----USB2_PROD (TLP deaggregation)--->
 *-----[IPA DMA]----USB_CONS (TLP aggregation byte limit)--->
 *-----USB4_PROD--->[IPA DMA]----USB2_CONS (TLP aggregation time limit)--->
 *----------------------------------------------------------------------
 */
#define PHASE_EIGHT_TEST_CONFIGURATION    8
/*----------------------------------------------------------------------
 *Configuration 9 (see configure_system_9 )
 *-----USB3_PROD--->[IPA DMA]----USB_CONS (MBIM aggregation byte limit)--->
 *-----USB_PROD (MBIM deaggregation)--->[IPA DMA]----USB3_CONS--->
 *-----USB2_PROD (MBIM deaggregation)--->
 *-----[IPA DMA]----USB_CONS (MBIM aggregation byte limit)--->
 *-----USB4_PROD--->[IPA DMA]----USB2_CONS (MBIM aggregation time limit)--->
 *----------------------------------------------------------------------
 */
#define PHASE_NINE_TEST_CONFIGURATION    9
/*----------------------------------------------------------------------
 *Configuration 10 (see configure_system_10 )
 *-----USB_PROD--->[IPA DMA]----USB_CONS (MBIM aggregation no limits)--->
 *----------------------------------------------------------------------
 */
#define PHASE_TEN_TEST_CONFIGURATION      10
/*----------------------------------------------------------------------
 *Configuration 11 (see configure_system_11 )
 *-----USB_PROD----->[IPA]----
 * USB2_CONS(MBIM aggregation byte limit)--->
 *-USB2_PROD (MBIM deaggregation)->[IPA]----USB3_CONS--->
 *------------------>[IPA]----
 *USB_CONS (MBIM aggregation time limit)----------->
 *------------------>[IPA]----
 *A5_LAN_WAN_CONS (MBIM aggregation no limits)---->
 *----------------------------------------------------------------------
 */
#define PHASE_ELEVEN_TEST_CONFIGURATION    11
/*----------------------------------------------------------------------
 *Configuration 12 (see configure_system_12 )
 *-----USB_PROD----->[IPA]----USB2_CONS (MBIM aggregation byte limit)--->
 *-USB2_PROD (MBIM deaggregation)->[IPA]----USB3_CONS--->
 *------------------>[IPA]----
 * USB_CONS (MBIM aggregation time limit)----------->
 *------------------>[IPA]----
 * A5_LAN_WAN_CONS (MBIM aggregation no limits)---->
 *----------------------------------------------------------------------
 */
#define PHASE_TWELVE_TEST_CONFIGURATION		12
/*----------------------------------------------------------------------
 *Configuration 9 (see configure_system_17 )
 *-----USB3_PROD--->[IPA DMA]----USB_CONS (RNDIS aggregation byte limit)--->
 *-----USB_PROD (RNDIS deaggregation)--->[IPA DMA]----USB3_CONS--->
 *-----USB2_PROD (RNDIS deaggregation)--->
 *-----[IPA DMA]----USB_CONS (RNDIS aggregation byte limit)--->
 *-----USB4--->[IPA DMA]----USB2_CONS (RNDIS aggregation time limit)--->
 *----------------------------------------------------------------------
 */
#define PHASE_SEVENTEEN_TEST_CONFIGURATION    17

/*
 * Data path test
 */
#define PHASE_EIGHTEEN_TEST_CONFIGURATION 18

/*----------------------------------------------------------------------
 *Configuration 19 (see configure_system_19 )
 *-----USB_PROD--->[IPA DMA]----USB_CONS--->
 *-----suspend [IPA DMA]----USB_CONS (for testing suspend interrupt)--->
 *----------------------------------------------------------------------
 */
#define PHASE_NINETEEN_TEST_CONFIGURATION 19

#define PHASE_TWENTY_TEST_CONFIGURATION 20
/*----------------------------------------------------------------------
 *Configuration 20 (see configure_system_20 )
 *-----PROD (WLAN header removal)--------------------->[IPA]----CONS---->
 *-----PROD (RNDIS de-aggregation + Header removal)--->[IPA]
 *----------------------------------------------------------------------
 */

enum IPATestConfiguration {
	IPA_TEST_CONFIFURATION_0 = PRE_PHASE_ZERO_TEST_CONFIGURATION,
	IPA_TEST_CONFIFURATION_1 = PHASE_ZERO_TEST_CONFIGURATION,
	IPA_TEST_CONFIFURATION_2 = PHASE_TWO_TEST_CONFIGURATION,
	IPA_TEST_CONFIFURATION_3 = PHASE_THREE_TEST_CONFIGURATION,
	IPA_TEST_CONFIFURATION_5 = PHASE_FIVE_TEST_CONFIGURATION,
	IPA_TEST_CONFIFURATION_6 = PHASE_SIX_TEST_CONFIGURATION,
	IPA_TEST_CONFIFURATION_7 = PHASE_SEVEN_TEST_CONFIGURATION,
	IPA_TEST_CONFIGURATION_8 = PHASE_EIGHT_TEST_CONFIGURATION,
	IPA_TEST_CONFIGURATION_9 = PHASE_NINE_TEST_CONFIGURATION,
	IPA_TEST_CONFIGURATION_10 = PHASE_TEN_TEST_CONFIGURATION,
	IPA_TEST_CONFIGURATION_11 = PHASE_ELEVEN_TEST_CONFIGURATION,
	IPA_TEST_CONFIGURATION_12 = PHASE_TWELVE_TEST_CONFIGURATION,
	IPA_TEST_CONFIGURATION_17 = PHASE_SEVENTEEN_TEST_CONFIGURATION,
	IPA_TEST_CONFIGURATION_18 = PHASE_EIGHTEEN_TEST_CONFIGURATION,
	IPA_TEST_CONFIGURATION_19 = PHASE_NINETEEN_TEST_CONFIGURATION,
	IPA_TEST_CONFIGURATION_20 = PHASE_NINETEEN_TEST_CONFIGURATION,
};
#define CONFIGURATION_NODE_PATH "/dev/ipa_test"

/*producer*/
#define INTERFACE0_TO_IPA_DATA_PATH			"/dev/to_ipa_0"
#define INTERFACE0_FROM_IPA_DATA_PATH		NULL

/*producer*/
#define INTERFACE4_TO_IPA_DATA_PATH			"/dev/to_ipa_1"
#define INTERFACE4_FROM_IPA_DATA_PATH		NULL

/*producer*/
#define INTERFACE5_TO_IPA_DATA_PATH			"/dev/to_ipa_2"
#define INTERFACE5_FROM_IPA_DATA_PATH		NULL

/*consumer*/
#define INTERFACE1_TO_IPA_DATA_PATH			NULL
#define INTERFACE1_FROM_IPA_DATA_PATH		"/dev/from_ipa_0"

/*consumer 2*/
#define INTERFACE2_TO_IPA_DATA_PATH			NULL
#define INTERFACE2_FROM_IPA_DATA_PATH		"/dev/from_ipa_1"

/*Default consumer*/
#define INTERFACE3_TO_IPA_DATA_PATH			NULL
#define INTERFACE3_FROM_IPA_DATA_PATH		"/dev/from_ipa_2"

/*Exceptions producer*/
#define INTERFACE_TO_IPA_EXCEPTION_PATH		NULL
#define INTERFACE_FROM_IPA_EXCEPTION_PATH	"/dev/ipa_exception_pipe"

/*The next configuration should be used by the ITAKEM as well.*/

/*----------------------------------------------------------------------
 *Configuration 1 (see configure_system_1 )
 *-----USB_PROD--->[IPA DMA]----USB_CONS--->
 *----------------------------------------------------------------------
 */
#define CONFIG_1_FROM_USB1_TO_IPA_DMA		"/dev/to_ipa_0"
#define CONFIG_1_FROM_IPA_TO_USB1_DMA		"/dev/from_ipa_0"

/*----------------------------------------------------------------------
 *Configuration 2 (see configure_system_2 )
 *                  [IPA]----USB2_CONS-->
 *-----USB_PROD--->[IPA]----USB3_CONS--->
 *                  [IPA]----Q6_LAN_CONS--->
 *---------------------------------------------------------------------
 */
#define CONFIG_2_FROM_USB_TO_IPA            "/dev/to_ipa_0"
#define CONFIG_2_FROM_IPA_TO_A2_NDUN        "/dev/from_ipa_0"
#define CONFIG_2_FROM_IPA_TO_A2_DUN         "/dev/from_ipa_1"
#define CONFIG_2_FROM_IPA_TO_Q6_LAN         "/dev/from_ipa_2"

/*USB1 is an EthernetII Client*/
#define FROM_USB1_TO_IPA				"/dev/to_ipa_0"
#define FROM_IPA_TO_USB1				"/dev/from_ipa_0"
#define USB1_CLIENT_HEADER_LENGTH		14

/*----------------------------------------------------------------------
 *Configuration 3 (see configure_system_3 )
 *                     [IPA]----USB_CONS------>
 *-----USB2_PROD--->[IPA]----USB2_CONS--->
 *                     [IPA]----Q6_LAN_CONS---->
 *----------------------------------------------------------------------
 */
#define CONFIG_3_FROM_A2_NDUN_TO_IPA            "/dev/to_ipa_0"
#define CONFIG_3_FROM_IPA_TO_USB1               "/dev/from_ipa_0"
#define CONFIG_3_FROM_IPA_TO_A2_NDUN            "/dev/from_ipa_1"
#define CONFIG_3_FROM_IPA_TO_Q6_LAN             "/dev/from_ipa_2"

/*----------------------------------------------------------------------
 *Configuration 8 (see configure_system_8 )
 *-----USB3_PROD--->[IPA DMA]----
 *-----USB_CONS (TLP aggregation byte limit)--->
 *-----USB_PROD (TLP deaggregation)--->[IPA DMA]----USB3_CONS--->
 *-----USB2_PROD (TLP deaggregation)--->
 * [IPA DMA]----USB_CONS (TLP aggregation byte limit)--->
 *-----USB4--->[IPA DMA]----USB2_CONS (TLP aggregation time limit)--->
 *----------------------------------------------------------------------
 */
#define CONFIG_8_NO_AGG_TO_IPA_AGG			"/dev/to_ipa_0"
#define CONFIG_8_DEAGG_TO_IPA_NO_AGG		"/dev/to_ipa_1"
#define CONFIG_8_DEAGG_TO_IPA_AGG			"/dev/to_ipa_2"
#define CONFIG_8_NO_AGG_TO_IPA_AGG_TIME     "/dev/to_ipa_3"
#define CONFIG_8_FROM_IPA_AGG				"/dev/from_ipa_0"
#define CONFIG_8_FROM_IPA_NO_AGG			"/dev/from_ipa_1"
#define CONFIG_8_DEAGG_FROM_IPA_AGG         "/dev/from_ipa_2"

/*----------------------------------------------------------------------
 *Configuration 9 (see configure_system_9 )
 *-----USB3_PROD--->[IPA DMA]----
 * USB_CONS (MBIM aggregation byte limit)--->
 *-----USB_PROD (MBIM deaggregation)--->[IPA DMA]----USB3_CONS--->
 *-----USB2_PROD (MBIM deaggregation)--->
 * [IPA DMA]----USB_CONS (MBIM aggregation byte limit)--->
 *-----USB4--->[IPA DMA]----
 * USB2_CONS (MBIM aggregation time limit)--->
 *----------------------------------------------------------------------
 */
#define CONFIG_9_NO_AGG_TO_IPA_AGG			"/dev/to_ipa_0"
#define CONFIG_9_DEAGG_TO_IPA_NO_AGG		"/dev/to_ipa_1"
#define CONFIG_9_DEAGG_TO_IPA_AGG			"/dev/to_ipa_2"
#define CONFIG_9_NO_AGG_TO_IPA_AGG_TIME     "/dev/to_ipa_3"
#define CONFIG_9_FROM_IPA_AGG				"/dev/from_ipa_0"
#define CONFIG_9_FROM_IPA_NO_AGG			"/dev/from_ipa_1"
#define CONFIG_9_DEAGG_FROM_IPA_AGG         "/dev/from_ipa_2"

/*----------------------------------------------------------------------
 *Configuration 10 (see configure_system_10 )
 *-----USB_PROD--->[IPA DMA]----
 *	USB_CONS (MBIM aggregation no limits)--->
 *----------------------------------------------------------------------
 */
#define CONFIG_10_TO_IPA_AGG_ZERO_LIMITS		"/dev/to_ipa_0"
#define CONFIG_10_FROM_IPA_AGG_ZERO_LIMITS		"/dev/from_ipa_0"

/*----------------------------------------------------------------------
 *Configuration 11 (see configure_system_11 )
 *-----USB_PROD----->[IPA]----
 * USB2_CONS (MBIM aggregation byte limit)--->
 * USB2_PROD (MBIM deaggregation)->[IPA]----USB3_CONS--->
 *------------------>[IPA]----
 * USB_CONS (MBIM aggregation time limit)----------->
 *------------------>[IPA
 * A5_LAN_WAN_CONS (MBIM aggregation no limits)---->
 *----------------------------------------------------------------------
 */
#define CONFIG_11_TO_IPA				"/dev/to_ipa_0"
#define CONFIG_11_TO_IPA_DEAGG			"/dev/to_ipa_1"
#define CONFIG_11_FROM_IPA_AGG			"/dev/from_ipa_0"
#define CONFIG_11_FROM_IPA				"/dev/from_ipa_1"
#define CONFIG_11_FROM_IPA_AGG_TIME		"/dev/from_ipa_2"
#define CONFIG_11_FROM_IPA_ZERO_LIMITS	"/dev/from_ipa_3"

/*----------------------------------------------------------------------
 *Configuration 12 (see configure_system_12 )
 *-----USB_PROD----->[IPA]----
 * USB2_CONS (MBIM aggregation byte limit)--->
 *-USB2_PROD (MBIM deaggregation)->[IPA]----USB3_CONS--->
 *------------------>[IPA]----
 * USB_CONS (MBIM aggregation time limit)----------->
 *------------------>[IPA]----
 * A5_LAN_WAN_CONS (MBIM aggregation no limits)---->
 *----------------------------------------------------------------------
 */
#define CONFIG_12_TO_IPA				"/dev/to_ipa_0"
#define CONFIG_12_TO_IPA_DEAGG			"/dev/to_ipa_1"
#define CONFIG_12_FROM_IPA_AGG			"/dev/from_ipa_0"
#define CONFIG_12_FROM_IPA				"/dev/from_ipa_1"
#define CONFIG_12_FROM_IPA_AGG_TIME		"/dev/from_ipa_2"
#define CONFIG_12_FROM_IPA_ZERO_LIMITS	"/dev/from_ipa_3"

/*Configuration 7 (see configure_system_7 )
 *                  [IPA]----USB2_CONS-->
 *-----USB_PROD--->[IPA]----USB3_CONS--->
 *                  [IPA]----Q6_LAN_CONS--->
 *                  [IPA]----A5 - Exception Pipe--->
 */
#define CONFIG_7_FROM_USB1_TO_IPA		"/dev/to_ipa_0"
#define CONFIG_7_FROM_IPA_TO_A5_EXCEPTION	"/dev/ipa_exception_pipe"

/*----------------------------------------------------------------------
 *Configuration 17 (see configure_system_17 )
 *-----USB_PROD----->[IPA]----
 * USB2_CONS (RNDIS aggregation byte limit)--->
 * USB2_PROD (RNDIS deaggregation)->[IPA]----USB3_CONS--->
 *------------------>[IPA]----
 * USB_CONS (RNDIS aggregation time limit)----------->
 *------------------>[IPA]----
 * A5_LAN_WAN_CONS (RNDIS aggregation no limits)---->
 *----------------------------------------------------------------------
 */
#define CONFIG_17_TO_IPA				"/dev/to_ipa_0"
#define CONFIG_17_TO_IPA_NO_HDR			"/dev/to_ipa_1"
#define CONFIG_17_TO_IPA_DEAGG			"/dev/to_ipa_2"
#define CONFIG_17_FROM_IPA_AGG			"/dev/from_ipa_0"
#define CONFIG_17_FROM_IPA				"/dev/from_ipa_1"
#define CONFIG_17_FROM_IPA_AGG_TIME		"/dev/from_ipa_2"
#define CONFIG_17_FROM_IPA_ZERO_LIMITS	"/dev/from_ipa_3"

/*----------------------------------------------------------------------
 *Configuration 18 (see configure_system_18 )---------------------------
 *-----USB_PROD----->[IPA]--------------->USB_CONS--------->A5----------
 *-----USB_PROD2 is a dummy endpoint handle for packet handling between-
 *-----user space and kernel space in the IPA driver--------------------
 *----------------------------------------------------------------------
 */
#define CONFIG_18_TO_IPA				"/dev/to_ipa_0"
#define CONFIG_18_DUMMY_ENDPOINT		"/dev/to_ipa_1"
#define CONFIG_18_FROM_IPA			"/dev/from_ipa_0"

/*----------------------------------------------------------------------
 *Configuration 19 (see configure_system_19 )
 *-----USB_PROD--->[IPA DMA]----USB_CONS--->
 *----------------------------------------------------------------------
 */
#define CONFIG_19_FROM_USB_TO_IPA_DMA		"/dev/to_ipa_0"
#define CONFIG_19_FROM_IPA_TO_USB_DMA		"/dev/from_ipa_0"

enum ipv6_ext_hdr_type {
	HOP_BY_HOP_OPT	= 0,
	DEST_OPT	= 60,
	ROUTING         = 43,
	FRAGMENT	= 44,
	AH		= 51,
	ESP		= 50,
	DEST_OPT_UL	= 60,
	Mobility	= 135,
	NONE		= 59
};
/*File that are being used by the test application:*/

#define IPV4_FILE_PATH				"Input/IPV4_3"

/*---------------------------------------------------------------------
 *XUnit tests results format file name
 *----------------------------------------------------------------------
 */
#define XUNIT_REPORT_PATH_AND_NAME	"junit_result.xml"

#endif /* CONSTANTS_H_ */
