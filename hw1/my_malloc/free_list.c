#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "my_malloc.h"

typedef struct free_list_t{
    size_t size;
    struct space_t* next;
    struct space_t* prev;
}free_list_t;

typedef struct free_list_t free_list;

space* head = NULL;
free_list head_free_list = NULL;

void *ff_malloc (size_t size){
  space * block;
  free_list free_block;
  if(head == NULL){
    //request
      block = request_space(NULL, size);
      head = block;
  }
  else{
    space * previous = head;
    block = find_space(&previous, size);
    //printf("\nprevious is %p\n\n",previous);
    //printf("\nblock is %p\n\n",block);
    //no such space, request new one
    if(block == NULL){
      block = request_space(previous, size);
      //request failure
      //printf("\n 1 new block from is %p\n\n",block);
    }
    //free space available
    else{
      if(block -> size - size > sizeof(space) + 1){
          block = split_space(block, size);
      }
    }
  }
  block->free = 0;
  //printf("block header is %p\n", block);
  //printf("block size is %ld\n", block->size);
  //printf("avaliable space start from %p\n", block + 1);
  return(block + 1);
}
