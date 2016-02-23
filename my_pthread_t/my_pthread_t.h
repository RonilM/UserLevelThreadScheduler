//
//  my_pthread_t.h
//  my_pthread_t
//
//  Created by Ronil Mehta on 16/02/16.
//  Copyright (c) 2016 Ronil Mehta. All rights reserved.
//

#ifndef my_pthread_t_my_pthread_t_h

#define my_pthread_t_my_pthread_t_h

#define MAX_THREAD_COUNT 10
#define THREAD_STACK (1024*1024)
#define THREAD_POOL_SATURATED_RETURN_VALUE 1;
#define MALLOC_ERROR 2;


#include <ucontext.h>

typedef struct {

    int id;
    void* stack;
    ucontext_t context;
    int isFinished;
    int isCleaned;

} my_pthread_t;



//Creates a pthread that executes function. Attributes are ignored.
extern int my_pthread_create(my_pthread_t * thread, void * attr, void (*function)(void), void * arg);

//Explicit call to the my_pthread_t scheduler requesting that the current context be swapped out and another be scheduled.
extern void my_pthread_yield();

//Explicit call to the my_pthread_t library to end the pthread that called it. If the value_ptr isn't NULL, any return value from the thread will be saved.
extern void my_pthread_exit(void *value_ptr);

//Call to the my_pthread_t library ensuring that the calling thread will not  execute until the one it references exits. If value_ptr is not null, the return value of the  exiting thread will be passed back.
int my_pthread_join(my_pthread_t * thread, void **value_ptr);

extern void thread_start(void (*t_func)(void));

extern void init_threads();

extern void deepCopyThreads(my_pthread_t *t1,my_pthread_t *t2);


#endif
