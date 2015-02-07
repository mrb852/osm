//
//  main.c
//  andersand
//
//  Created by ChristianEnevoldsen on 06/02/15.
//  Copyright (c) 2015 ChristianEnevoldsen. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include "heap.h"

char *createObject(char * c, int number) {
  char *s = malloc(20);
  sprintf(s, "%s: %d", c, number);
  return s;
}

int main(int argc, const char * argv[]) {

  heap_t heap;
  heap_initialize(&heap);
  int max = 30;

  for (int i = 0; i < max; ++i) {
    int p = i;
    heap_insert(&heap, createObject("Hello", p), p);
  }

  for (int i = 0; i < max; ++i) {
    char *c = heap_pop(&heap);
    printf("%s\n", c);
  }

  heap_clear(&heap);

  return 0;
}
