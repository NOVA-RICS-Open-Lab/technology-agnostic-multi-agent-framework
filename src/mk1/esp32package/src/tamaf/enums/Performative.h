#ifndef TAMAF_PERFORMATIVE_H
#define TAMAF_PERFORMATIVE_H

#include <Arduino.h>

namespace tamaf {
namespace enums {

/**
 * @brief Defines the FIPA Communicative Act (the intent of an ACLMessage).
 */
enum class Performative {
    ACCEPT_PROPOSAL,
    AGREE,
    CANCEL,
    CFP,
    CONFIRM,
    DISCONFIRM,
    FAILURE,
    INFORM,
    INFORM_IF,
    INFORM_REF,
    NOT_UNDERSTOOD,
    PROPAGATE,
    PROPOSE,
    PROXY,
    QUERY_IF,
    QUERY_REF,
    REFUSE,
    REJECT_PROPOSAL,
    REQUEST,
    REQUEST_WHEN,
    REQUEST_WHENEVER,
    SUBSCRIBE,
    KEEPALIVE, // Internal system performative
    UNKNOWN
};

inline String PerformativeToString(Performative perf) {
    switch (perf) {
        case Performative::ACCEPT_PROPOSAL: return "accept-proposal";
        case Performative::AGREE: return "agree";
        case Performative::CANCEL: return "cancel";
        case Performative::CFP: return "cfp";
        case Performative::CONFIRM: return "confirm";
        case Performative::DISCONFIRM: return "disconfirm";
        case Performative::FAILURE: return "failure";
        case Performative::INFORM: return "inform";
        case Performative::INFORM_IF: return "inform-if";
        case Performative::INFORM_REF: return "inform-ref";
        case Performative::NOT_UNDERSTOOD: return "not-understood";
        case Performative::PROPAGATE: return "propagate";
        case Performative::PROPOSE: return "propose";
        case Performative::PROXY: return "proxy";
        case Performative::QUERY_IF: return "query-if";
        case Performative::QUERY_REF: return "query-ref";
        case Performative::REFUSE: return "refuse";
        case Performative::REJECT_PROPOSAL: return "reject-proposal";
        case Performative::REQUEST: return "request";
        case Performative::REQUEST_WHEN: return "request-when";
        case Performative::REQUEST_WHENEVER: return "request-whenever";
        case Performative::SUBSCRIBE: return "subscribe";
        case Performative::KEEPALIVE: return "keep-alive";
        default: return "unknown";
    }
}

inline Performative StringToPerformative(const String& perf) {
    if (perf == "accept-proposal") return Performative::ACCEPT_PROPOSAL;
    if (perf == "agree") return Performative::AGREE;
    if (perf == "cancel") return Performative::CANCEL;
    if (perf == "cfp") return Performative::CFP;
    if (perf == "confirm") return Performative::CONFIRM;
    if (perf == "disconfirm") return Performative::DISCONFIRM;
    if (perf == "failure") return Performative::FAILURE;
    if (perf == "inform") return Performative::INFORM;
    if (perf == "inform-if") return Performative::INFORM_IF;
    if (perf == "inform-ref") return Performative::INFORM_REF;
    if (perf == "not-understood") return Performative::NOT_UNDERSTOOD;
    if (perf == "propagate") return Performative::PROPAGATE;
    if (perf == "propose") return Performative::PROPOSE;
    if (perf == "proxy") return Performative::PROXY;
    if (perf == "query-if") return Performative::QUERY_IF;
    if (perf == "query-ref") return Performative::QUERY_REF;
    if (perf == "refuse") return Performative::REFUSE;
    if (perf == "reject-proposal") return Performative::REJECT_PROPOSAL;
    if (perf == "request") return Performative::REQUEST;
    if (perf == "request-when") return Performative::REQUEST_WHEN;
    if (perf == "request-whenever") return Performative::REQUEST_WHENEVER;
    if (perf == "subscribe") return Performative::SUBSCRIBE;
    if (perf == "keep-alive") return Performative::KEEPALIVE;
    return Performative::UNKNOWN;
}

} // namespace enums
} // namespace tamaf

#endif // TAMAF_PERFORMATIVE_H
