#include "TAMAF/tamaf_emainteraction.h"
#include "TAMAF/tamaf_defines.h"
#include "TAMAF/tamaf_agentmanagementontology.h"
#include "TAMAF/tamaf_agent.h"
#include "TAMAF/tamaf_mts.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#define sleep_ms(x) Sleep(x)
#define strdup _strdup
#else
#include <unistd.h>
#define sleep_ms(x) usleep((x)*1000)
#endif

tamaf_emainteraction_t* tamaf_emainteraction_create(tamaf_behavior_t* b) {
    tamaf_emainteraction_t* ema = (tamaf_emainteraction_t*)malloc(sizeof(tamaf_emainteraction_t));
    if (!ema) return NULL;
    ema->behavior = b;
    ema->message_sent = false;
    char buf[64]; sprintf(buf, "ema-%p-%ld", (void*)b, (long)time(NULL));
    ema->conversation_id = strdup(buf);
    return ema;
}

void tamaf_emainteraction_destroy(tamaf_emainteraction_t* ema) {
    if (ema) { if (ema->conversation_id) free(ema->conversation_id); free(ema); }
}

bool tamaf_emainteraction_send_to_ema(tamaf_emainteraction_t* ema, tamaf_aclmessage_t* msg) {
    tamaf_mts_t* mts = (tamaf_mts_t*)ema->behavior->agent->mts;
    for (int i = 0; i < DEFAULT_EMA_MESSAGE_RETRYS; i++) {
        if (tamaf_mts_send(mts, msg)) return true;
        sleep_ms((int)(DEFAULT_EMA_MESSAGE_RETRY_INTERVAL * 1000));
    }
    return false;
}

void tamaf_emainteraction_keep_alive(tamaf_emainteraction_t* ema) {
    tamaf_agent_t* agent = ema->behavior->agent;
    tamaf_mts_t* mts = (tamaf_mts_t*)agent->mts;
    tamaf_aclmessage_t* msg = tamaf_aclmessage_create(TAMAF_KEEPALIVE);
    msg->sender = agent->aid;
    tamaf_agentid_t* ema_id = tamaf_agentid_create(DEFAULT_EMA_NAME, tamaf_address_create(mts->ema_address->ip, mts->ema_address->port));
    tamaf_aclmessage_add_receiver(msg, ema_id);
    msg->ontology = strdup(DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME);
    msg->content = tamaf_am_ontology_create_agent_description_content(DEFAULT_AGENTMANAGEMENT_ONTOLOGY_KEEPALIVE_ACTION, &agent->agent_description, 1);
    tamaf_emainteraction_send_to_ema(ema, msg);
    msg->sender = NULL; tamaf_aclmessage_destroy(msg);
}

tamaf_aclmessage_t* tamaf_emainteraction_register_agent(tamaf_emainteraction_t* ema) {
    tamaf_agent_t* agent = ema->behavior->agent;
    tamaf_mts_t* mts = (tamaf_mts_t*)agent->mts;
    if (!ema->message_sent) {
        tamaf_aclmessage_t* msg = tamaf_aclmessage_create(TAMAF_REQUEST);
        msg->sender = agent->aid;
        tamaf_agentid_t* ema_id = tamaf_agentid_create(DEFAULT_EMA_NAME, tamaf_address_create(mts->ema_address->ip, mts->ema_address->port));
        tamaf_aclmessage_add_receiver(msg, ema_id);
        msg->ontology = strdup(DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME);
        msg->conversation_id = strdup(ema->conversation_id);
        msg->content = tamaf_am_ontology_create_agent_description_content(DEFAULT_AGENTMANAGEMENT_ONTOLOGY_REGISTER_ACTION, &agent->agent_description, 1);
        if (tamaf_emainteraction_send_to_ema(ema, msg)) ema->message_sent = true;
        msg->sender = NULL; tamaf_aclmessage_destroy(msg);
    }
    if (ema->message_sent) {
        tamaf_aclmessage_t* resp = tamaf_mts_receive(mts, NULL);
        if (resp) {
            if (tamaf_am_ontology_validate(resp->content)) { ema->message_sent = false; return resp; }
            tamaf_aclmessage_destroy(resp);
        }
    }
    return NULL;
}

tamaf_aclmessage_t* tamaf_emainteraction_deregister_agent(tamaf_emainteraction_t* ema) {
    tamaf_agent_t* agent = ema->behavior->agent;
    tamaf_mts_t* mts = (tamaf_mts_t*)agent->mts;
    if (!ema->message_sent) {
        tamaf_aclmessage_t* msg = tamaf_aclmessage_create(TAMAF_REQUEST);
        msg->sender = agent->aid;
        tamaf_agentid_t* ema_id = tamaf_agentid_create(DEFAULT_EMA_NAME, tamaf_address_create(mts->ema_address->ip, mts->ema_address->port));
        tamaf_aclmessage_add_receiver(msg, ema_id);
        msg->ontology = strdup(DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME);
        msg->conversation_id = strdup(ema->conversation_id);
        msg->content = tamaf_am_ontology_create_agent_description_content(DEFAULT_AGENTMANAGEMENT_ONTOLOGY_DEREGISTER_ACTION, &agent->agent_description, 1);
        if (tamaf_emainteraction_send_to_ema(ema, msg)) ema->message_sent = true;
        msg->sender = NULL; tamaf_aclmessage_destroy(msg);
    }
    if (ema->message_sent) {
        tamaf_aclmessage_t* resp = tamaf_mts_receive(mts, NULL);
        if (resp) {
            if (tamaf_am_ontology_validate(resp->content)) { ema->message_sent = false; return resp; }
            tamaf_aclmessage_destroy(resp);
        }
    }
    return NULL;
}

tamaf_aclmessage_t* tamaf_emainteraction_modify_agent(tamaf_emainteraction_t* ema) {
    tamaf_agent_t* agent = ema->behavior->agent;
    tamaf_mts_t* mts = (tamaf_mts_t*)agent->mts;
    if (!ema->message_sent) {
        tamaf_aclmessage_t* msg = tamaf_aclmessage_create(TAMAF_REQUEST);
        msg->sender = agent->aid;
        tamaf_agentid_t* ema_id = tamaf_agentid_create(DEFAULT_EMA_NAME, tamaf_address_create(mts->ema_address->ip, mts->ema_address->port));
        tamaf_aclmessage_add_receiver(msg, ema_id);
        msg->ontology = strdup(DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME);
        msg->conversation_id = strdup(ema->conversation_id);
        msg->content = tamaf_am_ontology_create_agent_description_content(DEFAULT_AGENTMANAGEMENT_ONTOLOGY_MODIFY_ACTION, &agent->agent_description, 1);
        if (tamaf_emainteraction_send_to_ema(ema, msg)) ema->message_sent = true;
        msg->sender = NULL; tamaf_aclmessage_destroy(msg);
    }
    if (ema->message_sent) {
        tamaf_aclmessage_t* resp = tamaf_mts_receive(mts, NULL);
        if (resp) {
            if (tamaf_am_ontology_validate(resp->content)) { ema->message_sent = false; return resp; }
            tamaf_aclmessage_destroy(resp);
        }
    }
    return NULL;
}

static tamaf_agentdescription_t** ema_search_internal(tamaf_emainteraction_t* ema, const char* action, tamaf_agentdescription_t* template, size_t* out_count) {
    tamaf_agent_t* agent = ema->behavior->agent;
    tamaf_mts_t* mts = (tamaf_mts_t*)agent->mts;
    if (!ema->message_sent) {
        tamaf_aclmessage_t* msg = tamaf_aclmessage_create(TAMAF_REQUEST);
        msg->sender = agent->aid;
        tamaf_agentid_t* ema_id = tamaf_agentid_create(DEFAULT_EMA_NAME, tamaf_address_create(mts->ema_address->ip, mts->ema_address->port));
        tamaf_aclmessage_add_receiver(msg, ema_id);
        msg->ontology = strdup(DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME);
        msg->conversation_id = strdup(ema->conversation_id);
        msg->content = tamaf_am_ontology_create_agent_description_content(action, &template, 1);
        if (tamaf_emainteraction_send_to_ema(ema, msg)) ema->message_sent = true;
        msg->sender = NULL; tamaf_aclmessage_destroy(msg);
    }
    if (ema->message_sent) {
        tamaf_aclmessage_t* resp = tamaf_mts_receive(mts, NULL);
        if (resp) {
            if (tamaf_am_ontology_validate(resp->content)) {
                ema->message_sent = false;
                tamaf_agentdescription_t** results = tamaf_am_ontology_get_agent_descriptions(resp->content, out_count);
                tamaf_aclmessage_destroy(resp);
                return results;
            }
            tamaf_aclmessage_destroy(resp);
        }
    }
    return NULL;
}

tamaf_agentdescription_t** tamaf_emainteraction_search(tamaf_emainteraction_t* ema, tamaf_agentdescription_t* template, size_t* out_count) {
    return ema_search_internal(ema, DEFAULT_AGENTMANAGEMENT_ONTOLOGY_GLOBAL_SEARCH_ACTION, template, out_count);
}

tamaf_agentdescription_t** tamaf_emainteraction_local_search(tamaf_emainteraction_t* ema, tamaf_agentdescription_t* template, size_t* out_count) {
    return ema_search_internal(ema, DEFAULT_AGENTMANAGEMENT_ONTOLOGY_LOCAL_SEARCH_ACTION, template, out_count);
}

tamaf_agentdescription_t** tamaf_emainteraction_external_search(tamaf_emainteraction_t* ema, tamaf_agentdescription_t* template, size_t* out_count) {
    return ema_search_internal(ema, DEFAULT_AGENTMANAGEMENT_ONTOLOGY_EXTERNAL_SEARCH_ACTION, template, out_count);
}

void tamaf_modify_agent_ema_action(tamaf_behavior_t* self) {
    if (!self->ema_interaction) self->ema_interaction = tamaf_emainteraction_create(self);
    tamaf_emainteraction_modify_agent(self->ema_interaction);
}
