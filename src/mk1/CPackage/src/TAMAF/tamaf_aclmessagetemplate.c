#include "TAMAF/tamaf_aclmessagetemplate.h"
#include <string.h>

#ifdef _WIN32
#define strdup _strdup
#endif

tamaf_aclmessagetemplate_t* tamaf_aclmessagetemplate_create() {
    tamaf_aclmessagetemplate_t* tmpl = (tamaf_aclmessagetemplate_t*)calloc(1, sizeof(tamaf_aclmessagetemplate_t));
    return tmpl;
}

void tamaf_aclmessagetemplate_destroy(tamaf_aclmessagetemplate_t* tmpl) {
    if (tmpl) {
        if (tmpl->sender) tamaf_agentid_destroy(tmpl->sender);
        if (tmpl->performatives) free(tmpl->performatives);
        if (tmpl->ontology) free(tmpl->ontology);
        if (tmpl->conversation_id) free(tmpl->conversation_id);
        if (tmpl->protocol) free(tmpl->protocol);
        free(tmpl);
    }
}

void tamaf_aclmessagetemplate_set_sender(tamaf_aclmessagetemplate_t* tmpl, tamaf_agentid_t* sender) {
    if (!tmpl) return;
    if (tmpl->sender) tamaf_agentid_destroy(tmpl->sender);
    tmpl->sender = tamaf_agentid_from_string(tamaf_agentid_get_full_id(sender));
}

void tamaf_aclmessagetemplate_add_performative(tamaf_aclmessagetemplate_t* tmpl, tamaf_performative_t perf) {
    if (!tmpl) return;
    tmpl->performatives = (tamaf_performative_t*)realloc(tmpl->performatives, (tmpl->performatives_count + 1) * sizeof(tamaf_performative_t));
    tmpl->performatives[tmpl->performatives_count++] = perf;
}

void tamaf_aclmessagetemplate_set_ontology(tamaf_aclmessagetemplate_t* tmpl, const char* ontology) {
    if (!tmpl) return;
    if (tmpl->ontology) free(tmpl->ontology);
    tmpl->ontology = ontology ? strdup(ontology) : NULL;
}

void tamaf_aclmessagetemplate_set_conversation_id(tamaf_aclmessagetemplate_t* tmpl, const char* cid) {
    if (!tmpl) return;
    if (tmpl->conversation_id) free(tmpl->conversation_id);
    tmpl->conversation_id = cid ? strdup(cid) : NULL;
}

bool tamaf_aclmessagetemplate_match(const tamaf_aclmessagetemplate_t* tmpl, const tamaf_aclmessage_t* msg) {
    if (!tmpl || !msg) return false;
    
    if (tmpl->sender && !tamaf_agentid_matches(tmpl->sender, msg->sender)) return false;
    
    if (tmpl->performatives_count > 0) {
        bool perf_match = false;
        for (size_t i = 0; i < tmpl->performatives_count; i++) {
            if (tmpl->performatives[i] == msg->performative) {
                perf_match = true;
                break;
            }
        }
        if (!perf_match) return false;
    }
    
    if (tmpl->ontology && (!msg->ontology || strcmp(tmpl->ontology, msg->ontology) != 0)) return false;
    
    if (tmpl->conversation_id && (!msg->conversation_id || strcmp(tmpl->conversation_id, msg->conversation_id) != 0)) return false;
    
    if (tmpl->protocol && (!msg->protocol || strcmp(tmpl->protocol, msg->protocol) != 0)) return false;
    
    return true;
}
