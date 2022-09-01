#ifndef EVAL_H
#define EVAL_H

#include "env.h"
#include "value.h"

Value *eval(Env *env, Value *value);

#endif