//
//  stack.h
//  StackFun
//
//  Created by ChristianEnevoldsen on 03/02/15.
//  Copyright (c) 2015 ChristianEnevoldsen. All rights reserved.
//

#ifndef __StackFun__stack__
#define __StackFun__stack__

#include <stdlib.h>

typedef struct stack_type {
  size_t capacity;
  size_t size;
  void **data;
} stack_type;

/**
 initializes a new stack and returns it
 */
stack_type * stack_init();

void stack_free(stack_type *stack);

/**
 * Checks if a stack is empty
 * @param stack the stack to check
 * @return 0 if empty
 */
int stack_is_empty(stack_type *stack);

/**
 * Removes the top of the stack and returns the value
 * @param stack the stack
 * @return the top of the stack
 */
void *stack_pop(stack_type *stack);


/**
 * Pushes an object onto the stack and increments the stack top
 * @param stack the stack
 * @param obj the object to push onto the stack
 */
void stack_push(stack_type *stack, void *obj);

#endif /* defined(__StackFun__stack__) */
