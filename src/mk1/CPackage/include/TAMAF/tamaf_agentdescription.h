#ifndef TAMAF_AGENTDESCRIPTION_H
#define TAMAF_AGENTDESCRIPTION_H

#include "TAMAF/tamaf_agentid.h"
#include "TAMAF/tamaf_servicedescription.h"
#include "cJSON.h"

typedef struct {
    tamaf_agentid_t* aid;
    tamaf_servicedescription_t** services;
    size_t services_count;
} tamaf_agentdescription_t;

/**
 * Create a new AgentDescription object.
 */
tamaf_agentdescription_t* tamaf_agentdescription_create(tamaf_agentid_t* aid);

/**
 * Free the memory allocated for an AgentDescription object.
 */
void tamaf_agentdescription_destroy(tamaf_agentdescription_t* ad);

/**
 * Add a service to the agent description.
 * Takes ownership of the service description pointer.
 */
void tamaf_agentdescription_add_service(tamaf_agentdescription_t* ad, tamaf_servicedescription_t* sd);

/**
 * Check if the agent description matches a template.
 */
bool tamaf_agentdescription_matches(const tamaf_agentdescription_t* ad, const tamaf_agentdescription_t* template);

/**
 * Serialize the agent description to a cJSON object.
 */
cJSON* tamaf_agentdescription_to_json(const tamaf_agentdescription_t* ad);

/**
 * Deserialize the agent description from a cJSON object.
 */
tamaf_agentdescription_t* tamaf_agentdescription_from_json(const cJSON* json);

#endif // TAMAF_AGENTDESCRIPTION_H
