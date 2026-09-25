#ifndef TAMAF_ACLMESSAGETEMPLATE_H
#define TAMAF_ACLMESSAGETEMPLATE_H

#include "TAMAF/tamaf_agentid.h"
#include "TAMAF/tamaf_performative.h"
#include "TAMAF/tamaf_aclmessage.h"
#include <stdbool.h>
#include <stdlib.h>

typedef struct {
    tamaf_agentid_t* sender;
    tamaf_performative_t* performatives;
    size_t performatives_count;
    char* ontology;
    char* conversation_id;
    char* protocol;
} tamaf_aclmessagetemplate_t;

/**
 * Create a new ACLMessageTemplate object.
 */
tamaf_aclmessagetemplate_t* tamaf_aclmessagetemplate_create();

/**
 * Free the memory allocated for an ACLMessageTemplate object.
 */
void tamaf_aclmessagetemplate_destroy(tamaf_aclmessagetemplate_t* tmpl);

/**
 * Set the required sender in the template.
 */
void tamaf_aclmessagetemplate_set_sender(tamaf_aclmessagetemplate_t* tmpl, tamaf_agentid_t* sender);

/**
 * Add an allowed performative to the template.
 */
void tamaf_aclmessagetemplate_add_performative(tamaf_aclmessagetemplate_t* tmpl, tamaf_performative_t perf);

/**
 * Set the required ontology in the template.
 */
void tamaf_aclmessagetemplate_set_ontology(tamaf_aclmessagetemplate_t* tmpl, const char* ontology);

/**
 * Set the required conversation ID in the template.
 */
void tamaf_aclmessagetemplate_set_conversation_id(tamaf_aclmessagetemplate_t* tmpl, const char* cid);

/**
 * Check if a message matches the template.
 */
bool tamaf_aclmessagetemplate_match(const tamaf_aclmessagetemplate_t* tmpl, const tamaf_aclmessage_t* msg);

#endif // TAMAF_ACLMESSAGETEMPLATE_H
