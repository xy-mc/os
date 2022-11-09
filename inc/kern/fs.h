#ifndef MINIOS_KERN_FS_H
#define MINIOS_KERN_FS_H

#include <type.h>

ssize_t	kern_read(int fd, void *buf, size_t count);
ssize_t	kern_write(int fd, const void *buf, size_t count);

void	read_file(const char *filename, void *dst);
void    load_elf(u32 cr3);
void    ys_elf(phyaddr_t cr3,u32 ad_start,u32 ad_end);
#endif /* MINIOS_KERN_FS_H */