#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <atomic_ops.h> 
#include "queue.h"
#include "scheduler.h"
struct queue *ready_list;
struct thread_t *current_thread; 
AO_TS_t readyLock = AO_TS_INITIALIZER;

void spinlock_lock(AO_TS_t * lock) {
	while(AO_test_and_set_acquire(lock))
	{
	}

}
void spinlock_unlock(AO_TS_t * lock) {
	AO_CLEAR(lock);

}
#undef malloc
#undef free
void * safe_mem(int op, void * arg) {
	static AO_TS_t spinlock = AO_TS_INITIALIZER;
	void * result = 0;

	spinlock_lock(&spinlock);
	if(op == 0) {
		result = malloc((size_t)arg);
	} else {
		free(arg);
	}
	spinlock_unlock(&spinlock);
	return result;
}
#define malloc(arg) safe_mem(0, ((void*)(arg)))
#define free(arg) safe_mem(1, arg)
/*#undef printf
  void * print(args...)
  {
  static AO_TS_t spinlock = AO_TS_INITIALIZER;
  spinlock_lock(&spinlock);
  printf(##args);
  spinlock_unlock(&spinlock);
  }
#define printf(args...) print(args...)
 */
int kernel_thread_begin()
{
	struct thread_t *new_thread = malloc(sizeof(struct thread_t));
	new_thread->state = RUNNING;
	new_thread->initial_arg = "New Kernel Thread";
	set_current_thread(new_thread);
	while(1)
	{
		yield();
	}

}
//Initializing ready queue.
void scheduler_begin(int number)
{
	printf("\n");
	ready_list= malloc(sizeof(struct queue *));
	ready_list->tail = NULL;//malloc(sizeof(struct queue_node));
	ready_list->head = NULL; 
	set_current_thread(malloc(sizeof(struct thread_t)));
	current_thread->initial_arg = "Main Thread";
	int i;
	for(i=0;i<number-1;i++)
	{		
		void *stackPointer =  malloc(80000)+80000;
		clone(kernel_thread_begin, stackPointer, CLONE_THREAD | CLONE_VM | CLONE_SIGHAND | CLONE_FILES | CLONE_FS | CLONE_IO ,NULL);
	}	
}

//Until the ready queue is not empty the thread calling scheduler_end will yield.
void scheduler_end()
{

	spinlock_lock(&readyLock);
	while(!is_empty(ready_list))
	{
		spinlock_unlock(&readyLock);
		yield();
		spinlock_lock(&readyLock);
	}
	spinlock_unlock(&readyLock);

}

/* if the spawned thread finished first, the program would crash.
 * Because when a function returns, it pulls the return address of the calling function from the control stack and jumps to it.
 * But our thread had no calling function; 
 * The initial method began at the bottom of the control stack. 
 * Thus, the CPU simply grabbed some garbage from the next address in the heap and tried to jump to it, causing the program to crash.
 * This thread_finish() is a handler which we push on to bottom of the stack.
 * This will make forked therad to finish gracefully. 	
 */
void thread_finish()
{
	current_thread->state=DONE;
	if(strcmp(current_thread->initial_arg,"Main Thread")!=0)
		yield();
}

/*
 *Checks whether the current threads state is done , if not, it will push the current thread to ready queue.
 *Dequeues new thread from the ready queue.
 *Thread switch to new thread.
 */

void yield() 
{
	spinlock_lock(&readyLock);
	//Checks whether the current threads state is done , if not, it will push the current thread to ready queue
	if(current_thread->state != DONE && current_thread->state != BLOCKED)
	{
		current_thread->state =READY;
		thread_enqueue(ready_list, current_thread);
	}
	//Dequeues new thread from the ready queue.
	struct thread_t *new = thread_dequeue(ready_list);
	struct thread_t * temp = current_thread;
	set_current_thread(new);

	//current_thread = new;
	current_thread->state = RUNNING;
	new = temp;
	thread_switch(new,current_thread);
	spinlock_unlock(&readyLock);
}

void block(AO_TS_t * spinlock)  
{
	spinlock_lock(&readyLock);
	spinlock_unlock(spinlock);
	//Checks whether the current threads state is done , if not, it will push the current thread to ready queue
	if(current_thread->state != DONE && current_thread->state != BLOCKED)
	{
		current_thread->state =READY;
		thread_enqueue(ready_list, current_thread);
	}
	//Dequeues new thread from the ready queue.
	struct thread_t *new = thread_dequeue(ready_list);
	struct thread_t * temp = current_thread;
	set_current_thread(new);

	//current_thread = new;
	current_thread->state = RUNNING;
	new = temp;
	thread_switch(new,current_thread);
	spinlock_unlock(&readyLock);
}

/*
   Pushes the current thread to ready Queue;
   Switch the control to the new thread.
 */

void thread_fork(void(*target)(void*), void * arg)
{
	struct thread_t *new_thread = malloc(sizeof(struct thread_t));
	new_thread->stack_pointer = malloc(40000)+40000;
	new_thread->initial_function = target;
	new_thread->initial_arg = arg;
	new_thread->state = RUNNING;
	current_thread->state = READY;
	spinlock_lock(&readyLock);
	thread_enqueue(ready_list, current_thread);
	struct thread_t *temp = current_thread;
	set_current_thread(new_thread);
	new_thread = temp;
	thread_start(new_thread,current_thread);
	spinlock_unlock(&readyLock);
}

void thread_wrap(void(*target)(void*), void * arg)
{
	//void (*target)(void *) = current_thread->initial_function;
	//void * arg = current_thread->initial_arg;
	spinlock_unlock(&readyLock);
	target(arg);
	current_thread->state=DONE;
	yield();
}
