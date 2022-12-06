#include <assert.h>
#include <string.h>

#include <kern/fork.h>
#include <kern/syscall.h>
#include <kern/trap.h>
#include <kern/kmalloc.h>
#include <kern/process.h>
#include <kern/pmap.h>
u32 pid=1;
ssize_t
kern_fork(PROCESS_0 *p_fa)
{
	// 这可能是你第一次实现一个比较完整的功能，你可能会比较畏惧
	// 但是放心，别怕，先别想自己要实现一个这么大的东西而毫无思路
	// 这样你在焦虑的同时也在浪费时间，就跟你在实验五中被页表折磨一样
	// 人在触碰到未知的时候总是害怕的，这是天性，所以请你先冷静下来
	// fork系统调用会一步步引导你写出来，不会让你本科造火箭的
	//panic("Unimplement! CALM DOWN!");
	
	// 推荐是边写边想，而不是想一车然后写，这样非常容易计划赶不上变化

	// fork的第一步你需要找一个空闲（IDLE）的进程作为你要fork的子进程
	//panic("Unimplement! find a idle process");
	for(int i=0;i<PCB_SIZE;i++)
		if(proc_table[i].pcb.statu==IDLE)
			p_proc_ready=proc_table+i;
	// 再之后你需要做的是好好阅读一下pcb的数据结构，搞明白结构体中每个成员的语义
	// 别光扫一遍，要搞明白这个成员到底在哪里被用到了，具体是怎么用的
	// 可能exec和exit系统调用的代码能够帮助你对pcb的理解，不先理解好pcb你fork是无从下手的
	//panic("Unimplement! read pcb");
	PROCESS_0 *p_proc = &p_proc_ready->pcb;
	p_proc->kern_regs.esp = (u32)(p_proc_ready + 1) - 8;
	// 保证切换内核栈后执行流进入的是restart函数。
	*(u32 *)(p_proc->kern_regs.esp + 0) = (u32)restart;
	// 这里是因为restart要用`pop esp`确认esp该往哪里跳。
	*(u32 *)(p_proc->kern_regs.esp + 4) = (u32)p_proc;
	p_fa->pid=0;
	p_proc->pid=pid++;
	p_proc->priority=p_fa->priority;
	p_proc->ticks=p_fa->ticks;
	memcpy(&p_proc->user_regs, &p_fa->user_regs, sizeof(p_proc->user_regs));
	phyaddr_t new_cr3 = phy_malloc_4k();
	memset((void *)K_PHY2LIN(new_cr3), 0, PGSIZE);
	struct page_node *new_page_list = kmalloc(sizeof(struct page_node));
	new_page_list->nxt = NULL;
	new_page_list->paddr = new_cr3;
	new_page_list->laddr = -1;
	p_proc->cr3=new_cr3;
	p_proc->page_list=new_page_list;
	//panic("Unimplement! copy pcb?");
	for(struct page_node *p=p_fa->page_list;p;p=p->nxt)
	{
		if(p->laddr==-1)
			continue;
		struct page_node *new_list = kmalloc(sizeof(struct page_node));
		lin_mapping_phy(new_cr3,&new_list,p->laddr,(phyaddr_t)-1,PTE_P | PTE_W | PTE_U);
		new_list->nxt=NULL;
		new_list->laddr=p->laddr;
		p_proc->page_list->nxt=new_list;
	}
	//panic("Unimplement! copy pcb?");
	
	// 在阅读完pcb之后终于可以开始fork工作了
	// 本质上相当于将父进程的pcb内容复制到子进程pcb中
	// 但是你需要想清楚，哪些应该复制到子进程，哪些不应该复制，哪些应该子进程自己初始化
	// 其中有三个难点
	// 1. 子进程"fork"的返回值怎么处理？（需要你对系统调用整个过程都掌握比较清楚，如果非常清晰这个问题不会很大）
	// 2. 子进程内存如何复制？（别傻乎乎地复制父进程的cr3，本质上相当于与父进程共享同一块内存，
	// 而共享内存肯定不符合fork的语义，这样一个进程写内存某块地方会影响到另一个进程，这个东西需要你自己思考如何复制父进程的内存）
	// 3. 在fork结束后，肯定会调度到子进程，那么你怎么保证子进程能够正常进入用户态？
	// （你肯定会觉得这个问题问的莫名其妙的，只能说你如果遇到那一块问题了就会体会到这个问题的重要性，
	// 这需要你对调度整个过程都掌握比较清楚）
	//panic("Unimplement! copy pcb?");
	// 别忘了维护进程树，将这对父子进程关系添加进去
	//panic("Unimplement! maintain process tree");
	p_proc->fork_tree.p_fa=p_fa;
	p_proc->fork_tree.sons = NULL;
	struct son_node	*sons=kmalloc(sizeof (struct son_node));
	sons->p_son=p_proc;
	p_fa->fork_tree.sons->nxt->pre=sons;
	sons->nxt=p_fa->fork_tree.sons->nxt;
	p_fa->fork_tree.sons->nxt=sons;
	sons->pre=p_fa->fork_tree.sons;
	// 最后你需要将子进程的状态置为READY，说明fork已经好了，子进程准备就绪了
	//panic("Unimplement! change status to READY");
	p_proc->statu=READY;
	// 在你写完fork代码时先别急着运行跑，先要对自己来个灵魂拷问
	// 1. 上锁上了吗？所有临界情况都考虑到了吗？（永远要相信有各种奇奇怪怪的并发问题）
	// 2. 所有错误情况都判断到了吗？错误情况怎么处理？（RTFM->`man 2 fork`）
	// 3. 你写的代码真的符合fork语义吗？
	panic("Unimplement! soul torture");

	return 0;
}

ssize_t
do_fork(void)
{
	return kern_fork(&p_proc_ready->pcb);
}