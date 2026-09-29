#include "TAMAF/tamaf_mts.h"
#include "TAMAF/tamaf_transport.h"
#include "TAMAF/tamaf_agent.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define closesocket close
#endif

#ifdef _WIN32
static bool recv_all(int sock, void* buf, size_t len) {
    char* b = (char*)buf;
    while (len > 0) {
        int r = recv(sock, b, (int)len, 0);
        if (r <= 0) return false;
        b += r; len -= r;
    }
    return true;
}
static bool send_all(int sock, const void* buf, size_t len) {
    const char* b = (const char*)buf;
    while (len > 0) {
        int r = send(sock, b, (int)len, 0);
        if (r <= 0) return false;
        b += r; len -= r;
    }
    return true;
}
#else
static bool recv_all(int sock, void* buf, size_t len) {
    char* b = (char*)buf;
    while (len > 0) {
        ssize_t r = recv(sock, b, len, 0);
        if (r <= 0) return false;
        b += r; len -= r;
    }
    return true;
}
static bool send_all(int sock, const void* buf, size_t len) {
    const char* b = (const char*)buf;
    while (len > 0) {
        ssize_t r = send(sock, b, len, 0);
        if (r <= 0) return false;
        b += r; len -= r;
    }
    return true;
}
#endif

static void* server_thread_func(void* arg) {
    tamaf_mts_t* mts = (tamaf_mts_t*)arg;
    struct sockaddr_in server_addr;
    mts->server_sock = (int)socket(AF_INET, SOCK_STREAM, 0);
    if (mts->server_sock < 0) return NULL;
    int opt = 1; setsockopt(mts->server_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(mts->current_port);
    if (bind(mts->server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) { closesocket(mts->server_sock); return NULL; }
    if (listen(mts->server_sock, 10) < 0) { closesocket(mts->server_sock); return NULL; }
    mts->server_online = true;
    while (mts->running) {
        struct sockaddr_in client_addr; socklen_t client_len = sizeof(client_addr);
        int client_sock = (int)accept(mts->server_sock, (struct sockaddr*)&client_addr, &client_len);
        if (client_sock < 0) continue;
        uint32_t len_net;
        if (recv_all(client_sock, &len_net, 4)) {
            uint32_t len = ntohl(len_net); char* buf = (char*)malloc(len + 1);
            if (recv_all(client_sock, buf, len)) {
                buf[len] = '\0'; cJSON* json = cJSON_Parse(buf);
                if (json) {
                    tamaf_transport_message_t* tm = tamaf_transport_message_from_json(json);
                    if (tm && tm->aclmessage) {
                        pthread_mutex_lock(&mts->queue_lock);
                        mts->message_queue = (tamaf_aclmessage_t**)realloc(mts->message_queue, (mts->queue_count + 1) * sizeof(tamaf_aclmessage_t*));
                        mts->message_queue[mts->queue_count++] = tm->aclmessage;
                        mts->has_new_message = true;
                        pthread_mutex_unlock(&mts->queue_lock);
                        tm->aclmessage = NULL;
                    }
                    if (tm) tamaf_transport_message_destroy(tm);
                    cJSON_Delete(json);
                }
                const char* resp = "200"; uint32_t r_len = htonl((uint32_t)strlen(resp));
                send_all(client_sock, &r_len, 4); send_all(client_sock, resp, strlen(resp));
            }
            free(buf);
        }
        closesocket(client_sock);
    }
    closesocket(mts->server_sock); mts->server_online = false; return NULL;
}

tamaf_mts_t* tamaf_mts_create(struct tamaf_agent_t* agent) {
#ifdef _WIN32
    static bool wsa_started = false; if (!wsa_started) { WSADATA wsaData; WSAStartup(MAKEWORD(2, 2), &wsaData); wsa_started = true; }
#endif
    tamaf_mts_t* mts = (tamaf_mts_t*)calloc(1, sizeof(tamaf_mts_t));
    if (!mts) return NULL;
    mts->agent = agent; mts->register_address = tamaf_address_create("127.0.0.1", 4001); mts->ema_address = tamaf_address_create("127.0.0.1", 4000);
    pthread_mutex_init(&mts->queue_lock, NULL); return mts;
}

void tamaf_mts_destroy(tamaf_mts_t* mts) {
    if (!mts) return; tamaf_mts_stop_server(mts);
    pthread_mutex_lock(&mts->queue_lock);
    for (size_t i = 0; i < mts->queue_count; i++) tamaf_aclmessage_destroy(mts->message_queue[i]);
    free(mts->message_queue); pthread_mutex_unlock(&mts->queue_lock);
    pthread_mutex_destroy(&mts->queue_lock);
    if (mts->register_address) tamaf_address_destroy(mts->register_address);
    if (mts->ema_address) tamaf_address_destroy(mts->ema_address);
    free(mts);
}

void tamaf_mts_start_server(tamaf_mts_t* mts, int port) {
    mts->current_port = port; 
    if (mts->agent && mts->agent->aid) {
        tamaf_agentid_set_port(mts->agent->aid, port);
    }
    mts->running = true; 
    pthread_create(&mts->server_thread, NULL, server_thread_func, mts);
}

void tamaf_mts_stop_server(tamaf_mts_t* mts) {
    if (mts->running) {
        mts->running = false;
        int sock = (int)socket(AF_INET, SOCK_STREAM, 0); struct sockaddr_in addr; memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET; addr.sin_addr.s_addr = inet_addr("127.0.0.1"); addr.sin_port = htons(mts->current_port);
        connect(sock, (struct sockaddr*)&addr, sizeof(addr)); closesocket(sock); pthread_join(mts->server_thread, NULL);
    }
}

bool tamaf_mts_send(tamaf_mts_t* mts, tamaf_aclmessage_t* msg) {
    if (!mts || !msg) return false;
    if (!msg->sender) msg->sender = tamaf_agentid_from_string(tamaf_agentid_get_full_id(mts->agent->aid));
    for (size_t i = 0; i < msg->receivers_count; i++) {
        tamaf_agentid_t* receiver = msg->receivers[i]; if (!receiver->address) continue;
        tamaf_transport_message_t* tm = tamaf_transport_message_create(msg, mts->agent->aid->address, receiver->address);
        cJSON* json = tamaf_transport_message_to_json(tm); char* buf = cJSON_PrintUnformatted(json);
        int sock = (int)socket(AF_INET, SOCK_STREAM, 0); struct sockaddr_in addr; memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET; addr.sin_addr.s_addr = inet_addr(receiver->address->ip); addr.sin_port = htons(receiver->address->port);
        bool ok = false;
        if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) >= 0) {
            uint32_t len = (uint32_t)strlen(buf); uint32_t len_net = htonl(len);
            if (send_all(sock, &len_net, 4) && send_all(sock, buf, len)) ok = true;
        }
        if (ok) {
            uint32_t resp_len_net;
            if (recv_all(sock, &resp_len_net, 4)) {
                uint32_t resp_len = ntohl(resp_len_net); char* resp_buf = (char*)malloc(resp_len + 1);
                if (recv_all(sock, resp_buf, resp_len)) resp_buf[resp_len] = '\0';
                free(resp_buf);
            } else ok = false;
        }
        closesocket(sock); free(buf); cJSON_Delete(json); 
        tm->aclmessage = NULL; tamaf_transport_message_destroy(tm);
        if (!ok) return false;
    }
    return true;
}

tamaf_aclmessage_t* tamaf_mts_receive(tamaf_mts_t* mts, tamaf_aclmessagetemplate_t* template) {
    pthread_mutex_lock(&mts->queue_lock);
    if (mts->queue_count == 0) { pthread_mutex_unlock(&mts->queue_lock); return NULL; }
    int index = -1;
    if (template == NULL) index = 0;
    else {
        for (size_t i = 0; i < mts->queue_count; i++) {
            if (tamaf_aclmessagetemplate_match(template, mts->message_queue[i])) { index = (int)i; break; }
        }
    }
    if (index == -1) { pthread_mutex_unlock(&mts->queue_lock); return NULL; }
    tamaf_aclmessage_t* msg = mts->message_queue[index];
    for (size_t i = (size_t)index; i < mts->queue_count - 1; i++) mts->message_queue[i] = mts->message_queue[i+1];
    mts->queue_count--; pthread_mutex_unlock(&mts->queue_lock); return msg;
}

void tamaf_mts_put_back(tamaf_mts_t* mts, tamaf_aclmessage_t* msg) {
    pthread_mutex_lock(&mts->queue_lock);
    mts->message_queue = (tamaf_aclmessage_t**)realloc(mts->message_queue, (mts->queue_count + 1) * sizeof(tamaf_aclmessage_t*));
    for (int i = (int)mts->queue_count; i > 0; i--) mts->message_queue[i] = mts->message_queue[i-1];
    mts->message_queue[0] = msg; mts->queue_count++; pthread_mutex_unlock(&mts->queue_lock);
}
