#ifndef TAMAF_ENVIRONMENTMANAGEMENTONTOLOGY_H
#define TAMAF_ENVIRONMENTMANAGEMENTONTOLOGY_H

#include "cJSON.h"
#include <stdbool.h>

/**
 * Validate if the content follows the Environment Management Ontology.
 */
bool tamaf_em_ontology_validate(const cJSON* content);

/**
 * Get the action from the content.
 */
const char* tamaf_em_ontology_get_action(const cJSON* content);

/**
 * Get the update time from the attributes.
 */
double tamaf_em_ontology_get_update(const cJSON* content);

/**
 * Create content for an Environment Management update message.
 */
cJSON* tamaf_em_ontology_create_update_content(const char* action, double update_time);

#endif // TAMAF_ENVIRONMENTMANAGEMENTONTOLOGY_H
