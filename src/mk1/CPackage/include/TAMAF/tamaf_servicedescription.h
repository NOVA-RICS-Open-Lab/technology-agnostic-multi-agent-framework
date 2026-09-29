#ifndef TAMAF_SERVICEDESCRIPTION_H
#define TAMAF_SERVICEDESCRIPTION_H

#include "cJSON.h"
#include <stdbool.h>
#include <stdlib.h>

typedef struct {
    char* name;
    char* type;
    char* ownership;
    char** protocols;
    size_t protocols_count;
    char** ontologies;
    size_t ontologies_count;
    cJSON* properties;
} tamaf_servicedescription_t;

/**
 * Create a new ServiceDescription object.
 */
tamaf_servicedescription_t* tamaf_servicedescription_create(const char* name);

/**
 * Free the memory allocated for a ServiceDescription object.
 */
void tamaf_servicedescription_destroy(tamaf_servicedescription_t* sd);

/**
 * Add a protocol to the service.
 */
void tamaf_servicedescription_add_protocol(tamaf_servicedescription_t* sd, const char* protocol);

/**
 * Add an ontology to the service.
 */
void tamaf_servicedescription_add_ontology(tamaf_servicedescription_t* sd, const char* ontology);

/**
 * Set a property in the service.
 * value: cJSON object (will be duplicated).
 */
void tamaf_servicedescription_set_property(tamaf_servicedescription_t* sd, const char* key, const cJSON* value);

/**
 * Check if the service matches a template.
 */
bool tamaf_servicedescription_matches(const tamaf_servicedescription_t* sd, const tamaf_servicedescription_t* template);

/**
 * Serialize the service to a cJSON object.
 */
cJSON* tamaf_servicedescription_to_json(const tamaf_servicedescription_t* sd);

/**
 * Deserialize the service from a cJSON object.
 */
tamaf_servicedescription_t* tamaf_servicedescription_from_json(const cJSON* json);

#endif // TAMAF_SERVICEDESCRIPTION_H
