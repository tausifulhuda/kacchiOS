/* serial.h - Serial port driver interface */
#ifndef SERIAL_H
#define SERIAL_H

#include "types.h"

void serial_init(void);
void serial_putc(char c);
void serial_puts(const char* str);
void serial_putint(uint32_t n);
void serial_puthex(uint32_t n);
char serial_getc(void);

#endif