#ifndef PARSER_H
#define PARSER_H

#include "scanner.h"
#include "value.h"

char *read_file(char *path);
Value *parse_string(char *string);

#endif