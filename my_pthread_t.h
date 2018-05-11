#ifndef MY_PTHREAD_T_H
#define MY_PTHREAD_T_H

#include <sys/ucontext.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/ucontext.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>


#define TESTING 1
#define TIME_QUANTUM 10000
#define PriorityQueues 7
#define AGE_THRESHOLD 2000000
#define CHECK_FREQUENCY 40
#define NUM_THREADS 10
#define STACK_SIZE 16384
#define NUM_LOCKS 1


typedef enum state {
    NEW,
    READY,
    RUNNING,
    WAITING,
    TERMINATED,
    YIELD
} state;

typedef struct pthread_mutex my_pthread_mutex_t;

typedef struct my_pthread_t {
    
    ucontext_t ucp;
    struct my_pthread_t * next;
    state thr_state;
    long int thr_id;
    int num_runs;
    int time_runs;
    int priority;
    void * retval;
    long int start_tt;
    long int first_exe_tt;
    long int last_exe_tt;
    long int end_tt;
    my_pthread_mutex_t *join_mutex; 
    
} my_pthread_t;


typedef struct {
    
} my_pthread_attr_t;

typedef struct {
    my_pthread_t * head;
    my_pthread_t * tail;
    int size;
} Queue;

struct pthread_mutex {
    volatile int flag;
    volatile int guard;
    my_pthread_t owner;          // Thread owning the mutex
    Queue*  wait;
    //handle_t event;           // Mutex release notification to waiting threads
};

// MLPQ Struct
typedef struct {
    my_pthread_t * main; //thr_main *remove
    my_pthread_t * curr; //thr_curr *remove
    
    Queue * sched; //mlpq
    Queue * wait;
    
    int priorityList[PriorityQueues]; //prior_list[NUM_LEVELS] *remove
    
    long int schedNum; //num_sched *remove
    
} MultiQueue;



int my_pthread_create(my_pthread_t * thread, my_pthread_attr_t * attr, void *(*function)(void *), void * arg);
void my_pthread_yield();
void my_pthread_exit(void * value_ptr);
int my_pthread_join(my_pthread_t * thread, void ** value_ptr);


int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr);
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex);
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex);
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex);



void initQueue(Queue * first);
char isEmpty(Queue * first);
my_pthread_t * peek(Queue * first);
void priorityEnqueue(Queue * first, my_pthread_t * thr_node);
void enqueue(Queue * first, my_pthread_t * thr_node);
my_pthread_t * dequeue(Queue * first);


MultiQueue* initMultiQueue();
void addThreadMultiQueue(my_pthread_t * thr_node, int priority);
my_pthread_t * pickThreadMultiQueue();

my_pthread_t * thr_list;




#endif /* my_pthread_t_h */
