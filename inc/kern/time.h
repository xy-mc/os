#ifndef MINIOS_KERN_TIME_H
#define MINIOS_KERN_TIME_H

#include <type.h>

void	timecounter_inc(void);
size_t	kern_get_ticks(void);
ssize_t do_delay_ticks(u32 ticks);
#endif /* MINIOS_KERN_TIME_H */