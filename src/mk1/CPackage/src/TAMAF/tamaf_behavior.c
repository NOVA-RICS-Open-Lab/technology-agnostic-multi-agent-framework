#include "TAMAF/tamaf_behavior.h"
#include "TAMAF/tamaf_agent.h"
#include "TAMAF/tamaf_ams.h"

tamaf_behavior_t* tamaf_behavior_create(struct tamaf_agent_t* agent) {
    tamaf_behavior_t* b = (tamaf_behavior_t*)calloc(1, sizeof(tamaf_behavior_t));
    if (b) tamaf_behavior_init(b, agent);
    return b;
}

void tamaf_behavior_init(tamaf_behavior_t* behavior, struct tamaf_agent_t* agent) {
    if (!behavior) return;
    behavior->agent = agent;
    behavior->user_data = NULL;
    behavior->ema_interaction = NULL;
    behavior->is_blocked = false;
    behavior->is_started = false;
    behavior->messages_to_put_back = NULL;
    behavior->messages_to_put_back_count = 0;
    behavior->on_start = NULL;
    behavior->action = NULL;
    behavior->done = NULL;
    behavior->on_end = NULL;
    behavior->execute = tamaf_behavior_base_execute;
    behavior->destroy = tamaf_behavior_base_destroy;
}

void tamaf_behavior_base_destroy(tamaf_behavior_t* behavior) {
    if (behavior) {
        if (behavior->messages_to_put_back) {
            for (size_t i = 0; i < behavior->messages_to_put_back_count; i++) tamaf_aclmessage_destroy(behavior->messages_to_put_back[i]);
            free(behavior->messages_to_put_back);
        }
        free(behavior);
    }
}

void tamaf_behavior_block(tamaf_behavior_t* behavior) { if (behavior) behavior->is_blocked = true; }
void tamaf_behavior_unblock(tamaf_behavior_t* behavior) { if (behavior) behavior->is_blocked = false; }

void tamaf_behavior_base_execute(tamaf_behavior_t* behavior) {
    if (!behavior || !behavior->action) return;
    if (behavior->messages_to_put_back) {
        for (size_t i = 0; i < behavior->messages_to_put_back_count; i++) tamaf_ams_put_back((tamaf_ams_t*)behavior->agent->ams, behavior->messages_to_put_back[i]);
        free(behavior->messages_to_put_back); behavior->messages_to_put_back = NULL; behavior->messages_to_put_back_count = 0;
    }
    tamaf_behavior_unblock(behavior);
    if (setjmp(behavior->exception_env) == 0) behavior->action(behavior);
    else {
        tamaf_behavior_block(behavior);
        if (behavior->messages_to_put_back) {
            for (size_t i = 0; i < behavior->messages_to_put_back_count; i++) tamaf_ams_put_back((tamaf_ams_t*)behavior->agent->ams, behavior->messages_to_put_back[i]);
            free(behavior->messages_to_put_back); behavior->messages_to_put_back = NULL; behavior->messages_to_put_back_count = 0;
        }
    }
}

tamaf_aclmessage_t* tamaf_behavior_receive(tamaf_behavior_t* behavior, void* template) {
    if (!behavior || !behavior->agent) return NULL;
    tamaf_aclmessage_t* msg = tamaf_agent_receive(behavior->agent, template);
    if (!msg) longjmp(behavior->exception_env, 1);
    else {
        behavior->messages_to_put_back = (tamaf_aclmessage_t**)realloc(behavior->messages_to_put_back, (behavior->messages_to_put_back_count + 1) * sizeof(tamaf_aclmessage_t*));
        for (int i = (int)behavior->messages_to_put_back_count; i > 0; i--) behavior->messages_to_put_back[i] = behavior->messages_to_put_back[i-1];
        behavior->messages_to_put_back[0] = msg; behavior->messages_to_put_back_count++;
    }
    return msg;
}
