#ifndef STACK_H
#define STACK_H

#include "value.h"

#define STACK_SIZE 2048

typedef struct Stack {
	int size;
	Value values[STACK_SIZE];
} Stack;

Stack *stack_create();
void stack_destroy(Stack *stack);

void stack_push(Stack *stack, Value value);
Value stack_pop(Stack *stack);
void stack_empty(Stack *stack);

void stack_print(Stack *stack);

#endif