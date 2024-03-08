# LOAD BALANCER

## Description

This project simulates the way items can be kept on multiple servers in a balanced way.

For storing the items in a balanced manner, I used Consistent Hashing.
Consistent Hashing is a hashing method used so that when rescaling the table only n / m keys will be remaped (n no keys, m no servers);
Servers are placed on a "hash ring". The server resposible for holding an item is the closest one clockwise on the hashring.
For a better balancer, a server has an additional 2 copies of itself on the hash ring.

## Servers

Server structure is located  and server.h. A server contains a hashtable, a hash(placed on the hashring), and an id.

## Hash Ring/Load Balancer

The hash ring contains an array of servers, stored in ascending order, according to the hash

## Load Balancer operations

1. loader_store() - stores key-value pair on the hash ring on the appropriate server

2. loader_retrieve() - search for the server where the hash is stored and return searched item

3. loader_add_server() - add a server on the hash ring

4. loader_remove_server() - this function removes the server and moves data to another server
