#include "vm.h"
#include "stack.h"

static bool _equals(Value a, Value b) {
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

Status _run(Env *env, Code *code, Word *ip, Stack *stack, bool verbose) {
	while (*ip < code->size) {
		if (verbose) {
			code_print_instruction(code, *ip);
			printf("\n\n");
		}

		switch ((OpCode)code_read(code, ip)) {
			case OP_POP: stack_pop(stack); break;
			case OP_PUSH_NIL: stack_push(stack, value_make_nil()); break;
			case OP_PUSH_TRUE: stack_push(stack, value_make_true()); break;
			case OP_PUSH_FALSE: stack_push(stack, value_make_false()); break;
			case OP_PUSH_SYMBOL: stack_push(stack, value_make_symbol_borrow(code_read_chars(code, ip))); break;
			case OP_PUSH_NUMBER: stack_push(stack, value_make_number(code_read_number(code, ip))); break;
			case OP_PUSH_STRING: stack_push(stack, value_make_string_borrow(code_read_chars(code, ip))); break;
			case OP_SET_SYMBOL: {
				Value value = stack_pop(stack);
				Value key = stack_pop(stack);
				if (key.type != VALUE_SYMBOL) return error("expected symbol");
				env_set(env, key, value);
				stack_push(stack, value);
				break;
			}
			case OP_GET_SYMBOL: {
				Value key = stack_pop(stack);
				if (key.type != VALUE_SYMBOL) return error("expected symbol");
				stack_push(stack, env_get(env, key));
				break;
			}
			case OP_MAKE_FUNCTION: {
				Word start = *ip - 1;
				Word argCount = code_read_word(code, ip);
				Word codeLen = code_read_word(code, ip);

				Value *args = malloc(argCount * sizeof(Value));
				for (int i = 0; i < argCount; i++) args[i] = value_make_symbol_borrow(code_read_chars(code, ip));

				stack_push(stack, value_make_fn(env, argCount, args, *ip));
				*ip = start + codeLen;

				break;
			}
			case OP_CALL_FUNCTION: {
				Word argCount = code_read_word(code, ip);
				Stack *args = stack_create();
				for (Word i = 0; i < argCount; i++) stack_push(args, stack_pop(stack));
				Value function = stack_pop(stack);

				switch (function.type) {
					case VALUE_FN_PTR: {
						Status result = function.as.fnPtr(args);
						if (!result.ok) return result;
						if (args->size == 0) return error("expected 1+ return values");
						while (args->size > 0) stack_push(stack, stack_pop(args));
						break;
					}
					case VALUE_FN: {
						if (argCount != function.as.fn.argCount) return error("argument count not correct");

						Env *fnEnv = env_create(function.as.fn.outer);
						for (int i = 0; i < argCount; i++) env_set(fnEnv, function.as.fn.keys[i], stack_pop(args));

						stack_push(stack, value_make_state(env, *ip));
						*ip = function.as.fn.ip;
						env = fnEnv;
						break;
					}
					default: return error("expected function");
				}

				stack_destroy(args);
				break;
			}
			case OP_NEW_ENV: {
				stack_push(stack, value_make_state(env, -1));
				env = env_create(env);
				break;
			}
			case OP_RETURN: {
				Value top = stack_pop(stack);
				Value state = stack_pop(stack);
				stack_push(stack, top);

				free(env);
				env = state.as.state.env;
				if (state.as.state.ip != -1) *ip = state.as.state.ip;
				break;
			}
			case OP_JUMP: *ip = code_read_word(code, ip); break;
			case OP_JUMP_IF_FALSE: {
				Value condition = stack_pop(stack);
				Word newIp = code_read_word(code, ip);

				switch (condition.type) {
					case VALUE_TRUE: break;
					case VALUE_FALSE: *ip = newIp; break;
					default: return error("expected true or false");
				}
				break;
			}

			case OP_EQ: {
				int argCount = code_read_word(code, ip);
				if (argCount < 2) return error("expected 2+ arguments");

				bool equals = true;
				Value prev = stack_pop(stack);
				for (int i = 0; i < argCount - 1; i++) {
					Value current = stack_pop(stack);
					if (!_equals(current, prev)) equals = false;
					prev = current;
				}

				stack_push(stack, equals ? value_make_true() : value_make_false());
				break;
			}

			case OP_LESS: {
				int argCount = code_read_word(code, ip);
				if (argCount < 2) return error("expected 2+ arguments");

				bool equals = true;
				Value prev = stack_pop(stack);
				if (prev.type != VALUE_NUMBER) return error("expected number");
				for (int i = 0; i < argCount - 1; i++) {
					Value current = stack_pop(stack);
					if (current.type != VALUE_NUMBER) return error("expected number");
					if (!(current.as.number < prev.as.number)) equals = false;
					prev = current;
				}

				stack_push(stack, equals ? value_make_true() : value_make_false());
				break;
			}

			case OP_LESS_EQ: {
				int argCount = code_read_word(code, ip);
				if (argCount < 2) return error("expected 2+ arguments");

				bool equals = true;
				Value prev = stack_pop(stack);
				if (prev.type != VALUE_NUMBER) return error("expected number");
				for (int i = 0; i < argCount - 1; i++) {
					Value current = stack_pop(stack);
					if (current.type != VALUE_NUMBER) return error("expected number");
					if (!(current.as.number <= prev.as.number)) equals = false;
					prev = current;
				}

				stack_push(stack, equals ? value_make_true() : value_make_false());
				break;
			}

			case OP_GREATER: {
				int argCount = code_read_word(code, ip);
				if (argCount < 2) return error("expected 2+ arguments");

				bool equals = true;
				Value prev = stack_pop(stack);
				if (prev.type != VALUE_NUMBER) return error("expected number");
				for (int i = 0; i < argCount - 1; i++) {
					Value current = stack_pop(stack);
					if (current.type != VALUE_NUMBER) return error("expected number");
					if (!(current.as.number > prev.as.number)) equals = false;
					prev = current;
				}

				stack_push(stack, equals ? value_make_true() : value_make_false());
				break;
			}

			case OP_GREATER_EQ: {
				int argCount = code_read_word(code, ip);
				if (argCount < 2) return error("expected 2+ arguments");

				bool equals = true;
				Value prev = stack_pop(stack);
				if (prev.type != VALUE_NUMBER) return error("expected number");
				for (int i = 0; i < argCount - 1; i++) {
					Value current = stack_pop(stack);
					if (current.type != VALUE_NUMBER) return error("expected number");
					if (!(current.as.number >= prev.as.number)) equals = false;
					prev = current;
				}

				stack_push(stack, equals ? value_make_true() : value_make_false());
				break;
			}

			case OP_ADD: {
				int argCount = code_read_word(code, ip);
				Number result = 0.0;
				for (int i = 0; i < argCount; i++) {
					Value value = stack_pop(stack);
					if (value.type != VALUE_NUMBER) return error("expected number");
					result += value.as.number;
				}
				stack_push(stack, value_make_number(result));
				break;
			}
			case OP_SUB: {
				int argCount = code_read_word(code, ip);
				switch (argCount) {
					case 0: return error("expected 1+ arguments");
					case 1: {
						Value value = stack_pop(stack);
						if (value.type != VALUE_NUMBER) return error("expected number");
						stack_push(stack, value_make_number(-value.as.number));
						break;
					}
					default: {
						Number result = 0.0;
						for (int i = 0; i < argCount - 1; i++) {
							Value value = stack_pop(stack);
							if (value.type != VALUE_NUMBER) return error("expected number");
							result += value.as.number;
						}
						Value value = stack_pop(stack);
						if (value.type != VALUE_NUMBER) return error("expected number");
						stack_push(stack, value_make_number(value.as.number - result));
					}
				}
				break;
			}
			case OP_MUL: {
				int argCount = code_read_word(code, ip);
				Number result = 1.0;
				for (int i = 0; i < argCount; i++) {
					Value value = stack_pop(stack);
					if (value.type != VALUE_NUMBER) return error("expected number");
					result *= value.as.number;
				}
				stack_push(stack, value_make_number(result));
				break;
			}
			case OP_DIV: {
				int argCount = code_read_word(code, ip);
				switch (argCount) {
					case 0: return error("expected 1+ arguments");
					case 1: {
						Value value = stack_pop(stack);
						if (value.type != VALUE_NUMBER) return error("expected number");
						stack_push(stack, value_make_number(1.0 / value.as.number));
						break;
					}
					default: {
						Number result = 1.0;
						for (int i = 0; i < argCount - 1; i++) {
							Value value = stack_pop(stack);
							if (value.type != VALUE_NUMBER) return error("expected number");
							result *= value.as.number;
						}
						Value value = stack_pop(stack);
						if (value.type != VALUE_NUMBER) return error("expected number");
						stack_push(stack, value_make_number(value.as.number / result));
					}
				}
				break;
			}
		}

		if (verbose) {
			stack_print(stack);
			printf("\n");
		}
	}
	return ok();
}

Status run(Env *env, Code *code, bool verbose) {
	// vm layout: {code struct}{code bytes}{*ip(Word)}{stack struct}
	void *vm = malloc(sizeof(Code) + code->size + sizeof(Word) + sizeof(Stack));

	// copy code into vm
	memcpy(vm, code, sizeof(Code));
	memcpy(vm + sizeof(Code), code->bytes, code->size);

	// initialize code
	Code *_code = vm;
	_code->capacity = _code->size;
	_code->bytes = vm + sizeof(Code);

	// initialize *ip
	Word *ip = vm + sizeof(Code) + code->size;
	*ip = 0;

	// initialize stack
	Stack *stack = vm + sizeof(Code) + code->size + sizeof(Word);
	stack->size = 0;

	Status result = _run(env, code, ip, stack, verbose);
	free(vm);
	return result;
}