#include "vm.h"

VM *vm_create(Env *env) {
	VM *vm = malloc(sizeof(VM));
	vm->verbose = false;
	vm->ip = 0;
	vm->stack = NULL;
	return vm;
}

void vm_destroy(VM *vm) {
	if (vm != NULL) free(vm);
}

Status vm_run(VM *vm, Env *env, Code *code) {
	vm->ip = 0;
	if (vm->stack != NULL) stack_destroy(vm->stack);
	vm->stack = stack_create();
	if (vm->verbose) printf("\n==== RUN ====\n\n");

	while (vm->ip < code->size) {
		if (vm->verbose) {
			code_print_instruction(code, vm->ip);
			printf("\n\n");
		}

		switch ((OpCode)code_read(code, &vm->ip)) {
			case OP_RETURN: {
				Value top = stack_pop(vm->stack);
				Value state = stack_pop(vm->stack);
				stack_push(vm->stack, top);

				free(env);
				env = state.as.state.env;
				if (state.as.state.ip != -1) vm->ip = state.as.state.ip;
				break;
			}
			case OP_POP: stack_pop(vm->stack); break;
			case OP_PUSH_NIL: stack_push(vm->stack, value_make_nil()); break;
			case OP_PUSH_TRUE: stack_push(vm->stack, value_make_true()); break;
			case OP_PUSH_FALSE: stack_push(vm->stack, value_make_false()); break;
			case OP_PUSH_SYMBOL: stack_push(vm->stack, value_make_symbol_borrow(code_read_chars(code, &vm->ip))); break;
			case OP_PUSH_NUMBER: stack_push(vm->stack, value_make_number(code_read_number(code, &vm->ip))); break;
			case OP_PUSH_STRING: stack_push(vm->stack, value_make_string_borrow(code_read_chars(code, &vm->ip))); break;
			case OP_SET_SYMBOL: {
				Value value = stack_pop(vm->stack);
				Value key = stack_pop(vm->stack);
				if (key.type != VALUE_SYMBOL) return error("expected symbol");
				env_set(env, key, value);
				stack_push(vm->stack, value);
				break;
			}
			case OP_GET_SYMBOL: {
				Value key = stack_pop(vm->stack);
				if (key.type != VALUE_SYMBOL) return error("expected symbol");
				stack_push(vm->stack, env_get(env, key));
				break;
			}
			case OP_MAKE_FUNCTION: {
				Word start = vm->ip - 1;
				Word argCount = code_read_word(code, &vm->ip);
				Word codeLen = code_read_word(code, &vm->ip);

				Value *args = malloc(argCount * sizeof(Value));
				for (int i = 0; i < argCount; i++) args[i] = value_make_symbol_borrow(code_read_chars(code, &vm->ip));

				stack_push(vm->stack, value_make_fn(env, argCount, args, vm->ip));
				vm->ip = start + codeLen;

				break;
			}
			case OP_CALL_FUNCTION: {
				Word argCount = code_read_word(code, &vm->ip);
				Stack *args = stack_create();
				for (Word i = 0; i < argCount; i++) stack_push(args, stack_pop(vm->stack));
				Value function = stack_pop(vm->stack);

				switch (function.type) {
					case VALUE_FN_PTR: {
						Status result = function.as.fnPtr(args);
						if (!result.ok) return result;
						if (args->size == 0) return error("expected 1+ return values");
						while (args->size > 0) stack_push(vm->stack, stack_pop(args));
						break;
					}
					case VALUE_FN: {
						if (argCount != function.as.fn.argCount) return error("argument count not correct");

						Env *fnEnv = env_create(function.as.fn.outer);
						for (int i = 0; i < argCount; i++) env_set(fnEnv, function.as.fn.keys[i], stack_pop(args));

						stack_push(vm->stack, value_make_state(env, vm->ip));
						vm->ip = function.as.fn.ip;
						env = fnEnv;
						break;
					}
					default: return error("expected function");
				}

				stack_destroy(args);
				break;
			}
			case OP_NEW_ENV: {
				stack_push(vm->stack, value_make_state(env, -1));
				env = env_create(env);
				break;
			}
			case OP_JUMP: vm->ip = code_read_word(code, &vm->ip); break;
			case OP_JUMP_IF_FALSE: {
				Value condition = stack_pop(vm->stack);
				Word ip = code_read_word(code, &vm->ip);

				switch (condition.type) {
					case VALUE_TRUE: break;
					case VALUE_FALSE: vm->ip = ip; break;
					default: return error("expected true or false");
				}
			}
		}

		if (vm->verbose) {
			stack_print(vm->stack);
			printf("\n");
		}
	}
	return ok();
}

void vm_set_verbose(VM *vm, bool verbose) {
	vm->verbose = verbose;
}