#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PRINT_INDENT_SIZE 4
// #define PRINT_EVALUATION_STEPS

#define STRING_EQUALS(a, b) (strlen(a) == strlen(b) && memcmp((a), (b), strlen(a)) == 0)

#endif