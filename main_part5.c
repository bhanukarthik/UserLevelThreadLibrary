/* 	Author	:Bhanu Karthik Chilukuri 
 *	Reference for producer and consumber algotihm : http://www.csce.uark.edu/~aapon/courses/os/examples/bounded.c
 *	Date 	:11-21-2014
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <atomic_ops.h> 
#include <fcntl.h>
#include "scheduler.h"
#include "queue.h"

struct mutex_t sharedDataLock;
int sharedData=0;
//AO_TS_t spinlock = AO_TS_INITIALIZER;
void increment(void * arg)
{
	int i;
	for(i=0;i<1000;i++)
	{
		mutex_lock(&sharedDataLock);
		sharedData++;
		mutex_unlock(&sharedDataLock);
	}
}


int  main(int argc, char * argv[])
{	
	if(argc!=2)
	{
		printf("Inavlid arguments\n");
		exit(0);
	}
	int i=0;
	//Simple increment example.
//	printf("************Simple increment example ***************\n");
	int bi = sharedData;
	//printf("Before increment :%d\t", sharedData);
	mutex_init(&sharedDataLock);
	int number = atoi(argv[1]);
	if(number == 0)
	{
		printf("Kernel threads cant be zero!\n");
		exit(0);
	
	}
//	printf("enter number of kernel thread\n");
//	scanf("%d",&number);
	scheduler_begin(number);
	for(i=0;i<50;i++)
	{
		thread_fork(increment, (void *)"Thread-7");
	}
	scheduler_end();
	printf("Before Increment :%d After increment :%d\n", bi,sharedData);
	return 0;

}
