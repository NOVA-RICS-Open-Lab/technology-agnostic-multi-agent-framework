#ifndef TAMAF_PERFORMATIVE_H
#define TAMAF_PERFORMATIVE_H

typedef enum {
    TAMAF_ACCEPT_PROPOSAL,
    TAMAF_AGREE,
    TAMAF_CANCEL,
    TAMAF_CFP,
    TAMAF_CONFIRM,
    TAMAF_DISCONFIRM,
    TAMAF_FAILURE,
    TAMAF_INFORM,
    TAMAF_INFORM_IF,
    TAMAF_INFORM_REF,
    TAMAF_NOT_UNDERSTOOD,
    TAMAF_PROPAGATE,
    TAMAF_PROPOSE,
    TAMAF_PROXY,
    TAMAF_QUERY_IF,
    TAMAF_QUERY_REF,
    TAMAF_REFUSE,
    TAMAF_REJECT_PROPOSAL,
    TAMAF_REQUEST,
    TAMAF_REQUEST_WHEN,
    TAMAF_REQUEST_WHENEVER,
    TAMAF_SUBSCRIBE,
    TAMAF_KEEPALIVE,
    TAMAF_PERFORMATIVE_UNKNOWN
} tamaf_performative_t;

/**
 * Convert a performative enum to its string representation.
 */
const char* tamaf_performative_to_string(tamaf_performative_t perf);

/**
 * Convert a string representation to a performative enum.
 */
tamaf_performative_t tamaf_performative_from_string(const char* str);

#endif // TAMAF_PERFORMATIVE_H
