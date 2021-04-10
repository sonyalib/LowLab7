#include "mem.h"
#include <sys/mman.h>

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS 0x20
#endif

#define PAGE_SIZE 0x1000

static struct MemoryHeader *first = NULL;

static size_t _ceil_size(size_t query, size_t divisor) {
  size_t mod = query % divisor;
  return mod ? query + divisor - mod : query;
}

static void split_chunk(struct MemoryHeader *chunk, size_t query) {
  struct MemoryHeader *next;
  size_t new_capacity = _ceil_size(query, sizeof(void *));
  if (chunk->capacity >= new_capacity + sizeof(struct MemoryHeader)) {
    next = (void *)((char *)(chunk + 1) + new_capacity);
    next->next = chunk->next;
    next->prev = chunk;
    next->capacity =
        chunk->capacity - new_capacity - sizeof(struct MemoryHeader);
    next->flags = IS_FREE | (chunk->flags & IS_PAGE_LAST);
    chunk->capacity = new_capacity;
    chunk->next = next;
  }
  chunk->flags &= ~(IS_FREE | IS_PAGE_LAST);
}

static void remove_chunk(struct MemoryHeader *to_remove) {
  if (to_remove->next)
    to_remove->next->prev = to_remove->prev;
  if (to_remove->prev) {
    to_remove->prev->next = to_remove->next;
  } else {
    first = to_remove->next;
  }
}

static void remove_chunk_inpage(struct MemoryHeader *to_remove) {
  remove_chunk(to_remove);
  to_remove->prev->flags |= to_remove->flags & IS_PAGE_LAST;
  to_remove->prev->capacity +=
      to_remove->capacity + sizeof(struct MemoryHeader);
}

void *_malloc(size_t query) {
  struct MemoryHeader *header;
  if (!query)
    return NULL;
  if (!first)
    first = mmap_header(query);
  if (first == MAP_FAILED)
    return NULL;

  for (header = first; header->next; header = header->next) {
    if ((header->flags & IS_FREE) && header->capacity >= query) {
      split_chunk(header, query);
      return header + 1;
    }
  }

  if ((header->flags & IS_FREE) && header->capacity >= query) {
    split_chunk(header, query);
    return header + 1;
  }

  header->next = mmap_header(query);
  if (header->next == MAP_FAILED) {
    header->next = NULL;
    return NULL;
  }
  header->next->prev = header;
  split_chunk(header->next, query);
  return header->next + 1;
}

void _free(void *mem) {
  struct MemoryHeader *header = mem;
  if (!mem)
    return;
  --header;
  header->flags |= IS_FREE;
  if (header->next && (header->next->flags & IS_FREE) &&
      (char *)(header + 1) + header->capacity == (char *)header->next) {
    remove_chunk_inpage(header->next);
  }
  if (header->prev && (header->prev->flags & IS_FREE) &&
      (char *)(header->prev + 1) + header->prev->capacity == (char *)header) {
    header = header->prev;
    remove_chunk_inpage(header->next);
  }
  if ((header->flags & IS_PAGE_FIRST) && (header->flags & IS_PAGE_LAST)) {
    remove_chunk(header);
    munmap(header, header->capacity + sizeof(struct MemoryHeader));
  }
}

struct MemoryHeader *mmap_header(size_t query) {
  size_t size = query + sizeof(struct MemoryHeader);
  struct MemoryHeader *header;
  size = _ceil_size(size, sizeof(void *));
  size = _ceil_size(size, PAGE_SIZE);
  header = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS,
                -1, 0);

  if (header == MAP_FAILED)
    return NULL;

  header->prev = NULL;
  header->next = NULL;
  header->capacity = size - sizeof(struct MemoryHeader);
  header->flags = IS_FREE | IS_PAGE_FIRST | IS_PAGE_LAST;
  return header;
}

void memalloc_debug_struct_info(FILE *f, struct MemoryHeader const *address) {
  size_t i;
  fprintf(f,
          "\nstart: %p"
          "\nsize: %lu"
          "\nflags: %d\n",
          (void *)address, address->capacity, address->flags);
  for (i = 0; i < DEBUG_FIRST_BYTES && i < address->capacity; ++i)
    fprintf(f, "%X ", ((char *)address)[sizeof(struct MemoryHeader) + i]);
  putc('\n', f);
}

void memalloc_debug_heap(FILE *f, struct MemoryHeader const *ptr) {
  for (; ptr; ptr = ptr->next)
    memalloc_debug_struct_info(f, ptr);
}
