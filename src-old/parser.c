#include "parser.h"
#include "common.h"

#define PEEK() scanner_peak(scanner)
#define NEXT() scanner_next(scanner)

static bool _is_digit(char c) {
	return c >= '0' && c <= '9';
}

static bool _is_number(Token token) {
	if (!_is_digit(token.start[0])) return false;
	for (int i = 1; i < token.length; i++) {
		if (!_is_digit(token.start[i]) && token.start[i] != '.') return false;
		if (token.start[i] == '.' && !_is_digit(token.start[i + 1])) return false;
	}
	return true;
}

static bool _match_content(Token token, char *content) {
	return token.length == strlen(content) && memcmp(token.start, content, token.length) == 0;
}

static Value *_read_atom(Scanner *scanner) {
	Token token = NEXT();
	if (_match_content(token, "nil")) return MAKE_NIL();
	if (_match_content(token, "true")) return MAKE_TRUE();
	if (_match_content(token, "false")) return MAKE_FALSE();
	if (_match_content(token, "def")) return MAKE_DEF();
	if (_match_content(token, "let")) return MAKE_LET();
	if (_match_content(token, "do")) return MAKE_DO();
	if (_match_content(token, "if")) return MAKE_IF();
	if (_match_content(token, "fn")) return MAKE_FN();
	if (_match_content(token, "eval")) return MAKE_EVAL();
	if (_match_content(token, "quote")) return MAKE_QUOTE();
	if (_match_content(token, "import")) return MAKE_IMPORT();
	if (_is_number(token)) return MAKE_NUMBER(strtod(token.start, NULL));
	if (token.start[0] == '\"') return MAKE_STRING_LEN(token.start + 1, token.length - 2);
	else return MAKE_SYMBOL_LEN(token.start, token.length);
}

static Value *_parse(Scanner *scanner);

static Value *_read_list(Scanner *scanner) {
	NEXT();
	Value *list = MAKE_LIST();

	for (;;) {
		if (IS_END_TOKEN(PEEK())) return MAKE_ERROR("unterminated list, missing ')", NULL, list);
		if (PEEK().start[0] == ')') break;
		ADD_VALUE(list, _parse(scanner));
	}

	NEXT();
	return list;
}

static Value *_parse(Scanner *scanner) {
	return PEEK().start[0] == '(' ? _read_list(scanner) : _read_atom(scanner);
}

char *read_file(char *path) {
	FILE *fp = fopen(path, "rb");

	if (fp == NULL) return NULL;

	fseek(fp, 0L, SEEK_END);
	size_t fileSize = ftell(fp);
	rewind(fp);

	char *buff = (char *)malloc(fileSize + 1);
	fread(buff, sizeof(char), fileSize, fp);
	buff[fileSize] = '\0';

	fclose(fp);
	return buff;
}

Value *parse_string(char *string) {
	Scanner *scanner = scanner_create(string);
	if (scanner == NULL) return MAKE_ERROR("Unterminated string", NULL, NULL);

	Value *asts = MAKE_LIST();
	while (!IS_END_TOKEN(PEEK())) ADD_VALUE(asts, _parse(scanner));

	scanner_destroy(scanner);
	return asts;
}