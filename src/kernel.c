/* kernel.c - Main kernel with null process */
#include "types.h"
#include "serial.h"
#include "string.h"
#include "memory.h"

#define MAX_INPUT 128

void kmain(void) {
    memory_init();
    serial_puts("Heap Memory Initialized\n");

    void* p1 = kmalloc(100);
    void* p2 = kmalloc(200);
    void* p3 = kmalloc(300);

    if (p1 && p2 && p3)
        serial_puts("All Heaps Allocated.\n");
    else
        serial_puts("Allocation failed\n");

    serial_puts("\n---p1 Information---\n");
    memory_info(p1);
    
    serial_puts("\n---p2 Information---\n");
    memory_info(p2);

    serial_puts("\n---p3 Information---\n");
    memory_info(p3);

    serial_puts("\n");

    kfree(p1);
    serial_puts("p1 Freed\n");

    kfree(p2);
    serial_puts("p2 Freed\n");

    void* p4 = kmalloc(250);

    if (p4!=0)
        serial_puts("p4 Allocation successful\n");
    else
        serial_puts("Allocation failed\n");

    serial_puts("\n---p4 Information---\n");
    memory_info(p4);
    serial_puts("\n");

    stack_init();
    serial_puts("Stack Memory Initialized\n");

    void* s1 = kstalloc(100);
    void* s2 = kstalloc(200);
    void* s3 = kstalloc(300);

    if (s1 && s2 && s3)
        serial_puts("All Stacks allocated\n");
    else
        serial_puts("Allocation Failed\n");

    serial_puts("\n---s1 Information---\n");
    stack_memory_info(s1);

    serial_puts("\n---s2 Information---\n");
    stack_memory_info(s2);

    serial_puts("\n---s3 Information---\n");
    stack_memory_info(s3);
    
    serial_puts("\n");

    ksfree(s1);
    serial_puts("s1 Freed\n");

    ksfree(s2);
    serial_puts("s2 Freed\n");

    void* s4 = kstalloc(250);

    if (s4!=0)
        serial_puts("s4 Allocation successful\n");
    else
        serial_puts("Allocation failed\n");

    serial_puts("\n---s4 Information---\n");
    stack_memory_info(s4);
    serial_puts("\n");
    /* Should never reach here */
    for (;;) {
        __asm__ volatile ("hlt");
    }
}