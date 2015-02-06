#include <stdio.h>
#include <stdlib.h>
#include "heap.h"

#ifdef UNIT_TEST

int main(int argc, char const *argv[]) {
  heap_t heap;
  heap_initialize(&heap);
  heap_insert(&heap, "hello world", 1);
  printf("heap size: %lu\n", heap.size);

  return 0;
}

#endif
