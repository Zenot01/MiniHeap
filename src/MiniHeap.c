#include "MiniHeap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "custom_unistd.h"




struct memory_manager_t memory_manager;


uint64_t check_sum(void* start_of_chunk){
    uint64_t sum = 0;
    for (size_t i = 0; i < sizeof(struct memory_chunk_t) - sizeof(uint64_t); ++i) {
        sum += *((uint8_t*)start_of_chunk + i);
    }
    return sum;
}



int heap_setup(void){
    memory_manager.memory_start = custom_sbrk(0);
    if (memory_manager.memory_start == (void *)-1) return -1;
    memory_manager.memory_size = 0;
    memory_manager.first_memory_chunk = NULL;
    return 0;
}

void heap_clean(void){
    if (memory_manager.memory_start != NULL)
    {
        void *cur_ptr =(void *) custom_sbrk(0);
        intptr_t dif = (intptr_t)memory_manager.memory_start - (intptr_t)cur_ptr;

        custom_sbrk(dif);
        memory_manager.memory_start = NULL;
        memory_manager.memory_size = 0;
    }
    memory_manager.first_memory_chunk = NULL;
    memory_manager.memory_start = NULL;
    memory_manager.memory_size = 0;
}


void* heap_malloc(size_t size){
    if (size <= 0 || memory_manager.memory_start==NULL) return NULL;


    struct memory_chunk_t *cur = memory_manager.first_memory_chunk;
    while (1)
    {
        if (cur == NULL) break;
        if (cur->free == 1 && cur->full_size - 8 >= size)
        {
            cur->free = 0;
            cur->allocated_size = size;
            *((uint8_t *) cur+ sizeof(struct memory_chunk_t)) = '#';
            *((uint8_t *) cur+ sizeof(struct memory_chunk_t) + 1) = '#';
            *((uint8_t *) cur+ sizeof(struct memory_chunk_t) + 2) = '#';
            *((uint8_t *) cur+ sizeof(struct memory_chunk_t) + 3) = '#';
            *((uint8_t *) cur+ sizeof(struct memory_chunk_t) + size + 4) = '#';
            *((uint8_t *) cur+ sizeof(struct memory_chunk_t) + size + 5) = '#';
            *((uint8_t *) cur+ sizeof(struct memory_chunk_t) + size + 6) = '#';
            *((uint8_t *) cur+ sizeof(struct memory_chunk_t) + size + 7) = '#';

            cur->magic_number = check_sum(cur);
            return ((uint8_t *)cur + sizeof(struct memory_chunk_t) + 4);
        }
        if (cur->next == NULL) break;
        cur = cur->next;
    }

    void *new_block = (void *) custom_sbrk(size + 8 + sizeof(struct memory_chunk_t));
    if (new_block ==(void *) -1) return NULL;
    struct memory_chunk_t *new_chunk = (struct memory_chunk_t *) (new_block);
    new_chunk->free = 0;
    new_chunk->allocated_size = size;
    new_chunk->full_size = size + 8;
    new_chunk->next = NULL;
    if (cur == NULL){
        memory_manager.first_memory_chunk = new_chunk;
        new_chunk->prev = NULL;
    }
    else{
        cur->next= new_chunk;
        new_chunk->prev = cur;
        cur->magic_number = check_sum(cur);
    }
    memory_manager.memory_size += sizeof(struct memory_chunk_t) + new_chunk->full_size;
    *((uint8_t *) new_chunk + sizeof(struct memory_chunk_t)) = '#';
    *((uint8_t *) new_chunk+ sizeof(struct memory_chunk_t) + 1) = '#';
    *((uint8_t *) new_chunk+ sizeof(struct memory_chunk_t) + 2) = '#';
    *((uint8_t *) new_chunk+ sizeof(struct memory_chunk_t) + 3) = '#';

    *((uint8_t *) new_chunk+ sizeof(struct memory_chunk_t) + new_chunk->allocated_size + 4) = '#';
    *((uint8_t *) new_chunk+ sizeof(struct memory_chunk_t) + new_chunk->allocated_size + 5) = '#';
    *((uint8_t *) new_chunk+ sizeof(struct memory_chunk_t) + new_chunk->allocated_size + 6) = '#';
    *((uint8_t *) new_chunk+ sizeof(struct memory_chunk_t) + new_chunk->allocated_size + 7) = '#';
    new_chunk->magic_number = check_sum(new_chunk);
    return ((uint8_t *) new_chunk  + sizeof(struct memory_chunk_t) + 4);
}


size_t   heap_get_largest_used_block_size(void){
    if (memory_manager.first_memory_chunk == NULL || heap_validate() == 1) return 0;

    struct memory_chunk_t *cur = memory_manager.first_memory_chunk;
    size_t max = 0;
    while (1)
    {
        if (cur->allocated_size > max && cur->free == 0) max = cur->allocated_size;

        cur = cur->next;
        if (cur == NULL) return max;
    }
}

enum pointer_type_t get_pointer_type(const void* const pointer){
    if (pointer == NULL) return 0;
    if (heap_validate() == 1) return 1;


    struct memory_chunk_t *cur = memory_manager.first_memory_chunk;
    while (1)
    {
        if (cur == NULL) break;
        if (cur->free == 1){
            if ((uint8_t *)pointer >= (uint8_t *)cur && (uint8_t *)pointer < ((uint8_t *)cur + sizeof(struct memory_chunk_t))) return 2;
            if((uint8_t*)pointer >= ((uint8_t*)cur + sizeof(struct memory_chunk_t)) && (uint8_t*)pointer < ((uint8_t*)cur + sizeof(struct memory_chunk_t) + cur->full_size)) return 5;
        }
        else{
            if ((uint8_t *)pointer >= (uint8_t *)cur && (uint8_t *)pointer < ((uint8_t *)cur + sizeof(struct memory_chunk_t))) return 2;
            if ( (uint8_t*)pointer >=  ((uint8_t *)cur + sizeof(struct memory_chunk_t)) && (uint8_t*)pointer <= ((uint8_t *)cur + sizeof(struct memory_chunk_t) + 3)) return 3;
            if ((uint8_t*)pointer == ((uint8_t*)cur + sizeof(struct memory_chunk_t) + 4)) return 6;

            if ((uint8_t*)pointer >=  ((uint8_t *)cur + sizeof(struct memory_chunk_t) + cur->allocated_size + 4) && (uint8_t*)pointer <= ((uint8_t *)cur + sizeof(struct memory_chunk_t) + cur->allocated_size + 7)) return 3;

            if ((uint8_t*)pointer > ((uint8_t*)cur + sizeof(struct memory_chunk_t) + 4) && (uint8_t*)pointer < ((uint8_t*)cur + sizeof(struct memory_chunk_t) + 4 + cur->allocated_size)) return 4;
        }



        cur = cur->next;

    }

    return 5;
}

int heap_validate(void){
    if (memory_manager.memory_start == NULL) return 2;

    struct memory_chunk_t *cur = memory_manager.first_memory_chunk;
    while (1)
    {
        if (cur == NULL) break;
        uint64_t help_val = check_sum(cur);
        if (cur->magic_number != help_val || cur->allocated_size > cur->full_size ||  (cur->free != 1 && cur->free != 0)) return 3;
        if (cur->free == 0)
        {
            if (*((uint8_t*)cur + sizeof(struct memory_chunk_t)) != '#' ) return 1;
            if (*((uint8_t*)cur + sizeof(struct memory_chunk_t) + 1) != '#' ) return 1;
            if (*((uint8_t*)cur + sizeof(struct memory_chunk_t) + 2) != '#' ) return 1;
            if (*((uint8_t*)cur + sizeof(struct memory_chunk_t) + 3) != '#' ) return 1;

            if (*((uint8_t*)cur + sizeof(struct memory_chunk_t) + cur->allocated_size + 4) != '#' ) return 1;
            if (*((uint8_t*)cur + sizeof(struct memory_chunk_t) + cur->allocated_size + 5) != '#' ) return 1;
            if (*((uint8_t*)cur + sizeof(struct memory_chunk_t) + cur->allocated_size + 6) != '#' ) return 1;
            if (*((uint8_t*)cur + sizeof(struct memory_chunk_t) + cur->allocated_size + 7) != '#' ) return 1;
        }
        cur = cur->next;
    }

    return 0;
}

void* heap_calloc(size_t number, size_t size){
    if (number <= 0 || size <= 0) return NULL;

    size_t size_to_alloc = number * size;

    struct memory_chunk_t *cur = memory_manager.first_memory_chunk;
    while (1)
    {
        if (cur == NULL) break;
        if (cur->free == 1 && cur->full_size - 8 >= size_to_alloc)
        {
            cur->free = 0;
            cur->allocated_size = size_to_alloc;
            *((uint8_t *) cur+ sizeof(struct memory_chunk_t)) = '#';
            *((uint8_t *) cur+ sizeof(struct memory_chunk_t) + 1) = '#';
            *((uint8_t *) cur+ sizeof(struct memory_chunk_t) + 2) = '#';
            *((uint8_t *) cur+ sizeof(struct memory_chunk_t) + 3) = '#';
            for (size_t i = 0; i < size_to_alloc; ++i) {
                *((uint8_t *) cur + sizeof(struct memory_chunk_t) + 4 + i) = 0;
            }
            *((uint8_t *) cur+ sizeof(struct memory_chunk_t) + size_to_alloc + 4) = '#';
            *((uint8_t *) cur+ sizeof(struct memory_chunk_t) + size_to_alloc + 5) = '#';
            *((uint8_t *) cur+ sizeof(struct memory_chunk_t) + size_to_alloc + 6) = '#';
            *((uint8_t *) cur+ sizeof(struct memory_chunk_t) + size_to_alloc + 7) = '#';

            cur->magic_number = check_sum(cur);
            return ((uint8_t *)cur + sizeof(struct memory_chunk_t) + 4);
        }
        if (cur->next == NULL) break;
        cur = cur->next;
    }

    void *new_block = (void *) custom_sbrk(size_to_alloc + 8 + sizeof(struct memory_chunk_t));
    if (new_block ==(void *) -1) return NULL;
    struct memory_chunk_t *new_chunk = (struct memory_chunk_t *) (new_block);
    new_chunk->free = 0;
    new_chunk->allocated_size = size_to_alloc;
    new_chunk->full_size = size_to_alloc + 8;
    new_chunk->next = NULL;
    if (cur == NULL){
        memory_manager.first_memory_chunk = new_chunk;
        new_chunk->prev = NULL;
    }
    else{
        cur->next= new_chunk;
        new_chunk->prev = cur;
        cur->magic_number = check_sum(cur);
    }

    memory_manager.memory_size += sizeof(struct memory_chunk_t) + new_chunk->full_size;
    *((uint8_t *) new_chunk + sizeof(struct memory_chunk_t)) = '#';
    *((uint8_t *) new_chunk+ sizeof(struct memory_chunk_t) + 1) = '#';
    *((uint8_t *) new_chunk+ sizeof(struct memory_chunk_t) + 2) = '#';
    *((uint8_t *) new_chunk+ sizeof(struct memory_chunk_t) + 3) = '#';
    for (size_t i = 0; i < size_to_alloc; ++i) {
        *((uint8_t *) new_chunk+ sizeof(struct memory_chunk_t) + 4 + i) = 0;
    }

    *((uint8_t *) new_chunk+ sizeof(struct memory_chunk_t) + new_chunk->allocated_size + 4) = '#';
    *((uint8_t *) new_chunk+ sizeof(struct memory_chunk_t) + new_chunk->allocated_size + 5) = '#';
    *((uint8_t *) new_chunk+ sizeof(struct memory_chunk_t) + new_chunk->allocated_size + 6) = '#';
    *((uint8_t *) new_chunk+ sizeof(struct memory_chunk_t) + new_chunk->allocated_size + 7) = '#';

    new_chunk->magic_number = check_sum(new_chunk);
    return ((uint8_t *) new_chunk  + sizeof(struct memory_chunk_t) + 4);
}

void* heap_realloc(void* memblock, size_t size){
    if (memory_manager.memory_start == NULL || (get_pointer_type(memblock) != 6 && memblock != NULL) || (size <= 0 && memblock == NULL)) return NULL;
    if (size <= 0 ){
        heap_free(memblock);
        return NULL;
    }
    if (memblock == NULL)
    {
        void* res = heap_malloc(size);
        return res;
    }
    struct memory_chunk_t *chunk_to_change = (struct memory_chunk_t*)((uint8_t*)memblock - sizeof(struct memory_chunk_t) - 4);
    if (chunk_to_change->allocated_size == size) return memblock;
    if (chunk_to_change->full_size - 8 > size)
    {
        chunk_to_change->allocated_size = size;
        *((uint8_t*)chunk_to_change + sizeof(struct memory_chunk_t) + size + 4) = '#';
        *((uint8_t*)chunk_to_change + sizeof(struct memory_chunk_t) + size + 5) = '#';
        *((uint8_t*)chunk_to_change + sizeof(struct memory_chunk_t) + size + 6) = '#';
        *((uint8_t*)chunk_to_change + sizeof(struct memory_chunk_t) + size + 7) = '#';

        chunk_to_change->magic_number = check_sum(chunk_to_change);
        return memblock;
    }
    if (chunk_to_change->next != NULL)
    {
        struct memory_chunk_t *next = chunk_to_change->next;
        if (next->free == 1 && size <= chunk_to_change->full_size - 8 + next->full_size + sizeof(struct memory_chunk_t))
        {
            int new_size_next = next->full_size - (size - (chunk_to_change->full_size - 8));
            chunk_to_change->full_size = size + 8;
            chunk_to_change->allocated_size = size;
            if (new_size_next <= 0)
            {
                struct memory_chunk_t *next_next = next->next;
                chunk_to_change->next = next_next;
                if (next_next != NULL){
                    next_next->prev = chunk_to_change;
                    next_next->magic_number = check_sum(next_next);
                }
            }
            else
            {
                next->full_size = new_size_next;
                struct memory_chunk_t *new_block = (struct memory_chunk_t *)((uint8_t*)chunk_to_change + sizeof(struct memory_chunk_t) + chunk_to_change->full_size);
                new_block->next = next->next;
                new_block->prev = next->prev;
                new_block->free = 1;
                new_block->allocated_size = 0;
                new_block->full_size = next->full_size;
                if (next->next != NULL){
                    struct memory_chunk_t *next_next = next->next;
                    next_next->prev = new_block;
                    next_next->magic_number = check_sum(next_next);
                }
                chunk_to_change->next = new_block;
                new_block->magic_number = check_sum(new_block);
            }

            *((uint8_t*)chunk_to_change + sizeof(struct memory_chunk_t) + size + 4) = '#';
            *((uint8_t*)chunk_to_change + sizeof(struct memory_chunk_t) + size + 5) = '#';
            *((uint8_t*)chunk_to_change + sizeof(struct memory_chunk_t) + size + 6) = '#';
            *((uint8_t*)chunk_to_change + sizeof(struct memory_chunk_t) + size + 7) = '#';
            chunk_to_change->magic_number = check_sum(chunk_to_change);
            return memblock;
        }
    }
    else
    {
        void* more_memory = (void*)custom_sbrk(size - (chunk_to_change->full_size - 8));
        if (more_memory == (void *) -1) return NULL;

        chunk_to_change->full_size = size + 8;
        chunk_to_change->allocated_size = size;
        *((uint8_t *) chunk_to_change+ sizeof(struct memory_chunk_t) + chunk_to_change->allocated_size + 4) = '#';
        *((uint8_t *) chunk_to_change+ sizeof(struct memory_chunk_t) + chunk_to_change->allocated_size + 5) = '#';
        *((uint8_t *) chunk_to_change+ sizeof(struct memory_chunk_t) + chunk_to_change->allocated_size + 6) = '#';
        *((uint8_t *) chunk_to_change+ sizeof(struct memory_chunk_t) + chunk_to_change->allocated_size + 7) = '#';

        chunk_to_change->magic_number = check_sum(chunk_to_change);
        memory_manager.memory_size += size - (chunk_to_change->full_size - 8);
        return memblock;
    }

    void *new_place = heap_malloc(size);
    if (new_place == NULL) return NULL;
    for (size_t i = 0; i < chunk_to_change->allocated_size; ++i){
        *((uint8_t*) new_place + i) = *((uint8_t*)chunk_to_change + sizeof(struct memory_chunk_t) + 4 + i);
    }
    heap_free(((uint8_t*)chunk_to_change + sizeof(struct memory_chunk_t) + 4));
    return new_place;
}

void  heap_free(void* memblock){
    if (memblock == NULL || memory_manager.memory_start == NULL || get_pointer_type(memblock) != 6) return;
    struct memory_chunk_t *chunk_to_free = (struct memory_chunk_t*)((uint8_t*)memblock - sizeof(struct memory_chunk_t) - 4);
    chunk_to_free->free = 1;
    chunk_to_free->allocated_size = 0;
    chunk_to_free->magic_number = check_sum(chunk_to_free);

    if (chunk_to_free->next != NULL)
    {
        struct memory_chunk_t *next = chunk_to_free->next;
        struct memory_chunk_t *next_next = next->next;
        if (next->free == 1)
        {
            chunk_to_free->next = next_next;
            if (next_next != NULL)
            {
                next_next->prev = chunk_to_free;
                next_next->magic_number = check_sum(next_next);
            }
            chunk_to_free->allocated_size = 0;
            chunk_to_free->free = 1;
            chunk_to_free->full_size += next->full_size + sizeof(struct memory_chunk_t);
            chunk_to_free->magic_number = check_sum(chunk_to_free);
        }
    }

    if (chunk_to_free->prev != NULL)
    {
        struct memory_chunk_t *prev = chunk_to_free->prev;
        struct memory_chunk_t *next = chunk_to_free->next;
        if (prev->free == 1)
        {
            prev->next = next;
            if (next != NULL)
            {
                next->prev = prev;
                next->magic_number = check_sum(next);
            }
            prev->allocated_size=0;
            prev->full_size += chunk_to_free->full_size + sizeof(struct memory_chunk_t);
            prev->magic_number = check_sum(prev);
        }
    }

    return;
}
