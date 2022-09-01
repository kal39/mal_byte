#include "eval.h"
#include "common.h"
#include "print.h"

#define RETURN_IF_ERROR(value)                                                                                                                       \
	if (IS_ERROR(value)) return value
#define EVAL(env, ast, expression) _eval(env, ast, expression, depth + 1, line)
#define EXPECT(condition, message, expression, value)                                                                                                \
	if (!(condition)) {                                                                                                                              \
		result = MAKE_ERROR(message, expression, value);                                                                                             \
		break;                                                                                                                                       \
	}

static Value *_eval(Env *env, Value *ast, Value *expression, int depth, int *line);

static Value *_eval(Env *env, Value *ast, Value *expression, int depth, int *line) {
#ifdef PRINT_EVALUATION_STEPS
	*line = *line + 1;
	int startLine = *line;
	printf("\n");
	for (int i = 0; i < depth; i++) printf(" ║ ");
	value_print(ast);
#endif

	Value *result = ast;

	switch (TYPE(ast)) {
		case VALUE_TYPE_PAIR: {
			switch (TYPE(FIRST(ast))) {
				case VALUE_TYPE_PAIR:
				case VALUE_TYPE_SYMBOL: {
					Value *evaluatedAst = MAKE_LIST();
					ITERATE_LIST(i, ast) {
						Value *value = EVAL(env, FIRST(i), expression);
						RETURN_IF_ERROR(value);
						ADD_VALUE(evaluatedAst, value);
					}

					result = EVAL(env, evaluatedAst, expression);
					RETURN_IF_ERROR(result);
					break;
				}

				case VALUE_TYPE_DEF: {
					EXPECT(LEN(REST(ast)) % 2 == 0, "expected even number arguments", expression, REST(ast));

					for (Value *i = REST(ast); !IS_NIL(i); i = REST(REST(i))) {
						Value *key = FIRST(i);
						result = EVAL(env, SECOND(i), expression);
						RETURN_IF_ERROR(result);
						env_set(env, key, result);
					}
					break;
				}

				case VALUE_TYPE_LET: {
					EXPECT(LEN(REST(ast)) == 2, "expected 2 arguments", expression, REST(ast));
					EXPECT(LEN(SECOND(ast)) % 2 == 0, "expected even number arguments", expression, REST(ast));

					Env *newEnv = env_create(env);
					for (Value *i = SECOND(ast); !IS_NIL(i); i = REST(REST(i))) {
						Value *key = FIRST(i);
						Value *value = EVAL(env, SECOND(i), expression);
						RETURN_IF_ERROR(value);
						env_set(newEnv, key, value);
					}

					result = EVAL(newEnv, THIRD(ast), expression);
					RETURN_IF_ERROR(result);
					env_destroy(newEnv);
					break;
				}

				case VALUE_TYPE_DO: {
					Value *results = MAKE_LIST();
					ITERATE_LIST(i, REST(ast)) {
						Value *value = EVAL(env, FIRST(i), expression);
						RETURN_IF_ERROR(value);
						ADD_VALUE(results, value);
					}
					result = LAST(results);
					break;
				}

				case VALUE_TYPE_IF: {
					EXPECT(LEN(REST(ast)) == 3, "expected 3 arguments", expression, REST(ast));
					Value *condition = EVAL(env, SECOND(ast), expression);
					RETURN_IF_ERROR(condition);
					result = EVAL(env, IS_FALSE(condition) ? FOURTH(ast) : THIRD(ast), expression);
					RETURN_IF_ERROR(result);
					break;
				}

				case VALUE_TYPE_FN: {
					EXPECT(LEN(REST(ast)) == 2, "expected 2 arguments", expression, REST(ast));
					result = MAKE_FUNCTION(env_create(env), SECOND(ast), THIRD(ast));
					break;
				}

				case VALUE_TYPE_EVAL: {
					EXPECT(LEN(REST(ast)) == 1, "expected 1 arguments", expression, REST(ast));
					Value *value = EVAL(env, FIRST(REST(ast)), expression);
					RETURN_IF_ERROR(value);
					result = EVAL(env, value, value);
					RETURN_IF_ERROR(result);
					break;
				}

				case VALUE_TYPE_QUOTE: {
					EXPECT(LEN(REST(ast)) == 1, "expected 1 arguments", expression, REST(ast));
					result = REST(ast);
					break;
				}

				case VALUE_TYPE_IMPORT: {
					EXPECT(LEN(REST(ast)) == 1, "expected 1 arguments", expression, REST(ast));
					EXPECT(IS_STRING(SECOND(ast)), "expected string", expression, SECOND(ast))
					env_load_file(env, AS_STRING(SECOND(ast)));
					result = MAKE_NIL();
					break;
				}

				case VALUE_TYPE_FUNCTION: {
					Value *function = FIRST(ast);
					Value *argNames = AS_FUNCTION(function).args;
					Value *argValues = REST(ast);
					int argNamesLen = LEN(argNames);
					int argValuesLen = LEN(argValues);

					EXPECT(argNamesLen == argValuesLen, "incorrect argument count", expression, argValues);
					Env *funEnv = env_create(AS_FUNCTION(function).outer);

					for (int i = 0; i < LEN(argValues) + 1; i++) {
						env_set(funEnv, FIRST(argNames), FIRST(argValues));
						argNames = REST(argNames);
						argValues = REST(argValues);
					}

					result = EVAL(funEnv, AS_FUNCTION(function).body, AS_FUNCTION(function).body);
					RETURN_IF_ERROR(result);
					break;
				}

				case VALUE_TYPE_C_FUNCTION: {
					result = AS_C_FUNCTION(FIRST(ast))(REST(ast));
					if (IS_ERROR(result)) {
						value_print(FIRST(ast));
						if (AS_ERROR(result).value == NULL) AS_ERROR(result).value = FIRST(ast);
						AS_ERROR(result).expression = expression;
						return result;
					}
					break;
				}
				case VALUE_TYPE_NIL:
				case VALUE_TYPE_TRUE:
				case VALUE_TYPE_FALSE:
				case VALUE_TYPE_NUMBER:
				case VALUE_TYPE_STRING:
				case VALUE_TYPE_ERROR: MAKE_ERROR("expected function", expression, FIRST(ast)); break;
			}
			break;
		}
		case VALUE_TYPE_SYMBOL: {
			result = env_get(env, ast);
			if (IS_ERROR(result)) {
				AS_ERROR(result).expression = expression;
				return result;
			}
			break;
		}
		case VALUE_TYPE_NIL:
		case VALUE_TYPE_TRUE:
		case VALUE_TYPE_FALSE:
		case VALUE_TYPE_DEF:
		case VALUE_TYPE_LET:
		case VALUE_TYPE_DO:
		case VALUE_TYPE_IF:
		case VALUE_TYPE_FN:
		case VALUE_TYPE_EVAL:
		case VALUE_TYPE_QUOTE:
		case VALUE_TYPE_IMPORT:
		case VALUE_TYPE_NUMBER:
		case VALUE_TYPE_STRING:
		case VALUE_TYPE_FUNCTION:
		case VALUE_TYPE_C_FUNCTION:
		case VALUE_TYPE_ERROR: break;
	}

#ifdef PRINT_EVALUATION_STEPS
	if (*line == startLine) {
		printf(" ═ ");
		value_print(result);
	} else {
		*line = *line + 1;
		printf("\n");
		for (int i = 0; i < depth; i++) printf(" ║ ");
		printf(" ╚═ ");
		value_print(result);
	}
#endif

	return result;
}

Value *eval(Env *env, Value *ast) {
	int line = 0;
	Value *result = _eval(env, ast, ast, 0, &line);
#ifdef PRINT_EVALUATION_STEPS
	printf("\n");
#endif
	return result;
}
