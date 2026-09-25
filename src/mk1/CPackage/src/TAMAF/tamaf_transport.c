#include "TAMAF/tamaf_transport.h"
#include <string.h>

#ifdef _WIN32
#define strdup _strdup
#endif

static const char* transport_type_to_string(tamaf_transport_type_t type) {
    switch (type) {
        case TAMAF_TRANSPORT_HTTP: return "http";
        case TAMAF_TRANSPORT_UDP: return "udp";
        case TAMAF_TRANSPORT_TCP: return "tcp";
        default: return "unknown";
    }
}

static tamaf_transport_type_t transport_type_from_string(const char* str) {
    if (strcmp(str, "http") == 0) return TAMAF_TRANSPORT_HTTP;
    if (strcmp(str, "udp") == 0) return TAMAF_TRANSPORT_UDP;
    if (strcmp(str, "tcp") == 0) return TAMAF_TRANSPORT_TCP;
    return TAMAF_TRANSPORT_UNKNOWN;
}

// TransportDescription
tamaf_transport_description_t* tamaf_transport_description_create(const char* address, tamaf_transport_type_t type) {
    tamaf_transport_description_t* desc = (tamaf_transport_description_t*)malloc(sizeof(tamaf_transport_description_t));
    if (!desc) return NULL;
    desc->address = strdup(address);
    desc->type = type;
    return desc;
}

void tamaf_transport_description_destroy(tamaf_transport_description_t* desc) {
    if (desc) {
        if (desc->address) free(desc->address);
        free(desc);
    }
}

cJSON* tamaf_transport_description_to_json(const tamaf_transport_description_t* desc) {
    if (!desc) return NULL;
    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "transport-type", transport_type_to_string(desc->type));
    cJSON_AddStringToObject(root, "transport-specific-address", desc->address);
    return root;
}

tamaf_transport_description_t* tamaf_transport_description_from_json(const cJSON* json) {
    if (!json) return NULL;
    cJSON* addr_item = cJSON_GetObjectItem(json, "transport-specific-address");
    if (!addr_item || !cJSON_IsString(addr_item)) return NULL;
    
    tamaf_transport_type_t type = TAMAF_TRANSPORT_HTTP;
    cJSON* type_item = cJSON_GetObjectItem(json, "transport-type");
    if (type_item && cJSON_IsString(type_item)) {
        type = transport_type_from_string(type_item->valuestring);
    }
    
    return tamaf_transport_description_create(addr_item->valuestring, type);
}

// Envelope
tamaf_envelope_t* tamaf_envelope_create(tamaf_transport_description_t* sender, tamaf_transport_description_t* receiver) {
    tamaf_envelope_t* env = (tamaf_envelope_t*)malloc(sizeof(tamaf_envelope_t));
    if (!env) return NULL;
    env->sender = sender;
    env->receiver = receiver;
    return env;
}

void tamaf_envelope_destroy(tamaf_envelope_t* env) {
    if (env) {
        if (env->sender) tamaf_transport_description_destroy(env->sender);
        if (env->receiver) tamaf_transport_description_destroy(env->receiver);
        free(env);
    }
}

cJSON* tamaf_envelope_to_json(const tamaf_envelope_t* env) {
    if (!env) return NULL;
    cJSON* root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "sender-transport-description", tamaf_transport_description_to_json(env->sender));
    cJSON_AddItemToObject(root, "receiver-transport-description", tamaf_transport_description_to_json(env->receiver));
    return root;
}

tamaf_envelope_t* tamaf_envelope_from_json(const cJSON* json) {
    if (!json) return NULL;
    tamaf_transport_description_t* s = tamaf_transport_description_from_json(cJSON_GetObjectItem(json, "sender-transport-description"));
    tamaf_transport_description_t* r = tamaf_transport_description_from_json(cJSON_GetObjectItem(json, "receiver-transport-description"));
    return tamaf_envelope_create(s, r);
}

// TransportMessage
tamaf_transport_message_t* tamaf_transport_message_create(tamaf_aclmessage_t* acl, tamaf_address_t* sender_addr, tamaf_address_t* receiver_addr) {
    tamaf_transport_message_t* tm = (tamaf_transport_message_t*)malloc(sizeof(tamaf_transport_message_t));
    if (!tm) return NULL;
    tm->aclmessage = acl;
    tm->envelope = NULL;
    
    if (sender_addr && receiver_addr) {
        char* s_link = tamaf_address_get_http_link(sender_addr);
        char* r_link = tamaf_address_get_http_link(receiver_addr);
        
        tm->envelope = tamaf_envelope_create(
            tamaf_transport_description_create(s_link, TAMAF_TRANSPORT_HTTP),
            tamaf_transport_description_create(r_link, TAMAF_TRANSPORT_HTTP)
        );
        
        free(s_link);
        free(r_link);
    }
    
    return tm;
}

void tamaf_transport_message_destroy(tamaf_transport_message_t* tm) {
    if (tm) {
        // By default, we do NOT destroy the aclmessage because it's usually 
        // managed by the AMS or the caller. If ownership is intended, 
        // the caller should set tm->aclmessage to NULL before destroying.
        // Wait, if it's deserialized, it SHOULD own it.
        // Let's stick to: it DOES own it, but callers can detach it.
        if (tm->aclmessage) tamaf_aclmessage_destroy(tm->aclmessage);
        if (tm->envelope) tamaf_envelope_destroy(tm->envelope);
        free(tm);
    }
}

cJSON* tamaf_transport_message_to_json(const tamaf_transport_message_t* tm) {
    if (!tm) return NULL;
    cJSON* root = cJSON_CreateObject();
    if (tm->aclmessage) cJSON_AddItemToObject(root, "aclmessage", tamaf_aclmessage_to_json(tm->aclmessage));
    if (tm->envelope) cJSON_AddItemToObject(root, "envelope", tamaf_envelope_to_json(tm->envelope));
    return root;
}

tamaf_transport_message_t* tamaf_transport_message_from_json(const cJSON* json) {
    if (!json) return NULL;
    tamaf_transport_message_t* tm = (tamaf_transport_message_t*)calloc(1, sizeof(tamaf_transport_message_t));
    if (!tm) return NULL;
    
    tm->aclmessage = tamaf_aclmessage_from_json(cJSON_GetObjectItem(json, "aclmessage"));
    tm->envelope = tamaf_envelope_from_json(cJSON_GetObjectItem(json, "envelope"));
    
    return tm;
}
