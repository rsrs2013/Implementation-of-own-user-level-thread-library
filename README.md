# Implementation-of-own-user-level-thread-library
In this assignment we had implemented the Linux pthread library, 
which will require that we also write a scheduler to manaully switch between your various threads. 
Since we will be dealing with multiple threads, we will also need to implement pthread mutexes, 
mutual exclusion devices that keep a thread locked if it is waiting for a particular mutex.
This assignment is intended to illustrate the mechanics and difficulties of scheduling taks within an operating system.

"my_pthread_t.h" that contains implementations of:
Pthread note: The internal implementation of pthreads has a running and waiting queue.
Pthreads that are waiting for a mutex are moved to the waiting queue.
Threads that can be scheduled to run should be in the running queue.
int my_pthread_create( pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg);
Creates a pthread that executes function. Attributes are ignored.

void my_pthread_yield();
Explicit call to the my_pthread_t scheduler requesting that the current context be swapped out and another be scheduled.

void pthread_exit(void *value_ptr);
Explicit call to the my_pthread_t library to end the pthread that called it.
If the value_ptr isn't NULL, any return value from the thread will be saved.

int my_pthread_join(pthread_t thread, void **value_ptr);
Call to the my_pthread_t library ensuring that the calling thread will not execute until the one it references exits. 
If value_ptr is not null, the return value of the exiting thread will be passed back.
Mutex note: Both the unlock and lock functions should be very fast. If there are any threads that are meant to compete for these functions, my_pthread_yield should be called immediately after running the function in question. Relying on the internal timing will make the function run slower than using yield.

int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr);
Initializes a my_pthread_mutex_t created by the calling thread. Attributes are ignored.

int my_pthread_mutex_lock(my_pthread_mutex_t *mutex);
Locks a given mutex, other threads attempting to access this mutex will not run until it is unlocked.

int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex);
Unlocks a given mutex.

int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex);
Destroys a given mutex. Mutex should be unlocked before doing so.
