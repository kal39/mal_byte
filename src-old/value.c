#include "value.h"
#include "common.h"
#include "env.h"

Value *value_create(ValueType type) {
	Value *value = malloc(sizeof(Value));
	TYPE(value) = type;
	return value;
}

Value *value_create_pair(Value *first, Value *rest) {
	Value *value = value_create(VALUE_TYPE_PAIR);
	FIRST(value) = first;
	REST(value) = rest;
	return value;
}

Value *value_create_number(double number) {
	Value *value = value_create(VALUE_TYPE_NUMBER);
	AS_NUMBER(value) = number;
	return value;
}

Value *value_create_string_type(ValueType type, char *string, int stringLength) {
	Value *value = value_create(type);
	AS_STRING(value) = malloc(stringLength + 1);

	int i;
	int skippedChars = 0;
	for (i = 0; i < stringLength; i++) {
		if (string[i] == '\\') {
			skippedChars++;
			i++;
			switch (string[i]) {
				case 'a': AS_STRING(value)[i - skippedChars] = '\a'; break;
				case 'b': AS_STRING(value)[i - skippedChars] = '\b'; break;
				case 'e': AS_STRING(value)[i - skippedChars] = '\e'; break;
				case 'f': AS_STRING(value)[i - skippedChars] = '\f'; break;
				case 'n': AS_STRING(value)[i - skippedChars] = '\n'; break;
				case 'r': AS_STRING(value)[i - skippedChars] = '\r'; break;
				case 't': AS_STRING(value)[i - skippedChars] = '\t'; break;
				case '\\': AS_STRING(value)[i - skippedChars] = '\\'; break;
				case '\'': AS_STRING(value)[i - skippedChars] = '\''; break;
				case '"': AS_STRING(value)[i - skippedChars] = '"'; break;
				case '?': AS_STRING(value)[i - skippedChars] = '\?'; break;
				default: break;
			}
		} else {
			AS_STRING(value)[i - skippedChars] = string[i];
		}
	}

	AS_STRING(value)[i - skippedChars] = '\0';
	return value;
}

Value *value_create_function(Env *outer, Value *args, Value *body) {
	Value *value = value_create(VALUE_TYPE_FUNCTION);
	AS_FUNCTION(value).outer = outer;
	AS_FUNCTION(value).args = args;
	AS_FUNCTION(value).body = body;
	return value;
}

Value *value_create_c_function(Value *(*cFunction)(Value *args)) {
	Value *value = value_create(VALUE_TYPE_C_FUNCTION);
	AS_C_FUNCTION(value) = cFunction;
	return value;
}

Value *value_create_error(char *message, int messageLength, Value *expression, Value *value) {
	Value *ret = value_create_string_type(VALUE_TYPE_ERROR, message, messageLength);
	AS_ERROR(ret).value = value;
	AS_ERROR(ret).expression = expression;
	return ret;
}

Value *list_create() {
	return MAKE_NIL();
}

void list_add_value(Value *list, Value *value) {
	if (IS_NIL(list)) {
		TYPE(list) = VALUE_TYPE_PAIR;
		FIRST(list) = value;
		REST(list) = MAKE_NIL();
	} else if (IS_PAIR(list)) {
		if (IS_NIL(REST(list))) {
			TYPE(REST(list)) = VALUE_TYPE_PAIR;
			SECOND(list) = value;
			REST(REST(list)) = MAKE_NIL();
		} else {
			list_add_value(REST(list), value);
		}
	}
}

int list_length(Value *list) {
	int count = 0;
	ITERATE_LIST(i, list) count++;
	return count;
}

void value_destroy(Value *value) {
	if (value == NULL) return;
	switch (TYPE(value)) {
		case VALUE_TYPE_PAIR:
			value_destroy(AS_PAIR(value).first);
			value_destroy(AS_PAIR(value).rest);
			break;
		case VALUE_TYPE_SYMBOL: free(AS_SYMBOL(value)); break;
		case VALUE_TYPE_STRING: free(AS_STRING(value)); break;
		case VALUE_TYPE_FUNCTION:
			env_destroy(AS_FUNCTION(value).outer);
			value_destroy(AS_FUNCTION(value).args);
			value_destroy(AS_FUNCTION(value).body);
			break;
		case VALUE_TYPE_ERROR: free(AS_ERROR(value).string); break;
		default: break;
	}
	free(value);
}

Value *list_last(Value *list) {
	Value *last;
	for (last = list; !IS_NIL(REST(last)); last = REST(last)) {
	}
	return FIRST(last);
}
