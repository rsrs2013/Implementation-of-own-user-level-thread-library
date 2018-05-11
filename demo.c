#include <stdio.h>
#include <unistd.h>
#include "my_pthread_t.h"

#if TESTING

extern my_pthread_t * thr_list;
static my_pthread_mutex_t* mutex1;
static int sharedVariable = 0;
static int sharedVariable1 = 0;
//long int start=0;
//long int end=0;

void f0(void) {
    printf("Function f0 start\n");
    //char *s="That's good news";
    int i=0;
 
    while(i<1000){
        i++;
    }

    printf("Function f0 done\n");
}

void f1(void) {
    
    int j;
    
    for(j = 0; j < 10; j++) {
        printf("Number: %d\n", j);
    }
    printf("Function f1 Done\n");
}

void f2(void) {
    
    int j;
    
    for(j = 100; j < 125; j++) {
        printf("Number: %d\n", j);
        if (j == 115) {
            my_pthread_exit(NULL);
        }
    }
    printf("Function f2 done\n");
}

void mutexTestOne() {
    //char *s="That's good news";
    int i=0;
    int localCopy;
  
    my_pthread_mutex_lock(mutex1);
    localCopy = sharedVariable;
    printf("mutexTestOne read the sharedVariable, the value is %d\n", localCopy);
    while(i<100000){
       
        i++;
    }
   
    localCopy = localCopy+10;
    sharedVariable = localCopy;
    printf("mutexTestOne update the sharedVariable, the value now is %d\n", sharedVariable);
    my_pthread_mutex_unlock(mutex1);
    
}

void mutexTestTwo() {
    //char *s="That's good news";
    int i=0;
    int localCopy;
    
    my_pthread_mutex_lock(mutex1);
    localCopy = sharedVariable;
    printf("mutexTestTwo read the sharedVariable, the value is %d\n", localCopy);
    while(i<1048){
      
        i++;
    }
    
    localCopy = localCopy-5;
    sharedVariable = localCopy;
    printf("mutexTestTwo update the sharedVariable, the value now is %d\n", sharedVariable);
    my_pthread_mutex_unlock(mutex1);
    
}

void noMutexTestOne() {
    //char *s="That's good news";
    int i=0;
    int localCopy;
    localCopy = sharedVariable1;
    printf("noMutexTestOne read the sharedVariable, the value is %d\n", localCopy);
    while(i<10485){
     
        i++;
    }
  
    localCopy = localCopy+10;
    sharedVariable1 = localCopy;
    printf("noMutexTestOne update the sharedVariable, the value now is %d\n", sharedVariable1);
    
}

void noMutexTestTwo() {
    //char *s="That's good news";
    int i=0;
    int localCopy;
    
    localCopy = sharedVariable1;
    printf("noMutexTestTwo read the sharedVariable, the value is %d\n", localCopy);
    while(i<10485){
        i++;
    }
  
    localCopy = localCopy-5;
    sharedVariable1 = localCopy;
    printf("noMutexTestTwo update the sharedVariable, the value now is %d\n", sharedVariable1);
    
   
}

void test(int cap) {
    
    int i, j;
    int test;
    test = 1;
    for (i = 1; i < cap; i++) {
        for (j = 1; j < i; j++) {
            if (i % j == 0) {
                continue;
            }
            if (j == i - 1) {
                test = i;
            }
        }
    }
    printf("Final Test: %d\n", test);
}

void test_normal_bench(){
    printf("Starting Testing\n");
    
    
    printf("Allocating space for the thread array\n");
    thr_list = malloc(NUM_THREADS * sizeof(my_pthread_t));
    printf("Initializing the Scheduler\n");
    
    printf("Initializing the Mutex\n");
    mutex1 = malloc(sizeof(my_pthread_mutex_t));
    
    my_pthread_mutex_init(mutex1,NULL);
    
    printf("Initializing thread\n");
    
    // NUM_THREADS = 50;
    time_t t;
    long int i;
    long int base = 100;
    long int random[NUM_THREADS];
    //long int random_sec[NUM_THREADS];
    
  
    
    srand((unsigned) time(&t));
    
    for (i = 0; i < NUM_THREADS; i++) {
        random[i] = rand() % 1000 * base;
        printf("Random Number %li\n", random[i]);
    }
    
    
    my_pthread_attr_t * thread_attr = NULL;
    thr_list = malloc(NUM_THREADS * sizeof(my_pthread_t));
    for (i = 0; i < NUM_THREADS-4; i++) {
        if (my_pthread_create(&thr_list[i], thread_attr, (void *(*)(void *))test, (void *)random[i]) != 0) {
            printf("Error Creating Thread %li\n", i);
        }
    }
    if (my_pthread_create(&thr_list[NUM_THREADS-4], thread_attr, (void *(*)(void *))mutexTestOne, NULL)) {
        printf("Error Creating Thread %li\n", NUM_THREADS-4);
    }
    if (my_pthread_create(&thr_list[NUM_THREADS-3], thread_attr, (void *(*)(void *))mutexTestTwo, NULL)) {
        printf("Error Creating Thread %li\n", NUM_THREADS-3);
    }
    if (my_pthread_create(&thr_list[NUM_THREADS-2], thread_attr, (void *(*)(void *))noMutexTestOne, NULL)) {
        printf("Error Creating Thread %li\n", NUM_THREADS-2);
    }
    if (my_pthread_create(&thr_list[NUM_THREADS-1], thread_attr, (void *(*)(void *))noMutexTestTwo, NULL)) {
        printf("Error Creating Thread %li\n", NUM_THREADS-1);
    }
     
     my_pthread_t *p1 = (my_pthread_t*)malloc(sizeof(my_pthread_t));
     my_pthread_create(p1, thread_attr, (void *(*)(void *))noMutexTestTwo, NULL);
     
    if(p1!=NULL){
       printf("Calling the join function in test bench\n");
       printf("The thread id is %ld\n",p1->thr_id);
       void **ptr = NULL;
       my_pthread_join(p1, ptr);
    }
   
}


int main() {
    
    //	Code to test queue class
    test_normal_bench();
    while(1);
    
    return 0;
}

#endif
