#ifndef TAMAF_FIPA_PROTOCOLS_H
#define TAMAF_FIPA_PROTOCOLS_H

#include "TAMAF/tamaf_behavior.h"
#include "TAMAF/tamaf_aclmessage.h"

/**
 * FIPA Request Initiator
 */
typedef struct {
    tamaf_aclmessage_t* request_msg;
    void (*handle_agree)(tamaf_behavior_t* self, tamaf_aclmessage_t* msg);
    void (*handle_refuse)(tamaf_behavior_t* self, tamaf_aclmessage_t* msg);
    void (*handle_inform)(tamaf_behavior_t* self, tamaf_aclmessage_t* msg);
    void (*handle_failure)(tamaf_behavior_t* self, tamaf_aclmessage_t* msg);
    void (*handle_not_understood)(tamaf_behavior_t* self, tamaf_aclmessage_t* msg);
} tamaf_fipa_request_initiator_t;

tamaf_behavior_t* tamaf_fipa_request_initiator_create(struct tamaf_agent_t* agent, tamaf_aclmessage_t* request_msg);

/**
 * FIPA Request Responder
 */
typedef struct {
    struct tamaf_aclmessagetemplate_t* template;
    tamaf_aclmessage_t* (*prepare_response)(tamaf_behavior_t* self, tamaf_aclmessage_t* request);
    tamaf_aclmessage_t* (*prepare_result_notification)(tamaf_behavior_t* self, tamaf_aclmessage_t* request, tamaf_aclmessage_t* response);
} tamaf_fipa_request_responder_t;

tamaf_behavior_t* tamaf_fipa_request_responder_create(struct tamaf_agent_t* agent, void* template);

/**
 * FIPA Contract Net Initiator
 */
typedef struct {
    tamaf_aclmessage_t* cfp_msg;
    void (*handle_propose)(tamaf_behavior_t* self, tamaf_aclmessage_t* msg);
    void (*handle_refuse)(tamaf_behavior_t* self, tamaf_aclmessage_t* msg);
    void (*handle_inform)(tamaf_behavior_t* self, tamaf_aclmessage_t* msg);
    void (*handle_failure)(tamaf_behavior_t* self, tamaf_aclmessage_t* msg);
} tamaf_fipa_contractnet_initiator_t;

tamaf_behavior_t* tamaf_fipa_contractnet_initiator_create(struct tamaf_agent_t* agent, tamaf_aclmessage_t* cfp_msg);

/**
 * FIPA Request Responder For Dispatcher
 */
typedef struct {
    tamaf_aclmessage_t* request_msg;
    tamaf_aclmessage_t* (*handle_request)(tamaf_behavior_t* self, tamaf_aclmessage_t* request);
    tamaf_aclmessage_t* (*prepare_result_notification)(tamaf_behavior_t* self, tamaf_aclmessage_t* request, tamaf_aclmessage_t* response);
} tamaf_fipa_request_responder_for_dispatcher_t;

tamaf_behavior_t* tamaf_fipa_request_responder_for_dispatcher_create(struct tamaf_agent_t* agent, tamaf_aclmessage_t* request_msg);

/**
 * FIPA Request Dispatcher
 */
typedef struct {
    struct tamaf_aclmessagetemplate_t* template;
    tamaf_behavior_t* (*create_responder)(tamaf_behavior_t* self, tamaf_aclmessage_t* msg);
} tamaf_fipa_request_dispatcher_t;

tamaf_behavior_t* tamaf_fipa_request_dispatcher_create(struct tamaf_agent_t* agent, void* template);

/**
 * FIPA Contract Net Responder
 */
typedef struct {
    struct tamaf_aclmessagetemplate_t* cfp_template;
    void (*handle_cfp)(tamaf_behavior_t* self, tamaf_aclmessage_t* cfp);
    tamaf_aclmessage_t* (*prepare_response)(tamaf_behavior_t* self, tamaf_aclmessage_t* cfp);
    void (*handle_accept_proposal)(tamaf_behavior_t* self, tamaf_aclmessage_t* cfp, tamaf_aclmessage_t* propose, tamaf_aclmessage_t* accept);
    void (*handle_reject_proposal)(tamaf_behavior_t* self, tamaf_aclmessage_t* cfp, tamaf_aclmessage_t* propose, tamaf_aclmessage_t* reject);
    tamaf_aclmessage_t* (*prepare_result_notification)(tamaf_behavior_t* self, tamaf_aclmessage_t* cfp, tamaf_aclmessage_t* propose, tamaf_aclmessage_t* accept);
} tamaf_fipa_contractnet_responder_t;

tamaf_behavior_t* tamaf_fipa_contractnet_responder_create(struct tamaf_agent_t* agent, void* template);

#endif // TAMAF_FIPA_PROTOCOLS_H
