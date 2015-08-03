#define _GNU_SOURCE
#include <sched.h>
#include <atomic_ops.h>
#define current_thread (get_current_thread())
//void spinlock_lock(AO_TS_t *);
//void spinlock_unlock(AO_TS_t *);
extern void*safe_mem(int, void*);
#define malloc(arg) safe_mem(0, ((void*)(arg)))
#define free(arg) safe_mem(1, arg)

typedef enum {
	RUNNING,
	READY,
	BLOCKED,
	DONE
} state_t;
struct thread_t
{
	void *stack_pointer;
	void (*initial_function)(void *);
	void * initial_arg;
	state_t state; 

};
struct mutex_t
{
	struct thread_t *holdingThread;
	struct queue *waitList;	
	AO_TS_t spinlock;
};

struct condition_t
{	AO_TS_t spinlock;
	struct queue *waitList;
};
void scheduler_begin(int number);
void scheduler_end();
void thread_fork(void(*target)(void*), void * arg);
void thread_wrap();//void(*target)(void*), void * arg);
void thread_finish();
void yield();
