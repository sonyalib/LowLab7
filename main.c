#include "mem.h"
#include <stdio.h>

int main() {
  char *d1 = _malloc(0xf00);
  char *d2 = _malloc(0x100);
  char *d3 = _malloc(0x100);
  char *d4 = _malloc(0x100);
  struct MemoryHeader *h = (struct MemoryHeader *)d1 - 1;
  d1[0] = 1;
  d1[1] = 2;
  d1[2] = 3;
  d1[3] = 4;
  d2[0] = 5;
  d2[1] = 6;
  d2[2] = 7;
  d2[3] = 8;
  d3[0] = 9;
  d3[1] = 10;
  d3[2] = 11;
  d3[3] = 12;
  d4[0] = 13;
  d4[1] = 14;
  d4[2] = 15;
  d4[3] = 16;
  puts("\n\nAfter filling");
  memalloc_debug_heap(stdout, h);

  puts("\n\nAfter _free(d3);");
  _free(d3);
  memalloc_debug_heap(stdout, h);

  puts("\n\nAfter _free(d4);");
  _free(d4);
  memalloc_debug_heap(stdout, h);

  puts("\n\nAfter _free(d2);");
  _free(d2);
  memalloc_debug_heap(stdout, h);

  puts("\n\nAfter _free(d1); (segfault expected)");
  _free(d1);
  memalloc_debug_heap(stdout, h);
  return 0;
}
