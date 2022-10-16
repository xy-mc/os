#include "keymap.h"
#include "stdio.h"
#include "type.h"

#define KB_INBUF_SIZE 4

typedef struct kb_inbuf {
	u8*	p_head;
	u8*	p_tail;
	int	count;
	u8	buf[KB_INBUF_SIZE];
} KB_INPUT;

static KB_INPUT kb_input = {
	.p_head = kb_input.buf,
	.p_tail = kb_input.buf,
	.count = 0,
};

/*
 * 将ch这个字符放进内核的字符缓冲区
 */
void
add_keyboard_buf(u8 ch)
{
}

/*
 * 如果内核的字符缓冲区为空，则返回-1
 * 否则返回缓冲区队头的字符并弹出队头
 */
u8
getch(void)
{
	return -1;
}