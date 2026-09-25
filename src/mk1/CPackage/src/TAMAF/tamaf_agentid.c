#include "TAMAF/tamaf_agentid.h"
#include <string.h>

#ifdef _WIN32
#define strdup _strdup
#endif

tamaf_agentid_t* tamaf_agentid_create(const char* name, tamaf_address_t* address) {
    tamaf_agentid_t* aid = (tamaf_agentid_t*)malloc(sizeof(tamaf_agentid_t));
    if (!aid) return NULL;
    aid->name = strdup(name);
    aid->address = address;
    return aid;
}

void tamaf_agentid_destroy(tamaf_agentid_t* aid) {
    if (aid) {
        if (aid->name) free(aid->name);
        if (aid->address) tamaf_address_destroy(aid->address);
        free(aid);
    }
}

void tamaf_agentid_set_port(tamaf_agentid_t* aid, int port) {
    if (aid && aid->address) {
        tamaf_address_set_port(aid->address, port);
    }
}

char* tamaf_agentid_get_full_id(const tamaf_agentid_t* aid) {
    if (!aid) return NULL;
    char* addr_str = NULL;
    if (aid->address) {
        addr_str = tamaf_address_get_string(aid->address);
    } else {
        addr_str = strdup("None:None");
    }
    
    if (!addr_str) return NULL;

    size_t len = strlen(aid->name) + 1 + strlen(addr_str) + 1;
    char* full_id = (char*)malloc(len);
    if (full_id) {
        sprintf(full_id, "%s@%s", aid->name, addr_str);
    }
    free(addr_str);
    return full_id;
}

char* tamaf_agentid_get_id_no_port(const tamaf_agentid_t* aid) {
    if (!aid) return NULL;
    char* ip = "None";
    if (aid->address && aid->address->ip) {
        ip = aid->address->ip;
    }
    
    size_t len = strlen(aid->name) + 1 + strlen(ip) + 1;
    char* id = (char*)malloc(len);
    if (id) {
        sprintf(id, "%s@%s", aid->name, ip);
    }
    return id;
}

tamaf_agentid_t* tamaf_agentid_from_string(const char* aid_string) {
    if (!aid_string) return NULL;
    
    char* str = strdup(aid_string);
    char* at_ptr = strchr(str, '@');
    if (!at_ptr) {
        free(str);
        return NULL;
    }
    
    *at_ptr = '\0';
    char* name = str;
    char* network_part = at_ptr + 1;
    
    char* colon_ptr = strrchr(network_part, ':');
    if (!colon_ptr) {
        // Handle case with no port if necessary, though GetFullID always includes it
        tamaf_address_t* addr = tamaf_address_create(network_part, TAMAF_PORT_NONE);
        tamaf_agentid_t* aid = tamaf_agentid_create(name, addr);
        free(str);
        return aid;
    }
    
    *colon_ptr = '\0';
    char* ip = network_part;
    char* port_str = colon_ptr + 1;
    
    int port = TAMAF_PORT_NONE;
    if (strcmp(port_str, "None") != 0) {
        port = atoi(port_str);
    }
    
    tamaf_address_t* addr = tamaf_address_create(ip, port);
    tamaf_agentid_t* aid = tamaf_agentid_create(name, addr);
    
    free(str);
    return aid;
}

bool tamaf_agentid_matches(const tamaf_agentid_t* aidA, const tamaf_agentid_t* aidB) {
    if (aidA == aidB) return true;
    if (!aidA || !aidB) return false;
    
    char* idA = tamaf_agentid_get_full_id(aidA);
    char* idB = tamaf_agentid_get_full_id(aidB);
    
    bool match = (idA && idB && strcmp(idA, idB) == 0);
    
    if (idA) free(idA);
    if (idB) free(idB);
    
    return match;
}
