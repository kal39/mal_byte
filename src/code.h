#ifndef CODE_H
#define CODE_H

#include "value.h"

typedef enum OpCode {
	OP_RETURN,

	OP_POP,
	OP_PUSH_NIL,
	OP_PUSH_TRUE,
	OP_PUSH_FALSE,
	OP_PUSH_SYMBOL,
	OP_PUSH_NUMBER,
	OP_PUSH_STRING,

	OP_SET_SYMBOL,
	OP_GET_SYMBOL,
	OP_MAKE_FUNCTION,
	OP_CALL_FUNCTION,
	OP_NEW_ENV,

	OP_JUMP,
	OP_JUMP_IF_FALSE,
} OpCode;

typedef struct Code {
	int capacity;
	int size;
	Byte *bytes;
} Code;

Code *code_create();
void code_destroy(Code *code);

void code_write(Code *code, Byte byte);
void code_write_word(Code *code, Word word);
void code_write_number(Code *code, Number number);
void code_write_chars(Code *code, char *chars, int length);

void code_write_at(Code *code, Byte byte, int pos);
void code_write_word_at(Code *code, Word word, int pos);

Byte code_read(Code *code, Word *ip);
Word code_read_word(Code *code, Word *ip);
Number code_read_number(Code *code, Word *ip);
char *code_read_chars(Code *code, Word *ip);

int code_print_instruction(Code *code, Word ip);
void code_print(Code *code);

#endif