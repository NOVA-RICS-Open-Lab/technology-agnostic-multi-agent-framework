#include "TAMAF/tamaf_performative.h"
#include <string.h>

const char* tamaf_performative_to_string(tamaf_performative_t perf) {
    switch (perf) {
        case TAMAF_ACCEPT_PROPOSAL: return "accept-proposal";
        case TAMAF_AGREE: return "agree";
        case TAMAF_CANCEL: return "cancel";
        case TAMAF_CFP: return "cfp";
        case TAMAF_CONFIRM: return "confirm";
        case TAMAF_DISCONFIRM: return "disconfirm";
        case TAMAF_FAILURE: return "failure";
        case TAMAF_INFORM: return "inform";
        case TAMAF_INFORM_IF: return "inform-if";
        case TAMAF_INFORM_REF: return "inform-ref";
        case TAMAF_NOT_UNDERSTOOD: return "not-understood";
        case TAMAF_PROPAGATE: return "propagate";
        case TAMAF_PROPOSE: return "propose";
        case TAMAF_PROXY: return "proxy";
        case TAMAF_QUERY_IF: return "query-if";
        case TAMAF_QUERY_REF: return "query-ref";
        case TAMAF_REFUSE: return "refuse";
        case TAMAF_REJECT_PROPOSAL: return "reject-proposal";
        case TAMAF_REQUEST: return "request";
        case TAMAF_REQUEST_WHEN: return "request-when";
        case TAMAF_REQUEST_WHENEVER: return "request-whenever";
        case TAMAF_SUBSCRIBE: return "subscribe";
        case TAMAF_KEEPALIVE: return "keep-alive";
        default: return "unknown";
    }
}

tamaf_performative_t tamaf_performative_from_string(const char* str) {
    if (strcmp(str, "accept-proposal") == 0) return TAMAF_ACCEPT_PROPOSAL;
    if (strcmp(str, "agree") == 0) return TAMAF_AGREE;
    if (strcmp(str, "cancel") == 0) return TAMAF_CANCEL;
    if (strcmp(str, "cfp") == 0) return TAMAF_CFP;
    if (strcmp(str, "confirm") == 0) return TAMAF_CONFIRM;
    if (strcmp(str, "disconfirm") == 0) return TAMAF_DISCONFIRM;
    if (strcmp(str, "failure") == 0) return TAMAF_FAILURE;
    if (strcmp(str, "inform") == 0) return TAMAF_INFORM;
    if (strcmp(str, "inform-if") == 0) return TAMAF_INFORM_IF;
    if (strcmp(str, "inform-ref") == 0) return TAMAF_INFORM_REF;
    if (strcmp(str, "not-understood") == 0) return TAMAF_NOT_UNDERSTOOD;
    if (strcmp(str, "propagate") == 0) return TAMAF_PROPAGATE;
    if (strcmp(str, "propose") == 0) return TAMAF_PROPOSE;
    if (strcmp(str, "proxy") == 0) return TAMAF_PROXY;
    if (strcmp(str, "query-if") == 0) return TAMAF_QUERY_IF;
    if (strcmp(str, "query-ref") == 0) return TAMAF_QUERY_REF;
    if (strcmp(str, "refuse") == 0) return TAMAF_REFUSE;
    if (strcmp(str, "reject-proposal") == 0) return TAMAF_REJECT_PROPOSAL;
    if (strcmp(str, "request") == 0) return TAMAF_REQUEST;
    if (strcmp(str, "request-when") == 0) return TAMAF_REQUEST_WHEN;
    if (strcmp(str, "request-whenever") == 0) return TAMAF_REQUEST_WHENEVER;
    if (strcmp(str, "subscribe") == 0) return TAMAF_SUBSCRIBE;
    if (strcmp(str, "keep-alive") == 0) return TAMAF_KEEPALIVE;
    return TAMAF_PERFORMATIVE_UNKNOWN;
}
