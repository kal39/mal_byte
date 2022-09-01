#include "compiler.h"
#include "scanner.h"

static bool _is_digit(char c) {
	return c >= '0' && c <= '9';
}

static bool _matches(Token token, char *pattern) {
	return token.length == strlen(pattern) && memcmp(token.start, pattern, token.length) == 0;
}

static bool _is_list_start(Token token) {
	return token.length == 1 && token.start[0] == '(';
}

static bool _is_list_end(Token token) {
	return token.length == 1 && token.start[0] == ')';
}

static bool _is_string(Token token) {
	return token.start[0] == '\"';
}

static bool _is_number(Token token) {
	if (!_is_digit(token.start[0])) return false;
	for (int i = 1; i < token.length; i++) {
		if (!_is_digit(token.start[i]) && token.start[i] != '.') return false;
		if (token.start[i] == '.' && !_is_digit(token.start[i + 1])) return false;
	}
	return true;
}

static Status _compile(Code *code, Scanner *scanner);
static ValueType _compile_atom(Code *code, Token token);
static Status _compile_list(Code *code, Scanner *scanner);

static Status _compile_def(Code *code, Scanner *scanner) {
	for (;;) {
		// compile key
		Token key = scanner_next(scanner);
		if (_compile_atom(code, key) != VALUE_SYMBOL) return error("expected symbol");

		// compile value
		Status status = _compile(code, scanner);
		if (!status.ok) return status;

		code_write(code, OP_SET_SYMBOL);

		// only leave last value on the stack
		if (!_is_list_end(scanner_peek(scanner))) code_write(code, OP_POP);
		else break;
	}

	return ok();
}

static Status _compile_let(Code *code, Scanner *scanner) {
	code_write(code, OP_NEW_ENV);
	if (!_is_list_start(scanner_next(scanner))) return error("expected '('");

	// compile key-value pairs
	while (!_is_list_end(scanner_peek(scanner))) {
		// compile key
		Token key = scanner_next(scanner);
		if (_compile_atom(code, key) != VALUE_SYMBOL) return error("expected symbol");

		// compile value
		Status status = _compile(code, scanner);
		if (!status.ok) return status;

		code_write(code, OP_SET_SYMBOL);
		code_write(code, OP_POP); // leave the stack clean
	}

	if (!_is_list_end(scanner_next(scanner))) return error("expected ')'");

	Status status = _compile(code, scanner);
	if (!status.ok) return status;

	code_write(code, OP_RETURN);

	return ok();
}

static Status _compile_do(Code *code, Scanner *scanner) {
	for (;;) {
		Status status = _compile(code, scanner);
		if (!status.ok) return status;

		// only leave last result on the stack
		if (!_is_list_end(scanner_peek(scanner))) code_write(code, OP_POP);
		else break;
	}

	return ok();
}

static Status _compile_if(Code *code, Scanner *scanner) {
	// compile condition
	Status status = _compile(code, scanner);
	if (!status.ok) return status;

	code_write(code, OP_JUMP_IF_FALSE);

	// placeholder for false branch start location
	Word jump1 = code->size;
	code_write_word(code, 0);

	// compile true branch
	status = _compile(code, scanner);
	if (!status.ok) return status;

	code_write(code, OP_JUMP);

	// placeholder for false branch end location
	Word jump2 = code->size;
	code_write_word(code, 0);

	// overwrite first placeholder
	code_write_word_at(code, code->size, jump1);

	// compile false branch
	status = _compile(code, scanner);
	if (!status.ok) return status;

	// overwrite second placeholder
	code_write_word_at(code, code->size, jump2);

	return ok();
}

static Status _compile_fn(Code *code, Scanner *scanner) {
	if (!_is_list_start(scanner_next(scanner))) return error("expected '('");

	Word start = code->size;
	code_write(code, OP_MAKE_FUNCTION);

	// placeholders for argument count and code length
	code_write_word(code, 0);
	code_write_word(code, 0);

	// compile argument names (keys)
	Word argCount = 0;
	for (; !_is_list_end(scanner_peek(scanner)); argCount++) {
		Token arg = scanner_next(scanner);
		code_write_chars(code, arg.start, arg.length);
	}

	if (!_is_list_end(scanner_next(scanner))) return error("expected ')'");

	// compile main body of the function
	Status status = _compile(code, scanner);
	if (!status.ok) return status;

	code_write(code, OP_RETURN);

	// these values overwrite the placeholders made above
	code_write_word_at(code, argCount, start + 1);
	code_write_word_at(code, code->size - start, start + 1 + sizeof(Word));

	return ok();
}

static Status _compile_fn_call(Code *code, Scanner *scanner, Token token) {
	// if not a built-in keyword, it must be a function
	if (_is_list_start(token)) {
		// for when the function itself is the result of another operation (eg: ((fn add_1 (a) (+ a 1)) 2) )
		Status status = _compile_list(code, scanner);
		if (!status.ok) return status;
	} else if (_compile_atom(code, token) == VALUE_SYMBOL) {
		code_write(code, OP_GET_SYMBOL);
	} else {
		return error("expected symbol");
	}

	// compile arguments
	Word argCount = 0;
	for (;; argCount++) {
		if (IS_END_TOKEN(scanner_peek(scanner))) return error("unterminated list");
		if (_is_list_end(scanner_peek(scanner))) break;
		Status status = _compile(code, scanner);
		if (!status.ok) return status;
	}

	code_write(code, OP_CALL_FUNCTION);
	code_write_word(code, argCount);

	return ok();
}

static ValueType _compile_atom(Code *code, Token token) {
	if (_matches(token, "nil")) {
		code_write(code, OP_PUSH_NIL);
		return VALUE_NIL;
	} else if (_matches(token, "true")) {
		code_write(code, OP_PUSH_TRUE);
		return VALUE_TRUE;
	} else if (_matches(token, "false")) {
		code_write(code, OP_PUSH_FALSE);
		return VALUE_FALSE;
	} else if (_is_number(token)) {
		code_write(code, OP_PUSH_NUMBER);
		code_write_number(code, strtod(token.start, NULL));
		return VALUE_NUMBER;
	} else if (_is_string(token)) {
		code_write(code, OP_PUSH_STRING);
		code_write_chars(code, token.start + 1, token.length - 2);
		return VALUE_STRING;
	} else {
		code_write(code, OP_PUSH_SYMBOL);
		code_write_chars(code, token.start, token.length);
		return VALUE_SYMBOL;
	}
}

static Status _compile_list(Code *code, Scanner *scanner) {
	Token token = scanner_next(scanner);
	Status status = ok();

	if (_matches(token, "def")) status = _compile_def(code, scanner);
	else if (_matches(token, "let")) status = _compile_let(code, scanner);
	else if (_matches(token, "do")) status = _compile_do(code, scanner);
	else if (_matches(token, "if")) status = _compile_if(code, scanner);
	else if (_matches(token, "fn")) status = _compile_fn(code, scanner);
	else if (_matches(token, "eval")) status = error("\"eval\" not yet implemented");	// TODO implement
	else if (_matches(token, "quote")) status = error("\"quote\" not yet implemented"); // TODO implement
	else status = _compile_fn_call(code, scanner, token);

	if (!_is_list_end(scanner_next(scanner))) status = error("expected ')'");
	return status;
}

static Status _compile(Code *code, Scanner *scanner) {
	Token token = scanner_next(scanner);
	if (_is_list_end(token)) {
		return error("did not expect ')'");
	} else if (_is_list_start(token)) {
		return _compile_list(code, scanner);
	} else {
		// for a number, OP_GET_SYMBOL isn't needed
		if (_compile_atom(code, token) == VALUE_SYMBOL) code_write(code, OP_GET_SYMBOL);
		return ok();
	}
}

Status compile(Code *code, char *source) {
	Scanner *scanner = scanner_create(source);
	if (scanner == NULL) return error("unterminated string");

	while (!IS_END_TOKEN(scanner_peek(scanner))) {
		Status status = _compile(code, scanner);
		if (!status.ok) return status;
	}

	scanner_destroy(scanner);
	return ok();
}