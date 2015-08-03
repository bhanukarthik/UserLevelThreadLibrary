#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <atomic_ops.h> 
#include "queue.h"
#include "scheduler.h"

extern struct thread_t *current_thread;
extern struct queue *ready_list;
extern AO_TS_t readyLock; 

void mutex_init(struct mutex_t *lock)
{
	lock->holdingThread = NULL;
	lock->spinlock = AO_TS_INITIALIZER;
	lock->waitList = malloc(sizeof(struct queue *));	
	lock->waitList->tail = NULL;
	lock->waitList->head = NULL;
}

void mutex_lock(struct mutex_t *lock)
{	spinlock_lock(&(lock->spinlock));
	if(lock->holdingThread ==NULL)
	{
		lock->holdingThread = current_thread;
		spinlock_unlock(&(lock->spinlock));
	}

	else
	{
		current_thread->state = BLOCKED;				
		thread_enqueue(lock->waitList, current_thread);			
		block(&(lock->spinlock));
	}


}

void mutex_unlock(struct mutex_t *lock)
{
	spinlock_lock(&(lock->spinlock));
	if(is_empty(lock->waitList))
	{
		lock->holdingThread =NULL;
		spinlock_unlock(&(lock->spinlock));

	}

	else
	{
		struct thread_t *temp = thread_dequeue(lock->waitList);
		temp->state = READY;
		lock->holdingThread = temp;
		spinlock_lock(&(readyLock));
		thread_enqueue(ready_list,temp);
		spinlock_unlock(&(readyLock));
		spinlock_unlock(&(lock->spinlock));
	}


}

void condition_init(struct condition_t *cv)
{
	cv->spinlock = AO_TS_INITIALIZER;	
	cv->waitList = malloc(sizeof(struct queue *));
	cv->waitList->tail = NULL;
	cv->waitList->head=NULL;

}

void condition_wait(struct condition_t	*cv)
{
	spinlock_lock(&(cv->spinlock));
	current_thread-> state = BLOCKED;
	thread_enqueue(cv->waitList, current_thread);
	block(&(cv->spinlock));
}

void condition_signal(struct condition_t *cv)
{
	spinlock_lock(&(cv->spinlock));
	if(!is_empty(cv->waitList))
	{
		struct thread_t *temp = thread_dequeue(cv->waitList);
		temp->state = READY;
		spinlock_lock(&(readyLock));
		thread_enqueue(ready_list,temp);
		spinlock_unlock(&(readyLock));
		spinlock_unlock(&(cv->spinlock));
	}

	else
	{
		spinlock_unlock(&(cv->spinlock));	
	}
}

void condition_bordcast(struct condition_t *cv)
{

	while(!is_empty(cv->waitList))
	{
		struct thread_t *temp = thread_dequeue(cv->waitList);
		temp->state = READY;
		thread_enqueue(ready_list,temp);


	}

}
