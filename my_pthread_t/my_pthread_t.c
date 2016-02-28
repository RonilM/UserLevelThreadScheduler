//
//  my_pthread_t.c
//  my_pthread_t
//
//  Created by Ronil Mehta on 18/02/16.
//  Copyright (c) 2016 Ronil Mehta. All rights reserved.
//  Authors: Ronil Mehta (rvm41), Saurabh Deochake (srd117)

#ifdef __APPLE__
#define _XOPEN_SOURCE
#endif

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

#include <stdio.h>
#include <inttypes.h>
#include "my_pthread_t.h"
#include <ucontext.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include "my_queue.h"



//struct Queue *queue1;

struct Queue queue[MAX_QUEUE_COUNT];
static int currentQueue = 0;
static int nextQueueIndex = 0;
static my_pthread_t *currentThread = 0;
static int inMainThread = 1;
static ucontext_t main_context;
static int id = 1;
static int cycle_counter = 1;

void changeContext(int signum);
void scheduler();
long getCurrentTimestamp();
void setAlarm(int seconds,suseconds_t microseconds);


void init_threads()
{
    //queue1 = malloc(sizeof(struct Queue));
    /*int i;
     for ( i = 0; i < MAX_QUEUE_COUNT; ++ i )
     {
        
     }
    
    return;*/
}


void thread_start(void (*t_func)(void)){
    t_func();
    currentThread->isFinished = 1;
    my_pthread_yield();
}

void my_pthread_yield(){
    long currentTimestamp = getCurrentTimestamp();
    setAlarm(0,0);
    //alarm(0);
    if (inMainThread == 0) {

        currentThread->timeSpent = currentThread->timeSpent + currentTimestamp - currentThread->startTimestamp;
        
        inMainThread = 1;
        swapcontext(&currentThread->context, &main_context );
        inMainThread = 0;
        
        return;
    }
    else{
        
        cycle_counter++;
        cycle_counter = (cycle_counter%SCAN_INTERVAL_COUNT);
        if(cycle_counter == 0){
            scanSchedulerQueues();
        }
        
        scheduler();
        
    }
}


void scheduler(){
    
    
    if(currentThread!= 0 && currentThread->isFinished == 1){
        
        printf("Cleaning thread with id %d\n",currentThread->id);
        
        my_pthread_t *temp;
        removeElementFromQueue(&queue[currentQueue],&temp);
        free(temp->stack);
        temp->isCleaned = 1;
    }
    else{
        //printf("Round robin Sched!\n");
        my_pthread_t *temp;
        removeElementFromQueue(&queue[currentQueue],&temp);
        
        int nextQueue = (currentQueue + 1)%MAX_QUEUE_COUNT;
        if (temp->timeSpent < FIRST_QUEUE_QUANTA) {
            //printf("Shifting to thread%d to same queue\n",temp->id);
            nextQueue = currentQueue;
        }
        else{
            currentThread->timeSpent = 0;
        }
        
        if(nextQueue == 0){
            addElementToQueue(temp, &queue[currentQueue]);
        }
        else{
            addElementToQueue(temp, &queue[nextQueue]);
        }
                                                                          
    }
    if(queue[currentQueue].head == 0){
        int tempCurrentQueue = currentQueue;
        currentQueue = (currentQueue+1)%MAX_QUEUE_COUNT;
        while(queue[currentQueue].head == 0 && currentQueue != tempCurrentQueue){
            currentQueue = (currentQueue+1)%MAX_QUEUE_COUNT;
        }
        if(currentQueue == tempCurrentQueue){
            printf("Queues are empty\n");
            return;
        }
        //currentThread = 0;
        //return;
    }
    currentThread = queue[currentQueue].head->thread;
    

    long quantumAllocation = FIRST_QUEUE_QUANTA - currentThread->timeSpent;
    if (quantumAllocation <= 0) {
        printf("QA is 1000\n");
        quantumAllocation = 1000;
    }
    //printf("Q.A. for thread%d is %ld\n bcause timeSpnt is %ld\n",currentThread->id,quantumAllocation,currentThread->timeSpent);
    inMainThread = 0;
    signal(SIGALRM, changeContext);
    currentThread->startTimestamp = getCurrentTimestamp();
    //alarm(quantumAllocation);
    setAlarm((int)(quantumAllocation/1000),quantumAllocation%1000);
    swapcontext(&main_context,&currentThread->context);
    inMainThread = 1;
    
}

void changeContext(int signum){
    currentThread->timeSpent = FIRST_QUEUE_QUANTA;
    swapcontext(&currentThread->context,&main_context);

}

int my_pthread_create(my_pthread_t * thread, void * attr, void (*function)(void), void * arg){
    //printf("start creation!\n");
    //if(nextQueueIndex == MAX_THREAD_COUNT) return THREAD_POOL_SATURATED_RETURN_VALUE;

    currentQueue = 0;

    thread->isFinished = 0;
    thread->isCleaned = 0;
    thread->startTimestamp = 0;
    thread->timeSpent = 0;
    getcontext(&(thread->context));
    thread->context.uc_link = 0;
    thread->stack = malloc( THREAD_STACK );
    thread->context.uc_stack.ss_sp = thread->stack;
    thread->context.uc_stack.ss_size = THREAD_STACK;
    thread->context.uc_stack.ss_flags = 0;
    
    if ( thread->stack == 0 )
    {
        printf( "Error: Could not allocate stack.\n" );
        return MALLOC_ERROR;
    }
    thread->id = id;
    id++;
    addElementToQueue(thread, &queue[currentQueue]);
    makecontext( &thread->context, (void (*)(void)) &thread_start, 1, function );
    return 0;
}




int my_pthread_join(my_pthread_t * thread, void **value_ptr){
    while (1) {
        if(thread->isCleaned == 1){
            printf("Ending threadid %d\n",thread->id);
            return 0;
        }
        else{
            my_pthread_yield();
        }
    
    }
}

void deepCopyThreads(my_pthread_t *t1,my_pthread_t *t2){

    t1->id = t2->id;
    t1->context = t2->context;
    t1->isFinished = t2->isFinished;
    t1->stack = t2->stack;
    
}

void scanSchedulerQueues(){
    printf("Move all jobs to Topmost queue\n");
    currentQueue = 0;
    struct Queue *primary = &queue[0];
    int i;
    for(i = 1; i < MAX_QUEUE_COUNT ; i++){
        while(queue[i].head != 0){
            my_pthread_t *temp;
            removeElementFromQueue(&queue[i], &temp);
            addElementToQueue(temp, &queue[0]);
        }
    }

}

long getCurrentTimestamp(){

    struct timespec spec;
#ifdef __MACH__ // OS X does not have clock_gettime, using clock_get_time
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    spec.tv_sec = mts.tv_sec;
    spec.tv_nsec = mts.tv_nsec;
#else
    clock_gettime(CLOCK_REALTIME, &spec);
#endif
    
    return spec.tv_nsec;


}

void setAlarm(int seconds,suseconds_t microseconds){
    
    
    struct itimerval tout_val;
    
    //if(seconds == 0 && microseconds == 0){
        //tout_val.it_value = 0;
    //}else{
        tout_val.it_interval.tv_sec = 0;
        tout_val.it_interval.tv_usec = 0;
        tout_val.it_value.tv_sec = seconds; /* set timer for "INTERVAL (10) seconds */
        tout_val.it_value.tv_usec = microseconds;
    //}
    setitimer(ITIMER_REAL, &tout_val,0);
    
    
}

/* Name: my_pthread_mutex_init
 * Input:    mutex structure pointer
 * Output:   return on success
 * Function: Initialize the "mutex" structure and allocate
 *           the memory. Set the lock variable to 0.
 **/
int my_pthread_mutex_init(my_pthread_mutex_t *mutex){
    
    mutex = (my_pthread_mutex_t *) malloc(sizeof(my_pthread_mutex_t));
    mutex -> isLocked = 0;
    return(EXIT_SUCCESS);
}

/* Name: my_pthread_mutex_lock
 * Input:    mutex structure pointer
 * Output:   return on success
 * Function: If mutex is not locked, set the
 *           lock variable to 0. If not, call
 *           yeild function until mutex is available.
 **/
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex){
    if (mutex == NULL){
        my_pthread_mutex_init(mutex);
    }
    
    //check if the mutex is already locked
    while(1){
        if (mutex -> isLocked){
        my_pthread_yield();
        }
    //when it is not, set lock variable to 1
        else{
            mutex -> isLocked = 1;
            return(EXIT_SUCCESS);
        }
    }
}

/* Name:     my_pthread_mutex_unlock
 * Input:    mutex structure pointer
 * Output:   return on success
 * Function: If mutex is locked, set the lock
 *           value to 0. Return on success.  
 **/

int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex){  
    if (mutex == NULL){
        my_pthread_mutex_init(mutex);
    }
    
    // If mutex is locked, set the lock variable to 0.
    if (mutex-> isLocked){
        mutex-> isLocked = 0;    
    }
    return(EXIT_SUCCESS);
}

/* Name:     my_pthread_mutex_destroy
 * Input:    mutex structure pointer
 * Output:   return on success
 * Function: Free up the mutex variable. 
 *           **WARNING**
 *           Before you destroy the mutex varible,
 *           make sure you unlock it.
 **/

int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex){
    // Unlock the mutex variable before freeing up.
    my_pthread_mutex_unlock(mutex);
    mutex = NULL;
    
    return(EXIT_SUCCESS);
}
