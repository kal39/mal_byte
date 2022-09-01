#ifndef STACK_H
#define STACK_H

#include "value.h"

typedef struct Stack {
	int capacity;
	int size;
	Value *values;
} Stack;

Stack *stack_create();
void stack_destroy(Stack *stack);

void stack_push(Stack *stack, Value value);
Value stack_pop(Stack *stack);
void stack_empty(Stack *stack);

void stack_print(Stack *stack);

#endif