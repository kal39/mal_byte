#ifndef TABLE_H
#define TABLE_H

#include "value.h"

// TODO: table_remove()

typedef struct Entry {
	char *key;
	Value *value;
} Entry;

typedef struct Table {
	int capacity;
	int size;
	Entry *entries;
} Table;

#define TABLE_MAX_LOAD 0.5

Table *table_create();
void table_destroy(Table *table);

void table_set(Table *table, char *key, Value *value);
Value *table_get(Table *table, char *key);

void table_print(Table *table);

#endif