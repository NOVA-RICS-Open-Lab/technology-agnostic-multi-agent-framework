#include <stdio.h>
#include <assert.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#define sleep_ms(x) Sleep(x)
#else
#include <unistd.h>
#define sleep_ms(x) usleep((x)*1000)
#endif
#include "TAMAF/tamaf_ams.h"
#include "TAMAF/tamaf_mts.h"
#include "TAMAF/tamaf_agent.h"
#include "TAMAF/tamaf_common_behaviors.h"
#include "TAMAF/tamaf_agentdescription.h"

// Redefine setup/takedown for test if needed, but they are defined in tamaf_ams.c
// To avoid "multiple definition" error, I should make them weak in tamaf_ams.c 
// or just not define them here. 
// Since they are empty in tamaf_ams.c, it's fine for the test.

int behaviors_run_count = 0;
void test_action(tamaf_behavior_t* self) {
    behaviors_run_count++;
    printf("Behavior running in AMS kernel!\n");
}

void test_ams() {
    printf("Testing AMS Kernel...\n");
    
    tamaf_agent_t agent;
    memset(&agent, 0, sizeof(agent));
    agent.aid = tamaf_agentid_from_string("EMA@127.0.0.1:4000"); // Name it EMA to skip registration logic in test
    agent.agent_description = tamaf_agentdescription_create(tamaf_agentid_from_string("EMA@127.0.0.1:4000"));
    
    tamaf_mts_t* mts = tamaf_mts_create(&agent);
    agent.mts = mts;
    
    tamaf_ams_t* ams = tamaf_ams_create(&agent);
    agent.ams = ams;
    
    // tamaf_mts_start_server(mts, 4000); // AMS kernel will start it
    tamaf_ams_start(ams);
    
    // Wait for INITIATED -> ACTIVE transition (Initial behavior runs)
    sleep_ms(1000);
    
    // Add a behavior
    behaviors_run_count = 0;
    tamaf_ams_add_behavior(ams, tamaf_oneshot_behavior_create(&agent, test_action));
    
    // Wait for kernel to pick it up and run it
    sleep_ms(1000);
    
    assert(behaviors_run_count == 1);
    
    // Shutdown
    tamaf_ams_shutdown(ams);
    sleep_ms(1000); // Wait for closing behavior
    
    tamaf_ams_destroy(ams);
    tamaf_mts_destroy(mts);
    // tamaf_agentid_destroy(agent.aid); // Destroyed by ams or agent_description
    
    printf("AMS tests passed!\n");
}

int main() {
    test_ams();
    return 0;
}
