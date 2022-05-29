#include <unistd.h>
#include <mutex>
#include "memory.hpp"
#include <iostream>
#include <string.h>

using namespace std;

pthread_mutex_t  my_second_lock = PTHREAD_MUTEX_INITIALIZER;


memory_block * head = NULL;

void * my_malloc(size_t size){
    pthread_mutex_lock(&my_second_lock);
    void * res = malloc_imple(size);
    pthread_mutex_unlock(&my_second_lock);
    return res;
}

void * malloc_imple(size_t size){
    if(size == 0){
        throw runtime_error("cant allocate- size is 0");
        return NULL;
    }
    intptr_t * program_break = (intptr_t *) sbrk(0); //get the current program break of the data segment memory
    memory_block ** temp = &head; 
    /*
        we will iterate through our previous memory allocations and check if there is
        an available resess. if there isn`t we will allocate new adress. 
    */
    while((*temp) != NULL){
       if((*temp)-> memory_size >= size){  // if the block is big enough
           if((*temp)-> is_free != 0){ // if the block is free
               (*temp)->is_free = 0; //mark that this block is taken
               return ((*temp) + 1); // return the block resess
           }
           //the block is not suitable so we will try the next block
           temp = &((*temp) -> next);
       }
    }
    //there is no suitable block so we will allocate a new one
    if(sbrk(size + sizeof(struct memory_block)) == (void *) -1){
       throw runtime_error("couldnt allocate memory");
       return NULL;
    }
    //else
    memory_block * res = (memory_block*) program_break;
    res-> is_free = 0; //block is taken
    res-> memory_size = size; //update the size of the block
    res-> next = head; //add the block to the list
    head = res;
    return res + 1;
}

void * my_calloc (size_t num_of_items, size_t size_of_item){
    pthread_mutex_lock(&my_second_lock);
    void * res = calloc_imple(num_of_items, size_of_item);
    pthread_mutex_unlock(&my_second_lock);
    return res;
}

void * calloc_imple(size_t num_of_items, size_t size_of_item){
    size_t size = num_of_items * size_of_item; //calculate the size of the needed block
    void * res = my_malloc(size); //use malloc to allocate the memory
    if (res == NULL){
        throw runtime_error("couldnt calloc the memory"); //the malloc didnt work, throw exeption
        return NULL;
    }
    memset(res, 0, size); 
    return res;
}

void my_free(void *ptr){
    pthread_mutex_lock(&my_second_lock);
    if(ptr == NULL){
        pthread_mutex_unlock(&my_second_lock);
        throw runtime_error("the pointer is null");
    }
    free_imple(ptr);
    pthread_mutex_unlock(&my_second_lock);
}

void free_imple(void *ptr){
    memory_block * block_to_free = (memory_block*)ptr -1;
    if((block_to_free ->is_free) != 0){
        throw runtime_error("this block is already free");
    }
    /*
    we want to check if the block is in our list of blocks
    if he doesnt we will throw exeption
    */
    int is_in_the_list = 0;
    memory_block ** temp = &head;
    while((*temp) != NULL){
        if((*temp) == block_to_free){ // if temp is the wanted block
            is_in_the_list = 1; //so we know the block is on the list
            break; //no need to continue, we found the block
        }
        //else
        temp = &((*temp) ->next); //search in the next block
    }
    if(is_in_the_list == 0){
        throw runtime_error("the block is not exists"); //we couldnt find the block so we cant free him
    }
    //if we didnt throw exeption utill now everithing is okay and we can free the block
    block_to_free->is_free == 1;
}

int main(){

    size_t size = 10;
    void *ptr = my_malloc(size);
    cout << &ptr << endl;
    my_free(ptr);
}

