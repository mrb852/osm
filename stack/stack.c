//
//  stack.c
//  StackFun
//
//  Created by ChristianEnevoldsen on 03/02/15.
//  Copyright (c) 2015 ChristianEnevoldsen. All rights reserved.
//

#include "stack.h"
#include <stdlib.h>
#include <stdio.h>

stack_type *stack_init_capacity(int capacity) {
  stack_type *stack = (stack_type *)malloc(sizeof(stack_type));
  stack->capacity = capacity;
  stack->size = 0;
  stack->data = (void **)malloc(capacity * sizeof(void *));
  return stack;
}

stack_type *stack_init() {
  return stack_init_capacity(16);
}

int stack_is_empty(stack_type *stack) {
  return stack->size <= 0;
}

void *stack_pop(stack_type *stack) {
  if (!stack_is_empty(stack)) {
    void *object = stack->data[stack->size--];
    return object;
  }
  return NULL;
}


void stack_push(stack_type *stack, void *obj) {
  if (stack->size >= stack->capacity - 1) {
    stack->capacity *= 2;
    void *tmp = realloc(stack->data, sizeof(tmp) + sizeof(void *) * stack->capacity);
    stack->data = tmp;
  }

  stack->data[++stack->size] = obj;
}

/* This might need a bit attention since it could be doing the right thing or nothing. Idk really... */
void stack_free(stack_type *stack) {
  int i = 0;
  for (; i < stack->size; i++) {
    if (stack->data[i] != NULL) {
      free(stack->data[i]);
    }
  }
  stack->data = NULL;
  stack->size = 0;
  stack->capacity = 0;
  free(stack);
}
