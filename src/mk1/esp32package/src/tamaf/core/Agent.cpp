#include "Agent.h"
#include "tamaf/behaviors/InitializationBehavior.h"
#include "tamaf/behaviors/ClosingBehavior.h"
#include "defines.h"

namespace tamaf {
namespace core {

Agent::Agent(const String& name, const tamaf::types::Address& address)
    : agentId(name, address), mts(this), isRunning(false) {
    
    agentDescription.setAgentId(agentId);
    emaAddress = tamaf::types::Address(LOCALHOST, DEFAULT_EMA_PORT);
    mts.SetPort(address.getPort());
}

Agent::~Agent() {
    Stop();
    for (auto b : behaviors) {
        delete b;
    }
    behaviors.clear();
}

void Agent::Start() {
    if (isRunning) return;
    
    // We add InitializationBehavior which will handle EMA registration and eventually call Setup()
    AddBehavior(new tamaf::behaviors::InitializationBehavior(this));
    
    mts.Start();
    isRunning = true;
}

void Agent::Stop() {
    if (!isRunning) return;
    Shutdown();
    
    // Fire a ClosingBehavior synchronously to deregister
    tamaf::behaviors::ClosingBehavior closer(this, true);
    closer.Action(); 
    
    mts.Stop();
    isRunning = false;
}

void Agent::Loop() {
    if (!isRunning) return;

    // Transfer pending behaviors
    if (!behaviorsToAdd.empty()) {
        for (auto b : behaviorsToAdd) {
            b->setAgent(this);
            b->OnStart();
            behaviors.push_back(b);
        }
        behaviorsToAdd.clear();
    }

    // Execute behaviors
    auto it = behaviors.begin();
    while (it != behaviors.end()) {
        tamaf::behaviors::Behavior* b = *it;
        if (!b->IsBlocked()) {
            b->Execute();
        }
        
        if (b->Done()) {
            b->OnEnd();
            delete b;
            it = behaviors.erase(it);
        } else {
            ++it;
        }
    }
}

bool Agent::Send(const tamaf::messaging::ACLMessage& msg) {
    if (msg.getReceivers().empty()) return false;
    
    // Copy the message to safely mutate the sender
    tamaf::messaging::ACLMessage messageToSend = msg;
    if (messageToSend.getSender().getName().isEmpty()) {
        messageToSend.setSender(this->agentId);
    }
    
    bool allSent = true;
    for (const auto& receiver : messageToSend.getReceivers()) {
        const auto& addr = receiver.getAddress();
        if (!mts.TCPSend(messageToSend, addr.getIp(), addr.getPort())) {
            allSent = false;
        }
    }
    return allSent;
}

bool Agent::Receive(tamaf::messaging::ACLMessage& outMsg) {
    return mts.ReceiveMessage(outMsg);
}

std::optional<tamaf::messaging::ACLMessage> Agent::Receive(const tamaf::messaging::ACLMessageTemplate& tmpl) {
    tamaf::messaging::ACLMessage msg;
    if (mts.ReceiveMessage(tmpl, msg)) {
        return msg;
    }
    return std::nullopt;
}

std::optional<tamaf::messaging::ACLMessage> Agent::Receive(std::function<bool(const tamaf::messaging::ACLMessage&)> matchFunc) {
    tamaf::messaging::ACLMessage msg;
    if (mts.ReceiveMessage(matchFunc, msg)) {
        return msg;
    }
    return std::nullopt;
}

void Agent::NotifyNewMessage() {
    // Unblock behaviors waiting for messages
    for (auto b : behaviors) {
        if (b->IsBlocked()) {
            b->Restart(); // Simplistic wakeup
        }
    }
}

void Agent::AddBehavior(tamaf::behaviors::Behavior* behavior) {
    if (behavior) {
        behaviorsToAdd.push_back(behavior);
    }
}

tamaf::messaging::MessageTransportSystem& Agent::getMTS() {
    return mts;
}

const tamaf::types::AgentID& Agent::GetAgentID() const {
    return agentId;
}

const tamaf::types::AgentDescription& Agent::GetAgentDescription() const {
    return agentDescription;
}

const tamaf::types::Address& Agent::GetEMAAddress() const {
    return emaAddress;
}

void Agent::setEMAAddress(const tamaf::types::Address& address) {
    emaAddress = address;
}

} // namespace core
} // namespace tamaf
