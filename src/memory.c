#include "memory.h"
#include "serial.h"

extern char __kernel_end;

#define HEAP_END 0x02000000
#define STACK_BOTTOM 0x02500000
#define STACK_TOP 0x04000000
#define ALIGNMENT 8

typedef struct block{
    size_t size;
    int free;
    struct block* next;
}block_t;

static block_t* free_list=0;
static block_t* stack_free_list=0;

static size_t align_size(size_t size){
    size_t remainder=size%ALIGNMENT;
    
    if(remainder==0)
        return size;

    return size+(ALIGNMENT-remainder);
}

static uintptr_t align_ptr(uintptr_t ptr){
    uintptr_t remainder=ptr%ALIGNMENT;
    
    if(remainder==0)
        return ptr;

    return ptr+(ALIGNMENT-remainder);
}

static uintptr_t align_ptr_down(uintptr_t ptr){
    uintptr_t remainder=ptr%ALIGNMENT;
    
    if(remainder==0)
        return ptr;

    return ptr-remainder;
}

void memory_init(void){
    block_t* first;
    uintptr_t start;

    start=(uintptr_t)&__kernel_end;
    start=align_ptr(start);

    first=(block_t*)start;

    first->size=HEAP_END-start-sizeof(block_t);
    first->free=1;
    first->next=0;

    free_list=first;
}

void stack_init(void)
{
    block_t* first;
    uintptr_t start;

    start = STACK_TOP - sizeof(block_t);
    start = align_ptr_down(start);

    first = (block_t*)start;

    first->size = start - STACK_BOTTOM;
    first->free = 1;
    first->next = 0;

    stack_free_list = first;
}

void* kmalloc(size_t size){
    block_t* current;
    block_t* new_block;

    if(size==0)
        return 0;

    size=align_size(size);

    current=free_list;

    while(current!=0){
        if(current->free && current->size>=size){
            if(current->size>=size+sizeof(block_t)+ALIGNMENT){
                new_block=(block_t*)((char*)current+sizeof(block_t)+size);
                new_block->size=current->size-size-sizeof(block_t);
                new_block->free=1;
                new_block->next=current->next;
                current->size=size;
                current->next=new_block;
            }
            current->free=0;
            return (void*)((char*)current+sizeof(block_t));
        }
        current=current->next;
    }
    return 0;
}

void* kstalloc(size_t size){
    block_t* current;
    block_t* new_block;

    if(size==0)
        return 0;

    size=align_size(size);

    current=stack_free_list;

    while(current!=0){
        if(current->free && current->size>=size){
            if(current->size>=size+sizeof(block_t)+ALIGNMENT){
                new_block=(block_t*)((char*)current-sizeof(block_t)-size);
                new_block->size=current->size-size-sizeof(block_t);
                new_block->free=1;
                new_block->next=current->next;
                current->size=size;
                current->next=new_block;
            }
            current->free=0;
            return (void*)((char*)current-sizeof(block_t));
        }
        current=current->next;
    }
    return 0;
}

void kfree(void* ptr){
    block_t* block;

    if(ptr==0)
        return;

    block=(block_t*)((char*)ptr-sizeof(block_t));
    block->free=1;

    block_t* current=free_list;

    while(current!=0 && current->next!=0){
        if(current->free && current->next->free){
            current->size+=sizeof(block_t)+current->next->size;
            current->next=current->next->next;
        }
        else
            current=current->next;
    }
}

void ksfree(void* ptr){
    block_t* block;

    if(ptr==0)
        return;

    block=(block_t*)((char*)ptr+sizeof(block_t));
    block->free=1;

    block_t* current=stack_free_list;

    while(current!=0 && current->next!=0){
        if(current->free && current->next->free){
            current->size+=sizeof(block_t)+current->next->size;
            current->next=current->next->next;
        }
        else
            current=current->next;
    }
}

void memory_info(void* ptr){

    block_t *block = (block_t *)((char *)ptr - sizeof(block_t));

    serial_puts("Size: ");
    serial_putint(block->size);
    serial_puts("\n");

    serial_puts("Free: ");
    serial_putint(block->free);
    serial_puts("\n");

    serial_puts("Block address: ");
    serial_puthex((uint32_t)block);
    serial_puts("\n");

    serial_puts("Data address: ");
    serial_puthex((uint32_t)ptr);
    serial_puts("\n");

    serial_puts("Next block: ");
    serial_puthex((uint32_t)block->next);
    serial_puts("\n");
}

void stack_memory_info(void* ptr){

    block_t *block = (block_t *)((char *)ptr + sizeof(block_t));

    serial_puts("Size: ");
    serial_putint(block->size);
    serial_puts("\n");

    serial_puts("Free: ");
    serial_putint(block->free);
    serial_puts("\n");

    serial_puts("Block address: ");
    serial_puthex((uint32_t)block);
    serial_puts("\n");

    serial_puts("Data address: ");
    serial_puthex((uint32_t)ptr);
    serial_puts("\n");

    serial_puts("Next block: ");
    serial_puthex((uint32_t)block->next);
    serial_puts("\n");
}