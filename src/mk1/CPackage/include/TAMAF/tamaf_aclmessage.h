#ifndef TAMAF_ACLMESSAGE_H
#define TAMAF_ACLMESSAGE_H

#include "TAMAF/tamaf_agentid.h"
#include "TAMAF/tamaf_performative.h"
#include "cJSON.h"
#include <time.h>

typedef struct {
    tamaf_performative_t performative;
    tamaf_agentid_t* sender;
    
    tamaf_agentid_t** receivers;
    size_t receivers_count;
    
    cJSON* content; // Generic JSON content
    
    char* language;
    char* ontology;
    char* protocol;
    char* conversation_id;
    
    tamaf_agentid_t** reply_to;
    size_t reply_to_count;
    
    char* reply_by; // String ISO format for now
    char* reply_with;
    char* in_reply_to;
    char* encoding;
} tamaf_aclmessage_t;

/**
 * Create a new ACLMessage object.
 */
tamaf_aclmessage_t* tamaf_aclmessage_create(tamaf_performative_t performative);

/**
 * Free the memory allocated for an ACLMessage object.
 */
void tamaf_aclmessage_destroy(tamaf_aclmessage_t* msg);

/**
 * Add a receiver to the message.
 */
void tamaf_aclmessage_add_receiver(tamaf_aclmessage_t* msg, tamaf_agentid_t* receiver);

/**
 * Add a reply-to to the message.
 */
void tamaf_aclmessage_add_reply_to(tamaf_aclmessage_t* msg, tamaf_agentid_t* reply_to);

/**
 * Create a reply for the message.
 */
tamaf_aclmessage_t* tamaf_aclmessage_create_reply(tamaf_aclmessage_t* msg, tamaf_performative_t performative);

/**
 * Serialize the message to a cJSON object.
 */
cJSON* tamaf_aclmessage_to_json(const tamaf_aclmessage_t* msg);

/**
 * Deserialize the message from a cJSON object.
 */
tamaf_aclmessage_t* tamaf_aclmessage_from_json(const cJSON* json);

#endif // TAMAF_ACLMESSAGE_H
