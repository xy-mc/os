#include <type.h>

#include <kern/syscall.h>
#include <kern/time.h>

static size_t timecounter;

/*
 * 时间戳加一
 */
void
timecounter_inc(void)
{
	timecounter++;
}

/*
 * 获取内核当前的时间戳
 */
size_t
kern_get_ticks(void)
{
	return timecounter;
}

ssize_t
do_get_ticks(void)
{
	return (ssize_t)kern_get_ticks();
}
// ssize_t kern_delay_ticks(u32 ticks)
// {
// 	u32 old=kern_get_ticks();
//  	while(kern_get_ticks()<old+ticks)
//  	 	timecounter_inc();
// 	return 0;
// }
// ssize_t do_delay_ticks(u32 ticks)
// {
// 	return (ssize_t)kern_delay_ticks(ticks);
// }