#include "code.h"
#include "common.h"

Code *code_create() {
	Code *code = malloc(sizeof(Code));
	code->capacity = 8;
	code->size = 0;
	code->bytes = malloc(code->capacity);
	return code;
}

void code_destroy(Code *code) {
	if (code->bytes != NULL) free(code->bytes);
	if (code != NULL) free(code);

	code->bytes = NULL;
	code = NULL;
}

void code_write(Code *code, Byte byte) {
	if (code->size == code->capacity) code->bytes = realloc(code->bytes, code->capacity *= 2);
	code->bytes[code->size++] = byte;
}

void code_write_word(Code *code, Word word) {
	for (int i = 0; i < sizeof(Word); i++) code_write(code, ((Byte *)&word)[i]);
}

void code_write_number(Code *code, Number number) {
	for (int i = 0; i < sizeof(Number); i++) code_write(code, ((Byte *)&number)[i]);
}

void code_write_chars(Code *code, char *chars, int length) {
	for (int i = 0; i < length * sizeof(char); i++) code_write(code, chars[i]);
	code_write(code, '\0');
}

void code_write_at(Code *code, Byte byte, int pos) {
	code->bytes[pos] = byte;
}

void code_write_word_at(Code *code, Word word, int pos) {
	for (int i = 0; i < sizeof(Word); i++) code_write_at(code, ((Byte *)&word)[i], pos + i);
}

Byte code_read(Code *code, Word *ip) {
	Byte byte = code->bytes[*ip];
	*ip += 1;
	return byte;
}

Word code_read_word(Code *code, Word *ip) {
	Word number = *(Word *)(&code->bytes[*ip]);
	*ip += sizeof(Word);
	return number;
}

Number code_read_number(Code *code, Word *ip) {
	Number number = *(Number *)(&code->bytes[*ip]);
	*ip += sizeof(Number);
	return number;
}

char *code_read_chars(Code *code, Word *ip) {
	char *chars = (char *)&code->bytes[*ip];
	*ip += strlen(chars) + 1;
	return chars;
}

int code_print_instruction(Code *code, Word ip) {
	Word oldIp = ip;

	printf("┃ %04d ┃ ", ip);
	switch ((OpCode)code_read(code, &ip)) { // cast to make gcc check that all opcodes are accounted for
		case OP_RETURN: printf("\e[34mOP_RETURN\e[0m        ┃"); break;
		case OP_POP: printf("\e[34mOP_POP\e[0m           ┃"); break;
		case OP_PUSH_NIL: printf("\e[34mOP_PUSH_NIL\e[0m      ┃"); break;
		case OP_PUSH_TRUE: printf("\e[34mOP_PUSH_TRUE\e[0m     ┃"); break;
		case OP_PUSH_FALSE: printf("\e[34mOP_PUSH_FALSE\e[0m    ┃"); break;
		case OP_PUSH_SYMBOL: printf("\e[34mOP_SYMBOL\e[0m        ┃ %s", code_read_chars(code, &ip)); break;
		case OP_PUSH_NUMBER: printf("\e[34mOP_PUSH_NUMBER\e[0m   ┃ %g", code_read_number(code, &ip)); break;
		case OP_PUSH_STRING: printf("\e[34mOP_PUSH_STRING\e[0m   ┃ %s", code_read_chars(code, &ip)); break;
		case OP_SET_SYMBOL: printf("\e[34mOP_SET_SYMBOL\e[0m    ┃"); break;
		case OP_GET_SYMBOL: printf("\e[34mOP_GET_SYMBOL\e[0m    ┃"); break;
		case OP_MAKE_FUNCTION: {
			Word argCount = code_read_word(code, &ip);
			Word codeLen = code_read_word(code, &ip);
			printf("\e[34mOP_MAKE_FUNCTION\e[0m ┃ \e[2mcode length:\e[0m %d  \e[2marg count:\e[0m %d \e[2margs:\e[0m ", codeLen, argCount);
			for (int i = 0; i < argCount; i++) printf("%s ", code_read_chars(code, &ip));
			break;
		}
		case OP_CALL_FUNCTION: printf("\e[34mOP_CALL_FUNCTION\e[0m ┃ \e[2marg count:\e[0m %d", code_read_word(code, &ip)); break;
		case OP_NEW_ENV: printf("\e[34mOP_NEW_ENV\e[0m       ┃"); break;
		case OP_JUMP: printf("\e[34mOP_JUMP\e[0m          ┃ \e[2mto:\e[0m %04d", code_read_word(code, &ip)); break;
		case OP_JUMP_IF_FALSE: printf("\e[34mOP_JUMP_IF_FALSE\e[0m ┃ \e[2mto:\e[0m %04d", code_read_word(code, &ip)); break;
	}

	return ip - oldIp;
}

void code_print(Code *code) {
	printf("┏━ ip ━┳━━━━━ Opcode ━━━━━┳━━━━━ arguments ━━━━━\n");
	for (int ip = 0; ip < code->size;) {
		ip += code_print_instruction(code, ip);
		printf("\n");
	}
}