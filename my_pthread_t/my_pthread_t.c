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

my_pthread_t thread_queue[MAX_THREAD_COUNT];
static int nextQueueIndex = 0;
static int currentThread = -1;
static int inMainThread = 1;
static ucontext_t main_context;
static int id = 0;

void changeContext(int signum);
void scheduler();

void thread_start(void (*t_func)(void)){
    t_func();
    thread_queue[currentThread].isFinished = 1;
    my_pthread_yield();
}

void my_pthread_yield(){

    //If not in main thread
    if (inMainThread == 0) {
        
        //printf("Entered thread flow\n");
        
        inMainThread = 1;
        swapcontext(&thread_queue[currentThread].context, &main_context );
        inMainThread = 0;
        return;
    }
    else{
        
        scheduler();
    
    }
}


void scheduler(){
    
    //printf("Entered main flow!\n");
    
    if(currentThread >=0 && thread_queue[currentThread].isFinished == 1){
        printf("Deleting thread with id %d\n",thread_queue[currentThread].id);
        nextQueueIndex--;
        free( thread_queue[currentThread].stack );
        thread_queue[currentThread] = thread_queue[nextQueueIndex];
        
    }
    
    if(nextQueueIndex <= 0){
        return;
    }
    
    currentThread = (currentThread+1)%MAX_THREAD_COUNT;
    
    if(currentThread >= nextQueueIndex){
        currentThread = 0;
    }
    
    inMainThread = 0;
    signal(SIGALRM, changeContext);
    alarm(2);
    swapcontext(&main_context, &thread_queue[currentThread].context);
    inMainThread = 1;
    
}

void changeContext(int signum){
    
    swapcontext(&thread_queue[currentThread].context,&main_context);

}

int my_pthread_create(my_pthread_t * thread, void * attr, void (*function)(void), void * arg){
    //printf("start creation!\n");
    
    if(nextQueueIndex == MAX_THREAD_COUNT) return THREAD_POOL_SATURATED_RETURN_VALUE;
    
    //printf("about to getContext\n");
    thread_queue[nextQueueIndex].isFinished = 0;
    getcontext(&thread_queue[nextQueueIndex].context);
    //printf("GOTContext\n");
    thread_queue[nextQueueIndex].context.uc_link = 0;
    thread_queue[nextQueueIndex].stack = malloc( THREAD_STACK );
    //printf("Did malloc \n");
    thread_queue[nextQueueIndex].context.uc_stack.ss_sp = thread_queue[nextQueueIndex].stack;
    thread_queue[nextQueueIndex].context.uc_stack.ss_size = THREAD_STACK;
    thread_queue[nextQueueIndex].context.uc_stack.ss_flags = 0;
    
    if ( thread_queue[nextQueueIndex].stack == 0 )
    {
        printf( "Error: Could not allocate stack.\n" );
        return MALLOC_ERROR;
    }
    //printf("about to add address to queue\n");
    thread_queue[nextQueueIndex].id = id;
    thread->id = id;
    id++;
    
    //printf("about to makeContext\n");
    makecontext( &thread_queue[ nextQueueIndex ].context, (void (*)(void)) &thread_start, 1, function );
    nextQueueIndex++;
    return 0;
}



void init_threads()
{
    int i;
    for ( i = 0; i < MAX_THREAD_COUNT; ++ i )
    {
        thread_queue[i].isFinished = 0;
        thread_queue[i].id = -999;
    }
    
    return;
}

int my_pthread_join(my_pthread_t * thread, void **value_ptr){
    int isFinished = 0;
    while (1) {
        isFinished = 1;
        int i;
        for(i = 0; i < nextQueueIndex; i++){
            if(thread_queue[i].id == thread->id){
                isFinished = 0;
                break;
            }
        }
        
        if(isFinished)
            return 0;
       else
           my_pthread_yield();
    
    }
}

