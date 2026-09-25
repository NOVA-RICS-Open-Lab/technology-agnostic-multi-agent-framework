#include "TAMAF/tamaf_fipa_protocols.h"
#include "TAMAF/tamaf_common_behaviors.h"
#include "TAMAF/tamaf_aclmessagetemplate.h"
#include "TAMAF/tamaf_agent.h"
#include "TAMAF/tamaf_mts.h"
#include "TAMAF/tamaf_ams.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#define strdup _strdup
#endif

// Internal data for FIPARequestInitiator
typedef struct {
    tamaf_aclmessage_t* request_msg;
    tamaf_aclmessagetemplate_t* reply_template;
    tamaf_aclmessage_t* last_reply;
    
    void (*handle_agree)(tamaf_behavior_t* self, tamaf_aclmessage_t* msg);
    void (*handle_refuse)(tamaf_behavior_t* self, tamaf_aclmessage_t* msg);
    void (*handle_inform)(tamaf_behavior_t* self, tamaf_aclmessage_t* msg);
    void (*handle_failure)(tamaf_behavior_t* self, tamaf_aclmessage_t* msg);
    void (*handle_not_understood)(tamaf_behavior_t* self, tamaf_aclmessage_t* msg);
} request_init_data_t;

// State Behaviors
static void send_request_action(tamaf_behavior_t* self) {
    request_init_data_t* data = (request_init_data_t*)self->user_data;
    tamaf_mts_t* mts = (tamaf_mts_t*)self->agent->mts;
    tamaf_mts_send(mts, data->request_msg);
}

static void wait_first_response_action(tamaf_behavior_t* self) {
    request_init_data_t* data = (request_init_data_t*)self->user_data;
    tamaf_aclmessage_t* reply = tamaf_behavior_receive(self, data->reply_template);
    if (data->last_reply) tamaf_aclmessage_destroy(data->last_reply);
    data->last_reply = reply;
}

static int wait_first_response_end(tamaf_behavior_t* self) {
    request_init_data_t* data = (request_init_data_t*)self->user_data;
    tamaf_aclmessage_t* reply = data->last_reply;
    if (!reply) return 2; // Not understood
    switch (reply->performative) {
        case TAMAF_AGREE: return 0;
        case TAMAF_REFUSE: return 1;
        case TAMAF_NOT_UNDERSTOOD: return 2;
        case TAMAF_INFORM: return 3;
        case TAMAF_FAILURE: return 4;
        default: return 2;
    }
}

static void wait_result_action(tamaf_behavior_t* self) {
    request_init_data_t* data = (request_init_data_t*)self->user_data;
    tamaf_aclmessage_t* reply = tamaf_behavior_receive(self, data->reply_template);
    if (data->last_reply) tamaf_aclmessage_destroy(data->last_reply);
    data->last_reply = reply;
}

static int wait_result_end(tamaf_behavior_t* self) {
    request_init_data_t* data = (request_init_data_t*)self->user_data;
    tamaf_aclmessage_t* reply = data->last_reply;
    if (reply && reply->performative == TAMAF_INFORM) return 0;
    return 1; // Failure or other
}

static void handle_agree_action(tamaf_behavior_t* self) {
    request_init_data_t* data = (request_init_data_t*)self->user_data;
    if (data->handle_agree) data->handle_agree(self, data->last_reply);
}

static void handle_refuse_action(tamaf_behavior_t* self) {
    request_init_data_t* data = (request_init_data_t*)self->user_data;
    if (data->handle_refuse) data->handle_refuse(self, data->last_reply);
}

static void handle_not_understood_action(tamaf_behavior_t* self) {
    request_init_data_t* data = (request_init_data_t*)self->user_data;
    if (data->handle_not_understood) data->handle_not_understood(self, data->last_reply);
}

static void handle_inform_action(tamaf_behavior_t* self) {
    request_init_data_t* data = (request_init_data_t*)self->user_data;
    if (data->handle_inform) data->handle_inform(self, data->last_reply);
}

static void handle_failure_action(tamaf_behavior_t* self) {
    request_init_data_t* data = (request_init_data_t*)self->user_data;
    if (data->handle_failure) data->handle_failure(self, data->last_reply);
}

static void request_init_destroy(tamaf_behavior_t* self) {
    request_init_data_t* data = (request_init_data_t*)self->user_data;
    if (data) {
        if (data->request_msg) tamaf_aclmessage_destroy(data->request_msg);
        if (data->reply_template) tamaf_aclmessagetemplate_destroy(data->reply_template);
        if (data->last_reply) tamaf_aclmessage_destroy(data->last_reply);
        free(data);
    }
    tamaf_behavior_base_destroy(self);
}

tamaf_behavior_t* tamaf_fipa_request_initiator_create(struct tamaf_agent_t* agent, tamaf_aclmessage_t* request_msg) {
    tamaf_behavior_t* fsm = tamaf_fsm_behavior_create(agent);
    request_init_data_t* data = (request_init_data_t*)calloc(1, sizeof(request_init_data_t));
    data->request_msg = request_msg;
    if (!request_msg->conversation_id) {
        char buf[64]; sprintf(buf, "req-%p-%ld", (void*)fsm, (long)time(NULL));
        request_msg->conversation_id = strdup(buf);
    }
    data->reply_template = tamaf_aclmessagetemplate_create();
    tamaf_aclmessagetemplate_set_conversation_id(data->reply_template, request_msg->conversation_id);
    fsm->user_data = data; fsm->destroy = request_init_destroy;
    
    tamaf_behavior_t* b_send = tamaf_oneshot_behavior_create(agent, send_request_action);
    b_send->user_data = data; tamaf_fsm_add_initial_state(fsm, b_send, "sendRequest");
    
    tamaf_behavior_t* b_wait1 = tamaf_oneshot_behavior_create(agent, wait_first_response_action);
    b_wait1->user_data = data; b_wait1->on_end = wait_first_response_end;
    tamaf_fsm_add_state(fsm, b_wait1, "waitFirstResponse");
    
    tamaf_behavior_t* b_agree = tamaf_oneshot_behavior_create(agent, handle_agree_action);
    b_agree->user_data = data; tamaf_fsm_add_state(fsm, b_agree, "handleAgree");
    
    tamaf_behavior_t* b_wait_res = tamaf_oneshot_behavior_create(agent, wait_result_action);
    b_wait_res->user_data = data; b_wait_res->on_end = wait_result_end;
    tamaf_fsm_add_state(fsm, b_wait_res, "waitResult");
    
    tamaf_behavior_t* b_refuse = tamaf_oneshot_behavior_create(agent, handle_refuse_action);
    b_refuse->user_data = data; tamaf_fsm_add_final_state(fsm, b_refuse, "handleRefuse");
    
    tamaf_behavior_t* b_not_und = tamaf_oneshot_behavior_create(agent, handle_not_understood_action);
    b_not_und->user_data = data; tamaf_fsm_add_final_state(fsm, b_not_und, "handleNotUnderstood");
    
    tamaf_behavior_t* b_inform = tamaf_oneshot_behavior_create(agent, handle_inform_action);
    b_inform->user_data = data; tamaf_fsm_add_final_state(fsm, b_inform, "handleInform");
    
    tamaf_behavior_t* b_failure = tamaf_oneshot_behavior_create(agent, handle_failure_action);
    b_failure->user_data = data; tamaf_fsm_add_final_state(fsm, b_failure, "handleFailure");
    
    tamaf_fsm_add_transition(fsm, "sendRequest", "waitFirstResponse", 0);
    tamaf_fsm_add_transition(fsm, "waitFirstResponse", "handleAgree", 0);
    tamaf_fsm_add_transition(fsm, "waitFirstResponse", "handleRefuse", 1);
    tamaf_fsm_add_transition(fsm, "waitFirstResponse", "handleNotUnderstood", 2);
    tamaf_fsm_add_transition(fsm, "waitFirstResponse", "handleInform", 3);
    tamaf_fsm_add_transition(fsm, "waitFirstResponse", "handleFailure", 4);
    tamaf_fsm_add_transition(fsm, "handleAgree", "waitResult", 0);
    tamaf_fsm_add_transition(fsm, "waitResult", "handleInform", 0);
    tamaf_fsm_add_transition(fsm, "waitResult", "handleFailure", 1);
    
    return fsm;
}

// FIPA Request Responder For Dispatcher
static void req_resp_disp_handle_action(tamaf_behavior_t* self) {
    tamaf_fipa_request_responder_for_dispatcher_t* data = (tamaf_fipa_request_responder_for_dispatcher_t*)self->user_data;
    tamaf_aclmessage_t* reply = data->handle_request ? data->handle_request(self, data->request_msg) : NULL;
    if (reply) {
        if (!reply->conversation_id && data->request_msg->conversation_id) reply->conversation_id = strdup(data->request_msg->conversation_id);
        self->derived = reply; // sharedData['agreeMessage']
        tamaf_mts_send((tamaf_mts_t*)self->agent->mts, reply);
    }
}

static int req_resp_disp_handle_end(tamaf_behavior_t* self) {
    tamaf_aclmessage_t* reply = (tamaf_aclmessage_t*)self->derived;
    return (reply && reply->performative == TAMAF_AGREE) ? 0 : 1;
}

static void req_resp_disp_prepare_result_action(tamaf_behavior_t* self) {
    tamaf_fipa_request_responder_for_dispatcher_t* data = (tamaf_fipa_request_responder_for_dispatcher_t*)self->user_data;
    tamaf_aclmessage_t* agree = (tamaf_aclmessage_t*)self->derived;
    tamaf_aclmessage_t* result = data->prepare_result_notification ? data->prepare_result_notification(self, data->request_msg, agree) : NULL;
    if (result) {
        if (!result->conversation_id && data->request_msg->conversation_id) result->conversation_id = strdup(data->request_msg->conversation_id);
        tamaf_mts_send((tamaf_mts_t*)self->agent->mts, result);
        tamaf_aclmessage_destroy(result);
    }
}

static void req_resp_disp_destroy(tamaf_behavior_t* self) {
    tamaf_fipa_request_responder_for_dispatcher_t* data = (tamaf_fipa_request_responder_for_dispatcher_t*)self->user_data;
    if (data) {
        if (data->request_msg) tamaf_aclmessage_destroy(data->request_msg);
        free(data);
    }
    tamaf_aclmessage_t* agree = (tamaf_aclmessage_t*)self->derived;
    if (agree) tamaf_aclmessage_destroy(agree);
    tamaf_behavior_base_destroy(self);
}

tamaf_behavior_t* tamaf_fipa_request_responder_for_dispatcher_create(struct tamaf_agent_t* agent, tamaf_aclmessage_t* request_msg) {
    tamaf_behavior_t* fsm = tamaf_fsm_behavior_create(agent);
    tamaf_fipa_request_responder_for_dispatcher_t* data = (tamaf_fipa_request_responder_for_dispatcher_t*)calloc(1, sizeof(tamaf_fipa_request_responder_for_dispatcher_t));
    data->request_msg = request_msg;
    fsm->user_data = data;
    fsm->destroy = req_resp_disp_destroy;

    tamaf_behavior_t* b_handle = tamaf_oneshot_behavior_create(agent, req_resp_disp_handle_action);
    b_handle->user_data = data; b_handle->on_end = req_resp_disp_handle_end;
    tamaf_fsm_add_initial_state(fsm, b_handle, "handleRequest");

    tamaf_behavior_t* b_result = tamaf_oneshot_behavior_create(agent, req_resp_disp_prepare_result_action);
    b_result->user_data = data;
    tamaf_fsm_add_state(fsm, b_result, "prepareResult");

    tamaf_behavior_t* b_end = tamaf_oneshot_behavior_create(agent, NULL);
    tamaf_fsm_add_final_state(fsm, b_end, "end");

    tamaf_fsm_add_transition(fsm, "handleRequest", "prepareResult", 0);
    tamaf_fsm_add_transition(fsm, "handleRequest", "end", 1);
    tamaf_fsm_add_transition(fsm, "prepareResult", "end", 0);

    return fsm;
}

// FIPA Request Responder
static void request_resp_action(tamaf_behavior_t* self) {
    tamaf_fipa_request_responder_t* data = (tamaf_fipa_request_responder_t*)self->user_data;
    tamaf_aclmessage_t* request = tamaf_behavior_receive(self, (tamaf_aclmessagetemplate_t*)data->template);
    if (request) {
        tamaf_aclmessage_t* response = NULL;
        if (data->prepare_response) response = data->prepare_response(self, request);
        if (response) {
            tamaf_mts_send((tamaf_mts_t*)self->agent->mts, response);
            if (response->performative == TAMAF_AGREE) {
                tamaf_aclmessage_t* result = NULL;
                if (data->prepare_result_notification) result = data->prepare_result_notification(self, request, response);
                if (result) { tamaf_mts_send((tamaf_mts_t*)self->agent->mts, result); tamaf_aclmessage_destroy(result); }
            }
            tamaf_aclmessage_destroy(response);
        }
        tamaf_aclmessage_destroy(request);
    }
}

static void request_resp_destroy(tamaf_behavior_t* self) {
    tamaf_fipa_request_responder_t* data = (tamaf_fipa_request_responder_t*)self->user_data;
    if (data) { if (data->template) tamaf_aclmessagetemplate_destroy((tamaf_aclmessagetemplate_t*)data->template); free(data); }
    tamaf_behavior_base_destroy(self);
}

tamaf_behavior_t* tamaf_fipa_request_responder_create(struct tamaf_agent_t* agent, void* template) {
    tamaf_behavior_t* b = tamaf_behavior_create(agent);
    b->action = request_resp_action; b->destroy = request_resp_destroy;
    tamaf_fipa_request_responder_t* data = (tamaf_fipa_request_responder_t*)calloc(1, sizeof(tamaf_fipa_request_responder_t));
    data->template = (tamaf_aclmessagetemplate_t*)template;
    b->user_data = data; return b;
}

// FIPA Request Dispatcher
static void req_disp_action(tamaf_behavior_t* self) {
    tamaf_fipa_request_dispatcher_t* data = (tamaf_fipa_request_dispatcher_t*)self->user_data;
    tamaf_aclmessage_t* req = tamaf_behavior_receive(self, data->template);
    if (req) {
        tamaf_behavior_t* responder = data->create_responder ? data->create_responder(self, req) : tamaf_fipa_request_responder_for_dispatcher_create(self->agent, req);
        if (responder) {
            tamaf_ams_add_behavior((tamaf_ams_t*)self->agent->ams, responder);
        } else {
            tamaf_aclmessage_destroy(req);
        }
    }
}

static void req_disp_destroy(tamaf_behavior_t* self) {
    tamaf_fipa_request_dispatcher_t* data = (tamaf_fipa_request_dispatcher_t*)self->user_data;
    if (data) {
        if (data->template) tamaf_aclmessagetemplate_destroy(data->template);
        free(data);
    }
    tamaf_behavior_base_destroy(self);
}

tamaf_behavior_t* tamaf_fipa_request_dispatcher_create(struct tamaf_agent_t* agent, void* template) {
    tamaf_behavior_t* b = tamaf_cyclic_behavior_create(agent, req_disp_action);
    tamaf_fipa_request_dispatcher_t* data = (tamaf_fipa_request_dispatcher_t*)calloc(1, sizeof(tamaf_fipa_request_dispatcher_t));
    if (template) data->template = (tamaf_aclmessagetemplate_t*)template;
    else {
        data->template = tamaf_aclmessagetemplate_create();
        tamaf_aclmessagetemplate_add_performative(data->template, TAMAF_REQUEST);
    }
    b->user_data = data; b->destroy = req_disp_destroy;
    return b;
}

// FIPA Contract Net Initiator
typedef struct {
    tamaf_aclmessage_t* cfp_msg;
    tamaf_aclmessagetemplate_t* reply_template;
    size_t expected_cfps;
    tamaf_aclmessage_t** proposals; size_t proposals_count;
    tamaf_aclmessage_t** accepts; size_t accepts_count;
    tamaf_aclmessage_t** results; size_t results_count;
    void (*handle_propose)(tamaf_behavior_t* self, tamaf_aclmessage_t* msg);
    void (*handle_refuse)(tamaf_behavior_t* self, tamaf_aclmessage_t* msg);
    void (*handle_inform)(tamaf_behavior_t* self, tamaf_aclmessage_t* msg);
    void (*handle_failure)(tamaf_behavior_t* self, tamaf_aclmessage_t* msg);
} cnet_init_data_t;

static void cnet_send_cfp_action(tamaf_behavior_t* self) {
    cnet_init_data_t* data = (cnet_init_data_t*)self->user_data;
    tamaf_mts_send((tamaf_mts_t*)self->agent->mts, data->cfp_msg);
    data->expected_cfps = data->cfp_msg->receivers_count;
}

static void cnet_collect_action(tamaf_behavior_t* self) {
    cnet_init_data_t* data = (cnet_init_data_t*)self->user_data;
    tamaf_aclmessage_t* reply = tamaf_behavior_receive(self, data->reply_template);
    if (reply) {
        data->proposals = (tamaf_aclmessage_t**)realloc(data->proposals, (data->proposals_count + 1) * sizeof(tamaf_aclmessage_t*));
        data->proposals[data->proposals_count++] = reply;
        if (reply->performative == TAMAF_PROPOSE && data->handle_propose) data->handle_propose(self, reply);
        else if (reply->performative == TAMAF_REFUSE && data->handle_refuse) data->handle_refuse(self, reply);
    }
}

static bool cnet_collect_done(tamaf_behavior_t* self) {
    cnet_init_data_t* data = (cnet_init_data_t*)self->user_data;
    return data->proposals_count >= data->expected_cfps;
}

static int cnet_handle_proposals_end(tamaf_behavior_t* self) {
    cnet_init_data_t* data = (cnet_init_data_t*)self->user_data;
    return data->accepts_count > 0 ? 0 : 1;
}

static void cnet_wait_results_action(tamaf_behavior_t* self) {
    cnet_init_data_t* data = (cnet_init_data_t*)self->user_data;
    tamaf_aclmessage_t* reply = tamaf_behavior_receive(self, data->reply_template);
    if (reply) {
        data->results = (tamaf_aclmessage_t**)realloc(data->results, (data->results_count + 1) * sizeof(tamaf_aclmessage_t*));
        data->results[data->results_count++] = reply;
        if (reply->performative == TAMAF_INFORM && data->handle_inform) data->handle_inform(self, reply);
        else if (reply->performative == TAMAF_FAILURE && data->handle_failure) data->handle_failure(self, reply);
    }
}

static bool cnet_wait_results_done(tamaf_behavior_t* self) {
    cnet_init_data_t* data = (cnet_init_data_t*)self->user_data;
    return data->results_count >= data->accepts_count;
}

static void cnet_init_destroy(tamaf_behavior_t* self) {
    cnet_init_data_t* data = (cnet_init_data_t*)self->user_data;
    if (data) {
        if (data->cfp_msg) tamaf_aclmessage_destroy(data->cfp_msg);
        if (data->reply_template) tamaf_aclmessagetemplate_destroy(data->reply_template);
        for (size_t i = 0; i < data->proposals_count; i++) tamaf_aclmessage_destroy(data->proposals[i]);
        free(data->proposals);
        for (size_t i = 0; i < data->accepts_count; i++) tamaf_aclmessage_destroy(data->accepts[i]);
        free(data->accepts);
        for (size_t i = 0; i < data->results_count; i++) tamaf_aclmessage_destroy(data->results[i]);
        free(data->results);
        free(data);
    }
    tamaf_behavior_base_destroy(self);
}

tamaf_behavior_t* tamaf_fipa_contractnet_initiator_create(struct tamaf_agent_t* agent, tamaf_aclmessage_t* cfp_msg) {
    tamaf_behavior_t* fsm = tamaf_fsm_behavior_create(agent);
    cnet_init_data_t* data = (cnet_init_data_t*)calloc(1, sizeof(cnet_init_data_t));
    data->cfp_msg = cfp_msg;
    if (!cfp_msg->conversation_id) {
        char buf[64]; sprintf(buf, "cnet-%p-%ld", (void*)fsm, (long)time(NULL));
        cfp_msg->conversation_id = strdup(buf);
    }
    data->reply_template = tamaf_aclmessagetemplate_create();
    tamaf_aclmessagetemplate_set_conversation_id(data->reply_template, cfp_msg->conversation_id);
    fsm->user_data = data; fsm->destroy = cnet_init_destroy;
    
    tamaf_behavior_t* b_send = tamaf_oneshot_behavior_create(agent, cnet_send_cfp_action);
    b_send->user_data = data; tamaf_fsm_add_initial_state(fsm, b_send, "sendCFP");
    
    tamaf_behavior_t* b_collect = tamaf_behavior_create(agent);
    b_collect->action = cnet_collect_action; b_collect->done = cnet_collect_done; b_collect->user_data = data;
    tamaf_fsm_add_state(fsm, b_collect, "collect");
    
    tamaf_behavior_t* b_handle = tamaf_oneshot_behavior_create(agent, NULL);
    b_handle->user_data = data; b_handle->on_end = cnet_handle_proposals_end;
    tamaf_fsm_add_state(fsm, b_handle, "handleProposals");
    
    tamaf_behavior_t* b_wait_res = tamaf_behavior_create(agent);
    b_wait_res->action = cnet_wait_results_action; b_wait_res->done = cnet_wait_results_done; b_wait_res->user_data = data;
    tamaf_fsm_add_state(fsm, b_wait_res, "waitResults");
    
    tamaf_behavior_t* b_end = tamaf_oneshot_behavior_create(agent, NULL);
    tamaf_fsm_add_final_state(fsm, b_end, "end");
    
    tamaf_fsm_add_transition(fsm, "sendCFP", "collect", 0);
    tamaf_fsm_add_transition(fsm, "collect", "handleProposals", 0);
    tamaf_fsm_add_transition(fsm, "handleProposals", "waitResults", 0);
    tamaf_fsm_add_transition(fsm, "handleProposals", "end", 1);
    tamaf_fsm_add_transition(fsm, "waitResults", "end", 0);
    
    return fsm;
}

// FIPA Contract Net Responder
typedef struct {
    tamaf_aclmessage_t* cfp;
    tamaf_aclmessagetemplate_t* reply_template;
    tamaf_aclmessage_t* propose_msg;
    tamaf_aclmessage_t* accept_msg;
} cnet_resp_shared_t;

static void cnet_resp_wait_cfp_action(tamaf_behavior_t* self) {
    tamaf_fipa_contractnet_responder_t* data = (tamaf_fipa_contractnet_responder_t*)self->user_data;
    cnet_resp_shared_t* shared = (cnet_resp_shared_t*)self->derived;
    tamaf_aclmessage_t* cfp = tamaf_behavior_receive(self, data->cfp_template);
    if (cfp) {
        shared->cfp = cfp;
        shared->reply_template = tamaf_aclmessagetemplate_create();
        tamaf_aclmessagetemplate_set_conversation_id(shared->reply_template, cfp->conversation_id);
        if (data->handle_cfp) data->handle_cfp(self, cfp);
    }
}

static void cnet_resp_prepare_action(tamaf_behavior_t* self) {
    tamaf_fipa_contractnet_responder_t* data = (tamaf_fipa_contractnet_responder_t*)self->user_data;
    cnet_resp_shared_t* shared = (cnet_resp_shared_t*)self->derived;
    tamaf_aclmessage_t* reply = data->prepare_response ? data->prepare_response(self, shared->cfp) : NULL;
    if (reply) {
        if (!reply->conversation_id && shared->cfp->conversation_id) reply->conversation_id = strdup(shared->cfp->conversation_id);
        shared->propose_msg = reply;
        tamaf_mts_send((tamaf_mts_t*)self->agent->mts, reply);
    }
}

static int cnet_resp_prepare_end(tamaf_behavior_t* self) {
    cnet_resp_shared_t* shared = (cnet_resp_shared_t*)self->derived;
    return (shared->propose_msg && shared->propose_msg->performative == TAMAF_PROPOSE) ? 0 : 1;
}

static void cnet_resp_wait_accept_action(tamaf_behavior_t* self) {
    tamaf_fipa_contractnet_responder_t* data = (tamaf_fipa_contractnet_responder_t*)self->user_data;
    cnet_resp_shared_t* shared = (cnet_resp_shared_t*)self->derived;
    tamaf_aclmessage_t* reply = tamaf_behavior_receive(self, shared->reply_template);
    if (reply) {
        if (reply->performative == TAMAF_ACCEPT_PROPOSAL) {
            shared->accept_msg = reply;
            if (data->handle_accept_proposal) data->handle_accept_proposal(self, shared->cfp, shared->propose_msg, reply);
        } else {
            if (data->handle_reject_proposal) data->handle_reject_proposal(self, shared->cfp, shared->propose_msg, reply);
            tamaf_aclmessage_destroy(reply);
        }
    }
}

static int cnet_resp_wait_accept_end(tamaf_behavior_t* self) {
    cnet_resp_shared_t* shared = (cnet_resp_shared_t*)self->derived;
    return shared->accept_msg ? 0 : 1;
}

static void cnet_resp_execute_action(tamaf_behavior_t* self) {
    tamaf_fipa_contractnet_responder_t* data = (tamaf_fipa_contractnet_responder_t*)self->user_data;
    cnet_resp_shared_t* shared = (cnet_resp_shared_t*)self->derived;
    tamaf_aclmessage_t* result = data->prepare_result_notification ? data->prepare_result_notification(self, shared->cfp, shared->propose_msg, shared->accept_msg) : NULL;
    if (result) {
        if (!result->conversation_id && shared->cfp->conversation_id) result->conversation_id = strdup(shared->cfp->conversation_id);
        tamaf_mts_send((tamaf_mts_t*)self->agent->mts, result);
        tamaf_aclmessage_destroy(result);
    }
}

static void cnet_resp_destroy(tamaf_behavior_t* self) {
    tamaf_fipa_contractnet_responder_t* data = (tamaf_fipa_contractnet_responder_t*)self->user_data;
    cnet_resp_shared_t* shared = (cnet_resp_shared_t*)self->derived;
    if (shared) {
        if (shared->cfp) tamaf_aclmessage_destroy(shared->cfp);
        if (shared->reply_template) tamaf_aclmessagetemplate_destroy(shared->reply_template);
        if (shared->propose_msg) tamaf_aclmessage_destroy(shared->propose_msg);
        if (shared->accept_msg) tamaf_aclmessage_destroy(shared->accept_msg);
        free(shared);
    }
    if (data) free(data);
    tamaf_behavior_base_destroy(self);
}

tamaf_behavior_t* tamaf_fipa_contractnet_responder_create(struct tamaf_agent_t* agent, void* template) {
    tamaf_behavior_t* fsm = tamaf_fsm_behavior_create(agent);
    tamaf_fipa_contractnet_responder_t* data = (tamaf_fipa_contractnet_responder_t*)calloc(1, sizeof(tamaf_fipa_contractnet_responder_t));
    data->cfp_template = (tamaf_aclmessagetemplate_t*)template;
    fsm->user_data = data; fsm->destroy = cnet_resp_destroy;
    fsm->derived = calloc(1, sizeof(cnet_resp_shared_t));

    tamaf_behavior_t* b_wait_cfp = tamaf_oneshot_behavior_create(agent, cnet_resp_wait_cfp_action);
    b_wait_cfp->user_data = data; b_wait_cfp->derived = fsm->derived;
    tamaf_fsm_add_initial_state(fsm, b_wait_cfp, "waitCfp");

    tamaf_behavior_t* b_prepare = tamaf_oneshot_behavior_create(agent, cnet_resp_prepare_action);
    b_prepare->user_data = data; b_prepare->derived = fsm->derived; b_prepare->on_end = cnet_resp_prepare_end;
    tamaf_fsm_add_state(fsm, b_prepare, "prepareResponse");

    tamaf_behavior_t* b_wait_acc = tamaf_oneshot_behavior_create(agent, cnet_resp_wait_accept_action);
    b_wait_acc->user_data = data; b_wait_acc->derived = fsm->derived; b_wait_acc->on_end = cnet_resp_wait_accept_end;
    tamaf_fsm_add_state(fsm, b_wait_acc, "waitAcceptReject");

    tamaf_behavior_t* b_exec = tamaf_oneshot_behavior_create(agent, cnet_resp_execute_action);
    b_exec->user_data = data; b_exec->derived = fsm->derived;
    tamaf_fsm_add_state(fsm, b_exec, "executeTask");

    tamaf_fsm_add_transition(fsm, "waitCfp", "prepareResponse", 0);
    tamaf_fsm_add_transition(fsm, "prepareResponse", "waitAcceptReject", 0);
    tamaf_fsm_add_transition(fsm, "prepareResponse", "waitCfp", 1);
    tamaf_fsm_add_transition(fsm, "waitAcceptReject", "executeTask", 0);
    tamaf_fsm_add_transition(fsm, "waitAcceptReject", "waitCfp", 1);
    tamaf_fsm_add_transition(fsm, "executeTask", "waitCfp", 0);

    return fsm;
}
