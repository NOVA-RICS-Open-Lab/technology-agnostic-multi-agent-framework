#include "TAMAF/tamaf_common_behaviors.h"
#include <time.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
static long get_time_ms() {
    return GetTickCount();
}
#define strdup _strdup
#else
static long get_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}
#endif

// OneShot
static bool oneshot_done(tamaf_behavior_t* self) { return true; }

tamaf_behavior_t* tamaf_oneshot_behavior_create(struct tamaf_agent_t* agent, void (*action)(tamaf_behavior_t* self)) {
    tamaf_behavior_t* b = (tamaf_behavior_t*)malloc(sizeof(tamaf_behavior_t));
    tamaf_behavior_init(b, agent);
    b->action = action;
    b->done = oneshot_done;
    return b;
}

// Cyclic
static bool cyclic_done(tamaf_behavior_t* self) { return false; }

tamaf_behavior_t* tamaf_cyclic_behavior_create(struct tamaf_agent_t* agent, void (*action)(tamaf_behavior_t* self)) {
    tamaf_behavior_t* b = (tamaf_behavior_t*)malloc(sizeof(tamaf_behavior_t));
    tamaf_behavior_init(b, agent);
    b->action = action;
    b->done = cyclic_done;
    return b;
}

// Ticker
static void ticker_action(tamaf_behavior_t* self) {
    tamaf_ticker_data_t* data = (tamaf_ticker_data_t*)self->derived;
    long now = get_time_ms();
    if (now >= data->last_tick_ms + data->period_ms) {
        if (data->on_tick) data->on_tick(self);
        data->last_tick_ms = now;
    }
}

static void ticker_destroy(tamaf_behavior_t* self) {
    if (self->derived) free(self->derived);
    tamaf_behavior_base_destroy(self);
}

tamaf_behavior_t* tamaf_ticker_behavior_create(struct tamaf_agent_t* agent, int period_ms, void (*on_tick)(tamaf_behavior_t* self)) {
    tamaf_behavior_t* b = (tamaf_behavior_t*)malloc(sizeof(tamaf_behavior_t));
    tamaf_behavior_init(b, agent);
    tamaf_ticker_data_t* data = (tamaf_ticker_data_t*)malloc(sizeof(tamaf_ticker_data_t));
    data->period_ms = period_ms;
    data->last_tick_ms = get_time_ms();
    data->on_tick = on_tick;
    b->derived = data;
    b->action = ticker_action;
    b->done = cyclic_done;
    b->destroy = ticker_destroy;
    return b;
}

// Waker
static void waker_action(tamaf_behavior_t* self) {
    tamaf_waker_data_t* data = (tamaf_waker_data_t*)self->derived;
    if (!data->done_flag && get_time_ms() >= data->wake_up_time_ms) {
        if (data->on_wake) data->on_wake(self);
        data->done_flag = true;
    }
}

static bool waker_done(tamaf_behavior_t* self) {
    tamaf_waker_data_t* data = (tamaf_waker_data_t*)self->derived;
    return data->done_flag;
}

static void waker_destroy(tamaf_behavior_t* self) {
    if (self->derived) free(self->derived);
    tamaf_behavior_base_destroy(self);
}

tamaf_behavior_t* tamaf_waker_behavior_create(struct tamaf_agent_t* agent, int timeout_ms, void (*on_wake)(tamaf_behavior_t* self)) {
    tamaf_behavior_t* b = (tamaf_behavior_t*)malloc(sizeof(tamaf_behavior_t));
    tamaf_behavior_init(b, agent);
    tamaf_waker_data_t* data = (tamaf_waker_data_t*)malloc(sizeof(tamaf_waker_data_t));
    data->wake_up_time_ms = get_time_ms() + timeout_ms;
    data->done_flag = false;
    data->on_wake = on_wake;
    b->derived = data;
    b->action = waker_action;
    b->done = waker_done;
    b->destroy = waker_destroy;
    return b;
}

// Sequential
typedef struct {
    tamaf_behavior_t** sub;
    size_t count;
    size_t current_idx;
} sequential_data_t;

static void sequential_action(tamaf_behavior_t* self) {
    sequential_data_t* data = (sequential_data_t*)self->derived;
    if (data->current_idx < data->count) {
        data->sub[data->current_idx]->execute(data->sub[data->current_idx]);
        if (data->sub[data->current_idx]->is_blocked) tamaf_behavior_block(self);
    }
}

static bool sequential_done(tamaf_behavior_t* self) {
    sequential_data_t* data = (sequential_data_t*)self->derived;
    if (data->current_idx >= data->count) return true;
    tamaf_behavior_t* current = data->sub[data->current_idx];
    if (!current->done(current)) return false;
    if (current->on_end) current->on_end(current);
    data->current_idx++;
    if (data->current_idx >= data->count) return true;
    self->is_started = false; return false;
}

static void sequential_on_start(tamaf_behavior_t* self) {
    sequential_data_t* data = (sequential_data_t*)self->derived;
    if (data->current_idx < data->count) {
        if (data->sub[data->current_idx]->on_start) data->sub[data->current_idx]->on_start(data->sub[data->current_idx]);
    }
}

static void sequential_destroy(tamaf_behavior_t* self) {
    sequential_data_t* data = (sequential_data_t*)self->derived;
    for (size_t i = 0; i < data->count; i++) data->sub[i]->destroy(data->sub[i]);
    free(data->sub); free(data);
    tamaf_behavior_base_destroy(self);
}

tamaf_behavior_t* tamaf_sequential_behavior_create(struct tamaf_agent_t* agent) {
    tamaf_behavior_t* b = (tamaf_behavior_t*)malloc(sizeof(tamaf_behavior_t));
    tamaf_behavior_init(b, agent);
    sequential_data_t* data = (sequential_data_t*)calloc(1, sizeof(sequential_data_t));
    b->derived = data;
    b->on_start = sequential_on_start;
    b->action = sequential_action;
    b->done = sequential_done;
    b->destroy = sequential_destroy;
    return b;
}

void tamaf_sequential_behavior_add_sub(tamaf_behavior_t* seq, tamaf_behavior_t* sub) {
    sequential_data_t* data = (sequential_data_t*)seq->derived;
    data->sub = (tamaf_behavior_t**)realloc(data->sub, (data->count + 1) * sizeof(tamaf_behavior_t*));
    data->sub[data->count++] = sub;
}

// Parallel
typedef struct {
    tamaf_behavior_t** sub;
    size_t count;
    bool finish_when_any_done;
} parallel_data_t;

static void parallel_on_start(tamaf_behavior_t* self) {
    parallel_data_t* data = (parallel_data_t*)self->derived;
    for (size_t i = 0; i < data->count; i++) if (data->sub[i]->on_start) data->sub[i]->on_start(data->sub[i]);
}

static void parallel_action(tamaf_behavior_t* self) {
    parallel_data_t* data = (parallel_data_t*)self->derived;
    bool all_blocked = true;
    for (size_t i = 0; i < data->count; i++) {
        if (data->sub[i]->done(data->sub[i])) continue;
        if (!data->sub[i]->is_blocked) { data->sub[i]->execute(data->sub[i]); all_blocked = false; }
    }
    if (all_blocked) tamaf_behavior_block(self);
}

static bool parallel_done(tamaf_behavior_t* self) {
    parallel_data_t* data = (parallel_data_t*)self->derived;
    if (data->finish_when_any_done) {
        for (size_t i = 0; i < data->count; i++) if (data->sub[i]->done(data->sub[i])) {
            for (size_t j = 0; j < data->count; j++) if (data->sub[j]->on_end) data->sub[j]->on_end(data->sub[j]);
            return true;
        }
        return false;
    } else {
        for (size_t i = 0; i < data->count; i++) if (!data->sub[i]->done(data->sub[i])) return false;
        for (size_t j = 0; j < data->count; j++) if (data->sub[j]->on_end) data->sub[j]->on_end(data->sub[j]);
        return true;
    }
}

static void parallel_destroy(tamaf_behavior_t* self) {
    parallel_data_t* data = (parallel_data_t*)self->derived;
    for (size_t i = 0; i < data->count; i++) data->sub[i]->destroy(data->sub[i]);
    free(data->sub); free(data);
    tamaf_behavior_base_destroy(self);
}

tamaf_behavior_t* tamaf_parallel_behavior_create(struct tamaf_agent_t* agent, bool finish_when_any_done) {
    tamaf_behavior_t* b = (tamaf_behavior_t*)malloc(sizeof(tamaf_behavior_t));
    tamaf_behavior_init(b, agent);
    parallel_data_t* data = (parallel_data_t*)calloc(1, sizeof(parallel_data_t));
    data->finish_when_any_done = finish_when_any_done;
    b->derived = data;
    b->on_start = parallel_on_start;
    b->action = parallel_action;
    b->done = parallel_done;
    b->destroy = parallel_destroy;
    return b;
}

void tamaf_parallel_behavior_add_sub(tamaf_behavior_t* par, tamaf_behavior_t* sub) {
    parallel_data_t* data = (parallel_data_t*)par->derived;
    data->sub = (tamaf_behavior_t**)realloc(data->sub, (data->count + 1) * sizeof(tamaf_behavior_t*));
    data->sub[data->count++] = sub;
}

// FSM
typedef struct { char source[64]; char dest[64]; int code; } fsm_transition_t;
typedef struct { char name[64]; tamaf_behavior_t* b; bool is_final; } fsm_state_t;
typedef struct {
    fsm_state_t* states; size_t state_count;
    fsm_transition_t* transitions; size_t trans_count;
    char initial_state[64]; char current_state[64];
} fsm_data_t;

static fsm_state_t* fsm_find_state(fsm_data_t* data, const char* name) {
    for (size_t i = 0; i < data->state_count; i++) if (strcmp(data->states[i].name, name) == 0) return &data->states[i];
    return NULL;
}

static void fsm_on_start(tamaf_behavior_t* self) {
    fsm_data_t* data = (fsm_data_t*)self->derived;
    fsm_state_t* s = fsm_find_state(data, data->current_state);
    if (s && s->b->on_start) s->b->on_start(s->b);
}

static void fsm_action(tamaf_behavior_t* self) {
    fsm_data_t* data = (fsm_data_t*)self->derived;
    fsm_state_t* s = fsm_find_state(data, data->current_state);
    if (s) {
        s->b->execute(s->b);
        if (s->b->is_blocked) tamaf_behavior_block(self);
    }
}

static bool fsm_done(tamaf_behavior_t* self) {
    fsm_data_t* data = (fsm_data_t*)self->derived;
    fsm_state_t* s = fsm_find_state(data, data->current_state);
    if (!s) return true;
    if (!s->b->done(s->b)) return false;
    if (s->is_final) { if (s->b->on_end) s->b->on_end(s->b); return true; }
    int code = (s->b->on_end) ? s->b->on_end(s->b) : 0;
    char* next = NULL;
    for (size_t i = 0; i < data->trans_count; i++) {
        if (strcmp(data->transitions[i].source, data->current_state) == 0 && data->transitions[i].code == code) { next = data->transitions[i].dest; break; }
    }
    if (!next) {
        for (size_t i = 0; i < data->trans_count; i++) {
            if (strcmp(data->transitions[i].source, data->current_state) == 0 && data->transitions[i].code == TAMAF_FSM_DEFAULT) { next = data->transitions[i].dest; break; }
        }
    }
    if (next) {
        strncpy(data->current_state, next, 63);
        self->is_started = false;
        return false;
    }
    return true;
}

static void fsm_destroy(tamaf_behavior_t* self) {
    fsm_data_t* data = (fsm_data_t*)self->derived;
    for (size_t i = 0; i < data->state_count; i++) data->states[i].b->destroy(data->states[i].b);
    free(data->states);
    free(data->transitions);
    free(data);
    tamaf_behavior_base_destroy(self);
}

tamaf_behavior_t* tamaf_fsm_behavior_create(struct tamaf_agent_t* agent) {
    tamaf_behavior_t* b = (tamaf_behavior_t*)malloc(sizeof(tamaf_behavior_t));
    tamaf_behavior_init(b, agent);
    fsm_data_t* data = (fsm_data_t*)calloc(1, sizeof(fsm_data_t));
    b->derived = data;
    b->on_start = fsm_on_start;
    b->action = fsm_action;
    b->done = fsm_done;
    b->destroy = fsm_destroy;
    return b;
}

void tamaf_fsm_add_state(tamaf_behavior_t* fsm, tamaf_behavior_t* b, const char* name) {
    fsm_data_t* data = (fsm_data_t*)fsm->derived;
    data->states = (fsm_state_t*)realloc(data->states, (data->state_count + 1) * sizeof(fsm_state_t));
    strncpy(data->states[data->state_count].name, name, 63);
    data->states[data->state_count].b = b;
    data->states[data->state_count].is_final = false;
    data->state_count++;
}

void tamaf_fsm_add_initial_state(tamaf_behavior_t* fsm, tamaf_behavior_t* b, const char* name) {
    fsm_data_t* data = (fsm_data_t*)fsm->derived;
    tamaf_fsm_add_state(fsm, b, name);
    strncpy(data->initial_state, name, 63);
    strncpy(data->current_state, name, 63);
}

void tamaf_fsm_add_final_state(tamaf_behavior_t* fsm, tamaf_behavior_t* b, const char* name) {
    fsm_data_t* data = (fsm_data_t*)fsm->derived;
    tamaf_fsm_add_state(fsm, b, name);
    data->states[data->state_count - 1].is_final = true;
}

void tamaf_fsm_add_transition(tamaf_behavior_t* fsm, const char* source, const char* dest, int code) {
    fsm_data_t* data = (fsm_data_t*)fsm->derived;
    data->transitions = (fsm_transition_t*)realloc(data->transitions, (data->trans_count + 1) * sizeof(fsm_transition_t));
    strncpy(data->transitions[data->trans_count].source, source, 63);
    strncpy(data->transitions[data->trans_count].dest, dest, 63);
    data->transitions[data->trans_count].code = code;
    data->trans_count++;
}

void tamaf_fsm_add_default_transition(tamaf_behavior_t* fsm, const char* source, const char* dest) {
    tamaf_fsm_add_transition(fsm, source, dest, TAMAF_FSM_DEFAULT);
}
