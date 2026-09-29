#ifndef TAMAF_AGENT_H
#define TAMAF_AGENT_H

#include "TAMAF/tamaf_agentid.h"
#include "TAMAF/tamaf_aclmessage.h"
#include "TAMAF/tamaf_agentdescription.h"

typedef struct tamaf_agent_t {
    tamaf_agentid_t* aid;
    tamaf_agentdescription_t* agent_description;
    void* ams; // Opaque
    void* mts; // Opaque
} tamaf_agent_t;

struct tamaf_emainteraction_t;
struct tamaf_behavior_t;

/**
 * Create a new agent with a given name.
 * Handles internal AMS and MTS creation.
 */
tamaf_agent_t* tamaf_agent_create(const char* name);

/**
 * Destroy the agent and all its internal components.
 */
void tamaf_agent_destroy(tamaf_agent_t* agent);

void tamaf_agent_setup(tamaf_agent_t* agent, struct tamaf_emainteraction_t* ema);
void tamaf_agent_takedown(tamaf_agent_t* agent, struct tamaf_emainteraction_t* ema);

/**
 * Start the agent's AMS and MTS.
 */
void tamaf_agent_start(tamaf_agent_t* agent);

/**
 * Shutdown the agent.
 */
void tamaf_agent_stop(tamaf_agent_t* agent);

/**
 * Add a behavior to the agent.
 */
void tamaf_agent_add_behavior(tamaf_agent_t* agent, struct tamaf_behavior_t* b);

/**
 * Send an ACL message.
 */
bool tamaf_agent_send(tamaf_agent_t* agent, tamaf_aclmessage_t* msg);

/**
 * Receive a message matching a template (or any if NULL).
 */
tamaf_aclmessage_t* tamaf_agent_receive(tamaf_agent_t* agent, void* template);

/**
 * Put a message back into the queue.
 */
void tamaf_agent_put_back(tamaf_agent_t* agent, tamaf_aclmessage_t* msg);

/**
 * Update the agent description and notify EMA.
 */
void tamaf_agent_update_description(tamaf_agent_t* agent, tamaf_agentdescription_t* ad);

/**
 * Check if the agent's AMS kernel is alive.
 */
bool tamaf_agent_is_alive(tamaf_agent_t* agent);

#endif // TAMAF_AGENT_H
