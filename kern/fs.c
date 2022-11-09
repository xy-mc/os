#include <assert.h>
#include <fat32.h>
#include <mmu.h>
#include <string.h>
#include <x86.h>
#include <type.h>
#include <elf.h>
#include <fat32.h>

#include <kern/keyboard.h>
#include <kern/fs.h>
#include <kern/stdio.h>
#include <kern/syscall.h>
#include <kern/kmalloc.h>

/*
 * 内核读入函数，由于没有文件描述符概念，
 * 所以强制assert让fd为0，平时的读入流也默认是0的
 * 现在只有从键盘缓冲区获取字符的能力
 */
ssize_t
kern_read(int fd, void *buf, size_t count)
{
	assert(fd == 0);
	
	char *s = buf;

	for (size_t i = 0 ; i < count ; i++) {
		char c = read_keyboard_buf();
		if (c == -1)
			return i;
		*s++ = c;
	}

	return count;
}

ssize_t
do_read(int fd, void *buf, size_t count)
{
	assert(buf < (void *)KERNBASE);
	assert(buf + count < (void *)KERNBASE);
	return kern_read(fd, buf, count);
}

/*
 * 内核写入函数，由于没有文件描述符概念，
 * 所以强制assert让fd为1，平时的输出流也默认是1的
 * 现在只有输出字符到终端的能力
 */
ssize_t
kern_write(int fd, const void *buf, size_t count)
{
	assert(fd == 1);
	
	const char *s = buf;
	for (size_t i = 0 ; i < count ; i++)
		kprintf("%c", *s++);

	return count;
}

ssize_t
do_write(int fd, const void *buf, size_t count)
{
	assert(buf < (void *)KERNBASE);
	assert(buf + count < (void *)KERNBASE);
	return kern_write(fd, buf, count);
}

#define SECTSIZE	512
#define BUF_ADDR	K_PHY2LIN(48 * MB)
#define ELF_ADDR	K_PHY2LIN(48 * MB)
static void
waitdisk(void)
{
	// wait for disk reaady
	while ((inb(0x1F7) & 0xC0) != 0x40)
		/* do nothing */;
}

static void
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
	insl(0x1F0, dst, SECTSIZE/4);//$
}

u32 fat_start_sec;
u32 data_start_sec;
u32 fat_now_sec;
struct BPB bpb;
u32 elf_clus;
u32 elf_off;
/*
 * 获取下一个簇号
 */
static u32
get_next_clus(u32 current_clus)
{
	u32 sec = current_clus * 4 / SECTSIZE;
	u32 off = current_clus * 4 % SECTSIZE;

	static u32 buf[SECTSIZE / 4];
	if (fat_now_sec != fat_start_sec + sec) {
		readsect(buf, fat_start_sec + sec);
		fat_now_sec = fat_start_sec + sec;
	}
	return buf[off / 4];
}

/*
 * 读入簇号对应的数据区的所有数据
 * 读到dst开头的位置，返回读入的尾指针
 */
static void *
read_data_sec(void *dst, u32 current_clus)
{
	current_clus -= 2;
	current_clus *= bpb.BPB_SecPerClus;
	current_clus += data_start_sec;

	for (int i = 0 ; i < bpb.BPB_SecPerClus ; i++, dst += SECTSIZE)
		readsect(dst, current_clus + i);//$
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
			read_data_sec((void *)va, elf_clus);//$
			va += 8 * SECTSIZE;
			offset += 8 * SECTSIZE;
		}
		elf_off += 8 * SECTSIZE;
		elf_clus = get_next_clus(elf_clus);
	}

	return;
bad:
	panic("unimplement! readseg");	
	while (1)
		;// do nothing
}

/*
 * 根据filename读取文件到dst处
 * filename要求是短目录项名（11个字节）
 * dst推荐是3GB + 48MB
 */
void
read_file(const char *filename, void *dst)
{
	assert(strlen(filename) == 11);
	
	readsect(&bpb, 0);
	
	fat_start_sec = bpb.BPB_RsvdSecCnt;
	data_start_sec = fat_start_sec + bpb.BPB_FATSz32 * bpb.BPB_NumFATs;

	u32 root_clus = bpb.BPB_RootClus;
	u32 file_clus = 0;

	assert(bpb.BPB_BytsPerSec == SECTSIZE && bpb.BPB_SecPerClus == 8);

	static char buf[SECTSIZE * 8];
	// 遍历目录项获取文件起始簇号
	while (root_clus < 0x0FFFFFF8) {
		void *read_end = read_data_sec((void *)buf, root_clus);
		for (struct Directory_Entry *p = (void *)buf 
					; (void *)p < read_end ; p++) {
			if (strncmp(p->DIR_Name, filename, 11) == 0) {
				assert(p->DIR_FileSize <= 16 * MB);
				file_clus = (u32)p->DIR_FstClusHI << 16 | 
						p->DIR_FstClusLO;
				break;
			}
		}
		if (file_clus != 0)
			break;
		root_clus = get_next_clus(root_clus);
	}

	if (file_clus == 0)
		panic("file can't found! filename: %s", filename);
	// 读入文件
	while (file_clus < 0x0FFFFFF8) {
		dst = read_data_sec(dst, file_clus);
		file_clus = get_next_clus(file_clus);
	}
}
//映射
void ys_elf(phyaddr_t cr3,u32 ad_start,u32 ad_end)
{
	uintptr_t *pde_ptr = (uintptr_t *)K_PHY2LIN(cr3);
	u32 *pde_ptr_start = pde_ptr+PDX(ad_start);
	u32 *pde_ptr_end = pde_ptr+PDX(ad_end);
	pde_ptr=pde_ptr_start;
	while(pde_ptr<=pde_ptr_end)
	{
		u32 pte_phy;
		//uintptr_t *pte_ptr;
		if(((*pde_ptr)&PTE_P)==0)
		{
			pte_phy = phy_malloc_4k();
			assert(PGOFF(pte_phy) == 0);
			*pde_ptr = pte_phy | PTE_P | PTE_W | PTE_U;
			//pte_ptr = (uintptr_t *)K_PHY2LIN(pte_phy);
		}
		else
		{
			pte_phy=(*pde_ptr);
			//phyaddr_t pte_phy=((*pde_ptr)>>3)<<3;
			//pte_ptr = (uintptr_t *)K_PHY2LIN(pte_phy);
			//pde_ptr++;
		}
		u32 *pte_ptr=(u32 *)K_PHY2LIN(pte_phy);
		u32 *pte_ptr_start;
		u32 *pte_ptr_end;
		if(pde_ptr>pde_ptr_start)
		{
			pte_ptr_start=pte_ptr+PTX(0);
			//kprintf("a\n");
		}
		else
		{
			pte_ptr_start=pte_ptr+PTX(ad_start);
			//kprintf("b\n");
		}
		if(pde_ptr<pde_ptr_end)
		{
			pte_ptr_end=pte_ptr+NPTENTRIES-1;
			//kprintf("c\n");
		}
		else
		{
			pte_ptr_end=pte_ptr+PTX(ad_end);
			//kprintf("d\n");
		}
		//pte_ptr=pte_ptr_start;
		//kprintf("%x %x\n",pte_ptr_start,pde_ptr_end);
		if(pte_ptr_start>pte_ptr_end)
		{
			u32 *t=pte_ptr_end;
			pte_ptr_end=pte_ptr_start;
			pte_ptr_start=t;
			//kprintf("%x %x\n",pte_ptr_start,pde_ptr_end);
		}
		pte_ptr=pte_ptr_start;
		//kprintf("%x %x\n",pte_ptr_start,pde_ptr_end);
		while(pte_ptr<=pte_ptr_end)
		{
			//kprintf("%x %x\n",pte_ptr,pde_ptr_end);
			if(((*pte_ptr)&PTE_P)==0)
			{
				//kprintf("%x %x\n",pte_ptr,pde_ptr_end);
				phyaddr_t phy_addr=phy_malloc_4k();
				//kprintf("%x\n",phy_addr);
				*pte_ptr = phy_addr | PTE_P | PTE_W | PTE_U;
			}
			pte_ptr++;
		}
		pde_ptr++;
	}
}
//加载elf文件
void load_elf(u32 cr3)
{
	struct Elf *eh = (void *)ELF_ADDR;
	struct Proghdr *ph = (void *)eh + eh->e_phoff;
	u32 ad_start,ad_end;
	ad_start=ph->p_va;
	for(int i = 0 ; i < eh->e_phnum ; i++, ph++)
	{
		if (ph->p_type != PT_LOAD)
			continue;
		ad_end=ph->p_va+ph->p_memsz;
	}
	ys_elf(cr3,ad_start,ad_end);
	ph = (void *)eh + eh->e_phoff;
	for (int i = 0 ; i < eh->e_phnum ; i++, ph++) 
	{
		if (ph->p_type != PT_LOAD)
			continue;
		//ys_elf(cr3,ph->p_va,ph->p_va+ph->p_memsz-1);
		//kprintf("%x %x\n",ph->p_va, ph->p_va+ph->p_memsz-1);
		//if(i==1)panic("");
		memcpy((void*)ph->p_va,(void*)eh+ph->p_offset,ph->p_filesz);
		//if(i==1)panic("");
		memset((void *)ph->p_va + ph->p_filesz, 
		 	0, 
		 	ph->p_memsz - ph->p_filesz);
		//ys_elf(cr3,ph->p_va,ph->p_va+ph->p_filesz-1);
	}
	//panic("");
	//((void (*)(void))(eh->e_entry))();
}