//
//  my_pthread_t.c
//  my_pthread_t
//
//  Created by Ronil Mehta on 18/02/16.
//  Copyright (c) 2016 Ronil Mehta. All rights reserved.
//

#ifdef __APPLE__
#define _XOPEN_SOURCE
#endif

#include <stdio.h>
#include "my_pthread_t.h"
#include <ucontext.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "my_queue.h"



//struct Queue *queue1;

struct Queue queue[MAX_QUEUE_COUNT];
static int currentQueue = 0;
static int nextQueueIndex = 0;
static my_pthread_t *currentThread = 0;
static int inMainThread = 1;
static ucontext_t main_context;
static int id = 1;

void changeContext(int signum);
void scheduler();




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

    //If not in main thread
    if (inMainThread == 0) {
        
        //printf("Entered thread flow\n");
        
        inMainThread = 1;
        swapcontext(&currentThread->context, &main_context );
        inMainThread = 0;
        return;
    }
    else{
        //printf("scheduler()\n");
        scheduler();
    
    }
}


void scheduler(){
    
    //printf("Entered scheduler flow!\n");
    
    
    if(currentThread!= 0 && currentThread->isFinished == 1){
        
        printf("Cleaning thread with id %d\n",currentThread->id);
        
        my_pthread_t *temp;
        removeElementFromQueue(&queue[currentQueue],&temp);
        free(temp->stack);
        temp->isCleaned = 1;
    }
    else{
        my_pthread_t *temp;
        removeElementFromQueue(&queue[currentQueue],&temp);
        addElementToQueue(temp, &queue[currentQueue]);
    
    }
    if(queue[0].head == 0){
        printf("Queue is empty\n");
        currentThread = 0;
        return;
    }
    currentThread = queue[currentQueue].head->thread;
    inMainThread = 0;
    signal(SIGALRM, changeContext);
    alarm(FIRST_QUEUE_QUANTA);
    swapcontext(&main_context,&currentThread->context);
    inMainThread = 1;
    
}

void changeContext(int signum){
    
    swapcontext(&currentThread->context,&main_context);

}

int my_pthread_create(my_pthread_t * thread, void * attr, void (*function)(void), void * arg){
    //printf("start creation!\n");
    
    if(nextQueueIndex == MAX_THREAD_COUNT) return THREAD_POOL_SATURATED_RETURN_VALUE;
    
    thread->isFinished = 0;
    thread->isCleaned = 0;
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
    addElementToQueue(thread, &queue[0]);
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




