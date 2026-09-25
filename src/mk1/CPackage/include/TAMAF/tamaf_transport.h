#ifndef TAMAF_TRANSPORT_H
#define TAMAF_TRANSPORT_H

#include "TAMAF/tamaf_aclmessage.h"
#include "TAMAF/tamaf_address.h"

typedef enum {
    TAMAF_TRANSPORT_HTTP,
    TAMAF_TRANSPORT_UDP,
    TAMAF_TRANSPORT_TCP,
    TAMAF_TRANSPORT_UNKNOWN
} tamaf_transport_type_t;

typedef struct {
    tamaf_transport_type_t type;
    char* address; // transport-specific-address
} tamaf_transport_description_t;

typedef struct {
    tamaf_transport_description_t* sender;
    tamaf_transport_description_t* receiver;
} tamaf_envelope_t;

typedef struct {
    tamaf_aclmessage_t* aclmessage;
    tamaf_envelope_t* envelope;
} tamaf_transport_message_t;

/**
 * TransportDescription functions
 */
tamaf_transport_description_t* tamaf_transport_description_create(const char* address, tamaf_transport_type_t type);
void tamaf_transport_description_destroy(tamaf_transport_description_t* desc);
cJSON* tamaf_transport_description_to_json(const tamaf_transport_description_t* desc);
tamaf_transport_description_t* tamaf_transport_description_from_json(const cJSON* json);

/**
 * Envelope functions
 */
tamaf_envelope_t* tamaf_envelope_create(tamaf_transport_description_t* sender, tamaf_transport_description_t* receiver);
void tamaf_envelope_destroy(tamaf_envelope_t* env);
cJSON* tamaf_envelope_to_json(const tamaf_envelope_t* env);
tamaf_envelope_t* tamaf_envelope_from_json(const cJSON* json);

/**
 * TransportMessage functions
 */
tamaf_transport_message_t* tamaf_transport_message_create(tamaf_aclmessage_t* acl, tamaf_address_t* sender_addr, tamaf_address_t* receiver_addr);
void tamaf_transport_message_destroy(tamaf_transport_message_t* tm);
cJSON* tamaf_transport_message_to_json(const tamaf_transport_message_t* tm);
tamaf_transport_message_t* tamaf_transport_message_from_json(const cJSON* json);

#endif // TAMAF_TRANSPORT_H
