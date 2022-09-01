#include "common.h"
#include "compiler.h"
#include "core.h"
#include "vm.h"

int main(void) {
	Code *code = code_create();
	// Status error = compile(code, "(- 2 1)");
	// Status error = compile(code, "(println (+ 1 (* 2 3) 4) \" \" 5)");
	// Status error = compile(code, "(def add + a 2 b (* 2 3)) (add a b)");
	// Status error = compile(code, "(let (a 2 b 3) (+ a b))");
	// Status error = compile(code, "(do (def a 2) (def b 3) (+ a b))");
	// Status error = compile(code, "((fn (a b) (+ a b)) 2 3)");
	// Status error = compile(code, "(def add_1 (fn (a) (+ a 1))) (add_1 6)");
	// Status error = compile(code, "(if false 2 3)");
	Status error = compile(code, "(def fib (fn (i) (if (< i 2) i (+ (fib (- i 1)) (fib(- i 2)))))) (println (fib 30))");

	if (!error.ok) {
		printf("ERROR: %s\n", error.errorMessage);
		exit(-1);
	}

	code_print(code);

	Env *core = make_core();

	VM *vm = vm_create(core);
	// vm_set_verbose(vm, true);

	error = vm_run(vm, core, code);

	if (!error.ok) {
		printf("ERROR: %s\n", error.errorMessage);
		exit(-1);
	}

	code_destroy(code);
	env_destroy(core);
	vm_destroy(vm);

	return 0;
}