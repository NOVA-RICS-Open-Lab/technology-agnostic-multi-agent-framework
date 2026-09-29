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
#include "TAMAF/tamaf_agent.h"
#include "TAMAF/tamaf_ams.h"
#include "TAMAF/tamaf_mts.h"
#include "TAMAF/tamaf_fipa_protocols.h"
#include "TAMAF/tamaf_agentdescription.h"

int inform_received = 0;
void handle_inform(tamaf_behavior_t* self, tamaf_aclmessage_t* msg) {
    inform_received++;
    printf("Initiator received INFORM!\n");
}

tamaf_aclmessage_t* prepare_response(tamaf_behavior_t* self, tamaf_aclmessage_t* request) {
    return tamaf_aclmessage_create_reply(request, TAMAF_AGREE);
}

tamaf_aclmessage_t* prepare_result(tamaf_behavior_t* self, tamaf_aclmessage_t* request, tamaf_aclmessage_t* response) {
    return tamaf_aclmessage_create_reply(request, TAMAF_INFORM);
}

void test_fipa_request() {
    printf("Testing FIPA Request Protocol...\n");
    
    // Setup Agent A (Initiator)
    tamaf_agent_t* agentA = (tamaf_agent_t*)calloc(1, sizeof(tamaf_agent_t));
    agentA->aid = tamaf_agentid_from_string("Initiator@127.0.0.1:7000");
    agentA->agent_description = tamaf_agentdescription_create(tamaf_agentid_from_string("Initiator@127.0.0.1:7000"));
    agentA->mts = tamaf_mts_create(agentA);
    agentA->ams = tamaf_ams_create(agentA);
    tamaf_ams_set_standalone((tamaf_ams_t*)agentA->ams, true);
    
    // Setup Agent B (Responder)
    tamaf_agent_t* agentB = (tamaf_agent_t*)calloc(1, sizeof(tamaf_agent_t));
    agentB->aid = tamaf_agentid_from_string("Responder@127.0.0.1:7001");
    agentB->agent_description = tamaf_agentdescription_create(tamaf_agentid_from_string("Responder@127.0.0.1:7001"));
    agentB->mts = tamaf_mts_create(agentB);
    agentB->ams = tamaf_ams_create(agentB);
    tamaf_ams_set_standalone((tamaf_ams_t*)agentB->ams, true);
    
    tamaf_agent_start(agentA);
    tamaf_agent_start(agentB);
    
    sleep_ms(1000); 
    
    // Add Responder behavior to Agent B
    tamaf_aclmessagetemplate_t* tmpl = tamaf_aclmessagetemplate_create();
    tamaf_aclmessagetemplate_add_performative(tmpl, TAMAF_REQUEST);
    tamaf_behavior_t* resp = tamaf_fipa_request_responder_create(agentB, tmpl);
    tamaf_fipa_request_responder_t* resp_data = (tamaf_fipa_request_responder_t*)resp->user_data;
    resp_data->prepare_response = prepare_response;
    resp_data->prepare_result_notification = prepare_result;
    tamaf_ams_add_behavior((tamaf_ams_t*)agentB->ams, resp);
    
    // Create Request message
    tamaf_aclmessage_t* req_msg = tamaf_aclmessage_create(TAMAF_REQUEST);
    tamaf_aclmessage_add_receiver(req_msg, tamaf_agentid_from_string("Responder@127.0.0.1:7001"));
    
    // Add Initiator behavior to Agent A
    tamaf_behavior_t* init = tamaf_fipa_request_initiator_create(agentA, req_msg);
    tamaf_fipa_request_initiator_t* init_data = (tamaf_fipa_request_initiator_t*)init->user_data;
    init_data->handle_inform = handle_inform;
    tamaf_ams_add_behavior((tamaf_ams_t*)agentA->ams, init);
    
    printf("Test: Waiting for interaction...\n");
    for (int i = 0; i < 10; i++) {
        if (inform_received >= 1) break;
        sleep_ms(1000);
    }
    
    printf("Test: Checking results, inform_received = %d\n", inform_received);
    assert(inform_received == 1);
    
    tamaf_agent_stop(agentA);
    tamaf_agent_stop(agentB);
    
    sleep_ms(1000);
    printf("FIPA Request tests passed!\n");
}

int main() {
    test_fipa_request();
    return 0;
}
