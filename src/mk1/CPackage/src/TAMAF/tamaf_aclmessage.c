#include "TAMAF/tamaf_aclmessage.h"
#include <string.h>

#ifdef _WIN32
#define strdup _strdup
#endif

// Helper to clone AgentID
static tamaf_agentid_t* aid_clone(const tamaf_agentid_t* aid) {
    if (!aid) return NULL;
    tamaf_address_t* addr = NULL;
    if (aid->address) {
        addr = tamaf_address_create(aid->address->ip, aid->address->port);
    }
    return tamaf_agentid_create(aid->name, addr);
}

tamaf_aclmessage_t* tamaf_aclmessage_create(tamaf_performative_t performative) {
    tamaf_aclmessage_t* msg = (tamaf_aclmessage_t*)calloc(1, sizeof(tamaf_aclmessage_t));
    if (!msg) return NULL;
    msg->performative = performative;
    return msg;
}

void tamaf_aclmessage_destroy(tamaf_aclmessage_t* msg) {
    if (!msg) return;
    
    if (msg->sender) tamaf_agentid_destroy(msg->sender);
    
    if (msg->receivers) {
        for (size_t i = 0; i < msg->receivers_count; i++) {
            tamaf_agentid_destroy(msg->receivers[i]);
        }
        free(msg->receivers);
    }
    
    if (msg->reply_to) {
        for (size_t i = 0; i < msg->reply_to_count; i++) {
            tamaf_agentid_destroy(msg->reply_to[i]);
        }
        free(msg->reply_to);
    }
    
    if (msg->content) cJSON_Delete(msg->content);
    if (msg->language) free(msg->language);
    if (msg->ontology) free(msg->ontology);
    if (msg->protocol) free(msg->protocol);
    if (msg->conversation_id) free(msg->conversation_id);
    if (msg->reply_by) free(msg->reply_by);
    if (msg->reply_with) free(msg->reply_with);
    if (msg->in_reply_to) free(msg->in_reply_to);
    if (msg->encoding) free(msg->encoding);
    
    free(msg);
}

void tamaf_aclmessage_add_receiver(tamaf_aclmessage_t* msg, tamaf_agentid_t* receiver) {
    if (!msg || !receiver) return;
    msg->receivers = (tamaf_agentid_t**)realloc(msg->receivers, (msg->receivers_count + 1) * sizeof(tamaf_agentid_t*));
    msg->receivers[msg->receivers_count++] = receiver;
}

void tamaf_aclmessage_add_reply_to(tamaf_aclmessage_t* msg, tamaf_agentid_t* reply_to) {
    if (!msg || !reply_to) return;
    msg->reply_to = (tamaf_agentid_t**)realloc(msg->reply_to, (msg->reply_to_count + 1) * sizeof(tamaf_agentid_t*));
    msg->reply_to[msg->reply_to_count++] = reply_to;
}

tamaf_aclmessage_t* tamaf_aclmessage_create_reply(tamaf_aclmessage_t* msg, tamaf_performative_t performative) {
    if (!msg) return NULL;
    tamaf_aclmessage_t* reply = tamaf_aclmessage_create(performative);
    if (!reply) return NULL;
    
    if (msg->reply_to_count > 0) {
        for (size_t i = 0; i < msg->reply_to_count; i++) {
            tamaf_aclmessage_add_receiver(reply, aid_clone(msg->reply_to[i]));
        }
    } else if (msg->sender) {
        tamaf_aclmessage_add_receiver(reply, aid_clone(msg->sender));
    }
    
    if (msg->language) reply->language = strdup(msg->language);
    if (msg->ontology) reply->ontology = strdup(msg->ontology);
    if (msg->protocol) reply->protocol = strdup(msg->protocol);
    if (msg->conversation_id) reply->conversation_id = strdup(msg->conversation_id);
    
    if (msg->reply_with) {
        reply->in_reply_to = strdup(msg->reply_with);
    }
    
    return reply;
}

cJSON* tamaf_aclmessage_to_json(const tamaf_aclmessage_t* msg) {
    if (!msg) return NULL;
    cJSON* root = cJSON_CreateObject();
    
    cJSON_AddStringToObject(root, "performative", tamaf_performative_to_string(msg->performative));
    
    if (msg->sender) {
        char* sender_id = tamaf_agentid_get_full_id(msg->sender);
        cJSON_AddStringToObject(root, "sender", sender_id);
        free(sender_id);
    }
    
    if (msg->receivers_count > 0) {
        cJSON* receivers = cJSON_CreateArray();
        for (size_t i = 0; i < msg->receivers_count; i++) {
            char* rid = tamaf_agentid_get_full_id(msg->receivers[i]);
            cJSON_AddItemToArray(receivers, cJSON_CreateString(rid));
            free(rid);
        }
        cJSON_AddItemToObject(root, "receiver", receivers);
    }
    
    if (msg->reply_to_count > 0) {
        cJSON* reply_tos = cJSON_CreateArray();
        for (size_t i = 0; i < msg->reply_to_count; i++) {
            char* rtid = tamaf_agentid_get_full_id(msg->reply_to[i]);
            cJSON_AddItemToArray(reply_tos, cJSON_CreateString(rtid));
            free(rtid);
        }
        cJSON_AddItemToObject(root, "reply-to", reply_tos);
    }
    
    if (msg->content) cJSON_AddItemToObject(root, "content", cJSON_Duplicate(msg->content, 1));
    if (msg->language) cJSON_AddStringToObject(root, "language", msg->language);
    if (msg->ontology) cJSON_AddStringToObject(root, "ontology", msg->ontology);
    if (msg->protocol) cJSON_AddStringToObject(root, "protocol", msg->protocol);
    if (msg->conversation_id) cJSON_AddStringToObject(root, "conversation-id", msg->conversation_id);
    if (msg->reply_by) cJSON_AddStringToObject(root, "reply-by", msg->reply_by);
    if (msg->reply_with) cJSON_AddStringToObject(root, "reply-with", msg->reply_with);
    if (msg->in_reply_to) cJSON_AddStringToObject(root, "in-reply-to", msg->in_reply_to);
    if (msg->encoding) cJSON_AddStringToObject(root, "encoding", msg->encoding);
    
    return root;
}

tamaf_aclmessage_t* tamaf_aclmessage_from_json(const cJSON* json) {
    if (!json) return NULL;
    
    cJSON* perf_item = cJSON_GetObjectItem(json, "performative");
    if (!perf_item || !cJSON_IsString(perf_item)) return NULL;
    
    tamaf_aclmessage_t* msg = tamaf_aclmessage_create(tamaf_performative_from_string(perf_item->valuestring));
    if (!msg) return NULL;
    
    cJSON* sender_item = cJSON_GetObjectItem(json, "sender");
    if (sender_item && cJSON_IsString(sender_item)) {
        msg->sender = tamaf_agentid_from_string(sender_item->valuestring);
    }
    
    cJSON* receiver_item = cJSON_GetObjectItem(json, "receiver");
    if (receiver_item && cJSON_IsArray(receiver_item)) {
        int size = cJSON_GetArraySize(receiver_item);
        for (int i = 0; i < size; i++) {
            cJSON* r = cJSON_GetArrayItem(receiver_item, i);
            if (cJSON_IsString(r)) {
                tamaf_aclmessage_add_receiver(msg, tamaf_agentid_from_string(r->valuestring));
            }
        }
    }
    
    cJSON* reply_to_item = cJSON_GetObjectItem(json, "reply-to");
    if (reply_to_item && cJSON_IsArray(reply_to_item)) {
        int size = cJSON_GetArraySize(reply_to_item);
        for (int i = 0; i < size; i++) {
            cJSON* r = cJSON_GetArrayItem(reply_to_item, i);
            if (cJSON_IsString(r)) {
                tamaf_aclmessage_add_reply_to(msg, tamaf_agentid_from_string(r->valuestring));
            }
        }
    }
    
    cJSON* content_item = cJSON_GetObjectItem(json, "content");
    if (content_item) msg->content = cJSON_Duplicate(content_item, 1);
    
    cJSON* item;
    if ((item = cJSON_GetObjectItem(json, "language")) && cJSON_IsString(item)) msg->language = strdup(item->valuestring);
    if ((item = cJSON_GetObjectItem(json, "ontology")) && cJSON_IsString(item)) msg->ontology = strdup(item->valuestring);
    if ((item = cJSON_GetObjectItem(json, "protocol")) && cJSON_IsString(item)) msg->protocol = strdup(item->valuestring);
    if ((item = cJSON_GetObjectItem(json, "conversation-id")) && cJSON_IsString(item)) msg->conversation_id = strdup(item->valuestring);
    if ((item = cJSON_GetObjectItem(json, "reply-by")) && cJSON_IsString(item)) msg->reply_by = strdup(item->valuestring);
    if ((item = cJSON_GetObjectItem(json, "reply-with")) && cJSON_IsString(item)) msg->reply_with = strdup(item->valuestring);
    if ((item = cJSON_GetObjectItem(json, "in-reply-to")) && cJSON_IsString(item)) msg->in_reply_to = strdup(item->valuestring);
    if ((item = cJSON_GetObjectItem(json, "encoding")) && cJSON_IsString(item)) msg->encoding = strdup(item->valuestring);
    
    return msg;
}
