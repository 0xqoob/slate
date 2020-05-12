#ifndef __LIB__IO_H__
#define __LIB__IO_H__

#include <stdint.h>
#include <stdarg.h>
#include <str.h>
#include <mm/mem.h>
#include <sys/locks.h>
#include <drivers/serial.h>

void serial_printf(char* message, ...);

#endif