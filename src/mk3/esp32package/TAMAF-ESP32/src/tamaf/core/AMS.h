#ifndef TAMAF_CORE_AMS_H
#define TAMAF_CORE_AMS_H

#include <Arduino.h>
#include <vector>
#include <atomic>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "tamaf/enums/LifeCycleState.h"

namespace tamaf {
namespace behaviors {
    class Behavior;
    class HeartBeatBehavior;
    class ClosingBehavior;
}
namespace core {
    class Agent;

class AMS {
private:
    Agent* agent;
    
    // Behavior containers
    std::vector<tamaf::behaviors::Behavior*> activeBehaviors;
    std::vector<tamaf::behaviors::Behavior*> blockedBehaviors;
    std::vector<tamaf::behaviors::Behavior*> pendingAdditions;
    std::vector<tamaf::behaviors::Behavior*> pendingRemovals;

    // Concurrency and task primitives
    SemaphoreHandle_t behaviorMutex;
    TaskHandle_t amsTaskHandle;
    std::atomic<bool> isRunning;
    std::atomic<bool> hasNewMessages;
    std::atomic<bool> isExecutingCycle;

    // Lifecycle state
    tamaf::enums::LifeCycleState lifeCycleState;

    // System behaviors
    tamaf::behaviors::Behavior* initializationBehavior;
    tamaf::behaviors::HeartBeatBehavior* heartBeatBehavior;
    tamaf::behaviors::ClosingBehavior* closingBehavior;

public:
    AMS(Agent* agent);
    virtual ~AMS();

    // Lifecycle Control
    void Start();
    void Stop();
    void Suspend();
    void Resume();

    // Execution Loop & FreeRTOS Task
    void Loop();
    void NotifyNewMessage();
    void ProcessWakeup();
    static void AMSLoopTask(void* parameter);

    // Behavior Management
    void AddBehavior(tamaf::behaviors::Behavior* behavior);
    void RemoveBehavior(tamaf::behaviors::Behavior* behavior);
    void BlockBehavior(tamaf::behaviors::Behavior* behavior);
    void UnblockBehaviors();

    // Getters & Setters
    Agent* getAgent() const { return agent; }
    tamaf::enums::LifeCycleState GetLifeCycleState() const { return lifeCycleState; }
    void SetLifeCycleState(tamaf::enums::LifeCycleState state) { lifeCycleState = state; }
    bool IsRunning() const { return isRunning.load(std::memory_order_acquire); }
    bool IsSuspended() const { return lifeCycleState == tamaf::enums::LifeCycleState::SUSPENDED; }
    bool IsActive() const { return lifeCycleState == tamaf::enums::LifeCycleState::ACTIVE; }
    bool IsInitiated() const { return lifeCycleState == tamaf::enums::LifeCycleState::INITIATED; }
    bool IsClosing() const { return lifeCycleState == tamaf::enums::LifeCycleState::CLOSING; }

    void SetHeartBeatBehavior(tamaf::behaviors::HeartBeatBehavior* hb) { heartBeatBehavior = hb; }
    tamaf::behaviors::HeartBeatBehavior* GetHeartBeatBehavior() const { return heartBeatBehavior; }
    tamaf::behaviors::Behavior* GetInitializationBehavior() const { return initializationBehavior; }
    tamaf::behaviors::ClosingBehavior* GetClosingBehavior() const { return closingBehavior; }

    const std::vector<tamaf::behaviors::Behavior*>& getActiveBehaviors() const { return activeBehaviors; }
    const std::vector<tamaf::behaviors::Behavior*>& getBlockedBehaviors() const { return blockedBehaviors; }
};

} // namespace core
} // namespace tamaf

#endif // TAMAF_CORE_AMS_H
