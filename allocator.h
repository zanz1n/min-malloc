#include <stddef.h>

#ifndef __ALLOCATOR_H
#define __ALLOCATOR_H 1

#ifndef HEAP_ALIGNMENT
#define HEAP_ALIGNMENT 128
#endif

/*
 * Needs to be called at the program initialization.
 */
void mem_init(void *heap, size_t size);

/*
 * Returns the total memory that is beign held in the HEAP.
 *
 * **NOTE:** This counts memory that was already freed.
 */
size_t mem_usage();

void *malloc(size_t size);

void *realloc(void *ptr, size_t size);

void free(void *ptr);

#endif
