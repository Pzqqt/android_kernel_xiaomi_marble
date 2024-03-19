#include <linux/swap.h>
#include <linux/module.h>
#include <trace/hooks/binder.h>
#include <uapi/linux/android/binder.h>
#include <uapi/linux/sched/types.h>
#include <linux/sched/prio.h>
#include "../../android/binder_internal.h"
#include "../../../kernel/sched/sched.h"
#include <linux/string.h>

static const char *task_name[] = {
	"com.miui.home",
	".globallauncher",  // com.mi.android.globallauncher
	"droid.launcher3",  // com.android.launcher3
	"ndroid.systemui",  // com.android.systemui
	// "surfaceflinger",
	"com.tencent.mm",
};

static int to_userspace_prio(int policy, int kernel_priority) {
	if (fair_policy(policy))
		return PRIO_TO_NICE(kernel_priority);
	else
		return MAX_USER_RT_PRIO - 1 - kernel_priority;
}

static bool set_binder_rt_task(struct binder_transaction *t) {
	int i;

	if (t && t->from && t->from->task && t->to_proc && t->to_proc->tsk && (!(t->flags & TF_ONE_WAY)) &&
	    rt_policy(t->from->task->policy) && (t->from->task->pid == t->from->task->tgid)) {
		for (i = 0; i < ARRAY_SIZE(task_name); i++) {
			if (strncmp(t->from->task->comm, task_name[i], strlen(task_name[i])) == 0) {
				return true;
			}
		}
		if (!strncmp(t->from->task->group_leader->comm, "surfaceflinger", strlen("surfaceflinger")) &&
		    !strncmp(t->from->task->comm, "passBlur", strlen("passBlur")))
			return true;
	}
	return false;
}

static void extend_surfacefinger_binder_set_priority_handler(void *data, struct binder_transaction *t, struct task_struct *task) {
	struct sched_param params;
	struct binder_priority desired;
	unsigned int policy;
	struct binder_node *target_node = t->buffer->target_node;

	desired.prio = target_node->min_priority;
	desired.sched_policy = target_node->sched_policy;
	policy = desired.sched_policy;
	if (set_binder_rt_task(t)) {
		desired.sched_policy = SCHED_FIFO;
		desired.prio = 98;
		policy = desired.sched_policy;
	}
	if (rt_policy(policy) && task->policy != policy) {
		params.sched_priority = to_userspace_prio(policy, desired.prio);
		sched_setscheduler_nocheck(task, policy | SCHED_RESET_ON_FORK, &params);
	}
}

static void extend_surfacefinger_binder_trans_handler(void *data, struct binder_proc *target_proc,
    struct binder_proc *proc,struct binder_thread *thread, struct binder_transaction_data *tr) {
	if (target_proc && target_proc->tsk && strncmp(target_proc->tsk->comm, "surfaceflinger",
		strlen("surfaceflinger")) == 0) {
		if (thread && proc && tr && thread->transaction_stack
			&& (!(thread->transaction_stack->flags & TF_ONE_WAY))) {
			target_proc->default_priority.sched_policy = SCHED_FIFO;
			target_proc->default_priority.prio = 98;
		}
	}
}

static void extend_skip_binder_thread_priority_from_rt_to_normal_handler(void *data, struct task_struct *task, bool *skip) {
	if (task && rt_policy(task->policy)) {
		*skip = true;
	}
}

int __init binder_prio_init(void)
{
    pr_info("binder_prio: module init!");
    register_trace_android_vh_binder_set_priority(extend_surfacefinger_binder_set_priority_handler, NULL);
    register_trace_android_vh_binder_trans(extend_surfacefinger_binder_trans_handler, NULL);
    register_trace_android_vh_binder_priority_skip(extend_skip_binder_thread_priority_from_rt_to_normal_handler, NULL);

    return 0;
}

void __exit binder_prio_exit(void)
{
    unregister_trace_android_vh_binder_set_priority(extend_surfacefinger_binder_set_priority_handler, NULL);
    unregister_trace_android_vh_binder_trans(extend_surfacefinger_binder_trans_handler, NULL);
    unregister_trace_android_vh_binder_priority_skip(extend_skip_binder_thread_priority_from_rt_to_normal_handler, NULL);

    pr_info("binder_prio: module exit!");
}

module_init(binder_prio_init);
module_exit(binder_prio_exit);
MODULE_LICENSE("GPL");
