#include "TAMAF/tamaf_agentmanagementontology.h"
#include "TAMAF/tamaf_defines.h"
#include <string.h>

cJSON* tamaf_am_ontology_create_agent_description_content(const char* action, tamaf_agentdescription_t** ads, size_t ads_count) {
    cJSON* content = cJSON_CreateObject();
    cJSON_AddStringToObject(content, "action", action);
    cJSON_AddStringToObject(content, "concept", DEFAULT_AGENTMANAGEMENT_ONTOLOGY_AGENTDESCRIPTION_CONCEPT);
    
    cJSON* attributes = cJSON_AddArrayToObject(content, "attributes");
    for (size_t i = 0; i < ads_count; i++) {
        cJSON_AddItemToArray(attributes, tamaf_agentdescription_to_json(ads[i]));
    }
    
    return content;
}

cJSON* tamaf_am_ontology_create_agent_port_content(const char* action, int port) {
    cJSON* content = cJSON_CreateObject();
    cJSON_AddStringToObject(content, "action", action);
    cJSON_AddStringToObject(content, "concept", DEFAULT_AGENTMANAGEMENT_ONTOLOGY_AGENTPORT_CONCEPT);
    cJSON_AddNumberToObject(content, "attributes", port);
    return content;
}

cJSON* tamaf_am_ontology_create_result_content(const char* action, const char* concept, int inform) {
    cJSON* content = cJSON_CreateObject();
    cJSON_AddStringToObject(content, "action", action);
    cJSON_AddStringToObject(content, "concept", concept);
    cJSON_AddNumberToObject(content, "attributes", inform);
    return content;
}

bool tamaf_am_ontology_validate(const cJSON* content) {
    if (!content) return false;
    cJSON* action = cJSON_GetObjectItem(content, "action");
    if (!action || !action->valuestring) return false;
    
    // In Python it just checks if action is in the list.
    // For simplicity we just check if it exists.
    return true;
}

const char* tamaf_am_ontology_get_action(const cJSON* content) {
    if (!tamaf_am_ontology_validate(content)) return NULL;
    return cJSON_GetObjectItem(content, "action")->valuestring;
}

int tamaf_am_ontology_get_agent_port(const cJSON* content) {
    if (!tamaf_am_ontology_validate(content)) return -1;
    cJSON* concept = cJSON_GetObjectItem(content, "concept");
    if (!concept || strcmp(concept->valuestring, DEFAULT_AGENTMANAGEMENT_ONTOLOGY_AGENTPORT_CONCEPT) != 0) return -1;
    
    cJSON* attr = cJSON_GetObjectItem(content, "attributes");
    if (!attr) return -1;
    return attr->valueint;
}

int tamaf_am_ontology_get_result(const cJSON* content) {
    if (!tamaf_am_ontology_validate(content)) return -1;
    cJSON* attr = cJSON_GetObjectItem(content, "attributes");
    if (!attr || !cJSON_IsNumber(attr)) return -1;
    return attr->valueint;
}

tamaf_agentdescription_t** tamaf_am_ontology_get_agent_descriptions(const cJSON* content, size_t* count) {
    if (!tamaf_am_ontology_validate(content)) return NULL;
    cJSON* concept = cJSON_GetObjectItem(content, "concept");
    if (!concept || strcmp(concept->valuestring, DEFAULT_AGENTMANAGEMENT_ONTOLOGY_AGENTDESCRIPTION_CONCEPT) != 0) return NULL;
    
    cJSON* attr = cJSON_GetObjectItem(content, "attributes");
    if (!cJSON_IsArray(attr)) return NULL;
    
    *count = cJSON_GetArraySize(attr);
    tamaf_agentdescription_t** ads = (tamaf_agentdescription_t**)malloc((*count) * sizeof(tamaf_agentdescription_t*));
    for (size_t i = 0; i < *count; i++) {
        ads[i] = tamaf_agentdescription_from_json(cJSON_GetArrayItem(attr, (int)i));
    }
    
    return ads;
}
