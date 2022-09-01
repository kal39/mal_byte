#include "core.h"
#include "common.h"
#include "eval.h"
#include "parser.h"
#include "print.h"
#include "scanner.h"

#define EXPECT(condition, message, value)                                                                                                            \
	if (!(condition)) return MAKE_ERROR(message, NULL, value)

static bool __equals(Value *a, Value *b) {
	if (TYPE(a) != TYPE(b)) return false;

	switch (TYPE(a)) {
		case VALUE_TYPE_NIL:
		case VALUE_TYPE_TRUE:
		case VALUE_TYPE_FALSE: return true;
		case VALUE_TYPE_PAIR: {
			int lenA = LEN(a);
			int lenB = LEN(b);
			if (lenA != lenB) return false;
			for (int i = 0; i < lenA; i++) {
				if (!__equals(FIRST(a), FIRST(b))) return false;
			}
			return true;
		}
		case VALUE_TYPE_NUMBER: return AS_NUMBER(a) == AS_NUMBER(b);
		case VALUE_TYPE_STRING: {
			int lenA = strlen(AS_STRING(a));
			int lenB = strlen(AS_STRING(b));
			return lenA == lenB && memcmp(AS_STRING(a), AS_STRING(b), lenA) == 0 ? true : false;
		}
		default: return false;
	}
}

static Value *_add(Value *args) {
	double result = 0;
	ITERATE_LIST(i, args) {
		EXPECT(IS_NUMBER(FIRST(i)), "expected number", FIRST(i));
		result += AS_NUMBER(FIRST(i));
	}
	return MAKE_NUMBER(result);
}

static Value *_subtract(Value *args) {
	switch (LEN(args)) {
		case 0: return MAKE_ERROR("expected 1+ arguments", NULL, NULL);
		case 1: EXPECT(IS_NUMBER(FIRST(args)), "expected number", FIRST(args)); return MAKE_NUMBER(-AS_NUMBER(FIRST(args)));
		default: {
			double result = AS_NUMBER(FIRST(args));
			ITERATE_LIST(i, REST(args)) {
				EXPECT(IS_NUMBER(FIRST(i)), "expected number", FIRST(i));
				result -= AS_NUMBER(FIRST(i));
			}
			return MAKE_NUMBER(result);
		}
	}
}

static Value *_multiply(Value *args) {
	double result = 1;
	ITERATE_LIST(i, args) {
		EXPECT(IS_NUMBER(FIRST(i)), "expected number", FIRST(i));
		result *= AS_NUMBER(FIRST(i));
	}
	return MAKE_NUMBER(result);
}

static Value *_divide(Value *args) {
	switch (LEN(args)) {
		case 0: return MAKE_ERROR("expected 1+ arguments", NULL, NULL);
		case 1: EXPECT(IS_NUMBER(FIRST(args)), "expected number", FIRST(args)); return MAKE_NUMBER(1.0 / AS_NUMBER(FIRST(args)));
		default: {
			double result = AS_NUMBER(FIRST(args));
			ITERATE_LIST(i, REST(args)) {
				EXPECT(IS_NUMBER(FIRST(i)), "expected number", FIRST(i));
				result /= AS_NUMBER(FIRST(i));
			}
			return MAKE_NUMBER(result);
		}
	}
}

static Value *_equals(Value *args) {
	EXPECT(LEN(args) >= 2, "expected 2+ arguments", NULL);
	for (Value *i = args; !IS_NIL(REST(i)); i = REST(i)) {
		if (!__equals(FIRST(i), SECOND(i))) return MAKE_FALSE();
	}
	return MAKE_TRUE();
}

static Value *_less_than(Value *args) {
	EXPECT(LEN(args) >= 2, "expected 2+ arguments", NULL);
	for (Value *i = args; !IS_NIL(REST(i)); i = REST(i)) {
		if (AS_NUMBER(FIRST(i)) >= AS_NUMBER(SECOND(i))) return MAKE_FALSE();
	}
	return MAKE_TRUE();
}

static Value *_less_than_equals(Value *args) {
	EXPECT(LEN(args) >= 2, "expected 2+ arguments", NULL);
	for (Value *i = args; !IS_NIL(REST(i)); i = REST(i)) {
		if (AS_NUMBER(FIRST(i)) > AS_NUMBER(SECOND(i))) return MAKE_FALSE();
	}
	return MAKE_TRUE();
}

static Value *_greater_than(Value *args) {
	EXPECT(LEN(args) >= 2, "expected 2+ arguments", NULL);
	for (Value *i = args; !IS_NIL(REST(i)); i = REST(i)) {
		if (AS_NUMBER(FIRST(i)) <= AS_NUMBER(SECOND(i))) return MAKE_FALSE();
	}
	return MAKE_TRUE();
}

static Value *_greater_than_equals(Value *args) {
	EXPECT(LEN(args) >= 2, "expected 2+ arguments", NULL);
	for (Value *i = args; !IS_NIL(REST(i)); i = REST(i)) {
		if (AS_NUMBER(FIRST(i)) < AS_NUMBER(SECOND(i))) return MAKE_FALSE();
	}
	return MAKE_TRUE();
}

static Value *_cons(Value *args) {
	EXPECT(LEN(args) == 2, "expected 2 arguments", NULL);
	return MAKE_PAIR(FIRST(args), SECOND(args));
}

static Value *_first(Value *args) {
	EXPECT(LEN(args) == 1, "expected 1 arguments", NULL);
	return FIRST(FIRST(args));
}

static Value *_rest(Value *args) {
	EXPECT(LEN(args) == 1, "expected 1 arguments", NULL);
	return REST(FIRST(args));
}

static Value *_list(Value *args) {
	Value *list = MAKE_LIST();
	ITERATE_LIST(i, args) ADD_VALUE(list, FIRST(i));
	return list;
}

static Value *_is_list(Value *args) {
	EXPECT(LEN(args) == 1, "expected 1 arguments", NULL);
	return IS_LIST(FIRST(args)) || IS_NIL(FIRST(args)) ? MAKE_TRUE() : MAKE_FALSE();
}

static Value *_is_empty(Value *args) {
	EXPECT(LEN(args) == 1, "expected 1 arguments", NULL);
	return IS_NIL(FIRST(args)) ? MAKE_TRUE() : MAKE_FALSE();
}

static Value *_count(Value *args) {
	EXPECT(LEN(args) == 1, "expected 1 arguments", NULL);
	if (IS_NIL(FIRST(args))) return MAKE_NUMBER(0);
	EXPECT(IS_LIST(FIRST(args)), "expected list", FIRST(args));
	return MAKE_NUMBER(LEN(FIRST(args)));
}

static Value *_not(Value *args) {
	EXPECT(LEN(args) == 1, "expected 1 arguments", NULL);
	if (IS_TRUE(FIRST(args))) return MAKE_FALSE();
	if (IS_FALSE(FIRST(args))) return MAKE_TRUE();
	return MAKE_ERROR("expected boolean", NULL, args);
}

static Value *_string(Value *args) {
	EXPECT(LEN(args) >= 1, "expected 1+ arguments", NULL);

	int totalLen = 0;
	ITERATE_LIST(i, args) {
		switch (TYPE(FIRST(i))) {
			case VALUE_TYPE_NIL: totalLen += snprintf(NULL, 0, "nil"); break;
			case VALUE_TYPE_TRUE: totalLen += snprintf(NULL, 0, "true"); break;
			case VALUE_TYPE_FALSE: totalLen += snprintf(NULL, 0, "false"); break;
			case VALUE_TYPE_NUMBER: totalLen += snprintf(NULL, 0, "%g", AS_NUMBER(FIRST(i))); break;
			case VALUE_TYPE_STRING: totalLen += snprintf(NULL, 0, "%s", AS_STRING(FIRST(i))); break;
			default: return MAKE_ERROR("expected printable", NULL, FIRST(i));
		}
	}

	char *string = malloc(totalLen + 1);
	string[0] = '\0';

	ITERATE_LIST(i, args) {
		switch (TYPE(FIRST(i))) {
			case VALUE_TYPE_NIL: sprintf(string, "nil"); break;
			case VALUE_TYPE_TRUE: sprintf(string + strlen(string), "true"); break;
			case VALUE_TYPE_FALSE: sprintf(string + strlen(string), "false"); break;
			case VALUE_TYPE_NUMBER: sprintf(string + strlen(string), "%g", AS_NUMBER(FIRST(i))); break;
			case VALUE_TYPE_STRING: sprintf(string + strlen(string), "%s", AS_STRING(FIRST(i))); break;
			default: return MAKE_ERROR("expected printable", NULL, FIRST(i));
		}
	}

	Value *value = value_create(VALUE_TYPE_STRING);
	AS_STRING(value) = string;
	return value;
}

static Value *_print(Value *args) {
	EXPECT(LEN(args) >= 1, "expected 1+ arguments", NULL);
	Value *string = _string(args);
	printf("%s", AS_STRING(string));
	return MAKE_NIL();
}

static Value *_println(Value *args) {
	_print(args);
	printf("\n");
	return MAKE_NIL();
}

static Value *_print_debug(Value *args) {
	EXPECT(LEN(args) >= 1, "expected 1+ arguments", NULL);
	ITERATE_LIST(i, args) value_print(FIRST(i));
	return MAKE_NIL();
}

static Value *_println_debug(Value *args) {
	_print_debug(args);
	printf("\n");
	return MAKE_NIL();
}

static Value *_read_file(Value *args) {
	EXPECT(LEN(args) == 1, "expected 1 arguments", NULL);
	EXPECT(IS_STRING(FIRST(args)), "expected string", FIRST(args));
	char *string = read_file(AS_STRING(FIRST(args)));
	if (string == NULL) return MAKE_ERROR("file not found", NULL, FIRST(args));
	else return MAKE_STRING(string);
}

static Value *_parse_string(Value *args) {
	EXPECT(LEN(args) == 1, "expected 1 arguments", NULL);
	EXPECT(IS_STRING(FIRST(args)), "expected string", FIRST(args));
	return FIRST(parse_string(AS_STRING(FIRST(args))));
}

Env *make_core() {
	Env *core = env_create(NULL);
	env_set(core, MAKE_SYMBOL("+"), MAKE_C_FUNCTION(_add));
	env_set(core, MAKE_SYMBOL("-"), MAKE_C_FUNCTION(_subtract));
	env_set(core, MAKE_SYMBOL("*"), MAKE_C_FUNCTION(_multiply));
	env_set(core, MAKE_SYMBOL("/"), MAKE_C_FUNCTION(_divide));
	env_set(core, MAKE_SYMBOL("="), MAKE_C_FUNCTION(_equals));
	env_set(core, MAKE_SYMBOL("<"), MAKE_C_FUNCTION(_less_than));
	env_set(core, MAKE_SYMBOL("<="), MAKE_C_FUNCTION(_less_than_equals));
	env_set(core, MAKE_SYMBOL(">"), MAKE_C_FUNCTION(_greater_than));
	env_set(core, MAKE_SYMBOL(">="), MAKE_C_FUNCTION(_greater_than_equals));
	env_set(core, MAKE_SYMBOL("!"), MAKE_C_FUNCTION(_not));
	env_set(core, MAKE_SYMBOL("cons"), MAKE_C_FUNCTION(_cons));
	env_set(core, MAKE_SYMBOL("first"), MAKE_C_FUNCTION(_first));
	env_set(core, MAKE_SYMBOL("rest"), MAKE_C_FUNCTION(_rest));
	env_set(core, MAKE_SYMBOL("list"), MAKE_C_FUNCTION(_list));
	env_set(core, MAKE_SYMBOL("list?"), MAKE_C_FUNCTION(_is_list));
	env_set(core, MAKE_SYMBOL("empty?"), MAKE_C_FUNCTION(_is_empty));
	env_set(core, MAKE_SYMBOL("count"), MAKE_C_FUNCTION(_count));
	env_set(core, MAKE_SYMBOL("string"), MAKE_C_FUNCTION(_string));
	env_set(core, MAKE_SYMBOL("print"), MAKE_C_FUNCTION(_print));
	env_set(core, MAKE_SYMBOL("println"), MAKE_C_FUNCTION(_println));
	env_set(core, MAKE_SYMBOL("print-dbg"), MAKE_C_FUNCTION(_print_debug));
	env_set(core, MAKE_SYMBOL("println-dbg"), MAKE_C_FUNCTION(_println_debug));
	env_set(core, MAKE_SYMBOL("read-file"), MAKE_C_FUNCTION(_read_file));
	env_set(core, MAKE_SYMBOL("parse-string"), MAKE_C_FUNCTION(_parse_string));

	return core;
}
