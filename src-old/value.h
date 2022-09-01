#ifndef VALUE_H
#define VALUE_H

typedef enum ValueType {
	VALUE_TYPE_NIL,
	VALUE_TYPE_TRUE,
	VALUE_TYPE_FALSE,
	VALUE_TYPE_DEF,
	VALUE_TYPE_LET,
	VALUE_TYPE_DO,
	VALUE_TYPE_IF,
	VALUE_TYPE_FN,
	VALUE_TYPE_EVAL,
	VALUE_TYPE_QUOTE,
	VALUE_TYPE_IMPORT,
	VALUE_TYPE_PAIR,
	VALUE_TYPE_SYMBOL,
	VALUE_TYPE_NUMBER,
	VALUE_TYPE_STRING,
	VALUE_TYPE_FUNCTION,
	VALUE_TYPE_C_FUNCTION,
	VALUE_TYPE_ERROR,
} ValueType;

typedef struct Value Value;
typedef struct Env Env;

typedef struct Value {
	ValueType type;
	union {
		struct {
			Value *first;
			Value *rest;
		} pair;
		double number;
		char *string;
		struct {
			Env *outer;
			Value *args;
			Value *body;
		} function;
		Value *(*cFunction)(Value *args);
		struct {
			char *string;
			Value *expression;
			Value *value;
		} error;
	} as;
} Value;

#define TYPE(value) ((value)->type)

#define MAKE_NIL() value_create(VALUE_TYPE_NIL)
#define MAKE_TRUE() value_create(VALUE_TYPE_TRUE)
#define MAKE_FALSE() value_create(VALUE_TYPE_FALSE)
#define MAKE_DEF() value_create(VALUE_TYPE_DEF)
#define MAKE_LET() value_create(VALUE_TYPE_LET)
#define MAKE_DO() value_create(VALUE_TYPE_DO)
#define MAKE_IF() value_create(VALUE_TYPE_IF)
#define MAKE_FN() value_create(VALUE_TYPE_FN)
#define MAKE_EVAL() value_create(VALUE_TYPE_EVAL)
#define MAKE_QUOTE() value_create(VALUE_TYPE_QUOTE)
#define MAKE_IMPORT() value_create(VALUE_TYPE_IMPORT)
#define MAKE_PAIR(first, rest) value_create_pair(first, rest)
#define MAKE_NUMBER(number) value_create_number(number)
#define MAKE_FUNCTION(env, args, body) value_create_function(env, args, body)
#define MAKE_C_FUNCTION(function) value_create_c_function(function)

#define MAKE_SYMBOL(symbol) value_create_string_type(VALUE_TYPE_SYMBOL, symbol, strlen(symbol))
#define MAKE_SYMBOL_LEN(symbol, symbolLength) value_create_string_type(VALUE_TYPE_SYMBOL, symbol, symbolLength)
#define MAKE_STRING(string) value_create_string_type(VALUE_TYPE_STRING, string, strlen(string))
#define MAKE_STRING_LEN(string, stringLength) value_create_string_type(VALUE_TYPE_STRING, string, stringLength)
#define MAKE_ERROR(message, expression, value) value_create_error(message, strlen(message), expression, value)

#define IS_NIL(value) (TYPE(value) == VALUE_TYPE_NIL)
#define IS_TRUE(value) (TYPE(value) == VALUE_TYPE_TRUE)
#define IS_FALSE(value) (TYPE(value) == VALUE_TYPE_FALSE)
#define IS_DEF(value) (TYPE(value) == VALUE_TYPE_DEF)
#define IS_LET(value) (TYPE(value) == VALUE_TYPE_LET)
#define IS_DO(value) (TYPE(value) == VALUE_TYPE_DO)
#define IS_IF(value) (TYPE(value) == VALUE_TYPE_IF)
#define IS_FN(value) (TYPE(value) == VALUE_TYPE_FN)
#define IS_EVAL(value) (TYPE(value) == VALUE_TYPE_EVAL)
#define IS_QUOTE(value) (TYPE(value) == VALUE_TYPE_QUOTE)
#define IS_IMPORT(value) (TYPE(value) == VALUE_TYPE_IMPORT)
#define IS_PAIR(value) (TYPE(value) == VALUE_TYPE_PAIR)
#define IS_SYMBOL(value) (TYPE(value) == VALUE_TYPE_SYMBOL)
#define IS_NUMBER(value) (TYPE(value) == VALUE_TYPE_NUMBER)
#define IS_STRING(value) (TYPE(value) == VALUE_TYPE_STRING)
#define IS_FUNCTION(value) (TYPE(value) == VALUE_TYPE_FUNCTION)
#define IS_C_FUNCTION(value) (TYPE(value) == VALUE_TYPE_C_FUNCTION)
#define IS_ERROR(value) (TYPE(value) == VALUE_TYPE_ERROR)

#define AS_PAIR(value) (value)->as.pair
#define AS_SYMBOL(value) (value)->as.string
#define AS_NUMBER(value) (value)->as.number
#define AS_STRING(value) (value)->as.string
#define AS_FUNCTION(value) (value)->as.function
#define AS_C_FUNCTION(value) (value)->as.cFunction
#define AS_ERROR(value) (value)->as.error

#define MAKE_LIST() list_create()
#define IS_LIST(value) IS_PAIR(value)
#define LEN(list) list_length(list)
#define REST(list) AS_PAIR(list).rest
#define FIRST(list) AS_PAIR(list).first
#define SECOND(list) FIRST(REST(list))
#define THIRD(list) FIRST(REST(REST(list)))
#define FOURTH(list) FIRST(REST(REST(REST(list))))
#define LAST(list) list_last(list)
#define ADD_VALUE(list, value) list_add_value(list, value)
#define ITERATE_LIST(iterator, list) for (Value *iterator = (list); !IS_NIL(iterator); iterator = REST(iterator))

Value *value_create(ValueType type);

Value *value_create_pair(Value *first, Value *rest);
Value *value_create_number(double number);
Value *value_create_string_type(ValueType type, char *string, int stringLength);
Value *value_create_function(Env *env, Value *args, Value *body);
Value *value_create_c_function(Value *(*cFunction)(Value *args));
Value *value_create_error(char *message, int messageLength, Value *expression, Value *value);

Value *list_create();
void list_add_value(Value *list, Value *value);
int list_length(Value *list);
Value *list_last(Value *list);

void value_destroy(Value *value);

#endif