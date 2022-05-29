#include <stdio.h>
#include <stdlib.h>


typedef struct memory_block{

    size_t memory_size; //size of the block
    int is_free; //0 if is not free at all
    memory_block* next;
}memory_block;

void * my_malloc(size_t size); //size of memory to allocate in bytes
void * malloc_imple(size_t size); // same as above
void * my_calloc (size_t num_of_items, size_t size_of_item); 
void * calloc_imple(size_t num_of_items, size_t size_of_item);
void my_free(void *ptr); //pointer to the memory block
void free_imple(void *ptr);