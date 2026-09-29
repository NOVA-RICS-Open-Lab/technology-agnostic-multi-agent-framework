#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#define sleep_ms(x) Sleep(x)
#else
#include <unistd.h>
#define sleep_ms(x) usleep((x)*1000)
#endif

#include "TAMAF/tamaf_agent.h"
#include "TAMAF/tamaf_behavior.h"
#include "TAMAF/tamaf_common_behaviors.h"
#include "TAMAF/tamaf_emainteraction.h"

// --- ThreePingsBehavior Implementation ---
typedef struct {
    int ping_count;
} ping_data_t;

void ping_action(tamaf_behavior_t* self) {
    ping_data_t* data = (ping_data_t*)self->user_data;
    printf("Ping: %d\n", data->ping_count);
    data->ping_count++;
}

bool ping_done(tamaf_behavior_t* self) {
    ping_data_t* data = (ping_data_t*)self->user_data;
    return data->ping_count >= 3;
}

void ping_destroy(tamaf_behavior_t* self) {
    if (self->user_data) free(self->user_data);
    tamaf_behavior_base_destroy(self);
}

tamaf_behavior_t* create_three_pings_behavior(tamaf_agent_t* agent) {
    tamaf_behavior_t* b = (tamaf_behavior_t*)malloc(sizeof(tamaf_behavior_t));
    tamaf_behavior_init(b, agent);
    
    ping_data_t* data = (ping_data_t*)calloc(1, sizeof(ping_data_t));
    b->user_data = data;
    
    b->action = ping_action;
    b->done = ping_done;
    b->destroy = ping_destroy;
    return b;
}

// --- PimbaBehavior Implementation ---
void pimba_action(tamaf_behavior_t* self) {
    printf("PIMBA\n");
}

// --- PingerAgent Setup ---
void pinger_setup(tamaf_agent_t* self, tamaf_emainteraction_t* ema) {
    printf("PingerAgent setting up...\n");
    
    tamaf_behavior_t* sequential = tamaf_sequential_behavior_create(self);
    
    // Add sub-behaviors to match Java/Python: Ping -> Pimba -> Ping
    tamaf_sequential_behavior_add_sub(sequential, create_three_pings_behavior(self));
    tamaf_sequential_behavior_add_sub(sequential, tamaf_oneshot_behavior_create(self, pimba_action));
    tamaf_sequential_behavior_add_sub(sequential, create_three_pings_behavior(self));
    
    tamaf_agent_add_behavior(self, sequential);
}

int main() {
    printf("Starting C PingerAgent UX Test...\n");
    
    // Initialize agent with name
    tamaf_agent_t* agent = tamaf_agent_create("Test2Agent@127.0.0.1:4000");
    if (!agent) {
        fprintf(stderr, "Failed to create agent\n");
        return 1;
    }

    // Call setup manually as tamaf_agent_create doesn't know about our specific pinger_setup yet
    // In a more advanced UX, we might pass a function pointer to tamaf_agent_create.
    pinger_setup(agent, NULL);

    // Start agent services
    tamaf_agent_start(agent);

    // Run for a bit to allow behaviors to execute
    // In the real system, this would be managed by the AMS kernel thread.
    int timeout = 10; // 10 seconds max
    while (timeout-- > 0 && tamaf_agent_is_alive(agent)) {
        sleep_ms(1000);
    }

    printf("Shutting down agent...\n");
    tamaf_agent_stop(agent);
    tamaf_agent_destroy(agent);

    printf("UX Test Finished.\n");
    return 0;
}
