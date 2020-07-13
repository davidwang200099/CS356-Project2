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

MODULE_LICENSE("Dual BSD/GPL");
#define __NR_hellocall 383

static int (*oldcall)(void);

int set_mm_limit(uid_t uid,unsigned long mm_max){
    struct list_head *head;
    struct MMLimits *my_mm_limit_entry;
    char flag=0;
    list_for_each(head,&(my_mm_limits)){
        my_mm_limit_entry=list_entry(head,struct MMLimits,my_mm_limits_list);
        if(my_mm_limit_entry->uid==uid) {flag=1;break;}
    }
    if(flag){
        my_mm_limit_entry->mm_max=mm_max;
    }else{
        my_mm_limit_entry=(struct MMLimits *)kmalloc(sizeof(struct MMLimits),GFP_KERNEL);
        my_mm_limit_entry->uid=uid;
        my_mm_limit_entry->mm_max=mm_max;
        my_mm_limit_entry->uRSS=0;
        my_mm_limit_entry->taskptr=NULL;
        
        list_add_tail(&(my_mm_limit_entry->my_mm_limits_list),&my_mm_limits);
    }
    if(my_mm_limits.next==&my_mm_limits) printk("<0>" "Empty!\n");
    if(!list_empty(&my_mm_limits))
    list_for_each(head,&my_mm_limits){
        my_mm_limit_entry=list_entry(head,struct MMLimits,my_mm_limits_list);
        printk("<0>" "syscall:uid=%d,mm_max=%lu\n",my_mm_limit_entry->uid,my_mm_limit_entry->mm_max);
    }
    return 0;
}


void delete_mm_limits(void){
    struct MMLimits *my_mm_limits_entry;
    struct list_head *head,*head1;
    list_for_each(head,&my_mm_limits){
        my_mm_limits_entry=list_entry(head,struct MMLimits,my_mm_limits_list);
        struct list_head *tmp=head;
        head=head->prev;
        list_del(tmp);
        kfree(my_mm_limits_entry);
    }
    my_mm_limits.prev=&my_mm_limits;
    my_mm_limits.next=&my_mm_limits;
}

static int addsyscall_init(void){
	long *syscall = (long*)0xc000d8c4;
	oldcall=(int(*)(void))(syscall[__NR_hellocall]);
	syscall[__NR_hellocall]=(unsigned long)set_mm_limit;
	printk("<0>" "module load!\n");
    struct list_head *p=&my_mm_limits;
    my_mm_limits.prev=p;
    my_mm_limits.next=p;
    printk("<0>" "Module location:%x\n",set_mm_limit);
	return 0;
}

static void addsyscall_exit(void){
	long *syscall=(long*)0xc000d8c4;
	syscall[__NR_hellocall]=(unsigned long)oldcall;
	printk("<0>" "module exit!\n");
    delete_mm_limits();
}

module_init(addsyscall_init);
module_exit(addsyscall_exit);








