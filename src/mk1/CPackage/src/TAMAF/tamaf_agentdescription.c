#include "TAMAF/tamaf_agentdescription.h"
#include <string.h>

tamaf_agentdescription_t* tamaf_agentdescription_create(tamaf_agentid_t* aid) {
    tamaf_agentdescription_t* ad = (tamaf_agentdescription_t*)malloc(sizeof(tamaf_agentdescription_t));
    if (!ad) return NULL;
    ad->aid = aid;
    ad->services = NULL;
    ad->services_count = 0;
    return ad;
}

void tamaf_agentdescription_destroy(tamaf_agentdescription_t* ad) {
    if (ad) {
        if (ad->aid) tamaf_agentid_destroy(ad->aid);
        if (ad->services) {
            for (size_t i = 0; i < ad->services_count; i++) {
                tamaf_servicedescription_destroy(ad->services[i]);
            }
            free(ad->services);
        }
        free(ad);
    }
}

void tamaf_agentdescription_add_service(tamaf_agentdescription_t* ad, tamaf_servicedescription_t* sd) {
    if (!ad || !sd) return;
    ad->services = (tamaf_servicedescription_t**)realloc(ad->services, (ad->services_count + 1) * sizeof(tamaf_servicedescription_t*));
    ad->services[ad->services_count++] = sd;
}

bool tamaf_agentdescription_matches(const tamaf_agentdescription_t* ad, const tamaf_agentdescription_t* template) {
    if (!ad || !template) return false;

    if (template->aid) {
        char* id = tamaf_agentid_get_id_no_port(ad->aid);
        char* tid = tamaf_agentid_get_id_no_port(template->aid);
        bool match = (id && tid && strcmp(id, tid) == 0);
        free(id);
        free(tid);
        if (!match) return false;
    }

    for (size_t i = 0; i < template->services_count; i++) {
        bool found = false;
        for (size_t j = 0; j < ad->services_count; j++) {
            if (tamaf_servicedescription_matches(ad->services[j], template->services[i])) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }

    return true;
}

cJSON* tamaf_agentdescription_to_json(const tamaf_agentdescription_t* ad) {
    if (!ad) return NULL;
    cJSON* json = cJSON_CreateObject();
    
    if (ad->aid) {
        char* full_id = tamaf_agentid_get_full_id(ad->aid);
        cJSON_AddStringToObject(json, "agentid", full_id);
        free(full_id);
    }

    cJSON* services = cJSON_AddArrayToObject(json, "services");
    for (size_t i = 0; i < ad->services_count; i++) {
        cJSON_AddItemToArray(services, tamaf_servicedescription_to_json(ad->services[i]));
    }

    return json;
}

tamaf_agentdescription_t* tamaf_agentdescription_from_json(const cJSON* json) {
    if (!json) return NULL;
    
    cJSON* aid_item = cJSON_GetObjectItem(json, "agentid");
    tamaf_agentid_t* aid = aid_item ? tamaf_agentid_from_string(aid_item->valuestring) : NULL;
    
    tamaf_agentdescription_t* ad = tamaf_agentdescription_create(aid);
    
    cJSON* services_item = cJSON_GetObjectItem(json, "services");
    if (cJSON_IsArray(services_item)) {
        cJSON* item;
        cJSON_ArrayForEach(item, services_item) {
            tamaf_agentdescription_add_service(ad, tamaf_servicedescription_from_json(item));
        }
    }

    return ad;
}
