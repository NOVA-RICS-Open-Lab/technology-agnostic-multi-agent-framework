#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "TAMAF/tamaf_agentid.h"
#include "TAMAF/tamaf_performative.h"

void test_performative() {
    printf("Testing Performative...\n");
    assert(tamaf_performative_from_string("inform") == TAMAF_INFORM);
    assert(tamaf_performative_from_string("request") == TAMAF_REQUEST);
    assert(tamaf_performative_from_string("keep-alive") == TAMAF_KEEPALIVE);
    assert(tamaf_performative_from_string("unknown-perf") == TAMAF_PERFORMATIVE_UNKNOWN);
    
    assert(strcmp(tamaf_performative_to_string(TAMAF_INFORM), "inform") == 0);
    assert(strcmp(tamaf_performative_to_string(TAMAF_KEEPALIVE), "keep-alive") == 0);
    printf("Performative tests passed!\n");
}

void test_address() {
    printf("Testing Address...\n");
    tamaf_address_t* addr = tamaf_address_create("127.0.0.1", 4000);
    assert(addr != NULL);
    assert(strcmp(addr->ip, "127.0.0.1") == 0);
    assert(addr->port == 4000);

    char* str = tamaf_address_get_string(addr);
    assert(strcmp(str, "127.0.0.1:4000") == 0);
    free(str);

    char* link = tamaf_address_get_http_link(addr);
    assert(strcmp(link, "http://127.0.0.1:4000") == 0);
    free(link);

    tamaf_address_set_port(addr, 5000);
    assert(addr->port == 5000);

    tamaf_address_destroy(addr);
    
    // Test None port
    addr = tamaf_address_create("192.168.1.1", TAMAF_PORT_NONE);
    str = tamaf_address_get_string(addr);
    assert(strcmp(str, "192.168.1.1:None") == 0);
    free(str);
    tamaf_address_destroy(addr);
    
    printf("Address tests passed!\n");
}

void test_agentid() {
    printf("Testing AgentID...\n");
    tamaf_address_t* addr = tamaf_address_create("127.0.0.1", 4000);
    tamaf_agentid_t* aid = tamaf_agentid_create("Agent1", addr);
    
    char* full_id = tamaf_agentid_get_full_id(aid);
    assert(strcmp(full_id, "Agent1@127.0.0.1:4000") == 0);
    free(full_id);

    // Test FromString
    tamaf_agentid_t* aid2 = tamaf_agentid_from_string("Agent2@192.168.1.50:5001");
    assert(aid2 != NULL);
    assert(strcmp(aid2->name, "Agent2") == 0);
    assert(strcmp(aid2->address->ip, "192.168.1.50") == 0);
    assert(aid2->address->port == 5001);
    
    char* full_id2 = tamaf_agentid_get_full_id(aid2);
    assert(strcmp(full_id2, "Agent2@192.168.1.50:5001") == 0);
    free(full_id2);

    // Test Matches
    tamaf_agentid_t* aid3 = tamaf_agentid_from_string("Agent1@127.0.0.1:4000");
    assert(tamaf_agentid_matches(aid, aid3) == true);
    assert(tamaf_agentid_matches(aid, aid2) == false);

    tamaf_agentid_destroy(aid);
    tamaf_agentid_destroy(aid2);
    tamaf_agentid_destroy(aid3);
    
    printf("AgentID tests passed!\n");
}

int main() {
    test_performative();
    test_address();
    test_agentid();
    printf("All Phase 1 Step 1 & 2 tests passed!\n");
    return 0;
}
