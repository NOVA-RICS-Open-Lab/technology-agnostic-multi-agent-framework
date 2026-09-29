#ifndef TAMAF_EMA_INTERACTION_H
#define TAMAF_EMA_INTERACTION_H

#include <Arduino.h>
#include <optional>
#include "tamaf/messaging/ACLMessage.h"
#include "tamaf/types/AgentDescription.h"

// Forward declaration to avoid circular dependencies
namespace tamaf {
    namespace behaviors { class Behavior; }
}

namespace tamaf {
namespace ema {

class EMAInteraction {
private:
    tamaf::behaviors::Behavior* behavior;
    bool messageSent;
    String conversationID;

    bool SendToEMA(const tamaf::messaging::ACLMessage& aclMessage);

public:
    EMAInteraction(tamaf::behaviors::Behavior* behavior);

    void KeepAliveAgent();
    
    // These return std::nullopt if the response hasn't arrived yet (cooperative multitasking)
    std::optional<tamaf::messaging::ACLMessage> RegisterAgent();
    std::optional<tamaf::messaging::ACLMessage> DeRegisterAgent();
    std::optional<tamaf::messaging::ACLMessage> ModifyAgent();
    
    // Search methods
    std::optional<std::vector<tamaf::types::AgentDescription>> Search(const tamaf::types::AgentDescription& agentDescription);
    std::optional<std::vector<tamaf::types::AgentDescription>> LocalSearch(const tamaf::types::AgentDescription& agentDescription);
    std::optional<std::vector<tamaf::types::AgentDescription>> ExternalSearch(const tamaf::types::AgentDescription& agentDescription);
};

} // namespace ema
} // namespace tamaf

#endif // TAMAF_EMA_INTERACTION_H
