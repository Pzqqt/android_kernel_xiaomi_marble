/* Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef __RMNET_MODULE_H__
#define __RMNET_MODULE_H__

#include <linux/rcupdate.h>

enum {
	RMNET_MODULE_HOOK_OFFLOAD_INGRESS,
	RMNET_MODULE_HOOK_OFFLOAD_CHAIN_END,
	RMNET_MODULE_HOOK_SHS_SKB_ENTRY,
	RMNET_MODULE_HOOK_SHS_SWITCH,
	RMNET_MODULE_HOOK_PERF_TETHER_INGRESS,
	RMNET_MODULE_HOOK_PERF_TETHER_EGRESS,
	RMNET_MODULE_HOOK_PERF_TETHER_CMD,
	RMNET_MODULE_HOOK_PERF_INGRESS,
	RMNET_MODULE_HOOK_PERF_EGRESS,
	RMNET_MODULE_HOOK_APS_PRE_QUEUE,
	RMNET_MODULE_HOOK_APS_POST_QUEUE,
	RMNET_MODULE_HOOK_WLAN_FLOW_MATCH,
	__RMNET_MODULE_NUM_HOOKS,
};

struct rmnet_module_hook_register_info {
	int hooknum;
	void *func;
};

void
rmnet_module_hook_register(const struct rmnet_module_hook_register_info *info,
			   int hook_count);
bool rmnet_module_hook_is_set(int hook);
void
rmnet_module_hook_unregister_no_sync(const struct rmnet_module_hook_register_info *info,
				     int hook_count);
static inline void
rmnet_module_hook_unregister(const struct rmnet_module_hook_register_info *info,
			     int hook_count)
{
	rmnet_module_hook_unregister_no_sync(info, hook_count);
	synchronize_rcu();
}

/* Dummy macro. Can use kernel version later */
#define __CAT(a, b) a ## b
#define CAT(a, b) __CAT(a, b)

#define RMNET_HOOK_PARAMS(args...) args

#define RMNET_MODULE_HOOK_NUM(__hook) CAT(RMNET_MODULE_HOOK_, __hook)
#define RMNET_MODULE_HOOK_PROTOCOL(proto...) proto
#define RMNET_MODULE_HOOK_ARGS(args...) args
#define RMNET_MODULE_HOOK_RETURN_TYPE(type) type

/* A ...lovely... framework for checking if the argument passed in to a function
 * macro is a pair of parentheses.
 * If so, resolve to 1. Otherwise, 0.
 *
 * The idea here is that you pass the argument along with a "test" macro to
 * a "checker" macro. If the argument IS a pair of parentheses, this will cause
 * the tester macro to expand into multiple arguments.
 *
 * The key is that "checker" macro just returns the second argument it receives.
 * So have the "tester" macro expand to a set of arguments that makes 1 the
 * second argument, or 0 if it doesn't expand.
 */
#define __RMNET_HOOK_SECOND_ARG(_, arg, ...) arg
#define RMNET_HOOK_PARENTHESES_CHECKER(args...) \
	__RMNET_HOOK_SECOND_ARG(args, 0, )
#define __RMNET_HOOK_PARENTHESES_TEST(arg) arg, 1,
#define __RMNET_HOOK_IS_PARENTHESES_TEST(...) __RMNET_HOOK_PARENTHESES_TEST(XXX)
#define RMNET_HOOK_IS_PARENTHESES(arg) \
	RMNET_HOOK_PARENTHESES_CHECKER(__RMNET_HOOK_IS_PARENTHESES_TEST arg)

/* So what's the point of the above stuff, you ask?
 *
 * CPP can't actually do type checking ;). But what we CAN do is something
 * like this to determine if the type passed in is void. If so, this will
 * expand to (), causing the RMNET_HOOK_IS_PARENTHESES check to resolve to 1,
 * but if not, then the check resolves to 0.
 */
#define __RMNET_HOOK_CHECK_TYPE_IS_void(arg) arg
#define RMNET_HOOK_TYPE_IS_VOID(type) \
	RMNET_HOOK_IS_PARENTHESES( __RMNET_HOOK_CHECK_TYPE_IS_ ## type (()) )

/* And now, we have some logic macros. The main macro will resolve
 * to one of the branches depending on the bool value passed in.
 */
#define __IF_0(t_path, e_path...) e_path
#define __IF_1(t_path, e_path...) t_path
#define IF(arg) CAT(__IF_, arg)
#define __NOT_1 0
#define __NOT_0 1
#define NOT(arg) CAT(__NOT_, arg)

/* And now we combine this all, for a purely function macro way of splitting
 * return type handling...
 *
 * ...all to circumvent that you can't actually add #if conditionals in macro
 * expansions. It would have been much simpler that way. ;)
 */
#define RMNET_HOOK_IF_NON_VOID_TYPE(type) \
	IF(NOT(RMNET_HOOK_TYPE_IS_VOID(type)))

#define __RMNET_HOOK_PROTO(proto, ret_type)\
RMNET_HOOK_IF_NON_VOID_TYPE(ret_type) \
	(RMNET_HOOK_PARAMS(ret_type *__ret_code, proto), \
	 RMNET_HOOK_PARAMS(proto))

#define __RMNET_HOOK_DECLARE(call, proto, ret_type) \
int rmnet_module_hook_##call( \
__RMNET_HOOK_PROTO(RMNET_HOOK_PARAMS(proto), ret_type));

#undef RMNET_MODULE_HOOK
#define RMNET_MODULE_HOOK(call, hook_num, proto, args, ret_type) \
__RMNET_HOOK_DECLARE(call, RMNET_HOOK_PARAMS(proto), ret_type)

#include "rmnet_hook.h"

#endif
