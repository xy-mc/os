#include "assert.h"
#include "process.h"
#include "stdio.h"
#include "time.h"
#include "trap.h"
#include "x86.h"
#include "keyboard.h"
#include "keymap.h"
int ticks;
/*
 * 当前内核需要处理中断的数量
 */
int k_reenter;
void (*irq_table[16])(int) = {
	clock_interrupt_handler,
	keyboard_handler,
	default_interrupt_handler,
	default_interrupt_handler,
	default_interrupt_handler,
	default_interrupt_handler,
	default_interrupt_handler,
	default_interrupt_handler,
	default_interrupt_handler,
	default_interrupt_handler,
	default_interrupt_handler,
	default_interrupt_handler,
	default_interrupt_handler,
	default_interrupt_handler,
	default_interrupt_handler,
	default_interrupt_handler,
};

/*
 * 开启对应外设中断（将掩码对应位置为0）
 */
void
enable_irq(int irq)
{
	u8 mask = 1 << (irq % 8);
	if (irq < 8)
		outb(INT_M_CTLMASK, inb(INT_M_CTLMASK) & ~mask);
	else
		outb(INT_S_CTLMASK, inb(INT_S_CTLMASK) & ~mask);
}

/*
 * 关闭对应外设中断（将掩码对应位置为1）
 */
void
disable_irq(int irq)
{
	u8 mask = 1 << (irq % 8);
	if (irq < 8)
		outb(INT_M_CTLMASK, inb(INT_M_CTLMASK) | mask);
	else
		outb(INT_S_CTLMASK, inb(INT_S_CTLMASK) | mask);
}
/*
 * 中断默认处理函数
 * 理论上是不支持的，所以会给个warning
 */
void
default_interrupt_handler(int irq)
{
	warn("unsupport interrupt! irq = %d", irq);
}

/*
 * 异常默认处理函数
 * 由于没有任何处理异常的手段，所以会给个panic
 */
void
exception_handler(int vec_no, int err_code, int eip, int cs, int eflags)
{
	char err_description[][64] = {	"#DE Divide Error",
					"#DB RESERVED",
					"—  NMI Interrupt",
					"#BP Breakpoint",
					"#OF Overflow",
					"#BR BOUND Range Exceeded",
					"#UD Invalid Opcode (Undefined Opcode)",
					"#NM Device Not Available (No Math Coprocessor)",
					"#DF Double Fault",
					"    Coprocessor Segment Overrun (reserved)",
					"#TS Invalid TSS",
					"#NP Segment Not Present",
					"#SS Stack-Segment Fault",
					"#GP General Protection",
					"#PF Page Fault",
					"—  (Intel reserved. Do not use.)",
					"#MF x87 FPU Floating-Point Error (Math Fault)",
					"#AC Alignment Check",
					"#MC Machine Check",
					"#XF SIMD Floating-Point Exception"
				};
	panic("\x1b[H\x1b[2JException! --> %s\nEFLAGS: %x CS: %x EIP: %x\nError code: %x",
		err_description[vec_no], eflags, cs, eip, err_code);
}
/*
 * 时钟中断处理函数
 */
void
clock_interrupt_handler(int irq)
{
	ticks++;
	timecounter_inc();
	//kprintf("i%d",ticks);
	/*if(p_proc_ready-proc_table==0)
	{
	   for(int i=0;i<5e6;i++)
	    timecounter_inc();
	}*/
	p_proc_ready++;
	if (p_proc_ready >= proc_table + PCB_SIZE) {
		p_proc_ready = proc_table;
	}
}
void
keyboard_handler(int irq)
{
  u8 c = inb(0x60);
  if((c>=0x10&&c<=0x19)||(c>=0x1E&&c<=0x26)||(c>=0x2C&&c<=0x32))
  {
     u8 s=keymap[c];
     add_keyboard_buf(s);
  }
}