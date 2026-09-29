#include "TAMAF/tamaf_agent.h"
#include "TAMAF/tamaf_ams.h"
#include "TAMAF/tamaf_mts.h"
#include "TAMAF/tamaf_behavior.h"
#include "TAMAF/tamaf_common_behaviors.h"
#include "TAMAF/tamaf_emainteraction.h"
#include <stdlib.h>

tamaf_agent_t* tamaf_agent_create(const char* name) {
    tamaf_agent_t* agent = (tamaf_agent_t*)calloc(1, sizeof(tamaf_agent_t));
    if (!agent) return NULL;
    
    agent->aid = tamaf_agentid_from_string(name);
    agent->agent_description = tamaf_agentdescription_create(agent->aid);
    agent->mts = tamaf_mts_create(agent);
    agent->ams = tamaf_ams_create(agent);
    
    // Default to standalone to avoid blocking on EMA registration in simple use cases.
    if (agent->ams) tamaf_ams_set_standalone((tamaf_ams_t*)agent->ams, true);
    
    return agent;
}

void tamaf_agent_destroy(tamaf_agent_t* agent) {
    if (!agent) return;
    if (agent->ams) tamaf_ams_destroy((tamaf_ams_t*)agent->ams);
    if (agent->mts) tamaf_mts_destroy((tamaf_mts_t*)agent->mts);
    if (agent->agent_description) tamaf_agentdescription_destroy(agent->agent_description);
    // aid is destroyed by agent_description
    free(agent);
}

void tamaf_agent_add_behavior(tamaf_agent_t* agent, tamaf_behavior_t* b) {
    if (!agent || !agent->ams || !b) return;
    tamaf_ams_add_behavior((tamaf_ams_t*)agent->ams, b);
}

bool tamaf_agent_send(tamaf_agent_t* agent, tamaf_aclmessage_t* msg) {
    if (!agent || !agent->mts || !msg) return false;
    return tamaf_mts_send((tamaf_mts_t*)agent->mts, msg);
}

tamaf_aclmessage_t* tamaf_agent_receive(tamaf_agent_t* agent, void* template) {
    if (!agent || !agent->mts) return NULL;
    return tamaf_mts_receive((tamaf_mts_t*)agent->mts, (tamaf_aclmessagetemplate_t*)template);
}

void tamaf_agent_put_back(tamaf_agent_t* agent, tamaf_aclmessage_t* msg) {
    if (!agent || !agent->ams) return;
    tamaf_ams_put_back((tamaf_ams_t*)agent->ams, msg);
}

void tamaf_agent_start(tamaf_agent_t* agent) {
    if (!agent) return;
    if (agent->ams) tamaf_ams_start((tamaf_ams_t*)agent->ams);
}

void tamaf_agent_stop(tamaf_agent_t* agent) {
    if (!agent) return;
    if (agent->ams) tamaf_ams_shutdown((tamaf_ams_t*)agent->ams);
}

void tamaf_agent_update_description(tamaf_agent_t* agent, tamaf_agentdescription_t* ad) {
    if (!agent || !ad) return;
    for (size_t i = 0; i < agent->agent_description->services_count; i++) tamaf_servicedescription_destroy(agent->agent_description->services[i]);
    free(agent->agent_description->services); agent->agent_description->services = NULL; agent->agent_description->services_count = 0;
    for (size_t i = 0; i < ad->services_count; i++) {
        cJSON* json = tamaf_servicedescription_to_json(ad->services[i]);
        tamaf_agentdescription_add_service(agent->agent_description, tamaf_servicedescription_from_json(json));
        cJSON_Delete(json);
    }
    tamaf_behavior_t* modify_b = tamaf_oneshot_behavior_create(agent, NULL);
    extern void tamaf_modify_agent_ema_action(tamaf_behavior_t* self);
    modify_b->action = tamaf_modify_agent_ema_action;
    tamaf_ams_add_behavior((tamaf_ams_t*)agent->ams, modify_b);
}

bool tamaf_agent_is_alive(tamaf_agent_t* agent) {
    if (!agent || !agent->ams) return false;
    return ((tamaf_ams_t*)agent->ams)->running;
}

// Default empty implementations for user to override if needed.
// On Windows/MinGW, weak symbols are problematic. 
// We use these as defaults; if the user defines them, they will link those instead if using a library,
// but for single binary builds, we should be careful. 
// For now, providing them here to fix the "undefined reference" in the test build.
void tamaf_agent_setup(tamaf_agent_t* agent, tamaf_emainteraction_t* ema) { (void)agent; (void)ema; }
void tamaf_agent_takedown(tamaf_agent_t* agent, tamaf_emainteraction_t* ema) { (void)agent; (void)ema; }
