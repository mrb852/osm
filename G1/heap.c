#include "heap.h"
#include <stdio.h>
#include <stdlib.h>

void heapify(heap_t* heap, size_t i, size_t n) {

  long l = 2 * i + 1;
  long r = 2 * i + 2;
  long max = i;
  node_t *arr = heap->root;

  if (l < n && arr[l].priority > arr[max].priority) { max = l; }
  if (r < n && arr[r].priority > arr[max].priority) { max = r; }

  if (max != i) {
    node_t tmp = arr[i];
    arr[i] = arr[max];
    arr[max] = tmp;
    heapify(heap, max, n);
  }
}

void build_heap(heap_t* heap) {
  long n = heap_size(heap);
  long i = n / 2;

  while(i > 0) {
    heapify(heap, --i, n);
  }
}

void heap_initialize(heap_t* heap) {
  heap->alloc_size = 8 * sizeof(node_t);
  heap->size = 0;
  heap->root = malloc(heap->alloc_size);
}

size_t heap_size(heap_t* heap) {
  return heap->size / sizeof(node_t);
}

void* heap_top(heap_t* heap) {
  build_heap(heap);
  return heap->root->value;
}

void heap_insert(heap_t* heap, void* obj, int p) {

  heap->size += sizeof(node_t);

  if (heap->size > heap->alloc_size) {
    heap->alloc_size *= 2;
    heap->root = realloc(heap->root, heap->alloc_size);
  }
  node_t node = { .value = obj, .priority = p };

  heap->root[heap_size(heap) - 1] = node;
  build_heap(heap);

}

void* heap_pop(heap_t* heap) {

  build_heap(heap);
  node_t *root = heap->root;
  void *val = root->value;
  root->priority = -1;
  root->value = NULL;

  heap->size -= sizeof(node_t);

  if (heap->size <= heap->alloc_size / 2) {
    heap->alloc_size /= 2;
    heap->root = realloc(heap->root, heap->alloc_size);
  }

  return val;
}

void heap_clear(heap_t *heap) {
  for (long i = 0; i < heap_size(heap); ++i) {
    node_t node = heap->root[i];
    if (node.value != NULL) {
      free(node.value);
      node.priority = 0;
    }
  }
  free(heap->root);
  heap->alloc_size = 0;
  heap->size = 0;
}


