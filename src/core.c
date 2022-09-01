#include "core.h"
#include "common.h"
#include "stack.h"
#include "status.h"

static bool __equals(Value a, Value b) {
	if (a.type != b.type) return false;

	// TODO lists
	switch (a.type) {
		case VALUE_NIL:
		case VALUE_TRUE:
		case VALUE_FALSE: return true;
		case VALUE_NUMBER: return a.as.number == b.as.number;
		case VALUE_STRING: {
			int lenA = strlen(a.as.chars.data);
			int lenB = strlen(b.as.chars.data);
			return lenA == lenB && memcmp(a.as.chars.data, b.as.chars.data, lenA) == 0 ? true : false;
		}
		default: return false;
	}
}

static Status _equals(Stack *stack) {
	if (stack->size < 2) return error("expected 2+ arguments");

	Value prev = stack_pop(stack);
	while (stack->size > 0) {
		Value current = stack_pop(stack);
		if (!__equals(prev, current)) {
			stack_empty(stack);
			stack_push(stack, value_make_false());
			return ok();
		}
	}

	stack_push(stack, value_make_true());
	return ok();
}

static Status _less_than(Stack *stack) {
	if (stack->size < 2) return error("expected 2+ arguments");

	Value prev = stack_pop(stack);
	if (prev.type != VALUE_NUMBER) return error("expected number");

	while (stack->size > 0) {
		Value current = stack_pop(stack);
		if (current.type != VALUE_NUMBER) return error("expected number");
		if (!(prev.as.number < current.as.number)) {
			stack_empty(stack);
			stack_push(stack, value_make_false());
			return ok();
		}
	}

	stack_push(stack, value_make_true());
	return ok();
}

static Status _less_than_equals(Stack *stack) {
	if (stack->size < 2) return error("expected 2+ arguments");

	Value prev = stack_pop(stack);
	if (prev.type != VALUE_NUMBER) return error("expected number");

	while (stack->size > 0) {
		Value current = stack_pop(stack);
		if (current.type != VALUE_NUMBER) return error("expected number");
		if (!(prev.as.number <= current.as.number)) {
			stack_empty(stack);
			stack_push(stack, value_make_false());
			return ok();
		}
	}

	stack_push(stack, value_make_true());
	return ok();
}

static Status _greater_than(Stack *stack) {
	if (stack->size < 2) return error("expected 2+ arguments");

	Value prev = stack_pop(stack);
	if (prev.type != VALUE_NUMBER) return error("expected number");

	while (stack->size > 0) {
		Value current = stack_pop(stack);
		if (current.type != VALUE_NUMBER) return error("expected number");
		if (!(prev.as.number > current.as.number)) {
			stack_empty(stack);
			stack_push(stack, value_make_false());
			return ok();
		}
	}

	stack_push(stack, value_make_true());
	return ok();
}

static Status _greater_than_equals(Stack *stack) {
	if (stack->size < 2) return error("expected 2+ arguments");

	Value prev = stack_pop(stack);
	if (prev.type != VALUE_NUMBER) return error("expected number");

	while (stack->size > 0) {
		Value current = stack_pop(stack);
		if (current.type != VALUE_NUMBER) return error("expected number");
		if (!(prev.as.number >= current.as.number)) {
			stack_empty(stack);
			stack_push(stack, value_make_false());
			return ok();
		}
	}

	stack_push(stack, value_make_true());
	return ok();
}

static Status _not(Stack *stack) {
	if (stack->size != 1) return error("expected 1 arguments");

	switch (stack_pop(stack).type) {
		case VALUE_TRUE: stack_push(stack, value_make_false()); return ok();
		case VALUE_FALSE: stack_push(stack, value_make_true()); return ok();
		default: return error("expected true or false");
	}
}

static Status _add(Stack *stack) {
	Number result = 0.0;
	while (stack->size > 0) {
		Value value = stack_pop(stack);
		if (value.type != VALUE_NUMBER) return error("expected number");
		result += value.as.number;
	}
	stack_push(stack, value_make_number(result));
	return ok();
}

static Status _subtract(Stack *stack) {
	switch (stack->size) {
		case 0: return error("expected 1+ arguments");
		case 1: {
			Value value = stack_pop(stack);
			if (value.type != VALUE_NUMBER) return error("expected number");
			stack_push(stack, value_make_number(-value.as.number));
			break;
		}
		default: {
			Value value = stack_pop(stack);
			if (value.type != VALUE_NUMBER) return error("expected number");
			Number result = value.as.number;
			while (stack->size > 0) {
				value = stack_pop(stack);
				if (value.type != VALUE_NUMBER) return error("expected number");
				result -= value.as.number;
			}
			stack_push(stack, value_make_number(result));
		}
	}
	return ok();
}

static Status _multiply(Stack *stack) {
	Number result = 1.0;
	while (stack->size > 0) {
		Value value = stack_pop(stack);
		if (value.type != VALUE_NUMBER) return error("expected number");
		result *= value.as.number;
	}
	stack_push(stack, value_make_number(result));
	return ok();
}

static Status _divide(Stack *stack) {
	switch (stack->size) {
		case 0: return error("expected 1+ arguments");
		case 1: {
			Value value = stack_pop(stack);
			if (value.type != VALUE_NUMBER) return error("expected number");
			stack_push(stack, value_make_number(1.0 / value.as.number));
			break;
		}
		default: {
			Value value = stack_pop(stack);
			if (value.type != VALUE_NUMBER) return error("expected number");
			Number result = value.as.number;
			while (stack->size > 0) {
				value = stack_pop(stack);
				if (value.type != VALUE_NUMBER) return error("expected number");
				result /= value.as.number;
			}
			stack_push(stack, value_make_number(result));
		}
	}
	return ok();
}

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
	env_set(core, value_make_symbol_copy("=", strlen("=")), value_make_fn_ptr(_equals));
	env_set(core, value_make_symbol_copy("<", strlen("<")), value_make_fn_ptr(_less_than));
	env_set(core, value_make_symbol_copy("<=", strlen("<=")), value_make_fn_ptr(_less_than_equals));
	env_set(core, value_make_symbol_copy(">", strlen(">")), value_make_fn_ptr(_greater_than));
	env_set(core, value_make_symbol_copy(">=", strlen(">=")), value_make_fn_ptr(_greater_than_equals));
	env_set(core, value_make_symbol_copy("!", strlen("!")), value_make_fn_ptr(_not));
	env_set(core, value_make_symbol_copy("+", strlen("+")), value_make_fn_ptr(_add));
	env_set(core, value_make_symbol_copy("-", strlen("-")), value_make_fn_ptr(_subtract));
	env_set(core, value_make_symbol_copy("*", strlen("*")), value_make_fn_ptr(_multiply));
	env_set(core, value_make_symbol_copy("/", strlen("/")), value_make_fn_ptr(_divide));
	env_set(core, value_make_symbol_copy("print", strlen("print")), value_make_fn_ptr(_print));
	env_set(core, value_make_symbol_copy("println", strlen("println")), value_make_fn_ptr(_println));
	return core;
}