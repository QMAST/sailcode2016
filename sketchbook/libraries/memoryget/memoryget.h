// MemoryFree library based on code posted here:
// http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1213583720/15
// 
// Extended by Matthew Murdoch to include walking of the free list.

#ifndef MEMORY_FREE_H
#define MEMORY_FREE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

extern unsigned int __heap_start;
extern void *__brkval;

/* The head of the free list structure */
extern struct __freelist *__flp;

/*
 * The free list structure as maintained by the 
 * avr-libc memory allocation routines.
 */
struct __freelist {
  size_t sz;
  struct __freelist *nx;
};

static int freeListSize();
int getAvailableMemory();

#ifdef  __cplusplus
}
#endif
#endif
