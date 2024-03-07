/* Copyright 2023 <> */
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"


int compare_function_ints(void *a, void *b)
{
	int int_a = *((int *)a);
	int int_b = *((int *)b);

	if (int_a == int_b) {
		return 0;
	} else if (int_a < int_b) {
		return -1;
	} else {
		return 1;
	}
}

int compare_function_strings(void *a, void *b)
{
	char *str_a = (char *)a;
	char *str_b = (char *)b;

	return strcmp(str_a, str_b);
}

unsigned int hash_function_int(void *a)
{
	/*
	 * Credits: https://stackoverflow.com/a/12996028/7883884
	 */
	unsigned int uint_a = *((unsigned int *)a);

	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = (uint_a >> 16u) ^ uint_a;
	return uint_a;
}

unsigned int hash_function_string(void *a)
{
	/*
	 * Credits: http://www.cse.yorku.ca/~oz/hash.html
	 */
	unsigned char *puchar_a = (unsigned char*) a;
	unsigned long hash = 5381;
	int c;

	while ((c = *puchar_a++))
		hash = ((hash << 5u) + hash) + c; /* hash * 33 + c */

	return hash;
}

void key_val_free_function(void *data) {
	free((*(info *)data).key);
	free((*(info *)data).value);
}

hashtable_t *ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
		int (*compare_function)(void*, void*),
		void (*key_val_free_function)(void*))
{
    hashtable_t *ht = malloc(sizeof(hashtable_t));
	DIE(!ht, "Malloc failed!\n");
    ht->buckets = malloc(sizeof(linked_list_t*) * hmax);
	DIE(!ht->buckets, "Malloc failed!\n");

    for (unsigned int i = 0; i < hmax; i++)
        ht->buckets[i] = ll_create(sizeof(info));

    ht->hash_function = hash_function;
    ht->compare_function = compare_function;
    ht->key_val_free_function = key_val_free_function;
	ht->hmax = hmax;
	ht->size = 0;
    return ht;
}

int ht_has_key(hashtable_t *ht, void *key)
{
	int indx = ht->hash_function(key) % ht->hmax;
	linked_list_t *bucket = ht->buckets[indx];
	ll_node_t *curr = bucket->head;

	while (curr) {
		int aux = ht->compare_function(key, (*(info*)curr->data).key);
		if (!aux)
			return 1;
		curr = curr->next;
	}
    return 0;
}

void *ht_get(hashtable_t *ht, void *key)
{
	unsigned int indx = ht->hash_function(key) % ht->hmax;
	linked_list_t *bucket = ht->buckets[indx];
    ll_node_t *curr = bucket->head;

	while (curr) {
		int aux = ht->compare_function(key, (*(info*)curr->data).key);
		if (!aux) {
			return (*(info*)curr->data).value;
		}
		curr = curr->next;
	}

	return NULL;
}

void ht_put(hashtable_t *ht, void *key, unsigned int key_size,
	void *value, unsigned int value_size)
{
    unsigned int indx = ht->hash_function(key) % ht->hmax;
	linked_list_t *bucket = ht->buckets[indx];
    ll_node_t *curr = bucket->head;
	info *new_data = malloc(sizeof(info));
	DIE(!new_data, "Malloc failed!\n");
	new_data->key = malloc(key_size);
	DIE(!new_data->key, "Malloc failed!\n");
	memcpy(new_data->key, key, key_size);

	new_data->value = malloc(value_size);
	DIE(!new_data->value, "Malloc failed!\n");
	memcpy(new_data->value, value, value_size);

	if (ht_has_key(ht, key) == 1) {
		while (curr) {
			int aux = ht->compare_function(key, (*(info*)curr->data).key);
			if (!aux) {
				curr->data = new_data;
				return;
			}
		}
	}

	ll_add_nth_node(bucket, 0, new_data);

	ht->size++;
	free(new_data);
}

void ht_remove_entry(hashtable_t *ht, void *key)
{
	unsigned int indx = ht->hash_function(key) % ht->hmax;
	linked_list_t *bucket = ht->buckets[indx];
    ll_node_t *curr = bucket->head;

	int pos = 0;
	while (curr) {
		int aux = ht->compare_function(key, (*(info*)curr->data).key);
		if (!aux) {
			ll_node_t *remove = ll_remove_nth_node(bucket, pos);
			ht->key_val_free_function(curr->data);
			free(remove->data);
			free(remove);
			return;
		}
		pos++;
		curr = curr->next;
	}
}

void ht_free(hashtable_t *ht)
{
	if (!ht)
		return;

	for (unsigned int i = 0; i < ht->hmax; i++) {
		ll_node_t *curr = ht->buckets[i]->head;
		while (curr) {
			ht->key_val_free_function(curr->data);
			curr = curr->next;
		}
		ll_free(&ht->buckets[i]);
	}
	free(ht->buckets);
	free(ht);
}

unsigned int ht_get_size(hashtable_t *ht)
{
	if (ht == NULL)
		return 0;

	return ht->size;
}

unsigned int ht_get_hmax(hashtable_t *ht)
{
	if (ht == NULL)
		return 0;

	return ht->hmax;
}
