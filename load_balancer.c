/* Copyright 2023 <> */
#include <stdlib.h>
#include <string.h>

#include "load_balancer.h"
#include "utils.h"

unsigned int hash_function_servers(void *a) {
	unsigned int uint_a = *((unsigned int *)a);

	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = (uint_a >> 16u) ^ uint_a;
	return uint_a;
}

unsigned int hash_function_key(void *a) {
	unsigned char *puchar_a = (unsigned char *)a;
	unsigned int hash = 5381;
	int c;

	while ((c = *puchar_a++))
		hash = ((hash << 5u) + hash) + c;

	return hash;
}

int binary_search(load_balancer *main, unsigned int hash) {
	int left = 0, right = main->server_count;
	server_memory *servers = main->servers;

	while (left <= right) {
		int mid = (left + right) / 2;

		if (servers[mid].hash == hash)
			return mid;
		else if (servers[mid].hash < hash)
			left = mid + 1;
		else
			right = mid - 1;
	}

	return -1;
}

load_balancer *init_load_balancer() {
	load_balancer *balancer = calloc(1, sizeof(load_balancer));
	DIE(!balancer, "Calloc failed!\n");
	balancer->server_count = 0;

	return balancer;
}

void swap_servers(server_memory *a, server_memory *b) {
	server_memory tmp = *a;
	*a = *b;
	*b = tmp;
}

void add_element(load_balancer *main, server_memory *new_servers,
				unsigned int indx) {
	unsigned old_size = main->server_count - 3, j;

	for (j = 0; j < old_size + indx; j++) {
		if (main->servers[j].hash >= new_servers[indx].hash)
			break;
	}
	if (j == old_size + indx) {
		main->servers[old_size + indx] = new_servers[indx];
	} else {
		for (unsigned int k = old_size + indx; k > j; k--)
			main->servers[k] = main->servers[k - 1];
		main->servers[j] = new_servers[indx];
	}
}

int search_server(load_balancer *main, unsigned int hash_object) {
	int indx = 0;

	for (int i = main->server_count - 1; i > 0; i--) {
		if (main->servers[i].hash >= hash_object
			&& main->servers[i - 1].hash <= hash_object) {
			indx = i;
			break;
		}
	}
	return indx;
}

void redistribute_add(load_balancer *main, int min_indx) {
	int ok = 0;
	/* go through all the servers and check if objects need to be redistributed */

	if (min_indx == 0)
		min_indx = 1;

	for (int i = min_indx - 1; i < main->server_count; i++) {
		hashtable_t *curr_server = main->servers[i].server;
		for (unsigned int j = 0; j < curr_server->hmax; j++) {
			ll_node_t *curr = curr_server->buckets[j]->head;
			while (curr) {
				info *data = (info *)curr->data;
				int indx = search_server(main, hash_function_key(data->key));

				if (ht_has_key(main->servers[indx].server, data->key)) {
					curr = curr->next;
					continue;
				}

				if (main->servers[indx].original_id !=
					main->servers[i].original_id) {
					ht_put(main->servers[indx].server, data->key,
						   strlen(data->key) + 1, data->value,
						   strlen(data->value) + 1);
					ok = 1;
				}
				curr = curr->next;
			}
		}
	}

	/* if at least one object was moved, remove it from old server */
	if (ok == 1) {
		for (int i = min_indx - 1; i < main->server_count; i++) {
			hashtable_t *curr_server = main->servers[i].server;
			for (unsigned int j = 0; j < curr_server->hmax; j++) {
				ll_node_t *curr = curr_server->buckets[j]->head;
				while (curr) {
					info *data = (info *)curr->data;
					int indx = search_server(main,
											 hash_function_key(data->key));

					if (main->servers[indx].original_id !=
						main->servers[i].original_id) {
						ht_remove_entry(curr_server, data->key);
						break;
					}
					curr = curr->next;
				}
			}
		}
	}
}

void loader_add_server(load_balancer *main, int server_id) {
	unsigned int old_size = main->server_count;
	main->server_count += 3;

	if (old_size == 0) {
		main->servers = calloc(3, sizeof(server_memory));
		DIE(!main->servers, "Calloc failed!\n");
	} else {
		server_memory *new = calloc(main->server_count, sizeof(server_memory));
		DIE(!new, "Calloc failed!\n");
		for (unsigned int i = 0; i < old_size; i++)
			new[i] = main->servers[i];
		free(main->servers);
		main->servers = new;
	}

	server_memory *new_servers = calloc(3, sizeof(server_memory));
	new_servers[0].server = ht_create(500, hash_function_string,
									 compare_function_strings,
									 key_val_free_function);
	for (int i = 0; i < 3; i++) {
		new_servers[i].server = new_servers[0].server;
		new_servers[i].id = i * 100000 + server_id;
		new_servers[i].hash = hash_function_servers(&new_servers[i].id);
		new_servers[i].original_id = server_id;
	}

	/* order new servers */
	for (int i = 0; i < 2; i++) {
		for (int j = i + 1; j < 3; j++) {
			if (new_servers[i].hash > new_servers[j].hash)
				swap_servers(&new_servers[i], &new_servers[j]);
			if (new_servers[i].hash == new_servers[j].hash) {
				if (new_servers[i].id > new_servers[j].id)
					swap_servers(&new_servers[i], &new_servers[j]);
			}
		}
	}

	if (old_size == 0) {
		main->servers[0] = new_servers[0];
		main->servers[1] = new_servers[1];
		main->servers[2] = new_servers[2];
	}

	/* add first server to the main server */
	add_element(main, new_servers, 0);
	/* add second server to the main server */
	add_element(main, new_servers, 1);
	/* add third server to the main server */
	add_element(main, new_servers, 2);

	int min_indx = binary_search(main, new_servers[0].hash);
	redistribute_add(main, min_indx);

	free(new_servers);
	new_servers = NULL;
}

void move_last(load_balancer *main, int indx) {
	for (int i = indx; i < main->server_count - 1; i++)
		main->servers[i] = main->servers[i + 1];
}

int find_next(load_balancer *main, unsigned int hash) {
	int pos = 0;
	for (int i = 0; i < main->server_count; i++) {
		if (main->servers[i].hash >= hash) {
			pos = i;
			break;
		}
	}
	return pos;
}

void redistribute_remove(load_balancer *main, server_memory source, int indx) {
	ll_node_t *curr;
	for (unsigned int i = 0; i < source.server->hmax; i++) {
		if (source.server->buckets[i]->head) {
			curr = source.server->buckets[i]->head;

			while (curr) {
				info *data = (info *)curr->data;
				if (!ht_has_key(main->servers[indx].server, data->key))
					ht_put(main->servers[indx].server, data->key,
						   strlen(data->key) + 1, data->value,
						   strlen(data->value) + 1);
				curr = curr->next;
			}
		}
	}
}

void loader_remove_server(load_balancer *main, int server_id) {
	/* ids and hashes for original and replicas */
	int *server_ids = calloc(3, sizeof(int));
	unsigned int *server_hashes = calloc(3, sizeof(unsigned int));
	DIE(!server_ids, "Calloc failed!\n");
	DIE(!server_hashes, "Calloc failed!\n");

	for (int i = 0; i < 3; i++) {
		server_ids[i] = i * 100000 + server_id;
		server_hashes[i] = hash_function_servers(&server_ids[i]);
	}

	/* copy of servers to be deleted. Used for redistribution */
	server_memory *deleted_servers = calloc(3, sizeof(server_memory));
	DIE(!deleted_servers, "Calloc failed!\n");

	/* moves servers that need to be removed on the last positions */
	int indx_1 = binary_search(main, server_hashes[0]);
	deleted_servers[0] = main->servers[indx_1];
	move_last(main, indx_1);
	main->server_count--;

	int indx_2 = binary_search(main, server_hashes[1]);
	deleted_servers[1] = main->servers[indx_2];
	move_last(main, indx_2);
	main->server_count--;

	int indx_3 = binary_search(main, server_hashes[2]);
	deleted_servers[2] = main->servers[indx_3];
	move_last(main, indx_3);
	main->server_count--;

	if (main->server_count == 0) {
		ht_free(deleted_servers[0].server);
		free(deleted_servers);
		free(server_ids);
		free(server_hashes);
		return;
	}

	server_memory *copy = calloc(main->server_count, sizeof(server_memory));
	DIE(!copy, "Calloc failed!\n");
	for (int i = 0; i < main->server_count; i++)
		copy[i] = main->servers[i];
	free(main->servers);
	main->servers = copy;

	/* find the next highest server's index */
	indx_1 = find_next(main, deleted_servers[0].hash);
	indx_2 = find_next(main, deleted_servers[1].hash);
	indx_3 = find_next(main, deleted_servers[2].hash);

	/* redistribute elements */
	redistribute_remove(main, deleted_servers[0], indx_1);
	redistribute_remove(main, deleted_servers[1], indx_2);
	redistribute_remove(main, deleted_servers[2], indx_3);

	ht_free(deleted_servers[0].server);
	free(deleted_servers);
	free(server_ids);
	free(server_hashes);
}

void loader_store(load_balancer *main, char *key, char *value, int *server_id) {
	unsigned int hash_object = hash_function_key((void *)key);
	int indx = search_server(main, hash_object);

	if (ht_has_key(main->servers[indx].server, key))
		ht_remove_entry(main->servers[indx].server, key);

	ht_put(main->servers[indx].server, key, strlen(key) + 1, value,
		   strlen(value) + 1);
	*server_id = main->servers[indx].original_id;
}

char *loader_retrieve(load_balancer *main, char *key, int *server_id) {
	unsigned int hash_object = hash_function_key((void *)key);

	int indx = search_server(main, hash_object);

	if (ht_has_key(main->servers[indx].server, key)) {
		char *value = (char *)ht_get(main->servers[indx].server, key);
		*server_id = main->servers[indx].original_id;
		return value;
	}

	return NULL;
}

void free_load_balancer(load_balancer *main) {
	/* order servers by original id */
	for (int i = 0; i < main->server_count - 1; i++) {
		for (int j = i + 1; j < main->server_count; j++) {
			if (main->servers[i].original_id > main->servers[j].original_id)
				swap_servers(&main->servers[i], &main->servers[j]);
		}
	}

	/* free all servers */
	for (int i = 0; i < main->server_count; i += 3) {
		ht_free(main->servers[i].server);
		main->servers[i].server = NULL;
	}

	free(main->servers);
	free(main);
}
