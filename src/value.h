#ifndef VALUE_H
#define VALUE_H

#include "status.h"

typedef struct Stack Stack;
typedef struct Env Env;
typedef struct Value Value;

typedef enum ValueType {
	VALUE_NIL,
	VALUE_TRUE,
	VALUE_FALSE,

	VALUE_SYMBOL,
	VALUE_NUMBER,
	VALUE_STRING,

	VALUE_FN_PTR,
	VALUE_FN,
	VALUE_STATE,
} ValueType;

typedef unsigned char Byte;
typedef unsigned short Word;
typedef float Number;
typedef Status (*fnPtr)(Stack *);

typedef struct Value {
	ValueType type;
	union {
		Number number;
		struct {
			char *data;
			unsigned int hash;
		} chars;
		fnPtr fnPtr;
		struct {
			Env *outer;
			Word argCount;
			Value *keys;
			Word ip;
		} fn;
		struct {
			Env *env;
			Word ip;
		} state;
	} as;
} Value;

Value value_make_nil();
Value value_make_true();
Value value_make_false();
Value value_make_list(Word length);
Value value_make_symbol_borrow(char *symbol);
Value value_make_symbol_copy(char *symbol, int len);
Value value_make_number(Number number);
Value value_make_string_borrow(char *symbol);
Value value_make_string_copy(char *string, int len);
Value value_make_fn_ptr(fnPtr function);
Value value_make_fn(Env *outer, Word argCount, Value *keys, Word ip);
Value value_make_state(Env *env, Word ip);

void value_print(Value value);

#endif