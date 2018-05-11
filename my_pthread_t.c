//
//  my_pthread_t.c
//  Multilevel Queue
//
//  Created by Santhosh Rachakonda and Parth Shorey on 2/25/17.
//  Copyright © 2017 Santhosh Rachakonda and Parth Shorey. All rights reserved.
//

#include "my_pthread_t.h"


static long int thr_id = 0;
static long int check_flag = 0;
long int start;
long int end;
//static MultiQueue* sched;
static is_scheduler_initialized = 0;
static MultiQueue * mlpq;

void run_thread(my_pthread_t * thr_node, void *(*f)(void *), void * arg);

long int get_time_stamp(){
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    return 1000000 * current_time.tv_sec + current_time.tv_usec;
}

void initQueue(Queue * first) {
    
    first->head = NULL;
    first->tail = NULL;
    first->size = 0;
}

char isEmpty(Queue * first) {
    return first->size == 0;
}

void enqueue(Queue * first, my_pthread_t * thr_node) {
    if (first->size == 0) {
        first->head =  thr_node;
        first->tail = thr_node;
        first->size++;
        
    } else {
        
        first->tail->next = thr_node;
        first->tail = thr_node;
        first->size++;
        
    }
}
//void priorityEnqueue(Queue * first, my_pthread_t * thr_node) {
//    
//    if (!isEmpty(first)) {
//        if(thr_node->priority < first->head->priority){
//            thr_node->next = first->head;
//            first->head = thr_node;
//        }
//        else{
//            my_pthread_t* prevTempThrNode = (my_pthread_t*)malloc(sizeof(my_pthread_t));
//            my_pthread_t* tempThrNode = (my_pthread_t*)malloc(sizeof(my_pthread_t));
//            prevTempThrNode = first->head;
//            tempThrNode = first->head->next;
//            
//            while (tempThrNode->next != NULL && tempThrNode->priority < thr_node->priority) {
//                prevTempThrNode = tempThrNode;
//                tempThrNode = tempThrNode->next;
//            }
//            
//            if(tempThrNode->next == NULL){
//                prevTempThrNode->next = thr_node;
//                first->tail = thr_node;
//            }
//            else{
//                thr_node->next = tempThrNode;
//                prevTempThrNode->next = thr_node;
//            }
//        }
//        //        first->head = thr_node;
//        //        first->tail = thr_node;
//        //        first->size++;
//    } else {
//        first->head->next = thr_node;
//        first->tail = thr_node;
//        first->size++;
//    }
//}
//

my_pthread_t * dequeue(Queue * first) {
    
    if (first->size == 0) {
        printf("all queues empty and nothing to dequeue from empty queue, hence return NULL\n");
        return NULL;
    }
    my_pthread_t * temp;
    if (first->size == 1) {
        temp = first->head;
        first->head = NULL;
        first->tail = NULL;
    } else {
        temp = first->head;
        first->head = first->head->next;
    }
    temp->next = NULL;
    first->size--;
    return temp;
}

/*********************************************************************/
//          Multi Level Priority Queue Scheduler                      //
/*********************************************************************/

void scheduler_handler(){
    struct itimerval tick;
    ucontext_t sched_ctx;
    
    //clear the timer
    tick.it_value.tv_sec = 0;
    tick.it_value.tv_usec = 0;
    tick.it_interval.tv_sec = 0;
    tick.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &tick, NULL);
    
    //perform aging
    if(check_flag++ >= CHECK_FREQUENCY){
        // printf("Start scaling up threads...\n");
        int i;
        check_flag = 0;
        long int current_time = get_time_stamp();
        for (i = 1; i < PriorityQueues; i++) {
            if (mlpq->sched[i].head != NULL) {
                my_pthread_t* tmp = mlpq->sched[i].head;
                my_pthread_t* parent = NULL;
                while(tmp != NULL){
                    if(current_time - tmp->last_exe_tt >= AGE_THRESHOLD){
                        // printf("Find one candidate, its thread id is: %d\n", tmp->thr_id); *remove
                        //delete from current queue
                        if(parent == NULL){
                            mlpq->sched[i].head = tmp->next;
                        }else{
                            parent->next = tmp->next;
                        }
                        //put the thread to the highest queue
                        // printf("Put thread %d to highest level.\n", tmp->thr_id); *remove
                        addThreadMultiQueue(tmp, 0);
                    }else{
                        parent = tmp;
                    }
                    tmp = tmp->next;
                }
            }
        }
        // printf("Finish scaling up threads...\n"); *remove
    }
    
    //schelduling
    my_pthread_t* tmp = mlpq->curr;
    if(tmp != NULL){
        int old_priority = tmp->priority;
        tmp->time_runs += TIME_QUANTUM;
        if(tmp->time_runs >= mlpq->priorityList[old_priority] || tmp->thr_state == YIELD || tmp->thr_state == TERMINATED
           || tmp->thr_state == WAITING){
            if (tmp->thr_state == TERMINATED){
                // free(tmp);
            }else if(tmp->thr_state == WAITING){
                //do nothing, the thread is already in the wait queue of the mutex *remove
            }else if(tmp->thr_state == YIELD){
                //put the thread back into the original queue *remove
                addThreadMultiQueue(tmp, tmp->priority);
            }else{
                //put the thread back into the queue with the lower priority *remove
                int new_priority = (tmp->priority+1) > (PriorityQueues-1) ? (PriorityQueues-1) : (tmp->priority+1);
                addThreadMultiQueue(tmp, new_priority);
            }
            //pick another thread out and run *remove
            if((mlpq->curr = pickThreadMultiQueue()) != NULL){
                mlpq->curr->thr_state = RUNNING;
            }
        }
    }else{
        //pick another thread out and run
        if((mlpq->curr = pickThreadMultiQueue()) != NULL){
            mlpq->curr->thr_state = RUNNING;
        }
    }
    
    //set timer
    tick.it_value.tv_sec = 0;
    tick.it_value.tv_usec = 50000;
    tick.it_interval.tv_sec = 0;
    tick.it_interval.tv_usec = 0;
    
    setitimer(ITIMER_REAL, &tick, NULL);
    
    //if(tmp != NULL){
    //getcontext(&sched_ctx);
    //tmp->ucp = sched_ctx; *remove
    //}
    
    if(mlpq->curr != NULL){
        if(mlpq->curr->first_exe_tt == 0){
            mlpq->curr->first_exe_tt = get_time_stamp();
        }
        mlpq->curr->last_exe_tt = get_time_stamp();
        if( tmp != NULL)
            swapcontext(&(tmp->ucp), &(mlpq->curr->ucp));
        else
            swapcontext(&sched_ctx, &(mlpq->curr->ucp));
    }
    return;
}



/////////////////////////////////////////////Scheduler/////////////////////////////////////////////////
MultiQueue* initMultiQueue() {
    

    //initialize the start;
    start = get_time_stamp();
    
    mlpq = (MultiQueue*)malloc(sizeof(MultiQueue));
    mlpq->sched = malloc(PriorityQueues * sizeof(Queue)); // first level
    mlpq->wait = malloc(PriorityQueues * sizeof(Queue)); // second level
    mlpq->main = (my_pthread_t *) calloc(1, sizeof(my_pthread_t)); // main
    
    int i = 0;
    while(i < PriorityQueues) {
        initQueue((mlpq->sched) + i);
        i++;
    }
    
    int j = 0;
    while( j < PriorityQueues) {
        initQueue((mlpq->wait) + j);
        j++;
    }
    
    int k = 0;
    while (k < PriorityQueues) {	// This is a temporary placeholder
        mlpq->priorityList[k] = (k + 1)*TIME_QUANTUM;	// for storing scheduling times
        k++;
    }
    
    mlpq->schedNum = 0;
    
    mlpq->main->num_runs = 0;
    
    
    mlpq->main->thr_id = 0;
    mlpq->main->thr_state = NEW;
    mlpq->main->next = mlpq->main;
    mlpq->curr = NULL;
    
    signal(SIGALRM, scheduler_handler);
    scheduler_handler();
    printf("Initialized the scheduler\n");
    
    return mlpq;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////




void addThreadMultiQueue(my_pthread_t * thr_node, int priority) {
    /*
     This function adds a node to a particular queue. This function is
     used to make schedule insertion easy. The number of scheduled
     threads is increased by 1. Threads that are added to the
     scheduler have their states changed. *remove
     */
    if (priority < 0 || priority >= PriorityQueues) {
        printf("The priority is not within the Multi-Level Priority Queue.\n");
    } else {
        printf("Adding thread %d to level %d\n", thr_node->thr_id, priority);
        thr_node->thr_state = READY;
        thr_node->priority = priority; // keeptrack of the priority of the thread
        thr_node->time_runs = 0; // reset the running time of the thread
        enqueue(&(mlpq->sched[priority]), thr_node);
        mlpq->schedNum++;
    }
}

my_pthread_t * pickThreadMultiQueue() {
    /*
     This function picks a thread to be scheduled from the scheduler,
     returning the one in the lowest index queue (which is the
     highest priority queue by convention). *remove
     */
    int i;
    for (i = 0; i < PriorityQueues; i++) {
        if (mlpq->sched[i].head != NULL) {
            my_pthread_t * chosen = dequeue(&(mlpq->sched[i]));
            // printf("Found a thread to schedule in level %d, thread id: %d\n", i, chosen->thr_id);
            mlpq->schedNum--;
            return chosen;
        }
    }
    printf("Nothing to schedule. return NULL;\n");
    
    // Print Everything
    end = get_time_stamp();
    printf("Start timestamp: %ld\n", start);
    printf("End timestamp: %ld\n", end);
    printf("Total timestamp: %ld\n", end - start);
    
    printf("The number of threads is %d\n",NUM_THREADS);

    for (i = 0; i < NUM_THREADS; i++) {
        
        printf("The end time:%ld and start time is %ld\n",(&thr_list[i])->first_exe_tt,(&thr_list[i])->start_tt);
        printf("Response time of %d is : %ld\n", i, ( (&thr_list[i])->first_exe_tt  - (&thr_list[i])->start_tt) );
    }
    
    
    
    // exit(EXIT_SUCCESS);
    return NULL;
}



int my_pthread_create(my_pthread_t * thread, my_pthread_attr_t * attr, void *(*function)(void *), void * arg) {
    /*
     This function takes a thread that has already been malloc'd, gives the thread
     a stack, a successor, and creates a context that runs the function
     run_thread. run_thread is a function that handles the running of the
     function with the arg fed to make the context when scheduled to run
     */
    
    if(is_scheduler_initialized == 0){
        //initialize the scheduler;
        mlpq = initMultiQueue();
        is_scheduler_initialized = 1;
        
    }
    
    if(getcontext(&(thread->ucp)) == -1) {
        printf("getcontext error\n");
        return -1;
    }
    
    thread->ucp.uc_stack.ss_sp = malloc(STACK_SIZE); //func_stack
    thread->ucp.uc_stack.ss_size = STACK_SIZE;
    //thread->ucp.uc_link = &sched_ctx;//&(sched->thr_main->ucp);
    thread->thr_id = thr_id++;
    thread->start_tt = get_time_stamp();
    thread->first_exe_tt = 0;
    thread->join_mutex = NULL;
    printf("Allocating the stack\n");
    makecontext(&(thread->ucp), (void *)run_thread, 3, thread, function, arg);
    printf("Made Context\n");
    addThreadMultiQueue(thread,0);
    printf("Added Thread to the Scheduler.\n");
    return 0;
    
}


void my_pthread_yield() {
    /*
     This function swaps the current thread and runs another thread from the scheduler.
     The current function waits.
     */
    
    //mypthread_t * tmp;
    printf("Printing Scheduler Attributes\n");
    //tmp = sched->thr_cur;
    
    // call the scheduler
    mlpq->curr->thr_state = YIELD;
    scheduler_handler();
    
}


void my_pthread_exit(void * value_ptr) {
    /*
     This function forcibly shuts down the current thread. It does so by setting
     the current state to TERMINATED. This function first checks if the
     thread is already dead. When the thread is terminated, the thread
     perpetually yields.
     */
    if (mlpq->curr->thr_state == TERMINATED) {
        printf("This thread has already exited.\n");
    }
     
    mlpq->curr->thr_state = TERMINATED;
    mlpq->curr->retval = value_ptr;
    mlpq->curr->end_tt=get_time_stamp();
    
    if(mlpq->curr->join_mutex!=NULL){
       
      //unlock the mutex;
      my_pthread_mutex_unlock(mlpq->curr->join_mutex);      

    }    

    // call the scheduler
    scheduler_handler();
    

}


int my_pthread_join(my_pthread_t * thread, void ** value_ptr) {
    /*
     This function takes in the a thread pointer and has the current thread to the
     argument thread. Any return value
     */
    printf("Started the join");

   /* while (thread->thr_state != TERMINATED) {
        my_pthread_yield();
    }
    thread->retval = value_ptr;
    printf("Joined the thread\n");*/
   
    if(thread->thr_state != TERMINATED){
      
     my_pthread_mutex_t* mutex_join = NULL;
    mutex_join = malloc(sizeof(my_pthread_mutex_t));
    my_pthread_mutex_init(mutex_join, NULL);
    my_pthread_mutex_lock(mutex_join);
    thread->join_mutex = mutex_join;
    
    my_pthread_mutex_lock(mutex_join);
    my_pthread_mutex_unlock(mutex_join);
    
    printf("Joined the thread\n");
         
    
    }

    
    
    return 0;
}

int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr){
    int result = 0;
    
    if(mutex == NULL)
        return EINVAL;
    
    mutex->flag  = 0;
    mutex->guard = 0;
    mutex->wait = (Queue*)malloc(sizeof(Queue));
    initQueue(mutex->wait);
    
    return result;
}

int my_pthread_mutex_lock(my_pthread_mutex_t *mutex) {
    
    while (__sync_lock_test_and_set(&(mutex->flag), 1) == 1){
        //my_pthread_yield();
        mlpq->curr->thr_state = WAITING;
        printf("The thread is waiting for a mutex, put it to the waiting list\n");
        enqueue(mutex->wait, mlpq->curr);
        scheduler_handler();
    }
    
    return 0;
}

int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex){
    /*while (TestAndSet(&m->guard, 1) == 1)
     ; //acquire guard lock by spinning
     if (queue_empty(m->q))
     m->flag = 0; // let go of lock; no one wants it
     else
     gettid());
     unpark(queue_remove(m->q)); // hold lock (for next thread!)
     m->guard = 0;*/
    my_pthread_t * chosen;
    if (mutex->wait->head != NULL) {
        chosen = dequeue(mutex->wait);
        printf("Mutex is available, select one thread from the waiting list and put it back to the running queue\n");
        addThreadMultiQueue(chosen, chosen->priority);
        
    }
    
    //change the mutex flag back to zero;
    mutex->flag = 0;
    
    return 0;
}

int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex){
    int result = 0;
    
    if(mutex == NULL)
        return EINVAL;
    if(mutex->flag != 0)
        return EBUSY;
    return result;
}


void run_thread(my_pthread_t * thr_node, void *(*f)(void *), void * arg) {
    /*
     This function takes a thread and executes the function (with parameters arg).
     Any return value will be stored in retval. If a state is newly terminated,
     then it will not be scheduled any longer, and the number of scheduled
     threads is reduced by 1. The scheduler will now point to the currently
     running thread
     *remove
     */
    thr_node->thr_state = RUNNING;
    mlpq->curr = thr_node;
    thr_node->retval = f(arg);
    if (thr_node->thr_state != TERMINATED) {
        thr_node->thr_state = TERMINATED;
        //		sched->num_sched-- *remove;
    }
    if(mlpq->curr != NULL){
        mlpq->curr->end_tt=get_time_stamp();
    }
    scheduler_handler();
    
}



