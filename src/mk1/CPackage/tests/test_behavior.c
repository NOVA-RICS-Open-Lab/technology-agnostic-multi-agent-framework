#include <stdio.h>
#include <assert.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include "TAMAF/tamaf_behavior.h"
#include "TAMAF/tamaf_common_behaviors.h"
#include "TAMAF/tamaf_agent.h"
#include "TAMAF/tamaf_ams.h"

// Mock implementation of AMS and Agent
int put_back_called = 0;
void tamaf_ams_put_back(tamaf_ams_t* ams, tamaf_aclmessage_t* msg) {
    put_back_called++;
    // In real system, msg would be added to a queue. 
    // Here we just destroy it to avoid leaks in test.
    tamaf_aclmessage_destroy(msg);
}

int receive_calls = 0;
tamaf_aclmessage_t* tamaf_agent_receive(tamaf_agent_t* agent, void* template) {
    receive_calls++;
    if (receive_calls == 1) return NULL; // Fail first time
    return tamaf_aclmessage_create(TAMAF_INFORM); // Succeed second time
}

// Test Action
int action_called = 0;
void my_action(tamaf_behavior_t* self) {
    action_called++;
    tamaf_aclmessage_t* msg = tamaf_behavior_receive(self, NULL);
    printf("Received message in action!\n");
}

void test_oneshot_exception() {
    printf("Testing OneShot Behavior with simulated exception...\n");
    tamaf_agent_t agent;
    agent.ams = (void*)1; // Mock pointer
    
    tamaf_behavior_t* b = tamaf_oneshot_behavior_create(&agent, my_action);
    
    // First execution: should fail receive and block
    receive_calls = 0;
    action_called = 0;
    put_back_called = 0;
    
    b->execute(b);
    
    assert(action_called == 1);
    assert(b->is_blocked == true);
    assert(put_back_called == 0); // No message to put back if receive failed
    
    // Second execution: should succeed
    receive_calls = 1; // Start from 1 so next is success
    b->execute(b);
    
    assert(action_called == 2);
    assert(b->is_blocked == false);
    
    b->destroy(b);
    printf("OneShot exception tests passed!\n");
}

int tick_count = 0;
void my_tick(tamaf_behavior_t* self) {
    tick_count++;
}

void test_ticker() {
    printf("Testing Ticker Behavior...\n");
    tamaf_behavior_t* b = tamaf_ticker_behavior_create(NULL, 100, my_tick);
    
    tick_count = 0;
    b->execute(b); // Should not tick yet (last_tick was just set)
    assert(tick_count == 0);
    
    // Sleep 150ms
#ifdef _WIN32
    Sleep(150);
#else
    usleep(150000);
#endif
    
    b->execute(b);
    assert(tick_count == 1);
    
    b->destroy(b);
    printf("Ticker tests passed!\n");
}

void test_sequential() {
    printf("Testing Sequential Behavior...\n");
    tamaf_behavior_t* seq = tamaf_sequential_behavior_create(NULL);
    
    action_called = 0;
    tamaf_sequential_behavior_add_sub(seq, tamaf_oneshot_behavior_create(NULL, my_action));
    tamaf_sequential_behavior_add_sub(seq, tamaf_oneshot_behavior_create(NULL, my_action));
    
    receive_calls = 1; // Always succeed
    
    seq->execute(seq);
    assert(action_called == 1);
    assert(seq->done(seq) == false); // First sub done, move to next
    
    seq->execute(seq);
    assert(action_called == 2);
    assert(seq->done(seq) == true); // Second sub done, all done
    
    seq->destroy(seq);
    printf("Sequential tests passed!\n");
}

int state_a_called = 0;
void action_a(tamaf_behavior_t* self) { state_a_called++; }
int on_end_a(tamaf_behavior_t* self) { return 1; }

int state_b_called = 0;
void action_b(tamaf_behavior_t* self) { state_b_called++; }

void test_fsm() {
    printf("Testing FSM Behavior...\n");
    tamaf_behavior_t* fsm = tamaf_fsm_behavior_create(NULL);
    
    tamaf_behavior_t* sa = tamaf_oneshot_behavior_create(NULL, action_a);
    sa->on_end = on_end_a;
    tamaf_fsm_add_initial_state(fsm, sa, "A");
    
    tamaf_behavior_t* sb = tamaf_oneshot_behavior_create(NULL, action_b);
    tamaf_fsm_add_final_state(fsm, sb, "B");
    
    tamaf_fsm_add_transition(fsm, "A", "B", 1);
    
    state_a_called = 0;
    state_b_called = 0;
    
    fsm->execute(fsm); // Exec A
    assert(state_a_called == 1);
    assert(fsm->done(fsm) == false); // A done -> transition to B
    
    fsm->execute(fsm); // Exec B
    assert(state_b_called == 1);
    assert(fsm->done(fsm) == true); // B done and is final
    
    fsm->destroy(fsm);
    printf("FSM tests passed!\n");
}

int main() {
    test_oneshot_exception();
    test_ticker();
    test_sequential();
    test_fsm();
    return 0;
}
