/* Copyright 2023 <> */
#ifndef LOAD_BALANCER_H_
#define LOAD_BALANCER_H_

#include "server.h"

struct load_balancer {
    /* the load balancer contains an array of servers */
    server_memory *servers;
    int server_count;
};
typedef struct load_balancer load_balancer;

/**
 * init_load_balancer() - initializes the memory for a new load balancer and its fields and
 *                        returns a pointer to it
 *
 * Return: pointer to the load balancer struct
 */
load_balancer *init_load_balancer();

/**
 * free_load_balancer() - frees the memory of every field that is related to the
 * load balancer (servers, hashring)
 *
 * @arg1: Load balancer to free
 */
void free_load_balancer(load_balancer *main);

/**
 * load_store() - Stores the key-value pair inside the system.
 * @arg1: Load balancer which distributes the work.
 * @arg2: Key represented as a string.
 * @arg3: Value represented as a string.
 * @arg4: This function will RETURN via this parameter
 *        the server ID which stores the object.
 *
 * The load balancer will use Consistent Hashing to distribute the
 * load across the servers. The chosen server ID will be returned
 * using the last parameter.
 *
 * Hint:
 * Search the hashring associated to the load balancer to find the server where the entry
 * should be stored and call the function to store the entry on the respective server.
 *
 */
void loader_store(load_balancer *main, char *key, char *value, int *server_id);

/**
 * load_retrieve() - Gets a value associated with the key.
 * @arg1: Load balancer which distributes the work.
 * @arg2: Key represented as a string.
 * @arg3: This function will RETURN the server ID
          which stores the value via this parameter.
 *
 * The load balancer will search for the server which should posess the
 * value associated to the key. The server will return NULL in case
 * the key does NOT exist in the system.
 *
 * Hint:
 * Search the hashring associated to the load balancer to find the server where the entry
 * should be stored and call the function to store the entry on the respective server.
 */
char *loader_retrieve(load_balancer *main, char *key, int *server_id);

/**
 * load_add_server() - Adds a new server to the system.
 * @arg1: Load balancer which distributes the work.
 * @arg2: ID of the new server.
 *
 * The load balancer will generate 3 replica labels and it will
 * place them inside the hash ring. The neighbor servers will
 * distribute some the objects to the added server.
 *
 * Hint:
 * Resize the servers array to add a new one.
 * Add each label in the hashring in its appropiate position.
 * Do not forget to resize the hashring and redistribute the objects
 * after each label add (the operations will be done 3 times, for each replica).
 */
void loader_add_server(load_balancer *main, int server_id);

/**
 * load_remove_server() - Removes a specific server from the system.
 * @arg1: Load balancer which distributes the work.
 * @arg2: ID of the removed server.
 *
 * The load balancer will distribute ALL objects stored on the
 * removed server and will delete ALL replicas from the hash ring.
 *
 */
void loader_remove_server(load_balancer *main, int server_id);

/**
 * swap_servers - swap two servers
 * @arg1: first server to swap
 * @arg2: second server to swap
 * 
*/
void swap_servers(server_memory *a, server_memory *b);

/**
 * 
 * add_element - adds new servers to the main server, keeping them
 *               sorted by the hash value.
 * @arg1: Load balancer
 * @arg2: Array of three new servers
 * @arg3: Index used to determine which server to be added
 * 
*/
void add_element(load_balancer *main, server_memory *new_servers,
                 unsigned int indx);

/**
 * 
 * binary_search - returns index of server with given hash.
 *                 Returns -1 if server doesn't exist
 * 
 * @arg1: Load balancer
 * @arg2: Hash to be found
 * 
*/
int binary_search(load_balancer *main, unsigned int hash);

/**
 * 
 * move_last - moves server on the last position
 * @arg1: Load balancer
 * @arg2: Index of server that needs to be moved
*/
void move_last(load_balancer *main, int indx);

/**
 * 
 * search_server - returns the index with the next biggest hash after
 *                 the object hash. Used for searching for the server
 *                 an object should be stored on.
 * 
 * @arg1: Load balancer
 * @arg2: Object hash
 * 
*/
int search_server(load_balancer *main, unsigned int hash_object);

/**
 * 
 * redistribute_remove - used to redistribute the objects on a new server
 *                       in case old server is removed.
 * @arg1: Load balancer
 * @arg2: Old server
 * @arg3: Index of the new server
 * 
*/
void redistribute_remove(load_balancer *main, server_memory source, int indx);

/**
 * 
 * find_next - searches for server with next biggest hash. Used to determine
 *             the index of the server an object should be stored on, in case
 *             old server is removed.
 * @arg1: Load balancer
 * @arg2: Hash of the old server
 * 
*/
int find_next(load_balancer *main, unsigned int hash);

/**
 * 
 * redistribute_add - redistributes objects in case a new server
 *                    was added.
 * 
 * @arg1: Load balancer
 * @arg2: Index of the replica(newly added server) with the lowest hash
*/
void redistribute_add(load_balancer *main, int min_indx);

#endif  // LOAD_BALANCER_H_
