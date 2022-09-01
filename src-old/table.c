#include "table.h"
#include "common.h"
#include "print.h"

static unsigned int _hash(char *string) {
	int length = strlen(string);

	unsigned int hash = 2166136261;
	for (int i = 0; i < length; i++) {
		hash ^= string[i];
		hash *= 16777619;
	}
	return hash;
}

static Entry *_find(Table *table, char *key) {
	for (unsigned int i = _hash(key) % table->capacity;; i = (i + 1) % table->capacity) {
		Entry *entry = &table->entries[i];
		if (entry->key == NULL || STRING_EQUALS(entry->key, key)) return entry;
	}
}

static void _resize(Table *table, int newCapacity) {
	Entry *oldEntries = table->entries;
	int oldCapacity = table->capacity;

	table->capacity = newCapacity;
	table->entries = malloc(sizeof(Entry) * table->capacity);
	for (int i = 0; i < table->capacity; i++) table->entries[i].key = NULL;

	for (int i = 0; i < oldCapacity; i++) {
		Entry *entry = &oldEntries[i];
		if (entry->key != NULL) {
			Entry *dest = _find(table, entry->key);
			dest->key = entry->key;
			dest->value = entry->value;
		}
	}

	free(oldEntries);
}

Table *table_create() {
	Table *table = malloc(sizeof(Table));
	table->capacity = 0;
	table->size = 0;
	_resize(table, 8);

	return table;
}

void table_destroy(Table *table) {
	free(table->entries);
	free(table);
}

void table_set(Table *table, char *key, Value *value) {
	if (table->size == table->capacity * TABLE_MAX_LOAD) _resize(table, table->capacity * 2);

	Entry *entry = _find(table, key);
	if (entry->key == NULL) table->size++;

	entry->key = key;
	entry->value = value;
}

Value *table_get(Table *table, char *key) {
	Entry *entry = _find(table, key);
	return entry->key == NULL ? NULL : entry->value;
}

void table_print(Table *table) {
	for (int i = 0; i < table->capacity; i++) {
		Entry *entry = &table->entries[i];
		if (entry->key != NULL) {
			printf("\"%s\": ", entry->key);
			value_print(entry->value);
			printf("\n");
		}
	}
}