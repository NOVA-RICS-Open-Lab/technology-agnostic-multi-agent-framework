#include "TAMAF/tamaf_environmentmanagementontology.h"
#include "TAMAF/tamaf_defines.h"
#include <string.h>

bool tamaf_em_ontology_validate(const cJSON* content) {
    if (!content) return false;
    cJSON* action = cJSON_GetObjectItem(content, "action");
    if (!action || !action->valuestring) return false;
    return true;
}

const char* tamaf_em_ontology_get_action(const cJSON* content) {
    if (!tamaf_em_ontology_validate(content)) return NULL;
    return cJSON_GetObjectItem(content, "action")->valuestring;
}

double tamaf_em_ontology_get_update(const cJSON* content) {
    if (!tamaf_em_ontology_validate(content)) return -1.0;
    cJSON* concept = cJSON_GetObjectItem(content, "concept");
    if (!concept || strcmp(concept->valuestring, DEFAULT_ENVIRONMENTMANAGEMENT_ONTOLOGY_ENVIRONMENTUDPATE_CONCEPT) != 0) return -1.0;
    cJSON* attr = cJSON_GetObjectItem(content, "attributes");
    if (!attr) return -1.0;
    return attr->valuedouble;
}

cJSON* tamaf_em_ontology_create_update_content(const char* action, double update_time) {
    cJSON* content = cJSON_CreateObject();
    cJSON_AddStringToObject(content, "action", action);
    cJSON_AddStringToObject(content, "concept", DEFAULT_ENVIRONMENTMANAGEMENT_ONTOLOGY_ENVIRONMENTUDPATE_CONCEPT);
    cJSON_AddNumberToObject(content, "attributes", update_time);
    return content;
}
