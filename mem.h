#ifndef _MEM_H_
#define _MEM_H_

#include <stddef.h>
#include <stdio.h>

#define IS_FREE 1
#define IS_PAGE_FIRST 2
#define IS_PAGE_LAST 4

struct MemoryHeader {
  struct MemoryHeader *next;
  struct MemoryHeader *prev;
  size_t capacity;
  int flags;
};

void *_malloc(size_t query);
void _free(void *mem);
struct MemoryHeader *mmap_header(size_t initial_size);

#define DEBUG_FIRST_BYTES 4
void memalloc_debug_struct_info(FILE *f, struct MemoryHeader const *address);
void memalloc_debug_heap(FILE *f, struct MemoryHeader const *ptr);

#endif
