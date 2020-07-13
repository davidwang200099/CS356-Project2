#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/unistd.h>
#include <linux/list.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/oom.h>
#include <linux/mm.h>

MODULE_LICENSE("Dual BSD/GPL");
#define __NR_hellocall 378

static int (*ooldcall)(void);

static void buildup_dict(void){
    struct task_struct *newtask,*taskptr,*p;
    struct list_head *head,*head1;
    struct MMLimits *my_mm_limits_entry;
    char flag=0;
    int rss_taken=0;
	int rss_max=0;
	int new_rss=0;
	
    list_for_each(head,&my_mm_limits){
		my_mm_limits_entry=list_entry(head,struct MMLimits, my_mm_limits_list);
        int uid=my_mm_limits_entry->uid;
		taskptr=NULL;

		if(uid<0) continue;
		for_each_process(newtask){
			if(newtask->cred->uid==uid) {
				new_rss=newtask->mm?get_mm_rss(newtask->mm):0;
				rss_taken+=new_rss;
                if(new_rss>rss_max) taskptr=newtask;
			}
		}
		my_mm_limits_entry->uRSS=rss_taken;
		if((rss_taken<<12)>my_mm_limits_entry->mm_max) {
		    my_mm_limits_entry->taskptr=taskptr;
		}
		rss_taken=0;
	}
}

static void test_and_kill(void){
    printk("<0>" "syscall 378!\n");
    struct list_head *head;
	struct task_struct *p=NULL,*q;
	struct MMLimits *chosen_group;
	struct MMLimits *my_mm_limits_entry;
    if(list_empty(&my_mm_limits)) return;
    read_lock(&tasklist_lock);
    buildup_dict();
    read_unlock(&tasklist_lock);
    list_for_each(head,&my_mm_limits){
        my_mm_limits_entry=list_entry(head, struct MMLimits, my_mm_limits_list);
		if(p=my_mm_limits_entry->taskptr) {
			chosen_group=my_mm_limits_entry;
			my_mm_limits_entry->taskptr=NULL;
			break;
		}
	}
    if(p){
		printk("<0>" "name=%s\tuid=%d\tuRSS=%d\tmm_max=%d\tpid=%d\tpRSS=%d\n",p->comm,
		    chosen_group->uid,chosen_group->uRSS,chosen_group->mm_max,p->pid,p->mm?get_mm_rss(p->mm):0);
        send_sig(SIGKILL, p, true);
		chosen_group->uRSS=0;
		chosen_group->taskptr=NULL;
	}
	
}

static int addsyscall_init(void){
	long *syscall = (long*)0xc000d8c4;
	ooldcall=(int(*)(void))(syscall[__NR_hellocall]);
	syscall[__NR_hellocall]=(unsigned long)test_and_kill;
	printk("<0>" "module load!\n");
    printk("<0>" "Module location:%x\n",test_and_kill);
	return 0;
}

static void addsyscall_exit(void){
	long *syscall=(long*)0xc000d8c4;
	syscall[__NR_hellocall]=(unsigned long)ooldcall;
	printk("<0>" "module exit!\n");
}

module_init(addsyscall_init);
module_exit(addsyscall_exit);








