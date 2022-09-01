#ifndef SCANNER_H
#define SCANNER_H

#include "common.h"

typedef struct Token {
	char *start;
	int length;
} Token;

typedef struct Scanner {
	bool atEnd;
	char *source;
	char *startChar;
	char *nextChar;

	int currentToken;
	int tokensCapacity;
	int tokensSize;
	Token *tokens;
} Scanner;

#define IS_END_TOKEN(token) ((token).length == -1)

Scanner *scanner_create(char *source);
void scanner_destroy(Scanner *scanner);

Token scanner_peek(Scanner *scanner);
Token scanner_next(Scanner *scanner);

void scanner_print(Scanner *scanner);

#endif