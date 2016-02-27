//
//  main.c
//  my_pthread_t
//
//  Created by Ronil Mehta on 16/02/16.
//  Copyright (c) 2016 Ronil Mehta. All rights reserved.
//

#ifdef __APPLE__
#define _XOPEN_SOURCE
#endif

#include <stdio.h>
#include "my_pthread_t.h"
#include <unistd.h>


void threadfunc1(){
    int i;
    for(i = 0; i < 8 ; i++){
        sleep(1);
        printf("ThreadFunc1\n");}

}

void threadfunc2(){
    int i;
    for(i = 0; i < 6 ; i++){
        sleep(1);
        printf("ThreadFunc2\n");
    
    }
    
}

void threadfunc3(){
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
    
}


int main(int argc, const char * argv[]) {
    
    my_pthread_t thread1,thread2,thread3;
    init_threads();
    my_pthread_create(&thread1, NULL, &threadfunc1,NULL);
    my_pthread_create(&thread2, NULL, &threadfunc2,NULL);
    my_pthread_create(&thread3, NULL, &threadfunc3,NULL);
    
    my_pthread_join(&thread2,NULL);
    my_pthread_join(&thread1,NULL);
    my_pthread_join(&thread3,NULL);
    
    printf("Ending main!\n");
    
    return 0;
}

