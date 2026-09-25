#include "TAMAF/tamaf_ams.h"
#include "TAMAF/tamaf_agent.h"
#include "TAMAF/tamaf_mts.h"
#include "TAMAF/tamaf_common_behaviors.h"
#include "TAMAF/tamaf_emainteraction.h"
#include "TAMAF/tamaf_agentmanagementontology.h"
#include "TAMAF/tamaf_defines.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#define sleep_ms(x) Sleep(x)
#else
#include <unistd.h>
#define sleep_ms(x) usleep((x)*1000)
#endif

// Internal behavior declarations
static void ams_unblock_behaviors(tamaf_ams_t* ams);
static void ams_block_behavior(tamaf_ams_t* ams, tamaf_behavior_t* b);

typedef struct {
    bool registered;
} ams_init_data_t;

typedef struct {
    bool deRegister;
} ams_closing_data_t;

static void heartbeat_on_tick(tamaf_behavior_t* self) {
    tamaf_emainteraction_keep_alive(self->ema_interaction);
}

static void init_action(tamaf_behavior_t* self) {
    ams_init_data_t* data = (ams_init_data_t*)self->derived;
    tamaf_agent_t* agent = self->agent;
    tamaf_mts_t* mts = (tamaf_mts_t*)agent->mts;
    tamaf_ams_t* ams = (tamaf_ams_t*)agent->ams;
    
    if (!data->registered) {
        if (ams->standalone) {
            if (!mts->server_online) {
                tamaf_mts_start_server(mts, agent->aid->address->port);
                while (!mts->server_online) sleep_ms(100);
            }
            tamaf_agent_setup(agent, self->ema_interaction);
            data->registered = true;
            return;
        }

        if (!mts->server_online) {
            if (strcmp(agent->aid->name, DEFAULT_EMA_NAME) == 0) {
                tamaf_mts_start_server(mts, mts->ema_address->port);
                while (!mts->server_online) sleep_ms(100);
                tamaf_agent_setup(agent, self->ema_interaction);
                data->registered = true;
                return;
            }
            tamaf_mts_start_server(mts, mts->register_address->port);
            while (!mts->server_online) sleep_ms(100);
        }
        
        tamaf_aclmessage_t* agree = tamaf_emainteraction_register_agent(self->ema_interaction);
        if (agree) {
            tamaf_mts_stop_server(mts);
            int port = tamaf_am_ontology_get_agent_port(agree->content);
            if (port == DEFAULT_AGENT_ALREADY_EXISTS) {
                printf("Error: Agent already exists!\n");
                tamaf_aclmessage_destroy(agree);
                tamaf_ams_shutdown(ams);
                return;
            }
            tamaf_agentid_set_port(agent->aid, port);
            tamaf_mts_start_server(mts, port);
            while (!mts->server_online) sleep_ms(100);
            
            tamaf_behavior_t* hb = tamaf_ticker_behavior_create(agent, DEFAULT_EMA_HEARTBEAT_INTERVAL, heartbeat_on_tick);
            hb->ema_interaction = tamaf_emainteraction_create(hb);
            tamaf_ams_add_behavior((tamaf_ams_t*)agent->ams, hb);
            
            data->registered = true;
            tamaf_aclmessage_destroy(agree);
        } else {
            tamaf_behavior_block(self);
            return;
        }
    }
    tamaf_agent_setup(agent, self->ema_interaction);
}

static bool init_done(tamaf_behavior_t* self) {
    ams_init_data_t* data = (ams_init_data_t*)self->derived;
    return data->registered;
}

static void closing_action(tamaf_behavior_t* self) {
    ams_closing_data_t* data = (ams_closing_data_t*)self->derived;
    tamaf_agent_t* agent = self->agent;
    tamaf_agent_takedown(agent, self->ema_interaction);
    if (strcmp(agent->aid->name, DEFAULT_EMA_NAME) != 0 && data->deRegister) {
        tamaf_emainteraction_deregister_agent(self->ema_interaction);
    }
}

static void* ams_kernel_thread(void* arg) {
    tamaf_ams_t* ams = (tamaf_ams_t*)arg;
    tamaf_agent_t* agent = ams->agent;
    tamaf_mts_t* mts = (tamaf_mts_t*)agent->mts;
    
    tamaf_behavior_t* initialization_behavior = tamaf_behavior_create(agent);
    initialization_behavior->action = init_action;
    initialization_behavior->done = init_done;
    initialization_behavior->ema_interaction = tamaf_emainteraction_create(initialization_behavior);
    ams_init_data_t* init_data = (ams_init_data_t*)calloc(1, sizeof(ams_init_data_t));
    initialization_behavior->derived = init_data;
    
    tamaf_behavior_t* closing_behavior = tamaf_oneshot_behavior_create(agent, closing_action);
    closing_behavior->ema_interaction = tamaf_emainteraction_create(closing_behavior);
    ams_closing_data_t* closing_data = (ams_closing_data_t*)calloc(1, sizeof(ams_closing_data_t));
    closing_data->deRegister = true;
    closing_behavior->derived = closing_data;
    
    while (ams->running) {
        tamaf_lifecyclestate_t current_state;
        pthread_mutex_lock(&ams->state_lock);
        current_state = ams->life_cycle_state;
        pthread_mutex_unlock(&ams->state_lock);
        
        switch (current_state) {
            case TAMAF_STATE_SUSPENDED: sleep_ms(DEFAULT_KERNEL_STABILIZATION_TIME_MS); continue;
            case TAMAF_STATE_INITIATED:
                if (mts->has_new_message) {
                    if (initialization_behavior->is_blocked) tamaf_behavior_unblock(initialization_behavior);
                    mts->has_new_message = false;
                }
                if (!initialization_behavior->is_started) {
                    if (initialization_behavior->on_start) initialization_behavior->on_start(initialization_behavior);
                    initialization_behavior->is_started = true;
                }
                initialization_behavior->execute(initialization_behavior);
                if (initialization_behavior->is_blocked) { sleep_ms(DEFAULT_KERNEL_STABILIZATION_TIME_MS); continue; }
                if (initialization_behavior->done(initialization_behavior)) {
                    if (initialization_behavior->on_end) initialization_behavior->on_end(initialization_behavior);
                    pthread_mutex_lock(&ams->state_lock);
                    if (ams->life_cycle_state == TAMAF_STATE_INITIATED) ams->life_cycle_state = TAMAF_STATE_ACTIVE;
                    pthread_mutex_unlock(&ams->state_lock);
                }
                break;
                
            case TAMAF_STATE_ACTIVE:
                if (mts->has_new_message) { ams_unblock_behaviors(ams); mts->has_new_message = false; }
                
                pthread_mutex_lock(&ams->pending_add_lock);
                if (ams->pending_add_count > 0) {
                    if (ams->active_count + ams->pending_add_count > ams->active_capacity) {
                        ams->active_capacity = ams->active_count + ams->pending_add_count + 10;
                        ams->active_behaviors = (tamaf_behavior_t**)realloc(ams->active_behaviors, ams->active_capacity * sizeof(tamaf_behavior_t*));
                    }
                    memcpy(ams->active_behaviors + ams->active_count, ams->pending_additions, ams->pending_add_count * sizeof(tamaf_behavior_t*));
                    ams->active_count += ams->pending_add_count; ams->pending_add_count = 0;
                }
                pthread_mutex_unlock(&ams->pending_add_lock);
                
                pthread_mutex_lock(&ams->pending_rem_lock);
                if (ams->pending_rem_count > 0) {
                    for (size_t i = 0; i < ams->pending_rem_count; i++) {
                        tamaf_behavior_t* to_rem = ams->pending_removals[i];
                        for (size_t j = 0; j < ams->active_count; j++) {
                            if (ams->active_behaviors[j] == to_rem) {
                                for (size_t k = j; k < ams->active_count - 1; k++) ams->active_behaviors[k] = ams->active_behaviors[k+1];
                                ams->active_count--; break;
                            }
                        }
                    }
                    ams->pending_rem_count = 0;
                }
                pthread_mutex_unlock(&ams->pending_rem_lock);
                
                for (size_t i = 0; i < ams->active_count; i++) {
                    tamaf_behavior_t* b = ams->active_behaviors[i];
                    pthread_mutex_lock(&ams->state_lock);
                    if (ams->life_cycle_state != TAMAF_STATE_ACTIVE) { pthread_mutex_unlock(&ams->state_lock); break; }
                    pthread_mutex_unlock(&ams->state_lock);
                    
                    if (!b->is_started) {
                        if (!b->ema_interaction) b->ema_interaction = tamaf_emainteraction_create(b);
                        if (b->on_start) b->on_start(b);
                        b->is_started = true;
                    }
                    b->execute(b);
                    if (b->is_blocked) { ams_block_behavior(ams, b); i--; continue; }
                    if (b->done(b)) {
                        if (b->on_end) b->on_end(b);
                        for (size_t j = i; j < ams->active_count - 1; j++) ams->active_behaviors[j] = ams->active_behaviors[j+1];
                        ams->active_count--; i--;
                    }
                }
                break;
                
            case TAMAF_STATE_CLOSING:
                if (mts->has_new_message) { if (closing_behavior->is_blocked) tamaf_behavior_unblock(closing_behavior); mts->has_new_message = false; }
                if (!closing_behavior->is_started) { if (closing_behavior->on_start) closing_behavior->on_start(closing_behavior); closing_behavior->is_started = true; }
                closing_behavior->execute(closing_behavior);
                if (closing_behavior->is_blocked) { sleep_ms(DEFAULT_KERNEL_STABILIZATION_TIME_MS); continue; }
                if (closing_behavior->done(closing_behavior)) {
                    if (closing_behavior->on_end) closing_behavior->on_end(closing_behavior);
                    tamaf_mts_stop_server(mts); ams->running = false;
                }
                break;
            default: break;
        }
        sleep_ms(DEFAULT_KERNEL_STABILIZATION_TIME_MS);
    }
    
    tamaf_emainteraction_destroy(initialization_behavior->ema_interaction);
    free(init_data); initialization_behavior->destroy(initialization_behavior);
    tamaf_emainteraction_destroy(closing_behavior->ema_interaction);
    free(closing_data); closing_behavior->destroy(closing_behavior);
    return NULL;
}

tamaf_ams_t* tamaf_ams_create(struct tamaf_agent_t* agent) {
    tamaf_ams_t* ams = (tamaf_ams_t*)calloc(1, sizeof(tamaf_ams_t));
    if (!ams) return NULL;
    ams->agent = agent; ams->life_cycle_state = TAMAF_STATE_INITIATED; ams->running = true;
    pthread_mutex_init(&ams->state_lock, NULL); pthread_mutex_init(&ams->pending_add_lock, NULL);
    pthread_mutex_init(&ams->pending_rem_lock, NULL); pthread_mutex_init(&ams->blocked_lock, NULL);
    return ams;
}

void tamaf_ams_destroy(tamaf_ams_t* ams) {
    if (!ams) return;
    ams->running = false; pthread_join(ams->kernel_thread, NULL);
    pthread_mutex_destroy(&ams->state_lock); pthread_mutex_destroy(&ams->pending_add_lock);
    pthread_mutex_destroy(&ams->pending_rem_lock); pthread_mutex_destroy(&ams->blocked_lock);
    for (size_t i = 0; i < ams->active_count; i++) ams->active_behaviors[i]->destroy(ams->active_behaviors[i]);
    free(ams->active_behaviors);
    for (size_t i = 0; i < ams->blocked_count; i++) ams->blocked_behaviors[i]->destroy(ams->blocked_behaviors[i]);
    free(ams->blocked_behaviors); free(ams->pending_additions); free(ams->pending_removals); free(ams);
}

void tamaf_ams_start(tamaf_ams_t* ams) { pthread_create(&ams->kernel_thread, NULL, ams_kernel_thread, ams); }
void tamaf_ams_shutdown(tamaf_ams_t* ams) {
    pthread_mutex_lock(&ams->state_lock); ams->life_cycle_state = TAMAF_STATE_CLOSING; pthread_mutex_unlock(&ams->state_lock);
}
void tamaf_ams_set_standalone(tamaf_ams_t* ams, bool standalone) { if (ams) ams->standalone = standalone; }
void tamaf_ams_add_behavior(tamaf_ams_t* ams, tamaf_behavior_t* behavior) {
    pthread_mutex_lock(&ams->pending_add_lock);
    if (ams->pending_add_count >= ams->pending_add_capacity) {
        ams->pending_add_capacity = ams->pending_add_count + 10;
        ams->pending_additions = (tamaf_behavior_t**)realloc(ams->pending_additions, ams->pending_add_capacity * sizeof(tamaf_behavior_t*));
    }
    ams->pending_additions[ams->pending_add_count++] = behavior;
    pthread_mutex_unlock(&ams->pending_add_lock);
}

void tamaf_ams_remove_behavior(tamaf_ams_t* ams, tamaf_behavior_t* behavior) {
    pthread_mutex_lock(&ams->pending_rem_lock);
    if (ams->pending_rem_count >= ams->pending_rem_capacity) {
        ams->pending_rem_capacity = ams->pending_rem_count + 10;
        ams->pending_removals = (tamaf_behavior_t**)realloc(ams->pending_removals, ams->pending_rem_capacity * sizeof(tamaf_behavior_t*));
    }
    ams->pending_removals[ams->pending_rem_count++] = behavior;
    pthread_mutex_unlock(&ams->pending_rem_lock);
}

static void ams_unblock_behaviors(tamaf_ams_t* ams) {
    pthread_mutex_lock(&ams->blocked_lock);
    if (ams->blocked_count > 0) {
        for (size_t i = 0; i < ams->blocked_count; i++) tamaf_behavior_unblock(ams->blocked_behaviors[i]);
        if (ams->active_count + ams->blocked_count > ams->active_capacity) {
            ams->active_capacity = ams->active_count + ams->blocked_count + 10;
            ams->active_behaviors = (tamaf_behavior_t**)realloc(ams->active_behaviors, ams->active_capacity * sizeof(tamaf_behavior_t*));
        }
        memcpy(ams->active_behaviors + ams->active_count, ams->blocked_behaviors, ams->blocked_count * sizeof(tamaf_behavior_t*));
        ams->active_count += ams->blocked_count; ams->blocked_count = 0;
    }
    pthread_mutex_unlock(&ams->blocked_lock);
}

static void ams_block_behavior(tamaf_ams_t* ams, tamaf_behavior_t* b) {
    pthread_mutex_lock(&ams->blocked_lock);
    if (ams->blocked_count >= ams->blocked_capacity) {
        ams->blocked_capacity = ams->blocked_count + 10;
        ams->blocked_behaviors = (tamaf_behavior_t**)realloc(ams->blocked_behaviors, ams->blocked_capacity * sizeof(tamaf_behavior_t*));
    }
    ams->blocked_behaviors[ams->blocked_count++] = b;
    pthread_mutex_unlock(&ams->blocked_lock);
    for (size_t i = 0; i < ams->active_count; i++) {
        if (ams->active_behaviors[i] == b) {
            for (size_t j = i; j < ams->active_count - 1; j++) ams->active_behaviors[j] = ams->active_behaviors[j+1];
            ams->active_count--; break;
        }
    }
}

void tamaf_ams_put_back(tamaf_ams_t* ams, tamaf_aclmessage_t* msg) { tamaf_mts_put_back((tamaf_mts_t*)ams->agent->mts, msg); }
