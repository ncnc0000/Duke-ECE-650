#ifndef __MY_MALLOC__
#define __MY_MALLOC__

typedef struct space_t{
    size_t size;
    int free;
    struct space_t* next;
    struct space_t* prev;
}space_t;

typedef struct space_t space;

//space * find_space(space ** previous, size_t size);

void *ff_malloc (size_t size);
void ff_free (void *ptr);
void *bf_malloc (size_t size);
void bf_free (void *ptr);
unsigned long get_data_segment_size();
unsigned long get_data_segment_free_space_size();
#endif
