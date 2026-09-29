#ifndef TAMAF_LIFECYCLESTATE_H
#define TAMAF_LIFECYCLESTATE_H

#include <Arduino.h>

namespace tamaf {
namespace enums {

/**
 * @brief Defines the state machine status of the Agent inside the AMS Kernel.
 */
enum class LifeCycleState {
    INITIATED,  // Agent is booting and attempting to register with the EMA
    ACTIVE,     // Agent is fully operational and executing its behavior queue
    SUSPENDED,  // Agent execution is halted, continues to send keep-alive pings
    WAITING,    // Agent is explicitly waiting for an external trigger
    DELETED,    // Agent has been logically destroyed
    TRANSIT,    // Agent is moving between environments
    CLOSING     // Agent is actively deregistering and shutting down
};

/**
 * @brief Converts a LifeCycleState enum to its string representation.
 */
inline String LifeCycleStateToString(LifeCycleState state) {
    switch (state) {
        case LifeCycleState::INITIATED: return "INITIATED";
        case LifeCycleState::ACTIVE: return "ACTIVE";
        case LifeCycleState::SUSPENDED: return "SUSPENDED";
        case LifeCycleState::WAITING: return "WAITING";
        case LifeCycleState::DELETED: return "DELETED";
        case LifeCycleState::TRANSIT: return "TRANSIT";
        case LifeCycleState::CLOSING: return "CLOSING";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Parses a string into a LifeCycleState enum.
 */
inline LifeCycleState StringToLifeCycleState(const String& state) {
    if (state == "INITIATED") return LifeCycleState::INITIATED;
    if (state == "ACTIVE") return LifeCycleState::ACTIVE;
    if (state == "SUSPENDED") return LifeCycleState::SUSPENDED;
    if (state == "WAITING") return LifeCycleState::WAITING;
    if (state == "DELETED") return LifeCycleState::DELETED;
    if (state == "TRANSIT") return LifeCycleState::TRANSIT;
    if (state == "CLOSING") return LifeCycleState::CLOSING;
    return LifeCycleState::INITIATED; // Default fallback
}

} // namespace enums
} // namespace tamaf

#endif // TAMAF_LIFECYCLESTATE_H
