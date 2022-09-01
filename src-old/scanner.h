#ifndef READER_H
#define READER_H

#include "common.h"

typedef struct Token {
	char *start;
	int length;
} Token;

typedef struct Scanner {
	bool atEnd;
	char *source;
	char *startChar;
	char *currentChar;

	int currentToken;
	int tokensCapacity;
	int tokensSize;
	Token *tokens;
} Scanner;

#define IS_END_TOKEN(token) ((token).length == -1)

Scanner *scanner_create(char *source);
void scanner_destroy(Scanner *scanner);

Token scanner_peak(Scanner *scanner);
Token scanner_next(Scanner *scanner);
bool scanner_current_end(Scanner *scanner);
bool scanner_next_end(Scanner *scanner);

void scanner_print(Scanner *scanner);

#endif