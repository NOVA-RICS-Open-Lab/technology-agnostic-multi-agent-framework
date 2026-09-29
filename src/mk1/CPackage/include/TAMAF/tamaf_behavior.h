#ifndef TAMAF_BEHAVIOR_H
#define TAMAF_BEHAVIOR_H

#include <stdbool.h>
#include <stdlib.h>
#include <setjmp.h>
#include "TAMAF/tamaf_aclmessage.h"

struct tamaf_agent_t; // Forward declaration

typedef struct tamaf_behavior_t tamaf_behavior_t;

struct tamaf_behavior_t {
    void* derived; // Pointer to subclass-specific data
    void* user_data; // Pointer to user-specific data
    struct tamaf_agent_t* agent;
    struct tamaf_emainteraction_t* ema_interaction;
    bool is_blocked;
    bool is_started;
    
    // Virtual Functions (Hooks)
    void (*on_start)(tamaf_behavior_t* self);
    void (*action)(tamaf_behavior_t* self);
    bool (*done)(tamaf_behavior_t* self);
    int (*on_end)(tamaf_behavior_t* self);
    
    // Internal behavior methods
    void (*execute)(tamaf_behavior_t* self);
    void (*destroy)(tamaf_behavior_t* self);
    
    // Exception simulation
    jmp_buf exception_env;
    
    // Message queue for "put back" logic
    tamaf_aclmessage_t** messages_to_put_back;
    size_t messages_to_put_back_count;
};

/**
 * Create a new base behavior object.
 */
tamaf_behavior_t* tamaf_behavior_create(struct tamaf_agent_t* agent);

/**
 * Base behavior constructor.
 * Initializes common fields.
 */
void tamaf_behavior_init(tamaf_behavior_t* behavior, struct tamaf_agent_t* agent);

/**
 * Base behavior destructor.
 */
void tamaf_behavior_base_destroy(tamaf_behavior_t* behavior);

/**
 * Default execute implementation.
 */
void tamaf_behavior_base_execute(tamaf_behavior_t* behavior);

/**
 * Block the behavior.
 */
void tamaf_behavior_block(tamaf_behavior_t* behavior);
void tamaf_behavior_unblock(tamaf_behavior_t* behavior);

/**
 * Receive a message matching the template.
 * Throws "EmptyReceiveException" (longjmp) if no message is found.
 */
tamaf_aclmessage_t* tamaf_behavior_receive(tamaf_behavior_t* behavior, void* template);

#endif // TAMAF_BEHAVIOR_H
