#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRING_EQUALS(a, b) (strlen(a) == strlen(b) && memcmp((a), (b), strlen(a)) == 0)

#endif