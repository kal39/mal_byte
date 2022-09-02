#include "core.h"
#include "common.h"
#include "stack.h"
#include "status.h"

static Status _print(Stack *stack) {
	while (stack->size > 0) {
		Value value = stack_pop(stack);
		switch (value.type) {
			case VALUE_NUMBER: printf("%g", value.as.number); break;
			case VALUE_STRING: printf("%s", value.as.chars.data); break;
			default: return error("expected number or string");
		}
	}
	stack_push(stack, value_make_nil());
	return ok();
}

static Status _println(Stack *stack) {
	Status result = _print(stack);
	printf("\n");
	return result;
}

Env *make_core() {
	Env *core = env_create(NULL);
	env_set(core, value_make_symbol_copy("print", strlen("print")), value_make_fn_ptr(_print));
	env_set(core, value_make_symbol_copy("println", strlen("println")), value_make_fn_ptr(_println));
	return core;
}