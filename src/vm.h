#ifndef VM_H
#define VM_H

#include "code.h"
#include "common.h"
#include "env.h"
#include "stack.h"
#include "status.h"

typedef struct VM {
	bool verbose;
	Word ip;
	Stack *stack;
} VM;

VM *vm_create(Env *env);
void vm_destroy(VM *vm);

Status vm_run(VM *vm, Env *env, Code *code);
void vm_set_verbose(VM *vm, bool verbose);

#endif