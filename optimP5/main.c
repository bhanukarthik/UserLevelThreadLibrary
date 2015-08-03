/* 	Author	:Bhanu Karthik Chilukuri 
 *	Reference for producer and consumber algotihm : http://www.csce.uark.edu/~aapon/courses/os/examples/bounded.c
 *	Date 	:11-21-2014
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "scheduler.h"
#include "queue.h"
//size of shared buffer
#define BSIZE 10
//Actual size of the string from which I copy to buffer in the producer.
#define NUMITEMS 68


//Variables need for this file, ready_list declared in schduler.c as a global variable.
extern struct queue *ready_list;
long list[] ={9,8,7,6,5,4,3,2,1,0,15,14,13,12,11,22,45,46,51,20,19,18,17,16};
int n=24,i=0;
struct mutex_t listLock,sharedDataLock,sharedBufferLock;
struct condition_t pCond,cCond;
int sharedData=0;
char sharedBuffer[BSIZE];
int itemCount=0;
char sharedBuffer[BSIZE];
int occupied;
int nextin, nextout;
struct mutex_t mutex;
struct condition_t consumerCond;
struct condition_t producerCond;


/*BubbleSort:
	This is a bubble sort algorithm to force race conditions I yielded inside critical section
*/

void sort(void * arg)
{
	long c, d, t;
	mutex_lock(&listLock);
	#ifdef DEBUG 
	printf("Lock aqquired by %s\n",(char *)arg);
	#endif
	for (c = 0 ; c < ( (n/6) - 1 ); c++)
	{	
		for (d = 0 ; d < n - c - 1; d++)
		{
			if (list[d] > list[d+1])
			{
				t         = list[d];
				list[d]   = list[d+1];
				list[d+1] = t;
			}
		}

	}
	#ifdef DEBUG 
	printf("Lock released by %s\n",(char *)arg);
	#endif
	mutex_unlock(&listLock);
}

/*This program is a simple increment operation where we read a gloabl value to a local variable
 * and increment it, yield(to force a race condition), because we aqquired a lock before yielding
 * no other thread can access sharedData, because for accessing the shared data we need to aqquire lock first.
 * so what ever thread that tires to access sharedData will block until we relase the lock.
 * Thus syncronization achived.
 */


void increment(void * arg)
{
	mutex_lock(&sharedDataLock);
	printf("%s Accquired lock\n",(char *) arg);
	int c = sharedData;
	c=c+1;
	sharedData = c;
	printf("%s Released lock\n",(char *)arg);
	mutex_unlock(&sharedDataLock);
}

/*Producer: copies data from item to shared buffer from which consumer will consume.
 *Once the shared buffer is full, producer should wait on condition variable (producerCond).
 *Shared buffer is accessed bu aqquiring lock,once the shared buffer is filled with atleast one item, Consumer condition variable is signaled.
 *before going to sleep the producer thread will unlock the mutex it accquired and got to sleep. 
 */
void producer(void * arg)
{

	char item[NUMITEMS]="CS533 PRODUCER AND CONSUMER PROBLEM USING USERLEVEL THREAD LIBRARY.";
	int i;

	for(i=0;i<NUMITEMS;i++)
	{
		if(item[i]=='\0')break;
		mutex_lock(&(mutex));
		while(occupied >= BSIZE)
		{
			mutex_unlock(&mutex);
			condition_wait(&(producerCond));
			mutex_lock(&mutex);
		}
		sharedBuffer[nextin++] = item[i];
		if(nextin==BSIZE) nextin=0;
		occupied++;
		condition_signal(&(consumerCond));
		mutex_unlock(&(mutex));
	}
}
/*Consumer: reads data from shared buffer and print it to screen.
 *Once the shared buffer is read completely it will have nothing to read so it waits on conditin variable (consumerCond)
 *Shared buffer is accessed bu aqquiring lock,once the shared buffer is read  atleast onece, producer condition variable is signaled.
 *Once all data in shared buffer is read,before going to sleep the consumer thread will unlock the mutex it accquired and got to sleep. 
 */


void consumer(void * arg)
{
	char item;
	int i;
	for(i=0;i<=NUMITEMS;i++)
	{
		mutex_lock(&(mutex));
		while(occupied<=0)
		{
			mutex_unlock(&(mutex));
			condition_wait(&(consumerCond));
			mutex_lock(&(mutex));
		}
		item = sharedBuffer[nextout++];
		printf("%c",item);
		if(nextout==BSIZE)nextout=0;
		occupied--;
		condition_signal(&(producerCond));
		mutex_unlock(&(mutex));
	}

}




void main()
{	
	//Bubble sort example.
	printf("************Bubble sort example ***************\n");
	
	printf("Before sort:");
	for(i=0;i<n;i++)
		printf("%ld  ",list[i]);
	printf("\n");
	mutex_init(&listLock);
	scheduler_begin();
	thread_fork(sort, (void *)"Thread-1");
	thread_fork(sort, (void *)"Thread-2");
	thread_fork(sort, (void *)"Thread-3");
	thread_fork(sort, (void *)"Thread-4");
	thread_fork(sort, (void *)"Thread-5");
	thread_fork(sort, (void *)"Thread-6");
	scheduler_end();
	printf("After sort:");
	for(i=0;i<n;i++)
		printf("%ld  ",list[i]);
	printf("\n\n");

	//Simple increment example.
	printf("************Simple increment example ***************\n");
	printf("Before two increments value of sharedData :%d\n", sharedData);
	mutex_init(&sharedDataLock);
	scheduler_begin();
	thread_fork(increment, (void *)"Thread-7");
	thread_fork(increment, (void *)"Thread-8");
	scheduler_end();
	printf("After two increments value of sharedData :%d\n", sharedData);

	//Producer and consumer using condition variables.
	printf("\n\n************Producer Consumer using Condition Variables***************\n");
	mutex_init(&(mutex));
	condition_init(&(producerCond));
	condition_init(&(consumerCond));
	scheduler_begin();
	thread_fork(producer,(void *)"producer");
	thread_fork(consumer,(void *)"consumer");
	scheduler_end();
	printf("\n");
}
