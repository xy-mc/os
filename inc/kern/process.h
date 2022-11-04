#ifndef MINIOS_KERN_PROCESS_H
#define MINIOS_KERN_PROCESS_H

#include <type.h>
#include <mmu.h>

/* pcb中存储用户态进程的寄存器信息 */
struct user_context {	/* proc_ptr points here				↑ Low			*/
	u32	gs;		/* ┓						│			*/
	u32	fs;		/* ┃						│			*/
	u32	es;		/* ┃						│			*/
	u32	ds;		/* ┃						│			*/
	u32	edi;		/* ┃						│			*/
	u32	esi;		/* ┣ pushed by save()				│			*/
	u32	ebp;		/* ┃						│			*/
	u32	kernel_esp;	/* <- 'popad' will ignore it			│			*/
	u32	ebx;		/* ┃						↑栈从高地址往低地址增长     */
	u32	edx;		/* ┃						│			*/
	u32	ecx;		/* ┃						│			*/
	u32	eax;		/* ┛						│			*/
	u32	retaddr;	/* return address for assembly code save()	│			*/
	u32	eip;		/*  ┓						│			*/
	u32	cs;		/*  ┃						│			*/
	u32	eflags;		/*  ┣ these are pushed by CPU during interrupt	│			*/
	u32	esp;		/*  ┃						│			*/
	u32	ss;		/*  ┛						┷High			*/
};

struct kern_context {
	u32	eflags;
	u32	edi;
	u32	esi;
	u32	ebp;
	u32	ebx;
	u32	edx;
	u32	ecx;
	u32	eax;
	u32	esp;
};

/* pcb */
typedef struct s_proc {
	struct user_context	user_regs;
	struct kern_context	kern_regs;
	u32			pid;
	phyaddr_t		cr3;
	int			priority;
	int			ticks;
}PROCESS_0;

#define KERN_STACKSIZE	(8 * KB)

typedef union u_proc {
	PROCESS_0 pcb;			// 内核pcb
	char _pad[KERN_STACKSIZE];	// pcb往上剩下的空间用于内核栈
}PROCESS;				// 通过union控制每个进程占8kb空间

/* 指向当前进程pcb的指针 */
// kern/main.c
extern PROCESS *p_proc_ready;
/* pcb表 */
#define PCB_SIZE	2
// kern/main.c
extern PROCESS	proc_table[];

// 内核栈切换上下文函数(汇编接口)
void	switch_kern_context(
	struct kern_context *current_context,
	struct kern_context *next_context
);

// 处理函数
void	schedule(void);
u32	kern_get_pid(PROCESS *p_proc);
#endif /* MINIOS_KERN_PROCESS_H */