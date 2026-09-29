#ifndef TAMAF_ADDRESS_H
#define TAMAF_ADDRESS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAMAF_PORT_NONE -1

typedef struct {
    char* ip;
    int port;
} tamaf_address_t;

/**
 * Create a new address object.
 * ip: The IP address string (will be copied).
 * port: The port number, or TAMAF_PORT_NONE.
 * Returns a pointer to the new address object.
 */
tamaf_address_t* tamaf_address_create(const char* ip, int port);

/**
 * Free the memory allocated for an address object.
 */
void tamaf_address_destroy(tamaf_address_t* address);

/**
 * Set the port of an address object.
 */
void tamaf_address_set_port(tamaf_address_t* address, int port);

/**
 * Get the address as a string (ip:port).
 * Returns a newly allocated string that must be freed by the caller.
 */
char* tamaf_address_get_string(const tamaf_address_t* address);

/**
 * Get the HTTP link for the address (http://ip:port).
 * Returns a newly allocated string that must be freed by the caller.
 */
char* tamaf_address_get_http_link(const tamaf_address_t* address);

/**
 * Get the local IP address of the machine.
 * Returns a newly allocated string that must be freed by the caller.
 */
char* tamaf_address_get_local_ip();

#endif // TAMAF_ADDRESS_H
