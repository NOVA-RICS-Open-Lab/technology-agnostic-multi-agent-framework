#ifndef TAMAF_EMAINTERACTION_H
#define TAMAF_EMAINTERACTION_H

#include "TAMAF/tamaf_behavior.h"
#include "TAMAF/tamaf_aclmessage.h"
#include "TAMAF/tamaf_agentdescription.h"

typedef struct tamaf_emainteraction_t tamaf_emainteraction_t;

struct tamaf_emainteraction_t {
    tamaf_behavior_t* behavior;
    bool message_sent;
    char* conversation_id;
};

/**
 * Create a new EMAInteraction object.
 */
tamaf_emainteraction_t* tamaf_emainteraction_create(tamaf_behavior_t* b);

/**
 * Free the memory allocated for an EMAInteraction object.
 */
void tamaf_emainteraction_destroy(tamaf_emainteraction_t* ema);

/**
 * Send a message to EMA with retries.
 */
bool tamaf_emainteraction_send_to_ema(tamaf_emainteraction_t* ema, tamaf_aclmessage_t* msg);

/**
 * Keep the agent alive in EMA.
 */
void tamaf_emainteraction_keep_alive(tamaf_emainteraction_t* ema);

/**
 * Register the agent with EMA.
 * Returns the response message if successful, or NULL if pending/failed.
 */
tamaf_aclmessage_t* tamaf_emainteraction_register_agent(tamaf_emainteraction_t* ema);

/**
 * Deregister the agent from EMA.
 */
tamaf_aclmessage_t* tamaf_emainteraction_deregister_agent(tamaf_emainteraction_t* ema);

/**
 * Modify the agent description in EMA.
 */
tamaf_aclmessage_t* tamaf_emainteraction_modify_agent(tamaf_emainteraction_t* ema);

/**
 * Search for agents in EMA (Global).
 * Returns an array of AgentDescription pointers (count in out_count).
 */
tamaf_agentdescription_t** tamaf_emainteraction_search(tamaf_emainteraction_t* ema, tamaf_agentdescription_t* template, size_t* out_count);

/**
 * Search for agents in EMA (Local).
 */
tamaf_agentdescription_t** tamaf_emainteraction_local_search(tamaf_emainteraction_t* ema, tamaf_agentdescription_t* template, size_t* out_count);

/**
 * Search for agents in EMA (External).
 */
tamaf_agentdescription_t** tamaf_emainteraction_external_search(tamaf_emainteraction_t* ema, tamaf_agentdescription_t* template, size_t* out_count);

#endif // TAMAF_EMAINTERACTION_H
