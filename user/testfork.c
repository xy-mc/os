#include <user/stdio.h>
#include <user/syscall.h>

int main()
{
	int pid = fork();
	int pid1 =fork();
	fflush();
	printf("pid:%d\n",pid);
	printf("pid1:%d\n",pid1);
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