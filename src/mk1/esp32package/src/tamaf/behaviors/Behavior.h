#ifndef TAMAF_BEHAVIOR_H
#define TAMAF_BEHAVIOR_H

#include <Arduino.h>
#include <map>

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

public:
    Behavior();
    virtual ~Behavior() = default;

    virtual void Action() = 0;       // The core logic, must be implemented by subclasses
    virtual bool Done() = 0;         // Determines if the behavior should be removed

    // Lifecycle hooks
    virtual void OnStart();
    virtual int OnEnd();

    // Blocking / Execution Control
    bool IsBlocked() const;
    void Block();
    void Restart();
    virtual void Execute();                  // Called by the scheduler

    // Agent binding
    void setAgent(tamaf::core::Agent* agent);
    tamaf::core::Agent* getAgent() const;
};

} // namespace behaviors
} // namespace tamaf

#endif // TAMAF_BEHAVIOR_H
