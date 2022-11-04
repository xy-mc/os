#ifndef MINIOS_PROCESS_H
#define MINIOS_PROCESS_H

#include "type.h"

/* pcb中存储用户态进程的寄存器信息 */
typedef struct s_stackframe {	/* proc_ptr points here				↑ Low			*/
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
}STACK_FRAME;

/* pcb */
typedef struct s_proc {
	STACK_FRAME	regs;		/* process' registers saved in stack frame */
	u32		pid;		/* process id passed in from MM */
	char		p_name[16];	/* name of the process */
}PROCESS;

#define PCB_SIZE 3
/* 指向当前进程pcb的指针 */
extern PROCESS *p_proc_ready;
/* pcb表 */
extern PROCESS	proc_table[PCB_SIZE];

#endif