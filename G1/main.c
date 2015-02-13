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
#include <string.h>
#include <assert.h>

char *createObject(char * c, int number) {
  char *s = malloc(20);
  sprintf(s, "%s: %d", c, number);
  return s;
}

void assertTrue(char *message, int test) {
  printf("Performing %s: %s\n", message, test ? "true" : "false");
}

void testTop() { // Test that the top element is the largest element
  heap_t heap;
  heap_initialize(&heap);
  heap_insert(&heap, createObject("Tester", 10), 10);
  heap_insert(&heap, createObject("Tester", 11), 11);
  heap_insert(&heap, createObject("This will be the top", 1337), 1337);
  heap_insert(&heap, createObject("Tester", 210), 210);
  heap_insert(&heap, createObject("Tester", 110), 110);
  char *top = heap_top(&heap); // This should be 1337
  assertTrue("testTop", strcmp(top, "This will be the top: 1337") == 0);
  heap_clear(&heap);
}

void testTopAfterPop() { // check that top is the highest priority after pop
  heap_t heap;
  heap_initialize(&heap);
  heap_insert(&heap, createObject("Tester", 10), 10);
  heap_insert(&heap, createObject("Tester", 11), 11);
  heap_insert(&heap, createObject("Tester", 12), 12);
  heap_insert(&heap, createObject("Tester", 13), 13);
  heap_insert(&heap, createObject("Tester", 14), 14);
  heap_insert(&heap, createObject("Tester", 15), 15);
  heap_insert(&heap, createObject("This will be the top", 1337), 1337);
  heap_insert(&heap, createObject("Tester", 16), 16);
  heap_insert(&heap, createObject("Tester", 17), 17);
  heap_insert(&heap, createObject("Tester", 2100), 2100);
  heap_insert(&heap, createObject("Tester", 110), 110);

  heap_pop(&heap); // pop 2100, making 1337 as top
  char *top = heap_top(&heap);
  assertTrue("testTopAfterPop", strcmp(top, "This will be the top: 1337") == 0);
  heap_clear(&heap);
}

void testHeapClear() {
  heap_t heap;
  heap_initialize(&heap);
  for (int i = 0; i < 2000; ++i) { // Insert 2000 elements and clear them after
    heap_insert(&heap, createObject("Tester", i), i);
  }
  heap_clear(&heap);
  assertTrue("testHeapClear", heap_top(&heap) == NULL && heap_size(&heap) == 0);
}

void testHeapSize() {
  heap_t heap;
  heap_initialize(&heap);
  for (int i = 0; i < 16; ++i) {
    heap_insert(&heap, createObject("Tester", i), i);
  }
  heap_pop(&heap);
  heap_pop(&heap);
  heap_insert(&heap, createObject("Tester", 1), 1);
  assertTrue("testHeapSize", heap_size(&heap) == 15);
  heap_clear(&heap);
}

void testHeapInsert() {
  heap_t heap;
  heap_initialize(&heap);
  heap_insert(&heap, createObject("Tester", 1), 1);
  assertTrue("testHeapInsert", strcmp((char *)heap_pop(&heap), "Tester: 1") == 0);
  heap_clear(&heap);
}

int main(int argc, const char * argv[]) {
  testTop();
  testTopAfterPop();
  testHeapClear();
  testHeapSize();
  testHeapInsert();
  return 0;
}
