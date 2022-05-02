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

#include "rmnet_module.h"

struct rmnet_module_hook_info {
	void *func __rcu;
};

static struct rmnet_module_hook_info
rmnet_module_hooks[__RMNET_MODULE_NUM_HOOKS];

void
rmnet_module_hook_register(const struct rmnet_module_hook_register_info *info,
			   int hook_count)
{
	struct rmnet_module_hook_info *hook_info;
	int i;

	for (i = 0; i < hook_count; i++) {
		int hook = info[i].hooknum;

		if (hook < __RMNET_MODULE_NUM_HOOKS) {
			hook_info = &rmnet_module_hooks[hook];
			rcu_assign_pointer(hook_info->func, info[i].func);
		}
	}
}
EXPORT_SYMBOL(rmnet_module_hook_register);

bool rmnet_module_hook_is_set(int hook)
{
	if (hook >= __RMNET_MODULE_NUM_HOOKS)
		return false;

	return rcu_dereference(rmnet_module_hooks[hook].func) != NULL;
}
EXPORT_SYMBOL(rmnet_module_hook_is_set);

void
rmnet_module_hook_unregister_no_sync(const struct rmnet_module_hook_register_info *info,
				     int hook_count)
{
	struct rmnet_module_hook_info *hook_info;
	int i;

	for (i = 0; i < hook_count; i++) {
		int hook = info[i].hooknum;

		if (hook < __RMNET_MODULE_NUM_HOOKS) {
			hook_info = &rmnet_module_hooks[hook];
			rcu_assign_pointer(hook_info->func, NULL);
		}
	}
}
EXPORT_SYMBOL(rmnet_module_hook_unregister_no_sync);

#define __RMNET_HOOK_DEFINE(call, hook_num, proto, args, ret_type) \
int rmnet_module_hook_##call( \
__RMNET_HOOK_PROTO(RMNET_HOOK_PARAMS(proto), ret_type) \
) \
{ \
	ret_type (*__func)(proto); \
	struct rmnet_module_hook_info *__info = \
		&rmnet_module_hooks[hook_num]; \
	int __ret = 0; \
\
	rcu_read_lock(); \
	__func = rcu_dereference(__info->func); \
	if (__func) { \
		RMNET_HOOK_IF_NON_VOID_TYPE(ret_type)( ret_type __rc = ) \
		__func(args); \
		__ret = 1; \
\
		RMNET_HOOK_IF_NON_VOID_TYPE(ret_type)( if (__ret_code) \
			*__ret_code = __rc; )\
	} \
\
	rcu_read_unlock(); \
	return __ret; \
} \
EXPORT_SYMBOL(rmnet_module_hook_##call);

#undef RMNET_MODULE_HOOK
#define RMNET_MODULE_HOOK(call, hook_num, proto, args, ret_type) \
__RMNET_HOOK_DEFINE(call, hook_num, RMNET_HOOK_PARAMS(proto), \
		    RMNET_HOOK_PARAMS(args), ret_type)

#define __RMNET_HOOK_MULTIREAD__
#include "rmnet_hook.h"

