#ifndef TAMAF_MTS_H
#define TAMAF_MTS_H

#include "TAMAF/tamaf_aclmessage.h"
#include "TAMAF/tamaf_address.h"
#include <pthread.h>
#include <stdbool.h>

#include "TAMAF/tamaf_aclmessagetemplate.h"

struct tamaf_agent_t;

typedef struct {
    struct tamaf_agent_t* agent;
    
    tamaf_address_t* ema_address;
    tamaf_address_t* register_address;
    
    int current_port;
    bool server_online;
    bool running;
    bool has_new_message;
    
    // Message Queue
    tamaf_aclmessage_t** message_queue;
    size_t queue_count;
    pthread_mutex_t queue_lock;
    
    // Server state
    pthread_t server_thread;
    int server_sock;
} tamaf_mts_t;

/**
 * Initialize MTS.
 */
tamaf_mts_t* tamaf_mts_create(struct tamaf_agent_t* agent);

/**
 * Destroy MTS and stop server.
 */
void tamaf_mts_destroy(tamaf_mts_t* mts);

/**
 * Start the TCP server on a given port.
 */
void tamaf_mts_start_server(tamaf_mts_t* mts, int port);

/**
 * Stop the TCP server.
 */
void tamaf_mts_stop_server(tamaf_mts_t* mts);

/**
 * Send an ACLMessage to all its receivers.
 */
bool tamaf_mts_send(tamaf_mts_t* mts, tamaf_aclmessage_t* msg);

/**
 * Receive a message matching the template.
 */
tamaf_aclmessage_t* tamaf_mts_receive(tamaf_mts_t* mts, tamaf_aclmessagetemplate_t* template);

/**
 * Put a message back at the front of the queue.
 */
void tamaf_mts_put_back(tamaf_mts_t* mts, tamaf_aclmessage_t* msg);

#endif // TAMAF_MTS_H
