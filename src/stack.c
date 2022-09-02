#include "stack.h"
#include "common.h"

Stack *stack_create() {
	Stack *stack = malloc(sizeof(Stack));
	stack->size = 0;
	return stack;
}

void stack_destroy(Stack *stack) {
	if (stack != NULL) free(stack);
	stack = NULL;
}

void stack_push(Stack *stack, Value value) {
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