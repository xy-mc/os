#ifndef MINIOS_KERN_STDIO_H
#define MINIOS_KERN_STDIO_H

#include <stdio.h>

// lib/kern/terminal.c
int	kprintf(const char *fmt, ...);
int	vkprintf(const char *fmt, va_list);

#endif /* MINIOS_KERN_STDIO_H */