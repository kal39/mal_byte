#include "scanner.h"
#include "common.h"

static void _add_token(Scanner *scanner, Token token) {
	if (scanner->tokensSize == scanner->tokensCapacity) scanner->tokens = realloc(scanner->tokens, sizeof(Token) * (scanner->tokensCapacity *= 2));
	scanner->tokens[scanner->tokensSize++] = token;
}

static void _token(Scanner *scanner) {
	_add_token(scanner, (Token){.start = scanner->startChar, .length = scanner->nextChar - scanner->startChar});
	scanner->startChar = scanner->nextChar;
}

static char _next(Scanner *scanner) {
	return *scanner->nextChar++;
}

static char _peek(Scanner *scanner) {
	return *scanner->nextChar;
}

static bool _is_whitespace(char c, int *line) {
	switch (c) {
		case '\n': *line += 1;
		case ' ':
		case '\t':
		case '\v':
		case '\f':
		case '\r':
		case ',': return true;
		default: return false;
	}
}

static bool _is_special(char c) {
	switch (c) {
		case '(':
		case ')': return true;
		default: return false;
	}
}

static bool _at_end(Scanner *scanner) {
	if (_peek(scanner) == '\0') scanner->atEnd = true;
	return scanner->atEnd;
}

static bool _parse_string(Scanner *scanner) {
	int line = 0;

	while (!_at_end(scanner)) {
		char c = _next(scanner);

		if (_is_whitespace(c, &line)) { // skip whitespace
			scanner->startChar = scanner->nextChar;
		} else if (_is_special(c)) { // special characters
			_token(scanner);
		} else if (c == '"') { // strings
			while (_peek(scanner) != '"') {
				if (_at_end(scanner)) {
					return false;
				}
				_next(scanner);
				if (_peek(scanner) == '\\') _next(scanner);
			}
			_next(scanner);
			_token(scanner);
		} else if (c == ';') { // comments
			while (!_is_whitespace(_peek(scanner), &line) && !_at_end(scanner)) _next(scanner);
			_next(scanner);
			scanner->startChar = scanner->nextChar;
		} else { // symbols or numbers
			while (!_is_whitespace(_peek(scanner), &line) && !_is_special(_peek(scanner)) && !_at_end(scanner)) _next(scanner);
			_token(scanner);
		}
	}

	_add_token(scanner, (Token){.length = -1});
	return true;
}

Scanner *scanner_create(char *source) {
	Scanner *scanner = malloc(sizeof(Scanner));

	scanner->atEnd = false;
	scanner->source = source;
	scanner->startChar = scanner->source;
	scanner->nextChar = scanner->source;

	scanner->currentToken = 0;
	scanner->tokensCapacity = 8;
	scanner->tokensSize = 0;
	scanner->tokens = malloc(sizeof(Token) * scanner->tokensCapacity);

	if (_parse_string(scanner)) {
		return scanner;
	} else {
		free(scanner);
		return NULL;
	}
}

bool scanner_scan(Scanner *scanner, char *source) {
	scanner->atEnd = false;
	scanner->source = source;
	scanner->startChar = scanner->source;
	scanner->nextChar = scanner->source;

	scanner->currentToken = 0;
	scanner->tokensCapacity = 8;
	scanner->tokensSize = 0;
	scanner->tokens = malloc(sizeof(Token) * scanner->tokensCapacity);

	return _parse_string(scanner);
}

void scanner_destroy(Scanner *scanner) {
	free(scanner->tokens);
	free(scanner);
}

Token scanner_peek(Scanner *scanner) {
	return scanner->tokens[scanner->currentToken];
}

Token scanner_next(Scanner *scanner) {
	if (scanner->currentToken < scanner->tokensSize - 1) return scanner->tokens[scanner->currentToken++];
	else return scanner->tokens[scanner->currentToken];
}

void scanner_print(Scanner *scanner) {
	for (int i = 0; i < scanner->tokensSize; i++) {
		if (IS_END_TOKEN(scanner->tokens[i])) printf("END");
		else printf("%.*s", scanner->tokens[i].length, scanner->tokens[i].start);
		if (i != scanner->tokensSize - 1) printf(" · ");
	}
}