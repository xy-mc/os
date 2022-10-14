#include "type.h"
#include "protect.h"
#include "string.h"
#include "cmatrix.h"
#include "terminal.h"

u8 gdt_ptr[6]; /* 0~15:Limit  16~47:Base */
DESCRIPTOR gdt[GDT_SIZE];

void cstart()
{
	/* 将 LOADER 中的 GDT 复制到新的 GDT 中 */
	memcpy(&gdt,					/* New GDT */
	       (void *)(*((u32 *)(&gdt_ptr[2]))),	/* Base  of Old GDT */
	       *((u16 *)(&gdt_ptr[0])) + 1		/* Limit of Old GDT */
	);
	/* gdt_ptr[6] 共 6 个字节：0~15:Limit  16~47:Base。用作 sgdt/lgdt 的参数。*/
	u16 *p_gdt_limit = (u16 *)(&gdt_ptr[0]);
	u32 *p_gdt_base = (u32 *)(&gdt_ptr[2]);
	*p_gdt_limit = GDT_SIZE * sizeof(DESCRIPTOR) - 1;
	*p_gdt_base = (u32)&gdt;
	// 在终端的第1行依次打出N(黑底白字)，W(黑底蓝字)，P(白底蓝字)，U(白底黑字)
	kprintf(TERMINAL_POS(1, 0),
		"N%fW%bP%fU", LIGHT_BLUE, WHITE, BLACK);
	// 在终端的第2行依次输出白底绿紫相间的字符
	for (char *s = "even old new york once Amsterdam", 
					*st = s ; *s ; s += 4) {
		kprintf(TERMINAL_POS(2, s - st),
			"%b%f%c%f%c%f%c%f%c",
				WHITE, GREEN, *(s + 0),
				FUCHUSIA, *(s + 1), GREEN,
				*(s + 2), FUCHUSIA, *(s + 3));
	}
#ifdef TESTS
	//在终端第3，4行分别输出两句话，而且两行的格式相同
	//第3行为绿底黄字，第4行为黄底绿字
	for (char *s1 = "never gonna give you up ", *st1 = s1,
		*s2 = "never gonna let you down" ; *s1 ; s1 += 2, s2 += 2) {
		struct color_char c1 = {
			.background = YELLOW,
			.foreground = GREEN,
			.print_char = *(s2 + 0),
			.print_pos = TERMINAL_POS(4, s1 - st1 + 0),
		};
		struct color_char c2 = {
			.background = YELLOW,
			.foreground = GREEN,
			.print_char = *(s2 + 1),
			.print_pos = TERMINAL_POS(4, s1 - st1 + 1),
		};
		kprintf(TERMINAL_POS(3, s1 - st1),
			"%b%f%c%s%c%s",
			GREEN, YELLOW, *(s1 + 0), c1, *(s1 + 1), c2);
	} 
#endif
}
