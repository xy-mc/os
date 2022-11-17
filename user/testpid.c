/*#include <user/stdio.h>
#include <user/syscall.h>

int main()
{
	while (1) {
		printf("pid: %d!", get_pid());
		fflush();
		for (int i = 0 ; i < (int)1e8 ; i++)
			;//do nothing
	}
	return 0;
}*/
#include <assert.h>

#include <user/stdio.h>
#include <user/syscall.h>

int main()
{
	int pid = get_pid();
	int target_ticks = (pid + 1) * 1000;
	
	while(1) {
		printf("\x1b[%dmI'm %d!\x1b[0m", 90 + pid + 1, pid);
		fflush();
		int before_ticks = get_ticks();
		delay_ticks(target_ticks);
		int after_ticks = get_ticks();
		
		int real_ticks = after_ticks - before_ticks;
		int delta_ticks = real_ticks - target_ticks;
		assert(-5 <= delta_ticks && delta_ticks <= 5);
	}
}