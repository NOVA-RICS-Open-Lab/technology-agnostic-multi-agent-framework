#ifndef TAMAF_AGENTID_H
#define TAMAF_AGENTID_H

#include "TAMAF/tamaf_address.h"
#include <stdbool.h>

typedef struct {
    char* name;
    tamaf_address_t* address;
} tamaf_agentid_t;

/**
 * Create a new AgentID object.
 * name: The name of the agent (will be copied).
 * address: The address object (takes ownership or copies? In MAS usually shared or cloned).
 * Here we will assume ownership of the address pointer for simplicity, 
 * or we can clone it. Let's say it TAKES OWNERSHIP to avoid extra clones.
 */
tamaf_agentid_t* tamaf_agentid_create(const char* name, tamaf_address_t* address);

/**
 * Free the memory allocated for an AgentID object (including the address).
 */
void tamaf_agentid_destroy(tamaf_agentid_t* aid);

/**
 * Set the port in the AgentID's address.
 */
void tamaf_agentid_set_port(tamaf_agentid_t* aid, int port);

/**
 * Get the full ID string (name@ip:port).
 * Returns a newly allocated string that must be freed by the caller.
 */
char* tamaf_agentid_get_full_id(const tamaf_agentid_t* aid);

/**
 * Get the ID string without port (name@ip).
 * Returns a newly allocated string that must be freed by the caller.
 */
char* tamaf_agentid_get_id_no_port(const tamaf_agentid_t* aid);

/**
 * Create an AgentID from a string representation (name@ip:port).
 */
tamaf_agentid_t* tamaf_agentid_from_string(const char* aid_string);

/**
 * Check if two AgentIDs match.
 */
bool tamaf_agentid_matches(const tamaf_agentid_t* aidA, const tamaf_agentid_t* aidB);

#endif // TAMAF_AGENTID_H
