#include "stack.h"
#include "common.h"

Stack *stack_create() {
	Stack *stack = malloc(sizeof(Stack));
	stack->capacity = 8;
	stack->size = 0;
	stack->values = malloc(sizeof(Value) * stack->capacity);
	return stack;
}

void stack_destroy(Stack *stack) {
	if (stack->values != NULL) free(stack->values);
	if (stack != NULL) free(stack);

	stack->values = NULL;
	stack = NULL;
}

void stack_push(Stack *stack, Value value) {
	if (stack->size == stack->capacity) stack->values = realloc(stack->values, sizeof(Value) * (stack->capacity *= 2));
	stack->values[stack->size++] = value;
}

Value stack_pop(Stack *stack) {
	return stack->values[--stack->size];
}

void stack_empty(Stack *stack) {
	stack->size = 0;
}

void stack_print(Stack *stack) {
	printf("┏━━━━━━┳━━━━━ Value type ━━━━━┳━━━━━ Content ━━━━━\n");
	for (int i = 0; i < stack->size; i++) {
		printf("┃ %04d ┃ ", i);
		value_print(stack->values[i]);
		printf("\n");
	}
}