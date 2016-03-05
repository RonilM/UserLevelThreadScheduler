/*
*  main.c
*  my_pthread_t
*
*  Main function of the program which calls
*  my_pthread_t functions and scheduler to create threads,
*  mutexes and schedule threads
*  
*  Created by Ronil Mehta on 16/02/16.
*  Copyright (c) 2016 Ronil Mehta. All rights reserved.
*  Authors: Ronil Mehta (rvm41),
*           Saurabh Deochake (srd117),
*           Niraj Dholakia (nd387)
*/

#ifdef __APPLE__
#define _XOPEN_SOURCE
#endif

#include <stdio.h>
#include "my_pthread_t.h"
#include <unistd.h>

my_pthread_mutex_t mutex;

/* Name:     threadfunc1
 * Input:    None
 * Output:   None returned
 * Function: Print a statement for certain amount of times
 **/
void threadfunc1(){
    //my_pthread_mutex_lock(&mutex);
    int i;
    for(i = 0; i < 8 ; i++){
        sleep(1);
        printf("ThreadFunc1\n");}
    //my_pthread_mutex_unlock(&mutex);
}

/* Name:     threadfunc2
 * Input:    None
 * Output:   None returned
 * Function: Print a statement for certain amount of times
 **/
void threadfunc2(){
    //my_pthread_mutex_lock(&mutex);
    int i;
    for(i = 0; i < 6 ; i++){
        sleep(1);
        printf("ThreadFunc2\n");
    }
    //my_pthread_mutex_unlock(&mutex);
}

/* Name:     threadfunc3
 * Input:    None
 * Output:   None returned
 * Function: Print a statement for certain amount of times
 **/
void threadfunc3(){
    //my_pthread_mutex_lock(&mutex);
    int i;
    for(i = 0; i < 2 ; i++){
        sleep(1);
        printf("ThreadFunc3\n");
        
    }
    for(i = 0; i < 4 ; i++){
        my_pthread_yield();
        sleep(0.1);
        printf("POST YIELD: ThreadFunc3\n");
        
    }
    //my_pthread_mutex_unlock(&mutex);
}

/* Name:     main
 * Input:    (Consult Makefile)
 * Output:   0 on success
 * Function: Entry point of the program. The function
 *           calls the functions from my_pthread_t library
 *           to create, destroy the threads, schedule them
 *           and use mutexes over the threads.
 **/
int main(int argc, const char * argv[]) {
    
    my_pthread_t thread1,thread2,thread3;
    init_threads();
    
    /* Initialize the mutex variable.
     * This will reset the in-built count
     * variable to 0 making it available to threads.
     **/
    my_pthread_mutex_init(&mutex);
    
    //Create threads
    my_pthread_create(&thread1, NULL, &threadfunc1,NULL);
    my_pthread_create(&thread2, NULL, &threadfunc2,NULL);
    my_pthread_create(&thread3, NULL, &threadfunc3,NULL);
    
    //Call join on the threads one by one
    my_pthread_join(&thread2,NULL);
    my_pthread_join(&thread1,NULL);
    my_pthread_join(&thread3,NULL);
    
    /* Destroying the mutex variable.
     * Before destroying the mutex variable, we make sure
     * that we unlock it by internally calling the unlock.
     **/
    my_pthread_mutex_destroy(&mutex);
    
    printf("Ending main!\n");
    
    return 0;
}

