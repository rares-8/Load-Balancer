/* Copyright 2023 <> */
#include <stdlib.h>
#include <string.h>

#include "server.h"
#include "utils.h"

server_memory *init_server_memory()
{
	return NULL;
}

void server_store(server_memory *server, char *key, char *value) {
	/*TODO 2 */
	(void)server;
	(void)key;
	(void)value;
}

char *server_retrieve(server_memory *server, char *key) {
	/* TODO 3 */
	(void)server;
	(void)key;
	return NULL;
}

void server_remove(server_memory *server, char *key) {
	/* TODO 4 */
	(void)server;
	(void)key;
}

void free_server_memory(server_memory *server) {
	/*TODO 5 */
	(void)server;
}
