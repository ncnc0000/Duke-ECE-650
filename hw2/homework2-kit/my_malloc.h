#ifndef __MY_MALLOC_H__
#define __MY_MALLOC_H__
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include<pthread.h>
#include <unistd.h>

typedef struct space_t{
    size_t size;
    int free;
    struct space_t* next;
    struct space_t* prev;
    pthread_t thread_id;
}space_t;

typedef struct space_t space;

__thread space * head_nolock=NULL;
__thread space * tail_nolock = NULL;
pthread_mutex_t lock_lock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_nolock=PTHREAD_MUTEX_INITIALIZER;
space* head = NULL;
space* tail = NULL;

space* merge_blocks(space * previous);
space * find_BF_space(size_t size);
space* request_space(size_t size);
void *bf_malloc(size_t size);
void ff_free(void * ptr);
space * split_space(space * splited_block, size_t size);
space * find_BF_space_nolock(size_t size);
space * request_space_nolock(size_t size);


void *ts_malloc_lock(size_t size);
void ts_free_lock(void *ptr);
void *ts_malloc_nolock(size_t size);
void ts_free_nolock(void *ptr);
#endif