#include <assert.h>
#include <mmu.h>
#include <x86.h>
#include <string.h>
#include <x86.h>
#include <kern/syscall.h>
#include <kern/wait.h>
#include <kern/process.h>
#include <kern/fork.h>
#include <kern/trap.h>
#include <kern/kmalloc.h>
#include <kern/pmap.h>
#include <kern/sche.h>
#include <kern/stdio.h>
#include <errno.h>
ssize_t
kern_wait(int *wstatus)
{
	// 相比于fork来说，wait的实现简单很多
	// 语义实现比较清晰，没有fork那么多难点要处理，所以这里并不会给大家太多引导
	// 需要大家自己思考wait怎么实现。

	// 在实现之前你必须得读一遍文档`man 2 wait`
	// 了解到wait大概要做什么
	//panic("Unimplement! Read The F**king Manual");

	// 当然读文档是一方面，最重要的还是代码实现
	// wait系统调用与exit系统调用关系密切，所以在实现wait之前需要先读一遍exit为好
	// 可能读完exit的代码你可能知道wait该具体做什么了
	//panic("Unimplement! Read The F**king Source Code");
	// for(int i=0;i<20;i++)
	// {
	// 	kprintf("%d  %d\n",i,(proc_table+i)->pcb.lock);
	// }
	//xchg(&p_proc_ready->pcb.lock, 0);
	while (xchg(&p_proc_ready->pcb.lock, 1) == 1)
	{
		// kprintf("%d\n",p_proc_ready->pcb.pid);
		schedule();
	}
	//PROCESS_0 *p_proc = &p_proc_ready->pcb;
	//kprintf("333\n");
	// while (xchg(&p_proc->lock, 1) == 1)
	// {
	// 	//kprintf("444\n");
	// 	schedule();
	// }
	// if(p_proc_ready->pcb.fork_tree.sons==NULL)
	// {
	// 	xchg(&p_proc_ready->pcb.lock, 0);
	// 	return -ECHILD;
	// }
	// xchg(&p_proc_ready->pcb.lock, 0);
	//p_proc->statu=SLEEP;
	PROCESS_0 *p_proc = &p_proc_ready->pcb;
	while(1)
	{
		// while (xchg(&p_proc->lock, 1) == 1)
	   	// {
	 	// 	schedule();
	  	// }
		while(p_proc->statu==SLEEP)
		{
			schedule();
		}
		if(p_proc_ready->pcb.fork_tree.sons==NULL)
		{
			xchg(&p_proc_ready->pcb.lock, 0);
			return -ECHILD;
		}
		for (struct son_node *p = p_proc->fork_tree.sons ; p ;) 
		{
			PROCESS_0 *p_son = p->p_son;
			// for(int i=0;i<20;i++)
			// 	kprintf("pid%d:%d\n",(proc_table+i)->pcb.pid,(proc_table+i)->pcb.lock);
			while (xchg(&p_son->lock, 1) == 1)
				schedule();
			//kprintf("--%d\n",p_son->pid);
			struct son_node *p_nxt = p->nxt;
			// 上子进程的锁，因为需要修改子进程的父进程信息（移到初始进程下）
			// while (xchg(&p_son->lock, 1) == 1)
			// 	schedule();
			if(p_son->statu==ZOMBIE)
			{
				//kprintf("111\n");
				DISABLE_INT();
				if(wstatus!=NULL)
					*wstatus=p_son->exit_code;
				if(p->nxt==NULL&&p->pre==NULL)
					p_proc->fork_tree.sons=NULL;
				else
				{
					if (p->nxt != NULL)
					{
						p->nxt->pre = p->pre;
						//p->nxt=NULL;
					}
					// else
					// 	p->pre->nxt=NULL;
					if (p->pre != NULL)
					{
						p->pre->nxt = p->nxt;
						//p->pre=NULL;
					}
					else
						p_proc->fork_tree.sons=p->nxt;
				}
				//memset(&p_proc->user_regs, 0, sizeof(p_proc->user_regs));
				//memset(&p_proc->kern_regs, 0, sizeof(p_proc->kern_regs));
				p_son->fork_tree.p_fa=NULL;
				// phyaddr_t r_cr3=rcr3();
				// lcr3(p_son->cr3);
				recycle_pages(p_son->page_list);
				// lcr3(r_cr3);
				//p_son->page_list=NULL;
				kfree(p);
				p_son->statu=IDLE;
				ENABLE_INT();
				xchg(&p_son->lock, 0);
				xchg(&p_proc->lock, 0);
				return p_son->pid;
			}
			//kprintf("??\n");
			xchg(&p_son->lock, 0);
			//xchg(&p_proc->lock, 0);
			p = p_nxt;
		}
		//xchg(&p_proc->lock, 0);
		p_proc->statu=SLEEP;
		xchg(&p_proc->lock, 0);
		// while(p_proc->statu==SLEEP)
		// {
		// 	schedule();
		// }
		schedule();
		// while(1)
		// {
		// 	//kprintf("nitian:%d\n",p_proc->statu);
		// 	//xchg(&p_proc->lock, 0);
		// 	schedule();
		// }
	}
	// 接下来就是你自己的实现了，我们在设计的时候这段代码不会有太大问题
	// 在实现完后你任然要对自己来个灵魂拷问
	// 1. 上锁上了吗？所有临界情况都考虑到了吗？（永远要相信有各种奇奇怪怪的并发问题）
	// 2. 所有错误情况都判断到了吗？错误情况怎么处理？（RTFM->`man 2 wait`）
	// 3. 是否所有的资源都正确回收了？
	// 4. 你写的代码真的符合wait语义吗？
	//panic("Unimplement! soul torture");
	//kprintf("error:%d\n",p_proc->statu);
	return 0;
}

ssize_t
do_wait(int *wstatus)
{
	assert((uintptr_t)wstatus < KERNBASE);
	assert((uintptr_t)wstatus + sizeof(wstatus) < KERNBASE);
	return kern_wait(wstatus);
}
