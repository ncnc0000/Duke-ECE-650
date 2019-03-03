#include "my_malloc.h"

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
  tail = block_header;
  block_header->free = 0;
  block_header->size = size;
  block_header->next = NULL;
  return block_header;
}

void *bf_malloc(size_t size){
    space * block;
    if(head == NULL){
        //pthread_mutex_lock(&lock_lock);
        block = request_space(size);
        head = block;
        //pthread_mutex_unlock(&lock_lock);
    }
    else{
        block = find_BF_space(size);
        if(block == NULL){
            //pthread_mutex_lock(&lock_lock);
            block = request_space(size);
            //pthread_mutex_unlock(&lock_lock);
        }
        else{
            //pthread_mutex_lock(&lock_lock1);
            if(block -> size - size > sizeof(space) + 1){
                block = split_space(block, size);
            }
            //pthread_mutex_unlock(&lock_lock1);
        }
    }
    block->free = 0;
    return (block + 1);
}
void ff_free(void * ptr){
  space * free_block = (space*)ptr - 1;
  //pthread_mutex_lock(&lock_lock1);
  free_block->free = 1;
  if((free_block->prev != NULL) && (free_block->prev->free == 1)){
    free_block = merge_blocks(free_block->prev);
  }
  if((free_block->next != NULL) && (free_block->next->free == 1)){
    free_block = merge_blocks(free_block);
  }
  free_block->free = 1;
  // pthread_mutex_unlock(&lock_lock1);
}


void *ts_malloc_lock(size_t size){
    pthread_mutex_lock(&lock_lock);
    void *block = bf_malloc(size);
    pthread_mutex_unlock(&lock_lock);
    return block;
}

void ts_free_lock(void *ptr){
    pthread_mutex_lock(&lock_lock);
    ff_free(ptr);
    pthread_mutex_unlock(&lock_lock);
}



space * split_space(space * splited_block, size_t size){
    // printf("current size %ld, required size is %ld,  and address %p\n ", splited_block->size, size, splited_block);
    space * block_header = (void*)splited_block + size + sizeof(space);
    //printf("now address is %p \n", block_header);
    block_header->free = 1;
    block_header->size = splited_block->size - size - sizeof(space);
    //printf("%s\n", );("new block size  %ld \n", block_header->size);
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

space* merge_blocks(space * previous){
  if (previous->next != NULL && previous->next->free == 1 ){
      previous->next->free=0;
      previous->size += sizeof(space) + previous->next->size;
      previous->next = previous->next->next;
      if(previous->next != NULL){
          previous->next->prev = previous;
      }
  }
  return previous;
}

space* find_BF_space_nolock(size_t size){
    space* current=head_nolock;
    space* BF_space=NULL;
    while(current!=NULL){
        if(current->free && current->size>=size){
            if(BF_space==NULL || current->size<BF_space->size){
                BF_space=current;
            }
        }
        current=current->next;
    }
    return BF_space;
}

space* request_space_nolock(size_t size){
	pthread_mutex_lock(&lock_nolock);
	space* block_header=sbrk(0);
	if(sbrk(sizeof(space)+size)==(void *) -1){
		pthread_mutex_unlock(&lock_nolock);
		return NULL;
	}
	pthread_mutex_unlock(&lock_nolock);
    if(tail_nolock != NULL){
        block_header->prev = tail_nolock;
        tail_nolock->next = block_header;
    }
    else{
        block_header->prev = NULL;
    }
    tail_nolock = block_header;
    block_header->size = size;
	block_header->next=NULL;
	block_header->free=0;
	return block_header;
}

void *ts_malloc_nolock(size_t size){
	pthread_t temp_id=pthread_self();
    space* block;
	if(head_nolock==NULL){
		block=request_space_nolock(size);
        head_nolock=block;
	}
	else{
		block=find_BF_space_nolock(size);
		if(block==NULL){
			block=request_space_nolock(size);
		}
		else{
			block->free=0;
			if(block->size>=size+sizeof(space)+1){
				block = split_space(block, size);
			}
		}
	}
	block->thread_id=temp_id;
	return block+1;
}

void ts_free_nolock(void *ptr){
	if(head_nolock!=NULL && ptr!=NULL && (space*)ptr>head_nolock){
		space * free_block = (space*)ptr - 1;
		free_block->free=1;
		if(free_block->thread_id==pthread_self()){
			if(free_block->prev && free_block->prev->free){
				if((char *)(free_block->prev+1)+free_block->prev->size == (char *)free_block){
					free_block=merge_blocks(free_block->prev);
				}
			}
			if(free_block->next && free_block->next->free){
				if((char *)(free_block+1)+free_block->size ==(char *) free_block->next){
					merge_blocks(free_block);
				}
			}
		}
	}
}
