#ifndef TAMAF_AGENT_H
#define TAMAF_AGENT_H

#include <vector>
#include <memory>
#include <optional>
#include "tamaf/types/AgentID.h"
#include "tamaf/types/AgentDescription.h"
#include "tamaf/messaging/MessageTransportSystem.h"
#include "tamaf/behaviors/Behavior.h"
#include "tamaf/messaging/ACLMessage.h"
#include "tamaf/messaging/ACLMessageTemplate.h"
#include <functional>

namespace tamaf {
    namespace ema { class EMAInteraction; }
}
namespace tamaf {
namespace core {

class Agent {
protected:
    tamaf::types::AgentID agentId;
    tamaf::types::AgentDescription agentDescription;
    tamaf::types::Address emaAddress;
    tamaf::messaging::MessageTransportSystem mts;
    std::vector<tamaf::behaviors::Behavior*> behaviors;
    std::vector<tamaf::behaviors::Behavior*> behaviorsToAdd; // Thread-safe queueing
    bool isRunning;

public:
    Agent(const String& name, const tamaf::types::Address& address);
    virtual ~Agent();

    // Core Lifecycle
    virtual void Setup(tamaf::ema::EMAInteraction* emaInteraction) = 0; // User implements this
    virtual void Shutdown() {}
    
    // Engine Step (called from Arduino loop)
    void Loop();
    
    void Start();
    void Stop();

    // Messaging
    bool Send(const tamaf::messaging::ACLMessage& msg);
    bool Receive(tamaf::messaging::ACLMessage& outMsg);
    std::optional<tamaf::messaging::ACLMessage> Receive(const tamaf::messaging::ACLMessageTemplate& tmpl);
    std::optional<tamaf::messaging::ACLMessage> Receive(std::function<bool(const tamaf::messaging::ACLMessage&)> matchFunc);
    void NotifyNewMessage(); // Called by MTS when message arrives

    // Behaviors
    void AddBehavior(tamaf::behaviors::Behavior* behavior);

    // Getters / Setters
    tamaf::messaging::MessageTransportSystem& getMTS();
    const tamaf::types::AgentID& GetAgentID() const;
    const tamaf::types::AgentDescription& GetAgentDescription() const;
    const tamaf::types::Address& GetEMAAddress() const;
    void setEMAAddress(const tamaf::types::Address& address);
};

} // namespace core
} // namespace tamaf

#endif // TAMAF_AGENT_H
