#include "heap.h"
#include <stdlib.h>

void heap_initialize(heap_t* heap) {
  heap->alloc_size = 8 * sizeof(note_t);
  heap->size = 0;
  heap = malloc(heap->alloc_size);
}

// void heap_clear(heap*);

size_t heap_size(heap_t* heap) {
  return heap->size;
}

void* heap_top(heap_t* heap) {
  return heap->root->thing;
}

void heap_insert(heap_t* heap, void* obj, int p) {
  // check if
  if (heap->size * sizeof(note_t) >= heap->alloc_size) {
    size_t alloc_size = heap->alloc_size * 2;
    heap = realloc(heap, alloc_size);
  }
  node_t *node = malloc(sizeof(node_t));
  node->thing = obj;
  node->priority = p;
  heap->size++;
  *(&heap->root + heap->size) = node;
  // max-heapify here
}

void *heap_pop(heap_t* heap) {
  node_t *node = heap_top(heap);
  heap->size--;
  heap->root = NULL;
  // max-heapify here
  return node;
}

// int compare(node_t *n1, node_t *n2) {
//   int p1 = n1->priority;
//   int p2 = n2->priority;
//   if (p1 == p2) return 0;
//   else if (p1 < p2) return -1;
//   else return 1;
// }
