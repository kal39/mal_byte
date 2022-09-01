#include "table.h"
#include "common.h"

static unsigned int _hash(char *string) {
	int length = strlen(string);

	unsigned int hash = 2166136261;
	for (int i = 0; i < length; i++) {
		hash ^= string[i];
		hash *= 16777619;
	}
	return hash;
}

static Entry *_find(Table *table, Value key) {
	for (unsigned int i = key.as.chars.hash % table->capacity;; i = (i + 1) % table->capacity) {
		Entry *entry = &table->entries[i];
		if (entry->key == NULL || STRING_EQUALS(entry->key, key.as.chars.data)) return entry;
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
			Entry *dest = _find(table, (Value){.as.chars.hash = _hash(entry->key), .as.chars.data = entry->key});
			dest->key = entry->key;
			dest->value = entry->value;
		}
	}

	if (oldEntries != NULL) free(oldEntries);
}

Table *table_create() {
	Table *table = malloc(sizeof(Table));
	table->capacity = 0;
	table->size = 0;
	table->entries = NULL;

	_resize(table, 8);
	return table;
}

void table_destroy(Table *table) {
	for (int i = 0; i < table->capacity; i++) {
		if (table->entries[i].key != NULL) free(table->entries[i].key);
	}

	free(table->entries);
	free(table);
}

void table_set(Table *table, Value key, Value value) {
	if (table->size == table->capacity * TABLE_MAX_LOAD) _resize(table, table->capacity * 2);

	Entry *entry = _find(table, key);
	if (entry->key == NULL) table->size++;

	int len = strlen(key.as.chars.data) + 1;
	entry->key = malloc(len);
	memcpy(entry->key, key.as.chars.data, len);
	entry->value = value;
}

Value *table_get(Table *table, Value key) {
	Entry *entry = _find(table, key);
	return entry->key == NULL ? NULL : &entry->value;
}

void table_print(Table *table) {
	printf("\n==== TABLE(%d/%d) ====\n\n", table->size, table->capacity);
	for (int i = 0; i < table->capacity; i++) {
		if (table->entries[i].key != NULL) {
			printf("\"%s\": ", table->entries[i].key);
			value_print(table->entries[i].value);
			printf("\n");
		}
	}
}