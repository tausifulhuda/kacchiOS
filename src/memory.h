#ifndef MEMORY_H
#define MEMORY_H

#include"types.h"

void memory_init(void);
void stack_init(void);
void* kmalloc(size_t size);
void* kstalloc(size_t size);
void kfree(void* ptr);
void ksfree(void* ptr);
void memory_info(void* ptr);
void stack_memory_info(void* ptr);

#endif