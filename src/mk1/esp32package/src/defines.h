#ifndef TAMAF_DEFINES_H
#define TAMAF_DEFINES_H

#include <Arduino.h>

namespace tamaf {

// Network defaults
constexpr const char* LOCALHOST = "127.0.0.1";
constexpr const char* ALL_INTERFACES = "0.0.0.0";
constexpr int DEFAULT_EMA_PORT = 4000;
constexpr int DEFAULT_UDP_PACKETSIZE = 65535;
constexpr int DEFAULT_TCP_BACKLOG = 10;

// Timeouts and Intervals (in milliseconds)
constexpr uint32_t DEFAULT_KERNEL_STABILIZATION_TIME = 100;
constexpr uint32_t DEFAULT_LOOP_STABILIZATION_TIME = 100;
constexpr uint32_t DEFAULT_THREAD_JOIN_TIMEOUT_TIME = 2000;
constexpr uint32_t DEFAULT_REGISTRATION_PORT_RETRY_TIME = 500;
constexpr uint32_t DEFAULT_MESSAGE_TIMEOUT_TIME = 5000;
constexpr uint32_t SERVER_STARTING_TIME = 500;
constexpr uint32_t DEFAULT_EMA_HEARTBEAT_INTERVAL = 5000; // 5 seconds
constexpr uint32_t DEFAULT_EMA_CONNECTION_TIMEOUT = 10000; // 10 seconds

// EMA and Ontologies
constexpr const char* DEFAULT_EMA_NAME = "EMA";
constexpr const char* DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME = "tamaf-agent-management";
constexpr const char* DEFAULT_AGENTMANAGEMENT_ONTOLOGY_AGENTADDRESS_CONCEPT = "am-agent-address";
constexpr const char* DEFAULT_AGENTMANAGEMENT_ONTOLOGY_INFORM_CONCEPT = "am-inform";
constexpr const char* DEFAULT_AGENTMANAGEMENT_ONTOLOGY_AGENTDESCRIPTION_CONCEPT = "am-agent-description";

// Protocol / Action strings
namespace actions {
    constexpr const char* REGISTER = "register";
    constexpr const char* REGISTER_RESULT = "register-result";
    constexpr const char* DEREGISTER = "deregister";
    constexpr const char* DEREGISTER_RESULT = "deregister-result";
    constexpr const char* MODIFY = "modify";
    constexpr const char* MODIFY_RESULT = "modify-result";
    constexpr const char* LOCAL_SEARCH = "local-search";
    constexpr const char* EXTERNAL_SEARCH = "external-search";
    constexpr const char* GLOBAL_SEARCH = "global-search";
    constexpr const char* SEARCH_RESULT = "search-result";
    constexpr const char* KEEPALIVE = "keepalive";
}

// Status Flags (instead of Exceptions)
constexpr int DEFAULT_AGENT_ALREADY_EXISTS = -1;
constexpr int DEFAULT_EMA_HAS_NO_MORE_PORTS = -2;
constexpr int DEFAULT_OUTSIDE_AGENT = -3;

} // namespace tamaf

#endif // TAMAF_DEFINES_H
