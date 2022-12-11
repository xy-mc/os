#include <user/stdio.h>
#include <user/syscall.h>
// int pid[20];
int main()
{
	// int pid = fork();
	// int pid1 =fork();
	// fflush();
	// printf("pid:%d\n",pid);
	// printf("pid1:%d\n",pid1);
	
	for (int i = 1 ; i <= 19; i++) {
		ssize_t pid = fork();

		// assert(pid >= 0);

		if (pid == 0)
			exit(0);
		printf("pid%d:%d\n",i,pid);
		// xor_sum ^= pid;
	}
	// if (pid1) {
	// 	while (1) {
	// 		printf("I'm fa, son pid = %d", pid1);
	// 		fflush();
	// 		for (int i = 0 ; i < (int)1e8 ; i++)
	// 			;//do nothing
	// 	}
	// } else {
	// 	while (1) {
	// 		printf("I'm son");
	// 		fflush();
	// 		for (int i = 0 ; i < (int)1e8 ; i++)
	// 			;//do nothing
	// 	}
	// }
	// if (pid) 
	// {
	// 	 while (1) 
	// 	{
	// 		printf("I'm fa, son pid = %d", pid);
	// 		fflush();
	// 		for (int i = 0 ; i < (int)1e8 ; i++)
	// 			;//do nothing
	// 	}
	// } 
	// else {
	// 	while (1) 
	// 	{
	// 		printf("I'm son");
	// 		fflush();
	// 		for (int i = 0 ; i < (int)1e8 ; i++)
	// 			;//do nothing
	// 	 }
	// }
	// if (pid1) {
	// 	while (1) {
	// 		printf("I'm fa, son pid = %d", pid);
	// 		fflush();
	// 		for (int i = 0 ; i < (int)1e8 ; i++)
	// 			;//do nothing
	// 	}
	// } else {
	// 	while (1) {
	// 		printf("I'm son");
	// 		fflush();
	// 		for (int i = 0 ; i < (int)1e8 ; i++)
	// 			;//do nothing
	// 	}
	// }
}