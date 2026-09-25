#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "TAMAF/tamaf_aclmessage.h"

void test_aclmessage() {
    printf("Testing ACLMessage...\n");
    tamaf_aclmessage_t* msg = tamaf_aclmessage_create(TAMAF_INFORM);
    msg->sender = tamaf_agentid_from_string("SenderAgent@127.0.0.1:4000");
    tamaf_aclmessage_add_receiver(msg, tamaf_agentid_from_string("ReceiverAgent@127.0.0.1:5000"));
    
    msg->content = cJSON_CreateObject();
    cJSON_AddStringToObject(msg->content, "text", "Hello World");
    cJSON_AddNumberToObject(msg->content, "value", 42);
    
    msg->ontology = strdup("TestOntology");
    msg->protocol = strdup("fipa-request");
    
    // To JSON
    cJSON* json = tamaf_aclmessage_to_json(msg);
    char* json_str = cJSON_PrintUnformatted(json);
    printf("JSON: %s\n", json_str);
    
    // Verify JSON content
    assert(strstr(json_str, "\"performative\":\"inform\"") != NULL);
    assert(strstr(json_str, "\"sender\":\"SenderAgent@127.0.0.1:4000\"") != NULL);
    assert(strstr(json_str, "\"receiver\":[\"ReceiverAgent@127.0.0.1:5000\"]") != NULL);
    assert(strstr(json_str, "\"content\":{\"text\":\"Hello World\",\"value\":42}") != NULL);
    assert(strstr(json_str, "\"ontology\":\"TestOntology\"") != NULL);
    
    // From JSON
    tamaf_aclmessage_t* msg2 = tamaf_aclmessage_from_json(json);
    assert(msg2 != NULL);
    assert(msg2->performative == TAMAF_INFORM);
    assert(strcmp(msg2->sender->name, "SenderAgent") == 0);
    assert(msg2->receivers_count == 1);
    assert(strcmp(msg2->receivers[0]->name, "ReceiverAgent") == 0);
    assert(strcmp(cJSON_GetObjectItem(msg2->content, "text")->valuestring, "Hello World") == 0);
    
    // Reply
    tamaf_aclmessage_t* reply = tamaf_aclmessage_create_reply(msg, TAMAF_AGREE);
    assert(reply->performative == TAMAF_AGREE);
    assert(reply->receivers_count == 1);
    assert(strcmp(reply->receivers[0]->name, "SenderAgent") == 0);
    
    tamaf_aclmessage_destroy(msg);
    tamaf_aclmessage_destroy(msg2);
    tamaf_aclmessage_destroy(reply);
    cJSON_Delete(json);
    free(json_str);
    
    printf("ACLMessage tests passed!\n");
}

int main() {
    test_aclmessage();
    return 0;
}
