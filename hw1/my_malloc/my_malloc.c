#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "my_malloc.h"
size_t size_allocted;
size_t size_freed;
space* head = NULL;
space* tail = NULL;
space * find_space(size_t size){
  if (head == NULL) {
      return NULL;
  }
  space * current_location = head;
  while (current_location != NULL) {
      if (current_location -> free && current_location -> size >= size) {
          return current_location;
      }
      // *previous = current_location;
      current_location = current_location -> next;
    }
    return NULL;
}

space* request_space(size_t size){
  space * block_header = sbrk(0);
  // printf("\n sbrk results is %p\n\n", block_header);
  void *requested_space = sbrk(size + sizeof(space));
  //if sbrk fail, return NULL
  if (requested_space == (void*) -1) {
    return NULL; // sbrk failed.
  }
  // if not first request, make previous block point to new block
  if(tail != NULL){
      block_header->prev = tail;
      tail->next = block_header;
    //block_header->prev = previous;
  }
  else{
      block_header->prev = NULL;
  }
//block_header->prev = tail;
  tail = block_header;
  block_header->free = 0;
  block_header->size = size;
  block_header->next = NULL;
  size_allocted += block_header->size;
  return block_header;
}

space * split_space(space * splited_block, size_t size){
  // printf("current size %ld, required size is %ld,  and address %p\n ", splited_block->size, size, splited_block);
  space * block_header = (void*)splited_block + size + sizeof(space);
  //printf("now address is %p \n", block_header);
  block_header->free = 1;
  block_header->size = splited_block->size - size - sizeof(space);
  //printf("new block size  %ld \n", block_header->size);
  splited_block->free = 0;
  splited_block->size = size;
  if(splited_block-> next == NULL){
    block_header->next = NULL;
    block_header->prev = splited_block;
    splited_block->next = block_header;
  }
  else{
    block_header->next = splited_block->next;
    block_header->prev = splited_block;
    splited_block->next->prev = block_header;
    splited_block->next = block_header;
  }
  return splited_block;
}


void *ff_malloc (size_t size){
  space * block;
  if(head == NULL){
      block = request_space(size);
      head = block;
  }
  else{
      //space * previous = head;
    block = find_space(size);
    //printf("\nprevious is %p\n\n",previous);
    //printf("\nblock is %p\n\n",block);
    //no such space, request new one
    if(block == NULL){
      block = request_space(size);
      //request failure
      //printf("\n 1 new block from is %p\n\n",block);
    }
    //free space available
    else{
      if(block -> size - size > sizeof(space) + 1){
          block = split_space(block, size);
      }
      size_freed -= block->size;
    }
  }
  block->free = 0;
  //printf("block header is %p\n", block);
  //printf("block size is %ld\n", block->size);
  //printf("avaliable space start from %p\n", block + 1);
  return(block + 1);
}

space* merge_blocks(space * previous){
  if (previous->next != NULL && previous->next->free == 1 ){
      previous->size += sizeof(space) + previous->next->size;
      previous->next = previous->next->next;
      if(previous->next != NULL){
          previous->next->prev = previous;
      }
  }
  return previous;
}

void ff_free(void * ptr){
  //space * free_block;
  //printf("address of space to be freed is %p\n", ptr);
  space * free_block = (space*)ptr - 1;
  //printf("address of space header is %p\n", free_block);
  free_block->free = 1;
  size_freed += free_block->size;
  if((free_block->prev != NULL) && (free_block->prev->free == 1)){
    free_block = merge_blocks(free_block->prev);
  }
  if((free_block->next != NULL) && (free_block->next->free == 1)){
    free_block = merge_blocks(free_block);
  }
  //  printf("NEW Version free\n");
  free_block->free = 1;
}

space * find_BF_space(size_t size){
  space * BF_space = NULL;
  size_t BF_size;
  space * current_location = head;
  while (current_location != NULL) {
    if (current_location -> free && current_location -> size >= size ) {
        if(BF_space == NULL){
            BF_size = current_location->size - size;
            BF_space = current_location;
        }
        if(current_location->size - size == 0){
            return current_location; 
        }
        if (current_location->size - size < BF_size){
            BF_size = current_location->size - size;
            BF_space = current_location;
        }
    }
    current_location = current_location -> next;
  }
  return BF_space;
}

void *bf_malloc(size_t size){
   space * block;
   //   printf("I'm BF_MALLOC");
   if(head == NULL){
     //request new space
       block = request_space(size);
       head = block;
   }
   else{
     //BF, look for best fit space
     //space * previous = head;
     block = find_BF_space(size);
     if(block == NULL){
       block = request_space(size);
     }
     else{
         if(block -> size - size > sizeof(space) + 1){
             block = split_space(block, size);
         }
         size_freed -= block->size;
     }
   }
   block->free = 0;
   return (block + 1);
 }

void bf_free(void * ptr){
  ff_free(ptr);
}

unsigned long get_data_segment_size(){
    /*space * current_location = head;
  unsigned long ans;
  while(current_location -> next != NULL){
    ans += current_location->size +  sizeof(space);
    current_location = current_location->next;
  }*/
  return size_allocted;
}
unsigned long get_data_segment_free_space_size(){
    /*space * current_location = head;
  unsigned long ans;
   while(current_location -> next != NULL){
     if(current_location -> free == 1){
         ans += current_location->size + sizeof(space);
     }
     current_location = current_location->next;
     }*/
   return size_freed;
}
