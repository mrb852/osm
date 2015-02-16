#include "heap.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void heapify(heap_t* heap, size_t i, size_t n) {

  long unsigned l = 2 * i + 1; // long unsigned, since we can't have negative
  long unsigned r = 2 * i + 2; // heaps.
  long unsigned max = i; // max is always positive or zero
  node_t *arr = heap->root; // pointer to root

  // Find max
  if (l < n && arr[l].priority > arr[max].priority) { max = l; }
  if (r < n && arr[r].priority > arr[max].priority) { max = r; }

  if (max != i) {
    node_t tmp = arr[i]; // Backup element i
    arr[i] = arr[max]; // Overwrite [i] with [max]
    arr[max] = tmp; // [i] is new [max]
    heapify(heap, max, n); // Run heapify recurrsion
  }
}

void build_heap(heap_t* heap) {
  long unsigned n = heap_size(heap); // We expect size to be non-negative
  long unsigned i = n / 2;

  while(i) { // while is not zero (same as false)
    heapify(heap, --i, n);
  }
}

void heap_initialize(heap_t* heap) {
  heap->alloc_size = 64 * sizeof(node_t);
  heap->size = 0;
  heap->root = malloc(heap->alloc_size);
}

size_t heap_size(heap_t* heap) {
  return heap->size / sizeof(node_t); // logical size is given by dividing
                                      // bytes-size and size of a single element
}

void* heap_top(heap_t* heap) {
  if (heap->size <= 0) { // Make sure the heap is not empty
    return NULL;
  }

  return heap->root->value; // return top element
}

void heap_insert(heap_t* heap, void* obj, int p) {
  assert(heap->root != NULL); // Make sure the heap is not empty
  heap->size += sizeof(node_t); // Increment size of heap

  if (heap->size > heap->alloc_size) { // If all space is used:
    heap->alloc_size *= 2;             // Reserve double the space
    heap->root = realloc(heap->root, heap->alloc_size);
  }
  node_t node = { .value = obj, .priority = p };

  heap->root[heap_size(heap) - 1] = node;
  build_heap(heap);
}

void swap(node_t arr[], int i, int j) {
  node_t tmp = arr[i]; // Save temporary
  arr[j] = arr[i]; // overwrite [j] with [i]
  arr[i] = tmp; // overwrite [i] with tmp
}

void* heap_pop(heap_t* heap) {
  assert(heap->size > 0); // Can't pop an empty heap
  node_t *root = heap->root;
  void *val = root->value;

  for (unsigned i = 1; i < heap_size(heap); ++i) {
    swap(heap->root, i, i - 1);
  }

  heap->size -= sizeof(node_t); // decrement size

  if (heap->size <= heap->alloc_size / 2) { // If heap size is all than half the
    heap->alloc_size /= 2;                  // allocated size: Free up space.
    heap->root = realloc(heap->root, heap->alloc_size);
  }

  build_heap(heap);

  return val;
}

void heap_clear(heap_t *heap) {
  for (long unsigned i = 0; i < heap_size(heap); ++i) {
    node_t node = heap->root[i];
    if (node.value != NULL) { // If value exists, free space
      free(node.value);
    }
  }
  free(heap->root);
  heap->alloc_size = 0;
  heap->size = 0;
  heap->root = NULL;
}


