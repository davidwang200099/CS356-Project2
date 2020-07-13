#include <linux/module.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
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

struct timer_list mytimer;

void buildup_dict(void){
    struct task_struct *newtask,*taskptr,*p;
    struct list_head *head,*head1;
    struct NMMLimits *my_mm_limits_entry;
    char flag=0;
    int rss_taken=0;
	int rss_max=0;
	int new_rss=0;
	
    list_for_each(head,&n_my_mm_limits){
		my_mm_limits_entry=list_entry(head,struct NMMLimits, my_mm_limits_list);
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
			my_mm_limits_entry->time_passed+=1;
		}
		rss_taken=0;
	}
}

void test_and_kill(void){
    struct list_head *head;
	struct task_struct *p=NULL,*q;
	struct NMMLimits *chosen_group;
	struct NMMLimits *my_mm_limits_entry;
    if(list_empty(&n_my_mm_limits)) return;
    read_lock(&tasklist_lock);
    buildup_dict();
    read_unlock(&tasklist_lock);
    list_for_each(head,&n_my_mm_limits){
        my_mm_limits_entry=list_entry(head, struct NMMLimits, my_mm_limits_list);
		if(p=my_mm_limits_entry->taskptr) {
			chosen_group=my_mm_limits_entry;
			my_mm_limits_entry->taskptr=NULL;
			break;
		}
	}
    if(p&&chosen_group->time_passed>=chosen_group->time_limit){
		printk("<0>" "name=%s\tuid=%d\tuRSS=%d\tmm_max=%d\tpid=%d\tpRSS=%d\n",p->comm,
		    chosen_group->uid,chosen_group->uRSS,chosen_group->mm_max,p->pid,p->mm?get_mm_rss(p->mm):0);
        send_sig(SIGKILL, p, true);
		chosen_group->uRSS=0;
		chosen_group->taskptr=NULL;
		chosen_group->time_passed=0;
	}
	
}

static void myfunc(void){
    test_and_kill();
    mod_timer(&mytimer, jiffies + HZ);
}

static int __init mytimer_init(void){
    setup_timer(&mytimer, myfunc, NULL);
    mytimer.expires = jiffies + HZ;
    add_timer(&mytimer);
    return 0;
}

static void __exit mytimer_exit(void){
    del_timer(&mytimer);
}



module_init(mytimer_init);

module_exit(mytimer_exit);
