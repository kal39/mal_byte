#include "value.h"
#include "common.h"
#include "env.h"

static unsigned int _hash(char *string) {
	int length = strlen(string);

	unsigned int hash = 2166136261;
	for (int i = 0; i < length; i++) {
		hash ^= string[i];
		hash *= 16777619;
	}
	return hash;
}

static void _value_copy_chars(Value *value, char *chars, int len) {
	value->as.chars.data = malloc(len + 1);
	memcpy(value->as.chars.data, chars, len);
	value->as.chars.data[len] = '\0';
}

Value value_make_nil() {
	return (Value){.type = VALUE_NIL};
}

Value value_make_true() {
	return (Value){.type = VALUE_TRUE};
}

Value value_make_false() {
	return (Value){.type = VALUE_FALSE};
}

Value value_make_symbol_borrow(char *symbol) {
	Value value = (Value){.type = VALUE_SYMBOL, .as.chars.data = symbol};
	value.as.chars.hash = _hash(value.as.chars.data);
	return value;
}

Value value_make_symbol_copy(char *symbol, int len) {
	Value value = (Value){.type = VALUE_SYMBOL};
	_value_copy_chars(&value, symbol, len);
	value.as.chars.hash = _hash(value.as.chars.data);
	return value;
}

Value value_make_number(Number number) {
	return (Value){.type = VALUE_NUMBER, .as.number = number};
}

Value value_make_string_borrow(char *string) {
	Value value = (Value){.type = VALUE_STRING, .as.chars.data = string};
	value.as.chars.hash = _hash(value.as.chars.data);
	return value;
}

Value value_make_string_copy(char *string, int len) {
	Value value = (Value){.type = VALUE_STRING};
	_value_copy_chars(&value, string, len);
	value.as.chars.hash = _hash(value.as.chars.data);
	return value;
}

Value value_make_fn_ptr(fnPtr function) {
	return (Value){.type = VALUE_FN_PTR, .as.fnPtr = function};
}

Value value_make_fn(Env *outer, Word argCount, Value *keys, Word ip) {
	return (Value){.type = VALUE_FN, .as.fn.outer = outer, .as.fn.argCount = argCount, .as.fn.keys = keys, .as.fn.ip = ip};
}

Value value_make_state(Env *env, Word ip) {
	return (Value){.type = VALUE_STATE, .as.state.env = env, .as.state.ip = ip};
}

void value_free_content(Value value) {
	switch (value.type) {
		case VALUE_SYMBOL:
		case VALUE_STRING: free(value.as.chars.data); break;
		case VALUE_FN: free(value.as.fn.keys); break;
		case VALUE_STATE: free(value.as.state.env); break;
		default: break;
	}
}

void value_print(Value value) {
	switch (value.type) {
		case VALUE_NIL: printf("\e[35mVALUE_NIL\e[0m            ┃"); break;
		case VALUE_TRUE: printf("\e[35mVALUE_TRUE\e[0m           ┃"); break;
		case VALUE_FALSE: printf("\e[35mVALUE_FALSE\e[0m          ┃"); break;
		case VALUE_SYMBOL: printf("\e[35mVALUE_SYMBOL\e[0m         ┃ %s", value.as.chars.data); break;
		case VALUE_NUMBER: printf("\e[35mVALUE_NUMBER\e[0m         ┃ %g", value.as.number); break;
		case VALUE_STRING: printf("\e[35mVALUE_STRING\e[0m         ┃ \"%s\"", value.as.chars.data); break;
		case VALUE_FN_PTR: printf("\e[35mVALUE_FN_PTR\e[0m         ┃ %p", value.as.fnPtr); break;
		case VALUE_FN:
			printf("\e[35mVALUE_FN\e[0m             ┃ \e[2mouter:\e[0m %p \e[2margCount:\e[0m %d \e[2mkeys:\e[0m ", value.as.fn.outer, value.as.fn.argCount);
			for (int i = 0; i < value.as.fn.argCount; i++) printf("%s ", value.as.fn.keys[i].as.chars.data);
			printf("\e[2mip:\e[0m %04d", value.as.fn.ip);
			break;
		case VALUE_STATE:
			printf("\e[35mVALUE_STATE\e[0m          ┃ \e[2menv:\e[0m %p \e[2mip:\e[0m %04d", value.as.state.env, value.as.state.ip);
			break;
	}
}