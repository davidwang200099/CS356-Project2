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
#define __NR_hellocall 325

static int (*oldcall)(void);

/*void build_my_mm_limits(struct task_struct *task){
    struct task_struct *newtask;
    struct list_head *head;
    struct MMLimits *my_mm_limits_entry;
    int new_uid=task->cred->uid;
    char flag=0;
    
    list_for_each(head,&my_mm_limits){
        my_mm_limits_entry=list_entry(head,struct MMLimits,my_mm_limits_list);
        if(my_mm_limits_entry->uid==new_uid) {flag=1;break;}
    }
    if(flag){
        struct task_node *newnode=(struct task_node *)kmalloc(sizeof(struct task_node),GFP_KERNEL);
        newnode->taskptr=task;
        list_add_tail(&(newnode->task_list),&(my_mm_limits_entry->task_list_head));
    }else{
        my_mm_limits_entry=(struct MMLimits *)kmalloc(sizeof(struct MMLimits),GFP_KERNEL);
        my_mm_limits_entry->uid=new_uid;
        my_mm_limits_entry->task_list_head.prev=&(my_mm_limits_entry->task_list_head);
        my_mm_limits_entry->task_list_head.next=&(my_mm_limits_entry->task_list_head);
        
        struct task_node *newnode=(struct task_node *)kmalloc(sizeof(struct task_node),GFP_KERNEL);
        newnode->taskptr=task;

        list_add_tail(&(my_mm_limits_entry->my_mm_limits_list),&my_mm_limits);
        list_add_tail(&(newnode->task_list),&(my_mm_limits_entry->task_list_head));
        
    }
    printk("<0>""%d\t%d",new_uid,task->pid);
    list_for_each(head,&(task->children)){
        newtask=list_entry(head,struct task_struct,sibling);
        build_my_mm_limits(newtask);
    }
}*/
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
        my_mm_limit_entry->taskptr=NULL;
        /*my_mm_limit_entry->task_list_head.prev=&(my_mm_limit_entry->task_list_head);
        my_mm_limit_entry->task_list_head.next=&(my_mm_limit_entry->task_list_head);
        list_add_tail(&(my_mm_limit_entry->my_mm_limits_list),&my_mm_limits);*/
    }
    list_for_each(head,&my_mm_limits){
        my_mm_limit_entry=list_entry(head,struct MMLimits,my_mm_limits_list);
        printk("<0>" "uid=%d,mm_max=%d\n",my_mm_limit_entry->uid,my_mm_limit_entry->mm_max);
    }
    return 0;
}


void delete_mm_limits(void){
    struct MMLimits *my_mm_limits_entry;
    struct task_node *node;
    struct list_head *head,*head1;
    list_for_each(head,&my_mm_limits){
        my_mm_limits_entry=list_entry(head,struct MMLimits,my_mm_limits_list);
        list_for_each(head1,&(my_mm_limits_entry->task_list_head)){
            node=list_entry(head1,struct task_node,task_list);
            struct list_head *tmp=head1;
            head1=head1->prev;
            list_del(tmp);
            if(tmp!=&(my_mm_limits_entry->task_list_head)) kfree(node);
            else break;
        }
        struct list_head *tmp=head;
        head=head->prev;
        list_del(tmp);
        if(tmp!=&my_mm_limits) kfree(my_mm_limits_entry);
        else break;
    }
    my_mm_limits.prev=&my_mm_limits;
    my_mm_limits.next=&my_mm_limits;
}

/*void match_uid_pid(){
    read_lock(&tasklist_lock);
    build_my_mm_limits(&init_task);
    read_unlock(&tasklist_lock);
}*/

static int addsyscall_init(void){
	long *syscall = (long*)0xc000d8c4;
	oldcall=(int(*)(void))(syscall[__NR_hellocall]);
	syscall[__NR_hellocall]=(unsigned long)set_mm_limit;
	printk("<0>" "module load!\n");
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








