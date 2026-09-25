#ifndef TAMAF_AGENT_H
#define TAMAF_AGENT_H

#include <vector>
#include <memory>
#include <optional>
#include <functional>
#include <atomic>
#include "tamaf/types/AgentID.h"
#include "tamaf/types/AgentDescription.h"
#include "tamaf/messaging/MTS.h"
#include "tamaf/behaviors/Behavior.h"
#include "tamaf/messaging/ACLMessage.h"
#include "tamaf/messaging/ACLMessageTemplate.h"
#include "tamaf/enums/LifeCycleState.h"
#include "tamaf/core/AMS.h"

namespace tamaf {
    namespace ema { class EMAInteraction; }
    namespace behaviors { class HeartBeatBehavior; }
}
namespace tamaf {
namespace core {

class Agent {
protected:
    tamaf::types::AgentDescription agentDescription;
    tamaf::types::Address emaAddress;
    tamaf::types::Address registerAddress;
    tamaf::messaging::MTS mts;
    AMS ams;
    int debugLevel_;

public:
    Agent(const String& name, const tamaf::types::Address& address, int debugLevel = 1);
    Agent(const String& name, const tamaf::types::Address& address, const tamaf::types::Address& registerAddress, int debugLevel = 1);
    virtual ~Agent();

    int getDebugLevel() const { return debugLevel_; }
    bool isDebug() const { return debugLevel_ > 0; }
    void logDebug(const String& message);
    void logDebug(int level, const String& message);

    // Core Lifecycle
    virtual void Setup(tamaf::ema::EMAInteraction* emaInteraction) = 0; // User implements this
    virtual void TakeDown(tamaf::ema::EMAInteraction* emaInteraction) { Shutdown(); }
    virtual void Shutdown() {}
    
    // Engine Step (delegates to AMS)
    void Loop();
    
    void Start();
    void Stop();
    void Suspend();
    void Resume();
    tamaf::enums::LifeCycleState GetLifeCycleState() const;
    void SetLifeCycleState(tamaf::enums::LifeCycleState state);
    bool IsSuspended() const;
    bool IsActive() const;
    bool IsInitiated() const;
    bool IsRunning() const;
    void SetHeartBeatBehavior(tamaf::behaviors::HeartBeatBehavior* hb);
    tamaf::behaviors::HeartBeatBehavior* GetHeartBeatBehavior() const;

    // Messaging
    bool Send(const tamaf::messaging::ACLMessage& msg);
    bool Receive(tamaf::messaging::ACLMessage& outMsg);
    std::optional<tamaf::messaging::ACLMessage> Receive(const tamaf::messaging::ACLMessageTemplate& tmpl = tamaf::messaging::ACLMessageTemplate());
    void PutBack(const tamaf::messaging::ACLMessage& msg);
    void NotifyNewMessage(); // Called by MTS when message arrives

    // Behaviors
    void AddBehavior(tamaf::behaviors::Behavior* behavior);
    void RemoveBehavior(tamaf::behaviors::Behavior* behavior);
    void BlockBehavior(tamaf::behaviors::Behavior* behavior);
    void UnblockBehaviors();

    // Getters / Setters
    AMS& getAMS() { return ams; }
    const AMS& getAMS() const { return ams; }
    tamaf::messaging::MTS& getMTS();
    const tamaf::messaging::MTS& getMTS() const;
    const tamaf::types::AgentID& GetAgentID() const;
    const tamaf::types::AgentDescription& GetAgentDescription() const;
    tamaf::types::AgentDescription GetAgentDescriptionTemplate() const;
    void UpdateAgentDescription(const tamaf::types::AgentDescription& newDesc);
    const tamaf::types::Address& GetEMAAddress() const;
    void setEMAAddress(const tamaf::types::Address& address);
    const tamaf::types::Address& GetRegisterAddress() const;
    void SetRegisterAddress(const tamaf::types::Address& address);
    void SetPort(int port);
};

} // namespace core
} // namespace tamaf

#endif // TAMAF_AGENT_H
