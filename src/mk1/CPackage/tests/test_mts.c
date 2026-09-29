#include <stdio.h>
#include <assert.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include "TAMAF/tamaf_mts.h"
#include "TAMAF/tamaf_agent.h"

void test_mts_local() {
    printf("Testing MTS Local Send/Receive...\n");
    
    // Agent 1 (Receiver)
    tamaf_agent_t agent1;
    agent1.aid = tamaf_agentid_from_string("Agent1@127.0.0.1:5000");
    tamaf_mts_t* mts1 = tamaf_mts_create(&agent1);
    tamaf_mts_start_server(mts1, 5000);
    
    // Agent 2 (Sender)
    tamaf_agent_t agent2;
    agent2.aid = tamaf_agentid_from_string("Agent2@127.0.0.1:5001");
    tamaf_mts_t* mts2 = tamaf_mts_create(&agent2);
    
    // Create message from Agent 2 to Agent 1
    tamaf_aclmessage_t* msg = tamaf_aclmessage_create(TAMAF_INFORM);
    tamaf_aclmessage_add_receiver(msg, tamaf_agentid_from_string("Agent1@127.0.0.1:5000"));
    msg->content = cJSON_CreateString("Hello from Agent 2");
    
    // Send
    bool sent = tamaf_mts_send(mts2, msg);
    assert(sent == true);
    
    // Wait a bit for server to process
#ifdef _WIN32
    Sleep(500);
#else
    usleep(500000);
#endif
    
    // Receive on MTS 1
    tamaf_aclmessage_t* received = tamaf_mts_receive(mts1, NULL);
    assert(received != NULL);
    assert(received->performative == TAMAF_INFORM);
    assert(received->content != NULL);
    assert(strcmp(received->content->valuestring, "Hello from Agent 2") == 0);
    
    tamaf_aclmessage_destroy(received);
    tamaf_aclmessage_destroy(msg);
    tamaf_mts_destroy(mts1);
    tamaf_mts_destroy(mts2);
    tamaf_agentid_destroy(agent1.aid);
    tamaf_agentid_destroy(agent2.aid);
    
    printf("MTS tests passed!\n");
}

int main() {
    test_mts_local();
    return 0;
}
