#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "TAMAF/tamaf_transport.h"

void test_transport() {
    printf("Testing TransportMessage...\n");
    tamaf_aclmessage_t* msg = tamaf_aclmessage_create(TAMAF_INFORM);
    msg->sender = tamaf_agentid_from_string("SenderAgent@127.0.0.1:4000");
    
    tamaf_address_t* s_addr = tamaf_address_create("127.0.0.1", 4000);
    tamaf_address_t* r_addr = tamaf_address_create("127.0.0.1", 5000);
    
    tamaf_transport_message_t* tm = tamaf_transport_message_create(msg, s_addr, r_addr);
    assert(tm != NULL);
    assert(tm->envelope != NULL);
    assert(strcmp(tm->envelope->sender->address, "http://127.0.0.1:4000") == 0);
    
    cJSON* json = tamaf_transport_message_to_json(tm);
    char* json_str = cJSON_PrintUnformatted(json);
    printf("Transport JSON: %s\n", json_str);
    
    assert(strstr(json_str, "\"aclmessage\":{") != NULL);
    assert(strstr(json_str, "\"envelope\":{") != NULL);
    assert(strstr(json_str, "\"sender-transport-description\":{") != NULL);
    
    tamaf_transport_message_t* tm2 = tamaf_transport_message_from_json(json);
    assert(tm2 != NULL);
    assert(tm2->aclmessage != NULL);
    assert(tm2->aclmessage->performative == TAMAF_INFORM);
    assert(tm2->envelope != NULL);
    assert(strcmp(tm2->envelope->receiver->address, "http://127.0.0.1:5000") == 0);
    
    tamaf_transport_message_destroy(tm);
    tamaf_transport_message_destroy(tm2);
    tamaf_address_destroy(s_addr);
    tamaf_address_destroy(r_addr);
    cJSON_Delete(json);
    free(json_str);
    
    printf("Transport tests passed!\n");
}

int main() {
    test_transport();
    return 0;
}
