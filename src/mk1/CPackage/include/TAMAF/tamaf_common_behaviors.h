#ifndef TAMAF_COMMON_BEHAVIORS_H
#define TAMAF_COMMON_BEHAVIORS_H

#include "TAMAF/tamaf_behavior.h"

/**
 * OneShotBehavior: runs once and finishes.
 */
tamaf_behavior_t* tamaf_oneshot_behavior_create(struct tamaf_agent_t* agent, void (*action)(tamaf_behavior_t* self));

/**
 * CyclicBehavior: runs indefinitely.
 */
tamaf_behavior_t* tamaf_cyclic_behavior_create(struct tamaf_agent_t* agent, void (*action)(tamaf_behavior_t* self));

/**
 * TickerBehavior: runs action at regular intervals.
 */
typedef struct {
    int period_ms;
    long last_tick_ms;
    void (*on_tick)(tamaf_behavior_t* self);
} tamaf_ticker_data_t;

tamaf_behavior_t* tamaf_ticker_behavior_create(struct tamaf_agent_t* agent, int period_ms, void (*on_tick)(tamaf_behavior_t* self));

/**
 * WakerBehavior: wakes up after a timeout.
 */
typedef struct {
    long wake_up_time_ms;
    bool done_flag;
    void (*on_wake)(tamaf_behavior_t* self);
} tamaf_waker_data_t;

tamaf_behavior_t* tamaf_waker_behavior_create(struct tamaf_agent_t* agent, int timeout_ms, void (*on_wake)(tamaf_behavior_t* self));

/**
 * SequentialBehavior: runs sub-behaviors one after another.
 */
tamaf_behavior_t* tamaf_sequential_behavior_create(struct tamaf_agent_t* agent);
void tamaf_sequential_behavior_add_sub(tamaf_behavior_t* seq, tamaf_behavior_t* sub);

/**
 * ParallelBehavior: runs sub-behaviors simultaneously.
 */
tamaf_behavior_t* tamaf_parallel_behavior_create(struct tamaf_agent_t* agent, bool finish_when_any_done);
void tamaf_parallel_behavior_add_sub(tamaf_behavior_t* par, tamaf_behavior_t* sub);

/**
 * FSMBehavior: Finite State Machine behavior.
 */
#define TAMAF_FSM_DEFAULT -1

tamaf_behavior_t* tamaf_fsm_behavior_create(struct tamaf_agent_t* agent);
void tamaf_fsm_add_state(tamaf_behavior_t* fsm, tamaf_behavior_t* b, const char* name);
void tamaf_fsm_add_initial_state(tamaf_behavior_t* fsm, tamaf_behavior_t* b, const char* name);
void tamaf_fsm_add_final_state(tamaf_behavior_t* fsm, tamaf_behavior_t* b, const char* name);
void tamaf_fsm_add_transition(tamaf_behavior_t* fsm, const char* source, const char* dest, int code);
void tamaf_fsm_add_default_transition(tamaf_behavior_t* fsm, const char* source, const char* dest);

#endif // TAMAF_COMMON_BEHAVIORS_H
