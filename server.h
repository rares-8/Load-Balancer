/* Copyright 2023 <> */
#ifndef SERVER_H_
#define SERVER_H_

#include "hashtable.h"

struct server_memory {
	hashtable_t *server;
    unsigned int hash;
    int id;
    int original_id;
};
typedef struct server_memory server_memory;

#endif  // SERVER_H_
