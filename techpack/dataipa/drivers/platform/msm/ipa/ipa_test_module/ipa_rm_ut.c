// SPDX-License-Identifier: GPL-2.0-only
/*
* Copyright (c) 2017, The Linux Foundation. All rights reserved.
*/

#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/ipa.h>
#include <linux/msm_ipa.h>
#include <linux/kernel.h>
#include "ipa_rm_ut.h"

#define IPA_UT_DBG(x...) pr_err(x)

/**
 * enum ut_wq_cmd - workqueue commands
 */
enum ut_wq_cmd {
	UT_WQ_REQ,
	UT_WQ_REL
};

/**
 * struct ipa_rm_ut_wq_work_type - IPA RM worqueue specific
 *				work type
 * @work: work struct
 * @wq_cmd: command that should be processed in workqueue context
 * @resource_name: name of the resource on which this work
 *			should be done
 * @dep_graph: data structure to search for resource if exists
 * @event: event to notify
 */
struct ipa_rm_ut_wq_work_type {
	struct work_struct		work;
	enum ut_wq_cmd			wq_cmd;
	enum ipa_rm_resource_name	resource_name;
	enum ipa_rm_event		event;
};



static struct {
	int (*add_dependency)(enum ipa_rm_resource_name
					dependant_name,
					enum ipa_rm_resource_name
					dependency_name);
	int (*resource_request)(enum ipa_rm_resource_name resource_name);
	int (*resource_release)(enum ipa_rm_resource_name resource_name);
	int (*consumer_cb)(enum ipa_rm_event,
			enum ipa_rm_resource_name resource_name);
	struct workqueue_struct *wq;
} ipa_rm_ut_cb;

struct device_manager_type {
	void *user_data;
	ipa_rm_notify_cb notify_cb;
	int (*release_function)(void);
	int (*request_function)(void);
};

static void ipa_ut_wq_handler(struct work_struct *work);
int ipa_ut_wq_send_cmd(enum ut_wq_cmd wq_cmd,
		enum ipa_rm_resource_name resource_name,
		enum ipa_rm_event event);
static int usb_mgr_release_function(void);
static int usb_mgr_request_function(void);
static void usb_mgr_notify_function(void *user_data,
		enum ipa_rm_event event,
		unsigned long data);

static struct device_manager_type usb_device_manager = {
	NULL,
	usb_mgr_notify_function,
	usb_mgr_release_function,
	usb_mgr_request_function
};

/* USB device manager */
static int usb_mgr_release_function(void)
{
	IPA_UT_DBG("USB Released\n");
	IPA_UT_DBG("ASYNC CALL USB calling to IPA RM provided CB\n");
	ipa_ut_wq_send_cmd(UT_WQ_REL,
			IPA_RM_RESOURCE_USB_CONS,
			IPA_RM_RESOURCE_RELEASED);

	return -EINPROGRESS;
}

static int usb_mgr_request_function(void)
{
	IPA_UT_DBG("USB Requested\n");
	IPA_UT_DBG("ASYNC CALL USB calling to IPA RM provided CB\n");
	ipa_ut_wq_send_cmd(UT_WQ_REQ,
			IPA_RM_RESOURCE_USB_CONS,
			IPA_RM_RESOURCE_GRANTED);

	return -EINPROGRESS;
}

static void usb_mgr_notify_function(void *notify_cb_data,
		enum ipa_rm_event event,
		unsigned long data)
{
	IPA_UT_DBG("USB got event [%d]\n", event);
}

/* HSIC device manager */
static int hsic_mgr_release_function(void)
{
	int result = 0;
	IPA_UT_DBG("HSIC Released\n");
	IPA_UT_DBG("HSIC calling to IPA RM provided CB\n");
	result = ipa_rm_ut_cb.consumer_cb(IPA_RM_RESOURCE_RELEASED,
			IPA_RM_RESOURCE_HSIC_CONS);

	return -EINPROGRESS;
}

static int hsic_mgr_request_function(void)
{
	int result = 0;
	IPA_UT_DBG("HSIC Requested\n");
	IPA_UT_DBG("HSIC calling to IPA RM provided CB\n");
	result = ipa_rm_ut_cb.consumer_cb(IPA_RM_RESOURCE_GRANTED,
			IPA_RM_RESOURCE_HSIC_CONS);

	return -EINPROGRESS;
}

static void hsic_notify_function(void *notify_cb_data,
		enum ipa_rm_event event,
		unsigned long data)
{
	IPA_UT_DBG("HSIC got event [%d]\n", event);
}

static struct device_manager_type hsic_device_manager = {
	NULL,
	hsic_notify_function,
	hsic_mgr_release_function,
	hsic_mgr_request_function
};

static void rmnet_bridge_mgr_notify_function
		(void *notify_cb_data,
		enum ipa_rm_event event,
		unsigned long data)
{
	IPA_UT_DBG("RmNet got event [%d]\n", event);
}

static struct device_manager_type rmnet_bridge_device_manager = {
	NULL,
	rmnet_bridge_mgr_notify_function,
	NULL,
	NULL
};


static void ipa_ut_wq_handler(struct work_struct *work)
{
	enum ut_wq_cmd ut_cmd;
	struct ipa_rm_ut_wq_work_type *ipa_rm_work =
		(struct ipa_rm_ut_wq_work_type *)work;
	if (!ipa_rm_work)
		return;
	ut_cmd = (enum ut_wq_cmd)ipa_rm_work->wq_cmd;
	IPA_UT_DBG("***UT CMD Q command [%d]\n", ut_cmd);
	switch (ut_cmd) {
	case UT_WQ_REQ:
		switch (ipa_rm_work->resource_name) {
		case IPA_RM_RESOURCE_USB_CONS:
			IPA_UT_DBG
			("***calling to USB consumer notify request CB\n");
			ipa_rm_ut_cb.consumer_cb(IPA_RM_RESOURCE_GRANTED,
					IPA_RM_RESOURCE_USB_CONS);
			break;
		case IPA_RM_RESOURCE_HSIC_CONS:
			break;
		default:
			return;
		}
		break;
	case UT_WQ_REL:
		switch (ipa_rm_work->resource_name) {
		case IPA_RM_RESOURCE_USB_CONS:
			IPA_UT_DBG
			("***calling to USB consumer notify release CB\n");
			ipa_rm_ut_cb.consumer_cb(IPA_RM_RESOURCE_RELEASED,
					IPA_RM_RESOURCE_USB_CONS);
			break;
		case IPA_RM_RESOURCE_HSIC_CONS:
			break;
		default:
			return;
		}
		break;
	default:
		break;
	}

	kfree((void *) work);
}

int ipa_ut_wq_send_cmd(enum ut_wq_cmd wq_cmd,
		enum ipa_rm_resource_name resource_name,
		enum ipa_rm_event event)
{
	int result = 0;
	struct ipa_rm_ut_wq_work_type *work =
	(struct ipa_rm_ut_wq_work_type *)
		kzalloc(sizeof(*work), GFP_KERNEL);
	if (work) {
		INIT_WORK((struct work_struct *)work, ipa_ut_wq_handler);
		work->wq_cmd = (enum ut_wq_cmd) wq_cmd;
		work->resource_name = resource_name;
		work->event = event;
		result = queue_work(ipa_rm_ut_cb.wq,
				(struct work_struct *)work);
	} else {
		result = -ENOMEM;
	}
	return result;
}

/**
 * build_rmnet_bridge_use_case_graph() - simulate resource creation
 *
 * @create_resource: create resource function provided by ipa_rm
 * unit under test
 * @consumer_cb: consumer CB function provided by ipa_rm
 * unit under test
 *
 * Returns: 0 on success, negative on failure
 */
int build_rmnet_bridge_use_case_graph(
		int (*create_resource)
			(struct ipa_rm_create_params *create_params),
		int (*consumer_cb)(enum ipa_rm_event event,
				enum ipa_rm_resource_name resource_name))
{
	int result = 0;
	struct ipa_rm_create_params create_params = {0};

	IPA_UT_DBG("build_rmnet_bridge_use_case_graph ENTER\n");

	ipa_rm_ut_cb.consumer_cb = consumer_cb;

	/* create USB PROD */
	create_params.name = IPA_RM_RESOURCE_USB_PROD;
	create_params.reg_params.notify_cb =
			usb_device_manager.notify_cb;
	create_params.reg_params.user_data =
			usb_device_manager.user_data;
	result = create_resource(&create_params);
	if (result)
		goto bail;

	/* create USB CONS */
	create_params.name = IPA_RM_RESOURCE_USB_CONS;
	create_params.release_resource =
			usb_device_manager.release_function;
	create_params.request_resource =
			usb_device_manager.request_function;
	result = create_resource(&create_params);
	if (result)
		goto bail;

	/* create HSIC PROD */
	create_params.name = IPA_RM_RESOURCE_HSIC_PROD;
	create_params.reg_params.notify_cb =
			hsic_device_manager.notify_cb;
	create_params.reg_params.user_data =
			hsic_device_manager.user_data;
	result = create_resource(&create_params);
	if (result)
		goto bail;

	/* create HSIC CONS */
	create_params.name = IPA_RM_RESOURCE_HSIC_CONS;
	create_params.release_resource =
			hsic_device_manager.release_function;
	create_params.request_resource =
			hsic_device_manager.request_function;
	result = create_resource(&create_params);
	if (result)
		goto bail;

	/* BRIDGE PROD */
	create_params.name = IPA_RM_RESOURCE_WWAN_0_PROD;
	create_params.reg_params.notify_cb =
			rmnet_bridge_device_manager.notify_cb;
	create_params.reg_params.user_data =
			rmnet_bridge_device_manager.user_data;
	result = create_resource(&create_params);
	if (result)
		goto bail;

	ipa_rm_ut_cb.wq = create_singlethread_workqueue("ut_wq");
	if (!ipa_rm_ut_cb.wq) {
		result = -ENOMEM;
		goto bail;
	}

	IPA_UT_DBG("build_rmnet_bridge_use_case_graph EXIT SUCCESS\n");

bail:

	return result;
}

/**
 * build_rmnet_bridge_use_case_dependencies() - simulate build
 *			dependency graph process
 * @add_dependency: add dependency function provided by ipa_rm
 * unit under test
 *
 * Returns: 0 on success, negative on failure
 */
int build_rmnet_bridge_use_case_dependencies(
		int (*add_dependency)
			(enum ipa_rm_resource_name dependant_name,
		enum ipa_rm_resource_name dependency_name))
{
	int result = 0;

	IPA_UT_DBG("build_rmnet_bridge_use_case_dependencies ENTER\n");

	ipa_rm_ut_cb.add_dependency = add_dependency;

	result = add_dependency(IPA_RM_RESOURCE_USB_PROD,
			IPA_RM_RESOURCE_HSIC_CONS);
	if (result)
		goto bail;
	result = add_dependency(IPA_RM_RESOURCE_HSIC_PROD,
			IPA_RM_RESOURCE_USB_CONS);
	if (result)
		goto bail;
	result = add_dependency(IPA_RM_RESOURCE_WWAN_0_PROD,
			IPA_RM_RESOURCE_HSIC_CONS);
	if (result)
		goto bail;
	result = add_dependency(IPA_RM_RESOURCE_WWAN_0_PROD,
			IPA_RM_RESOURCE_USB_CONS);

	if (result)
		goto bail;

bail:
	IPA_UT_DBG(
		"build_rmnet_bridge_use_case_dependencies EXIT result [%d]\n",
		result);
	return result;
}

/**
 * request_release_resource_sequence() - simulate request / release
 *			resource sequence
 * @resource_request: request resource function provided by ipa_rm
 * unit under test
 * @resource_release: release resource function provided by ipa_rm
 * unit under test
 *
 * Returns: 0 on success, negative on failure
 */
int request_release_resource_sequence(
		int (*resource_request)
			(enum ipa_rm_resource_name resource_name),
		int (*resource_release)
			(enum ipa_rm_resource_name resource_name))
{
	int result = 0;
	ipa_rm_ut_cb.resource_request = resource_request;
	ipa_rm_ut_cb.resource_release = resource_release;

	IPA_UT_DBG("request_release_resource_sequence ENTER\n");

	result = resource_request(IPA_RM_RESOURCE_USB_PROD);
	IPA_UT_DBG("result [%d]\n", result);
	result = resource_request(IPA_RM_RESOURCE_HSIC_PROD);
	IPA_UT_DBG("result [%d]\n", result);

	result = resource_release(IPA_RM_RESOURCE_USB_PROD);
	IPA_UT_DBG("result [%d]\n", result);
	result = resource_release(IPA_RM_RESOURCE_HSIC_PROD);
	IPA_UT_DBG("result [%d]\n", result);

	IPA_UT_DBG("request_release_resource_sequence EXIT SUCCESS\n");
	return result;
}

/**
 * clean_ut() - free unit test module resources
 *
 */
void clean_ut(void)
{
	IPA_UT_DBG("clean_ut ENTER\n");
	if (ipa_rm_ut_cb.wq)
		destroy_workqueue(ipa_rm_ut_cb.wq);
	IPA_UT_DBG("clean_ut EXIT SUCCESS\n");
}
