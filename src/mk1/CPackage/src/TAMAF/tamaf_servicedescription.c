#include "TAMAF/tamaf_servicedescription.h"
#include <string.h>

#ifdef _WIN32
#define strdup _strdup
#endif

tamaf_servicedescription_t* tamaf_servicedescription_create(const char* name) {
    tamaf_servicedescription_t* sd = (tamaf_servicedescription_t*)malloc(sizeof(tamaf_servicedescription_t));
    if (!sd) return NULL;
    sd->name = name ? strdup(name) : NULL;
    sd->type = NULL;
    sd->ownership = NULL;
    sd->protocols = NULL;
    sd->protocols_count = 0;
    sd->ontologies = NULL;
    sd->ontologies_count = 0;
    sd->properties = cJSON_CreateObject();
    return sd;
}

void tamaf_servicedescription_destroy(tamaf_servicedescription_t* sd) {
    if (sd) {
        if (sd->name) free(sd->name);
        if (sd->type) free(sd->type);
        if (sd->ownership) free(sd->ownership);
        if (sd->protocols) {
            for (size_t i = 0; i < sd->protocols_count; i++) {
                free(sd->protocols[i]);
            }
            free(sd->protocols);
        }
        if (sd->ontologies) {
            for (size_t i = 0; i < sd->ontologies_count; i++) {
                free(sd->ontologies[i]);
            }
            free(sd->ontologies);
        }
        if (sd->properties) cJSON_Delete(sd->properties);
        free(sd);
    }
}

void tamaf_servicedescription_add_protocol(tamaf_servicedescription_t* sd, const char* protocol) {
    if (!sd || !protocol) return;
    sd->protocols = (char**)realloc(sd->protocols, (sd->protocols_count + 1) * sizeof(char*));
    sd->protocols[sd->protocols_count++] = strdup(protocol);
}

void tamaf_servicedescription_add_ontology(tamaf_servicedescription_t* sd, const char* ontology) {
    if (!sd || !ontology) return;
    sd->ontologies = (char**)realloc(sd->ontologies, (sd->ontologies_count + 1) * sizeof(char*));
    sd->ontologies[sd->ontologies_count++] = strdup(ontology);
}

void tamaf_servicedescription_set_property(tamaf_servicedescription_t* sd, const char* key, const cJSON* value) {
    if (!sd || !key || !value) return;
    if (!sd->properties) sd->properties = cJSON_CreateObject();
    cJSON_AddItemToObject(sd->properties, key, cJSON_Duplicate(value, true));
}

bool tamaf_servicedescription_matches(const tamaf_servicedescription_t* sd, const tamaf_servicedescription_t* template) {
    if (!sd || !template) return false;

    if (template->name && (!sd->name || strcmp(sd->name, template->name) != 0)) return false;
    if (template->type && (!sd->type || strcmp(sd->type, template->type) != 0)) return false;
    if (template->ownership && (!sd->ownership || strcmp(sd->ownership, template->ownership) != 0)) return false;

    // Check protocols
    for (size_t i = 0; i < template->protocols_count; i++) {
        bool found = false;
        for (size_t j = 0; j < sd->protocols_count; j++) {
            if (strcmp(sd->protocols[j], template->protocols[i]) == 0) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }

    // Check ontologies
    for (size_t i = 0; i < template->ontologies_count; i++) {
        bool found = false;
        for (size_t j = 0; j < sd->ontologies_count; j++) {
            if (strcmp(sd->ontologies[j], template->ontologies[i]) == 0) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }

    // Check properties
    if (template->properties) {
        cJSON* prop = template->properties->child;
        while (prop) {
            cJSON* sd_prop = cJSON_GetObjectItem(sd->properties, prop->string);
            if (!sd_prop || !cJSON_Compare(sd_prop, prop, true)) return false;
            prop = prop->next;
        }
    }

    return true;
}

cJSON* tamaf_servicedescription_to_json(const tamaf_servicedescription_t* sd) {
    if (!sd) return NULL;
    cJSON* json = cJSON_CreateObject();
    if (sd->name) cJSON_AddStringToObject(json, "name", sd->name);
    if (sd->type) cJSON_AddStringToObject(json, "type", sd->type);
    if (sd->ownership) cJSON_AddStringToObject(json, "ownership", sd->ownership);
    
    if (sd->protocols_count > 0) {
        cJSON* protocols = cJSON_AddArrayToObject(json, "protocols");
        for (size_t i = 0; i < sd->protocols_count; i++) {
            cJSON_AddItemToArray(protocols, cJSON_CreateString(sd->protocols[i]));
        }
    } else {
        cJSON_AddArrayToObject(json, "protocols");
    }

    if (sd->ontologies_count > 0) {
        cJSON* ontologies = cJSON_AddArrayToObject(json, "ontologies");
        for (size_t i = 0; i < sd->ontologies_count; i++) {
            cJSON_AddItemToArray(ontologies, cJSON_CreateString(sd->ontologies[i]));
        }
    } else {
        cJSON_AddArrayToObject(json, "ontologies");
    }

    if (sd->properties) {
        cJSON_AddItemToObject(json, "properties", cJSON_Duplicate(sd->properties, true));
    } else {
        cJSON_AddObjectToObject(json, "properties");
    }

    return json;
}

tamaf_servicedescription_t* tamaf_servicedescription_from_json(const cJSON* json) {
    if (!json) return NULL;
    
    cJSON* name_item = cJSON_GetObjectItem(json, "name");
    tamaf_servicedescription_t* sd = tamaf_servicedescription_create(name_item ? name_item->valuestring : NULL);
    
    cJSON* type_item = cJSON_GetObjectItem(json, "type");
    if (type_item && type_item->valuestring) sd->type = strdup(type_item->valuestring);

    cJSON* ownership_item = cJSON_GetObjectItem(json, "ownership");
    if (ownership_item && ownership_item->valuestring) sd->ownership = strdup(ownership_item->valuestring);

    cJSON* protocols_item = cJSON_GetObjectItem(json, "protocols");
    if (cJSON_IsArray(protocols_item)) {
        cJSON* item;
        cJSON_ArrayForEach(item, protocols_item) {
            if (cJSON_IsString(item)) tamaf_servicedescription_add_protocol(sd, item->valuestring);
        }
    }

    cJSON* ontologies_item = cJSON_GetObjectItem(json, "ontologies");
    if (cJSON_IsArray(ontologies_item)) {
        cJSON* item;
        cJSON_ArrayForEach(item, ontologies_item) {
            if (cJSON_IsString(item)) tamaf_servicedescription_add_ontology(sd, item->valuestring);
        }
    }

    cJSON* properties_item = cJSON_GetObjectItem(json, "properties");
    if (cJSON_IsObject(properties_item)) {
        cJSON_Delete(sd->properties);
        sd->properties = cJSON_Duplicate(properties_item, true);
    }

    return sd;
}
