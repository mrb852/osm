//
//  main.c
//  StackFun
//
//  Created by ChristianEnevoldsen on 03/02/15.
//  Copyright (c) 2015 ChristianEnevoldsen. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include "stack.h"

void printStack(stack_type *);

int main() {

  printf("Initializing stack\n");
  // Create the stack
  stack_type *stack = stack_init();

  printf("Filling stack...\n");
  // Fill the stack with hello (some number)
  int i = 0;
  for (; i < 10; ++i) {
    // Create a string buffer and alloc it in the heap ( otherwise it is released immediately afer )
    char *str = malloc(sizeof(char) * 7);
    sprintf(str, "Hello %d", i);
    printf("Pushed %s into the stack\n", str);

    // Push it onto the stack
    stack_push(stack, str);
  }

  // Pop all items on the stack
  for (; i > 0; i--) {
    printf("popped: %s\n", (char *)stack_pop(stack));
  }

  // Try to pop when its empty
  if (!stack_pop(stack)) {
    printf("cannot pop because stack is empty.\n");
  }

  stack_free(stack);

  return 0;
}
