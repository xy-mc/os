#include <type.h>
#include <elf.h>
#include <fat32.h>
#include <x86.h>

/*
 * 显示相关
 */
#define TERMINAL_COLUMN	80
#define TERMINAL_ROW	25

#define TERMINAL_POS(row, column) ((u16)(row) * TERMINAL_COLUMN + (column))
/*
 * 终端默认色，黑底白字
 */
#define DEFAULT_COLOR 0x0f00

/*
 * 这个函数将content数据（2字节）写到终端第disp_pos个字符
 * 第0个字符在0行0列，第1个字符在0行1列，第80个字符在1行0列，以此类推
 * 用的是内联汇编，等价于mov word [gs:disp_pos * 2], content
 */
inline static void
write_to_terminal(u16 disp_pos, u16 content)
{
	asm(
	    "mov %1, %%gs:(%0)" ::"r"(disp_pos * 2), "r"(content)
	    : "memory");
}

/*
 * 清屏
 */
void
clear_screen()
{
	for (int i = 0; i < TERMINAL_ROW; i++)
		for (int j = 0; j < TERMINAL_COLUMN; j++)
			write_to_terminal(TERMINAL_POS(i, j), 
							DEFAULT_COLOR | ' ');
}

void *
memset(void *v, int c, size_t n)
{
	char *p;
	int m;

	p = v;
	m = n;
	while (--m >= 0)
		*p++ = c;

	return v;
}

int
strncmp(const char *p, const char *q, size_t n)
{
	while (n > 0 && *p && *p == *q)
		n--, p++, q++;
	if (n == 0)
		return 0;
	else
		return (int) ((unsigned char) *p - (unsigned char) *q);
}

#define SECTSIZE	512
#define BUF_ADDR	0x30000
#define ELF_ADDR	0x40000

void
waitdisk(void)
{
	// wait for disk reaady
	while ((inb(0x1F7) & 0xC0) != 0x40)
		/* do nothing */;
}

void
readsect(void *dst, u32 offset)
{
	// wait for disk to be ready
	waitdisk();

	outb(0x1F2, 1);		// count = 1
	outb(0x1F3, offset);
	outb(0x1F4, offset >> 8);
	outb(0x1F5, offset >> 16);
	outb(0x1F6, (offset >> 24) | 0xE0);
	outb(0x1F7, 0x20);	// cmd 0x20 - read sectors

	// wait for disk to be ready
	waitdisk();

	// read a sector
	insl(0x1F0, dst, SECTSIZE/4);
}

u32 fat_start_sec;
u32 data_start_sec;
u32 elf_clus;
u32 elf_off;
u32 fat_now_sec;
struct BPB bpb;

u32
get_next_clus(u32 current_clus)
{
	u32 sec = current_clus * 4 / SECTSIZE;
	u32 off = current_clus * 4 % SECTSIZE;
	if (fat_now_sec != fat_start_sec + sec) {
		readsect((void *)BUF_ADDR, fat_start_sec + sec);
		fat_now_sec = fat_start_sec + sec;
	}
	return *(u32 *)(BUF_ADDR + off);
}

/*
 * 读入簇号对应的数据区的所有数据
 */
void *
read_data_sec(void *dst, u32 current_clus)
{
	current_clus -= 2;
	current_clus *= bpb.BPB_SecPerClus;
	current_clus += data_start_sec;

	for (int i = 0 ; i < bpb.BPB_SecPerClus ; i++, dst += SECTSIZE)
		readsect(dst, current_clus + i);
	return dst;
}

/*
 * 根据输入的参数读取对应的一段，由于kernel.bin是正经ld链接的，所以文件偏移和
 * 虚拟地址的增长速度相同，所以采取激进的读取策略，如果文件偏移不是4096对齐或
 * 不能从当前的elf_clus推出到需要读取的簇号则直接默认为读取失败，
 */
void
readseg(u32 va, u32 count, u32 offset)
{
	u32 end_va;

	end_va = va + count;
	if ((offset & (8 * SECTSIZE - 1)) != 0)
		goto bad;
	
	if (offset < elf_off)
		goto bad;
	
	while (va < end_va) {
		if (elf_off == offset) {
			read_data_sec((void *)va, elf_clus);
			va += 8 * SECTSIZE;
			offset += 8 * SECTSIZE;
		}
		elf_off += 8 * SECTSIZE;
		elf_clus = get_next_clus(elf_clus);
	}

	return;
bad:
	for (char *s = "----fail to kernel elf----", *st = s; *s; s++)
		write_to_terminal(TERMINAL_POS(1, s - st), DEFAULT_COLOR | *s);
	while (1)
		;// do nothing
}

/*
 * 初始化函数，加载kernel.bin的elf文件并跳过去。
 */
void 
load_kernel()
{
	clear_screen();
	for (char *s = "----start loading kernel elf----", *st = s; *s; s++)
		write_to_terminal(TERMINAL_POS(0, s - st), DEFAULT_COLOR | *s);
	
	readsect((void *)&bpb, 0);

	if (bpb.BPB_SecPerClus != 8 || bpb.BPB_BytsPerSec != SECTSIZE)
		goto bad;

	fat_start_sec = bpb.BPB_RsvdSecCnt;
	data_start_sec = fat_start_sec + bpb.BPB_FATSz32 * bpb.BPB_NumFATs;

	u32 root_clus = bpb.BPB_RootClus;
	
	while (root_clus < 0x0FFFFFF8) {
		void *read_end = read_data_sec((void *)BUF_ADDR, root_clus);
		for (struct Directory_Entry *p = (void *)BUF_ADDR 
					; (void *)p < read_end ; p++) {
			if (strncmp(p->DIR_Name, "KERNEL  BIN", 11) == 0) {
				elf_clus = (u32)p->DIR_FstClusHI << 16 | 
						p->DIR_FstClusLO;
				break;
			}
		}
		if (elf_clus != 0)
			break;
		root_clus = get_next_clus(root_clus);
	}

	if (elf_clus == 0)
		goto bad;

	read_data_sec((void *)ELF_ADDR, elf_clus);
	
	struct Elf *eh = (void *)ELF_ADDR;
	struct Proghdr *ph = (void *)eh + eh->e_phoff;
	for (int i = 0 ; i < eh->e_phnum ; i++, ph++) {
		if (ph->p_type != PT_LOAD)
			continue;
		readseg(ph->p_va, ph->p_filesz, ph->p_offset);
		memset(
			(void *)ph->p_va + ph->p_filesz, 
			0, 
			ph->p_memsz - ph->p_filesz);
	}
	for (char *s = "----finish loading kernel elf----", *st = s; *s; s++)
		write_to_terminal(TERMINAL_POS(1, s - st), DEFAULT_COLOR | *s);
	((void (*)(void))(eh->e_entry))();
bad:
	for (char *s = "----fail to load kernel elf----", *st = s; *s; s++)
		write_to_terminal(TERMINAL_POS(1, s - st), DEFAULT_COLOR | *s);
	while (1)
		;// do nothing
}