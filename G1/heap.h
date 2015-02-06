#include <stddef.h> // size_t

typedef struct node_t {
  void* thing;
  int priority;
} node_t;

typedef struct heap_t {
  node_t* root;
  size_t size;
  size_t alloc_size;
} heap_t;

void heap_initialize(heap_t*);
void heap_clear(heap_t*);
size_t heap_size(heap_t*);
void* heap_top(heap_t*);
void heap_insert(heap_t*, void*, int);
void *heap_pop(heap_t*);
