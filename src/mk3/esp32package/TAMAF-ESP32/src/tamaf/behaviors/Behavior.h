#ifndef TAMAF_BEHAVIOR_H
#define TAMAF_BEHAVIOR_H

#include <Arduino.h>
#include <vector>
#include <map>
#include <optional>
#include <functional>
#include "defines.h"
#include "tamaf/messaging/ACLMessage.h"
#include "tamaf/messaging/ACLMessageTemplate.h"

// Forward declaration of Agent to avoid circular dependency
namespace tamaf {
namespace core {
    class Agent;
}
}

namespace tamaf {
namespace behaviors {

class Behavior {
protected:
    tamaf::core::Agent* myAgent;          // Pointer to the agent executing this behavior
    bool doneFlag;           // True if behavior has finished execution
    bool blockedFlag;        // True if behavior is blocked waiting for messages/events
    bool startedFlag;        // True if behavior has run OnStart
    std::vector<tamaf::messaging::ACLMessage> messagesToPutBack;

public:
    Behavior();
    virtual ~Behavior() = default;

    virtual void Action() = 0;       // The core logic, must be implemented by subclasses
    virtual bool Done() = 0;         // Determines if the behavior should be removed

    // Lifecycle hooks
    virtual void OnStart();
    virtual int OnEnd();

    // Lifecycle Start Control
    bool IsStarted() const;
    void Started();
    void SetStarted(bool started);

    // Blocking / Execution Control
    bool IsBlocked() const;
    void Block();
    void Unblock();
    virtual void Execute();                  // Called by the scheduler

    // Messaging convenience methods (matching Python/Java Behavior API)
    bool Send(const tamaf::messaging::ACLMessage& msg);
    
    // Symmetric primary Receive method: returns ACLMessage or throws EmptyReceiveException
    tamaf::messaging::ACLMessage Receive(const tamaf::messaging::ACLMessageTemplate& tmpl = tamaf::messaging::ACLMessageTemplate());

    // Optional / legacy compatibility methods
    bool Receive(tamaf::messaging::ACLMessage& outMsg, bool blockOnEmpty = true);
    std::optional<tamaf::messaging::ACLMessage> ReceiveOptional(const tamaf::messaging::ACLMessageTemplate& tmpl);

    void PutBack(const tamaf::messaging::ACLMessage& msg);

    // Agent binding
    virtual void setAgent(tamaf::core::Agent* agent);
    tamaf::core::Agent* getAgent() const;
};

} // namespace behaviors
} // namespace tamaf

#endif // TAMAF_BEHAVIOR_H
