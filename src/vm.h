#ifndef VM_H
#define VM_H

#include "code.h"
#include "common.h"
#include "env.h"
#include "status.h"

Status run(Env *env, Code *code, bool verbose);

#endif