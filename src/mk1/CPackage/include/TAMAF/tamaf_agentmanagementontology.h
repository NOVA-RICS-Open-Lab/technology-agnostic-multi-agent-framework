#ifndef TAMAF_AGENTMANAGEMENTONTOLOGY_H
#define TAMAF_AGENTMANAGEMENTONTOLOGY_H

#include "TAMAF/tamaf_agentdescription.h"
#include "cJSON.h"

/**
 * Create content for an Agent Management message with agent descriptions.
 */
cJSON* tamaf_am_ontology_create_agent_description_content(const char* action, tamaf_agentdescription_t** ads, size_t ads_count);

/**
 * Create content for an Agent Management message with an agent port.
 */
cJSON* tamaf_am_ontology_create_agent_port_content(const char* action, int port);

/**
 * Create content for an Agent Management result message.
 */
cJSON* tamaf_am_ontology_create_result_content(const char* action, const char* concept, int inform);

/**
 * Validate if the content follows the Agent Management Ontology.
 */
bool tamaf_am_ontology_validate(const cJSON* content);

/**
 * Get the action from the content.
 */
const char* tamaf_am_ontology_get_action(const cJSON* content);

/**
 * Get the agent port from the content.
 */
int tamaf_am_ontology_get_agent_port(const cJSON* content);

/**
 * Get the result inform code from the content.
 */
int tamaf_am_ontology_get_result(const cJSON* content);

/**
 * Get the agent descriptions from the content.
 * Returns a newly allocated array of pointers that must be freed (the descriptions themselves should be freed too if they are new).
 * Actually, it's better to return a list that the caller owns.
 */
tamaf_agentdescription_t** tamaf_am_ontology_get_agent_descriptions(const cJSON* content, size_t* count);

#endif // TAMAF_AGENTMANAGEMENTONTOLOGY_H
