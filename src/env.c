#include "env.h"
#include "common.h"

Env *env_create(Env *outer) {
	Env *env = malloc(sizeof(Env));
	env->outer = outer;
	env->table = table_create();
	return env;
}

void env_destroy(Env *env) {
	table_destroy(env->table);
	free(env);
}

void env_set(Env *env, Value key, Value value) {
	table_set(env->table, key, value);
}

Value env_get(Env *env, Value key) {
	for (Env *e = env; e != NULL; e = e->outer) {
		Value *value = table_get(e->table, key);
		if (value != NULL) return *value;
	}
	return value_make_nil();
}

void env_print(Env *env) {
	printf("\n==== ENV ====\n\n");
	table_print(env->table);
	printf("\n");
	if (env->outer != NULL) env_print(env->outer);
}