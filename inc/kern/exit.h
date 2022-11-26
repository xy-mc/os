#ifndef MINIOS_KERN_EXIT_H
#define MINIOS_KERN_EXIT_H

#include <kern/process.h>

ssize_t	kern_exit(PROCESS_0 *p_proc, int exit_code);

#endif /* MINIOS_KERN_EXIT_H */