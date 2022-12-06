#ifndef MINIOS_KERN_FORK_H
#define MINIOS_KERN_FORK_H

#include <type.h>
#include <kern/process.h>

ssize_t	kern_fork(PROCESS_0 *p_father);
extern u32 pid;
#endif /* MINIOS_KERN_FORK_H */