/*
 *  linux/arch/arm/kernel/init_task.c
 */
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/init_task.h>
#include <linux/mqueue.h>
#include <linux/uaccess.h>
#include <linux/oom.h>
#include <asm/pgtable.h>
#include <linux/list.h>

static struct signal_struct init_signals = INIT_SIGNALS(init_signals);
static struct sighand_struct init_sighand = INIT_SIGHAND(init_sighand);
/*
 * Initial thread structure.
 *
 * We need to make sure that this is 8192-byte aligned due to the
 * way process stacks are handled. This is done by making sure
 * the linker maps this in the .text segment right after head.S,
 * and making head.S ensure the proper alignment.
 *
 * The things we do for performance..
 */
union thread_union init_thread_union __init_task_data =
	{ INIT_THREAD_INFO(init_task) };

/*
 * Initial task structure.
 *
 * All other task structs will be allocated on slabs in fork.c
 */
struct task_struct init_task = INIT_TASK(init_task);
//int my_mm_limit=0;

struct list_head my_mm_limits={
    .prev=&my_mm_limits,
    .next=&my_mm_limits
};

struct list_head n_my_mm_limits={
    .prev=&n_my_mm_limits,
    .next=&n_my_mm_limits
};

EXPORT_SYMBOL(my_mm_limits);
EXPORT_SYMBOL(n_my_mm_limits);
EXPORT_SYMBOL(init_task);
