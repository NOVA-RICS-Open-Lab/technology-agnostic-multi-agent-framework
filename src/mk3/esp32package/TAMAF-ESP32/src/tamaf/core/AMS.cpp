#include "AMS.h"
#include "tamaf/core/Agent.h"
#include "tamaf/behaviors/Behavior.h"
#include "tamaf/behaviors/InitializationBehavior.h"
#include "tamaf/behaviors/HeartBeatBehavior.h"
#include "tamaf/behaviors/ClosingBehavior.h"
#include "defines.h"
#include <algorithm>

namespace tamaf {
namespace core {

AMS::AMS(Agent* agent)
    : agent(agent),
      behaviorMutex(nullptr),
      amsTaskHandle(nullptr),
      isRunning(false),
      hasNewMessages(false),
      isExecutingCycle(false),
      lifeCycleState(tamaf::enums::LifeCycleState::INITIATED),
      initializationBehavior(nullptr),
      heartBeatBehavior(nullptr),
      closingBehavior(nullptr) {
    behaviorMutex = xSemaphoreCreateMutex();
}

AMS::~AMS() {
    if (isRunning.load(std::memory_order_acquire)) {
        Stop();
        uint32_t startWait = millis();
        while (isRunning.load(std::memory_order_acquire) && (millis() - startWait < 200)) {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
    if (amsTaskHandle != nullptr) {
        TaskHandle_t task = amsTaskHandle;
        amsTaskHandle = nullptr;
        vTaskDelete(task);
    }
    if (initializationBehavior) {
        delete initializationBehavior;
        initializationBehavior = nullptr;
    }
    if (closingBehavior) {
        delete closingBehavior;
        closingBehavior = nullptr;
    }
    for (auto b : activeBehaviors) {
        delete b;
    }
    activeBehaviors.clear();
    for (auto b : blockedBehaviors) {
        delete b;
    }
    blockedBehaviors.clear();
    for (auto b : pendingAdditions) {
        delete b;
    }
    pendingAdditions.clear();
    pendingRemovals.clear();
    heartBeatBehavior = nullptr;

    if (behaviorMutex != nullptr) {
        vSemaphoreDelete(behaviorMutex);
        behaviorMutex = nullptr;
    }
}

void AMS::Start() {
    if (isRunning.load(std::memory_order_acquire)) return;

    lifeCycleState = tamaf::enums::LifeCycleState::INITIATED;
    if (initializationBehavior) {
        delete initializationBehavior;
        initializationBehavior = nullptr;
    }
    initializationBehavior = new tamaf::behaviors::InitializationBehavior(agent);

    if (closingBehavior) {
        delete closingBehavior;
        closingBehavior = nullptr;
    }
    closingBehavior = new tamaf::behaviors::ClosingBehavior(agent);

    isRunning.store(true, std::memory_order_release);

    BaseType_t res = xTaskCreatePinnedToCore(
        AMSLoopTask,
        "AMSLoop",
        8192,
        this,
        1,
        &amsTaskHandle,
        1 // Pinned to Core 1
    );

    if (res != pdPASS) {
        if (agent) agent->logDebug(1, "Failed to create AMSLoopTask");
    } else {
        if (agent) agent->logDebug(1, "Successfully created AMSLoopTask");
    }
}

void AMS::Stop() {
    if (!isRunning.load(std::memory_order_acquire) || lifeCycleState == tamaf::enums::LifeCycleState::CLOSING) return;
    lifeCycleState = tamaf::enums::LifeCycleState::CLOSING;
    if (agent) agent->logDebug(1, "Agent Closing");
    if (amsTaskHandle != nullptr) {
        xTaskNotifyGive(amsTaskHandle);
    }
}

void AMS::Suspend() {
    if (!isRunning.load(std::memory_order_acquire) || lifeCycleState == tamaf::enums::LifeCycleState::SUSPENDED) return;
    lifeCycleState = tamaf::enums::LifeCycleState::SUSPENDED;
    if (agent) agent->logDebug(1, "Agent Suspended");
}

void AMS::Resume() {
    if (!isRunning.load(std::memory_order_acquire) || lifeCycleState != tamaf::enums::LifeCycleState::SUSPENDED) return;
    lifeCycleState = tamaf::enums::LifeCycleState::ACTIVE;
    if (agent) agent->logDebug(1, "Agent Resumed");
    NotifyNewMessage(); // Wake up any behaviors that received messages while suspended
}

void AMS::NotifyNewMessage() {
    hasNewMessages.store(true, std::memory_order_release);
    if (amsTaskHandle != nullptr) {
        xTaskNotifyGive(amsTaskHandle);
    }
}

void AMS::ProcessWakeup() {
    if (lifeCycleState == tamaf::enums::LifeCycleState::INITIATED) {
        if (initializationBehavior && initializationBehavior->IsBlocked()) {
            initializationBehavior->Unblock();
        }
        return;
    }
    if (lifeCycleState == tamaf::enums::LifeCycleState::SUSPENDED) {
        if (heartBeatBehavior && heartBeatBehavior->IsBlocked()) {
            heartBeatBehavior->Unblock();
        }
        return;
    }
    if (lifeCycleState == tamaf::enums::LifeCycleState::CLOSING) {
        if (closingBehavior && closingBehavior->IsBlocked()) {
            closingBehavior->Unblock();
        }
        return;
    }
    UnblockBehaviors();
}

void AMS::AMSLoopTask(void* parameter) {
    AMS* ams = static_cast<AMS*>(parameter);
    while (ams->isRunning.load(std::memory_order_acquire)) {
        ams->Loop();
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(10));
    }
    ams->amsTaskHandle = nullptr;
    vTaskDelete(NULL);
}

void AMS::Loop() {
    if (!isRunning.load(std::memory_order_acquire)) return;

    // Guard against concurrent execution if sketch calls agent->Loop()
    bool expected = false;
    if (!isExecutingCycle.compare_exchange_strong(expected, true)) {
        return;
    }

    // Wake up blocked behaviors if new messages arrived
    if (hasNewMessages.exchange(false, std::memory_order_acq_rel)) {
        ProcessWakeup();
    }

    // 1. INITIATED STATE: Gated execution - only initializationBehavior runs
    if (lifeCycleState == tamaf::enums::LifeCycleState::INITIATED) {
        if (initializationBehavior) {
            if (!initializationBehavior->IsStarted()) {
                initializationBehavior->OnStart();
                initializationBehavior->Started();
            }
            if (!initializationBehavior->IsBlocked()) {
                try {
                    initializationBehavior->Execute();
                } catch (const tamaf::AgentAlreadyExistsException& e) {
                    if (agent) agent->logDebug(1, String("Agent Exception: ") + e.what());
                    if (closingBehavior) closingBehavior->SetDeRegisterFalse();
                    lifeCycleState = tamaf::enums::LifeCycleState::CLOSING;
                    isExecutingCycle.store(false, std::memory_order_release);
                    return;
                } catch (const tamaf::EMANoMorePortsException& e) {
                    if (agent) agent->logDebug(1, String("Agent Exception: ") + e.what());
                    if (closingBehavior) closingBehavior->SetDeRegisterFalse();
                    lifeCycleState = tamaf::enums::LifeCycleState::CLOSING;
                    isExecutingCycle.store(false, std::memory_order_release);
                    return;
                } catch (const tamaf::EMAFailureException& e) {
                    if (agent) agent->logDebug(1, String("Agent Exception: ") + e.what());
                    if (closingBehavior) closingBehavior->SetDeRegisterFalse();
                    lifeCycleState = tamaf::enums::LifeCycleState::CLOSING;
                    isExecutingCycle.store(false, std::memory_order_release);
                    return;
                } catch (const std::exception& e) {
                    if (agent) agent->logDebug(1, String("Agent Exception: ") + e.what());
                    if (closingBehavior) closingBehavior->SetDeRegisterFalse();
                    lifeCycleState = tamaf::enums::LifeCycleState::CLOSING;
                    isExecutingCycle.store(false, std::memory_order_release);
                    return;
                }
            }
            if (initializationBehavior->IsBlocked()) {
                isExecutingCycle.store(false, std::memory_order_release);
                return;
            }
            if (initializationBehavior->Done()) {
                initializationBehavior->OnEnd();
                delete initializationBehavior;
                initializationBehavior = nullptr;
                lifeCycleState = tamaf::enums::LifeCycleState::ACTIVE;
                if (agent) agent->logDebug(1, "Agent Active");
            }
        }
        isExecutingCycle.store(false, std::memory_order_release);
        return;
    }

    // 2. Container maintenance: flush pending removals and additions (protected by behaviorMutex)
    std::vector<tamaf::behaviors::Behavior*> toRemove;
    std::vector<tamaf::behaviors::Behavior*> toAdd;

    if (behaviorMutex != nullptr && xSemaphoreTake(behaviorMutex, portMAX_DELAY) == pdTRUE) {
        if (!pendingRemovals.empty()) {
            toRemove = std::move(pendingRemovals);
            pendingRemovals.clear();
        }
        if (!pendingAdditions.empty()) {
            toAdd = std::move(pendingAdditions);
            pendingAdditions.clear();
        }
        xSemaphoreGive(behaviorMutex);
    }

    if (!toRemove.empty()) {
        for (auto* b : toRemove) {
            auto itActive = std::find(activeBehaviors.begin(), activeBehaviors.end(), b);
            if (itActive != activeBehaviors.end()) {
                activeBehaviors.erase(itActive);
            }
        }
    }

    if (!toAdd.empty()) {
        for (auto* b : toAdd) {
            b->setAgent(agent);
            activeBehaviors.push_back(b);
        }
    }

    // 3. SUSPENDED STATE: Only heartbeat runs, application behaviors are paused
    if (lifeCycleState == tamaf::enums::LifeCycleState::SUSPENDED) {
        if (heartBeatBehavior) {
            if (!heartBeatBehavior->IsStarted()) {
                heartBeatBehavior->OnStart();
                heartBeatBehavior->Started();
            }
            if (!heartBeatBehavior->IsBlocked()) {
                heartBeatBehavior->Execute();
            }
            if (heartBeatBehavior->Done()) {
                heartBeatBehavior->OnEnd();
                auto it = std::find(activeBehaviors.begin(), activeBehaviors.end(), heartBeatBehavior);
                if (it != activeBehaviors.end()) activeBehaviors.erase(it);
                auto itBlocked = std::find(blockedBehaviors.begin(), blockedBehaviors.end(), heartBeatBehavior);
                if (itBlocked != blockedBehaviors.end()) blockedBehaviors.erase(itBlocked);
                delete heartBeatBehavior;
                heartBeatBehavior = nullptr;
            }
        }
        isExecutingCycle.store(false, std::memory_order_release);
        return;
    }

    // 4. ACTIVE STATE: Normal round-robin execution ONLY of active behaviors
    if (lifeCycleState == tamaf::enums::LifeCycleState::ACTIVE) {
        auto it = activeBehaviors.begin();
        while (it != activeBehaviors.end()) {
            if (lifeCycleState != tamaf::enums::LifeCycleState::ACTIVE) {
                break;
            }

            tamaf::behaviors::Behavior* b = *it;
            
            if (!b->IsStarted()) {
                b->OnStart();
                b->Started();
            }

            b->Execute();
            
            if (b->IsBlocked()) {
                it = activeBehaviors.erase(it);
                blockedBehaviors.push_back(b);
                continue;
            }

            if (b->Done()) {
                b->OnEnd();
                if (b == heartBeatBehavior) {
                    heartBeatBehavior = nullptr;
                }
                delete b;
                it = activeBehaviors.erase(it);
            } else {
                ++it;
            }
        }
        isExecutingCycle.store(false, std::memory_order_release);
        return;
    }

    // 5. CLOSING STATE: Teardown and deregistration
    if (lifeCycleState == tamaf::enums::LifeCycleState::CLOSING) {
        if (initializationBehavior) {
            delete initializationBehavior;
            initializationBehavior = nullptr;
        }
        if (!closingBehavior) {
            closingBehavior = new tamaf::behaviors::ClosingBehavior(agent);
        }
        if (closingBehavior) {
            if (!closingBehavior->IsStarted()) {
                closingBehavior->OnStart();
                closingBehavior->Started();
            }
            if (!closingBehavior->IsBlocked()) {
                closingBehavior->Execute();
            }
            if (closingBehavior->IsBlocked()) {
                isExecutingCycle.store(false, std::memory_order_release);
                return;
            }
            if (closingBehavior->Done()) {
                closingBehavior->OnEnd();
                if (agent) {
                    agent->getMTS().StopServer();
                    agent->logDebug(1, "Agent Taken Down");
                }
                isRunning.store(false, std::memory_order_release);
                delete closingBehavior;
                closingBehavior = nullptr;
                isExecutingCycle.store(false, std::memory_order_release);
                return;
            }
        } else {
            if (agent) {
                agent->getMTS().StopServer();
                agent->logDebug(1, "Agent Taken Down");
            }
            isRunning.store(false, std::memory_order_release);
        }
        isExecutingCycle.store(false, std::memory_order_release);
        return;
    }

    isExecutingCycle.store(false, std::memory_order_release);
}

void AMS::AddBehavior(tamaf::behaviors::Behavior* behavior) {
    if (!behavior) return;
    if (behaviorMutex != nullptr && xSemaphoreTake(behaviorMutex, portMAX_DELAY) == pdTRUE) {
        pendingAdditions.push_back(behavior);
        xSemaphoreGive(behaviorMutex);
    } else {
        pendingAdditions.push_back(behavior);
    }
}

void AMS::RemoveBehavior(tamaf::behaviors::Behavior* behavior) {
    if (!behavior) return;
    if (behaviorMutex != nullptr && xSemaphoreTake(behaviorMutex, portMAX_DELAY) == pdTRUE) {
        pendingRemovals.push_back(behavior);
        xSemaphoreGive(behaviorMutex);
    } else {
        pendingRemovals.push_back(behavior);
    }
}

void AMS::BlockBehavior(tamaf::behaviors::Behavior* behavior) {
    if (!behavior) return;
    auto it = std::find(activeBehaviors.begin(), activeBehaviors.end(), behavior);
    if (it != activeBehaviors.end()) {
        activeBehaviors.erase(it);
    }
    blockedBehaviors.push_back(behavior);
}

void AMS::UnblockBehaviors() {
    for (auto* b : blockedBehaviors) {
        b->Unblock();
        activeBehaviors.push_back(b);
    }
    blockedBehaviors.clear();
}

} // namespace core
} // namespace tamaf
