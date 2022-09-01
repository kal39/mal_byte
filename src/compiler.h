#ifndef COMPILER_H
#define COMPILER_H

#include "code.h"
#include "status.h"

Status compile(Code *code, char *source);

#endif