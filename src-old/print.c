#include "print.h"
#include "common.h"

// TODO: handle pairs
void _value_print(Value *value, Value *error, int *charsPrinted, int *errorAt) {
	if (value == NULL) {
		*charsPrinted += printf("NULL");
		return;
	}

	if (value == error) *errorAt = *charsPrinted;

	switch (TYPE(value)) {
		case VALUE_TYPE_NIL: *charsPrinted += printf("nil"); return;
		case VALUE_TYPE_TRUE: *charsPrinted += printf("true"); return;
		case VALUE_TYPE_FALSE: *charsPrinted += printf("false"); return;
		case VALUE_TYPE_DEF: *charsPrinted += printf("def"); return;
		case VALUE_TYPE_LET: *charsPrinted += printf("let"); return;
		case VALUE_TYPE_DO: *charsPrinted += printf("do"); return;
		case VALUE_TYPE_IF: *charsPrinted += printf("if"); return;
		case VALUE_TYPE_FN: *charsPrinted += printf("fn"); return;
		case VALUE_TYPE_EVAL: *charsPrinted += printf("eval"); return;
		case VALUE_TYPE_QUOTE: *charsPrinted += printf("quote"); return;
		case VALUE_TYPE_IMPORT: *charsPrinted += printf("import"); return;
		case VALUE_TYPE_PAIR: {
			*charsPrinted += printf("(");
			ITERATE_LIST(i, value) {
				_value_print(FIRST(i), error, charsPrinted, errorAt);
				*charsPrinted += printf(!IS_NIL(REST(i)) ? " " : ")");
			}
			return;
		}
		case VALUE_TYPE_SYMBOL: *charsPrinted += printf("'%s'", AS_SYMBOL(value)); return;
		case VALUE_TYPE_NUMBER: *charsPrinted += printf("%g", AS_NUMBER(value)); return;
		case VALUE_TYPE_STRING: *charsPrinted += printf("\"%s\"", AS_STRING(value)); return;
		case VALUE_TYPE_FUNCTION: *charsPrinted += printf("FUNCTION"); return;
		case VALUE_TYPE_C_FUNCTION: *charsPrinted += printf("C_FUNCTION"); return;
		case VALUE_TYPE_ERROR: *charsPrinted += printf("ERROR"); return;
	}
}

void value_print(Value *value) {
	int charsPrinted = 0;
	int errorAt = 0;
	_value_print(value, NULL, &charsPrinted, &errorAt);
}

void error_print(Value *error) {
	int charsPrinted = 0;
	int errorAt = 0;

	if (AS_ERROR(error).expression != NULL && AS_ERROR(error).value != NULL) {
		_value_print(AS_ERROR(error).expression, AS_ERROR(error).value, &charsPrinted, &errorAt);
		printf("\n");
		for (int i = 0; i < errorAt; i++) printf(" ");
		printf("^ ERROR: %s\n", AS_ERROR(error).string);
	} else {
		printf("ERROR: %s\n", AS_ERROR(error).string);
	}
}