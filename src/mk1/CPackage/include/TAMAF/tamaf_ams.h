#ifndef TAMAF_AMS_H
#define TAMAF_AMS_H

#include "TAMAF/tamaf_aclmessage.h"
#include "TAMAF/tamaf_behavior.h"
#include "TAMAF/tamaf_lifecyclestate.h"
#include <pthread.h>

struct tamaf_agent_t;

typedef struct tamaf_ams_t {
    struct tamaf_agent_t* agent;
    
    tamaf_behavior_t** active_behaviors;
    size_t active_count;
    size_t active_capacity;
    
    tamaf_behavior_t** pending_additions;
    size_t pending_add_count;
    size_t pending_add_capacity;
    pthread_mutex_t pending_add_lock;
    
    tamaf_behavior_t** pending_removals;
    size_t pending_rem_count;
    size_t pending_rem_capacity;
    pthread_mutex_t pending_rem_lock;
    
    tamaf_behavior_t** blocked_behaviors;
    size_t blocked_count;
    size_t blocked_capacity;
    pthread_mutex_t blocked_lock;
    
    tamaf_lifecyclestate_t life_cycle_state;
    pthread_mutex_t state_lock;
    
    bool running;
    bool standalone;
    pthread_t kernel_thread;
} tamaf_ams_t;

/**
 * Initialize AMS.
 */
tamaf_ams_t* tamaf_ams_create(struct tamaf_agent_t* agent);

/**
 * Destroy AMS.
 */
void tamaf_ams_destroy(tamaf_ams_t* ams);

/**
 * Start the AMS kernel thread.
 */
void tamaf_ams_start(tamaf_ams_t* ams);

/**
 * Shutdown the agent.
 */
void tamaf_ams_shutdown(tamaf_ams_t* ams);

/**
 * Set standalone mode (skip registration with EMA).
 */
void tamaf_ams_set_standalone(tamaf_ams_t* ams, bool standalone);

/**
 * Add a behavior to the agent.
 */
void tamaf_ams_add_behavior(tamaf_ams_t* ams, tamaf_behavior_t* behavior);

/**
 * Remove a behavior from the agent.
 */
void tamaf_ams_remove_behavior(tamaf_ams_t* ams, tamaf_behavior_t* behavior);

/**
 * Put a message back into the transport system.
 */
void tamaf_ams_put_back(tamaf_ams_t* ams, tamaf_aclmessage_t* msg);

#endif // TAMAF_AMS_H
