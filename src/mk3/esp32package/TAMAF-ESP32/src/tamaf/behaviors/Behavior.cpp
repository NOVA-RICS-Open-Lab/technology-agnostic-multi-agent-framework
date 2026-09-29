#include "Behavior.h"
#include "tamaf/core/Agent.h"

namespace tamaf {
namespace behaviors {

Behavior::Behavior() : myAgent(nullptr), doneFlag(false), blockedFlag(false), startedFlag(false) {}

void Behavior::OnStart() {
    // Default empty implementation
}

int Behavior::OnEnd() {
    // Default implementation returning 0 (success)
    return 0;
}

bool Behavior::IsStarted() const {
    return startedFlag;
}

void Behavior::Started() {
    startedFlag = true;
}

void Behavior::SetStarted(bool started) {
    startedFlag = started;
}

bool Behavior::IsBlocked() const {
    return blockedFlag;
}

void Behavior::Block() {
    blockedFlag = true;
}

void Behavior::Unblock() {
    blockedFlag = false;
}

void Behavior::Execute() {
    messagesToPutBack.clear();
    Unblock(); // Unblock before running action

    try {
        Action();
    } catch (const EmptyReceiveException& e) {
        Block();
        // Atomic rollback: restore messages extracted during this cycle to MTS queue
        for (const auto& msg : messagesToPutBack) {
            PutBack(msg);
        }
        messagesToPutBack.clear();
    }
}

bool Behavior::Send(const tamaf::messaging::ACLMessage& msg) {
    if (myAgent) {
        return myAgent->Send(msg);
    }
    return false;
}

tamaf::messaging::ACLMessage Behavior::Receive(const tamaf::messaging::ACLMessageTemplate& tmpl) {
    if (!myAgent) {
        throw EmptyReceiveException();
    }
    auto opt = myAgent->Receive(tmpl);
    if (!opt.has_value()) {
        throw EmptyReceiveException();
    }
    messagesToPutBack.insert(messagesToPutBack.begin(), opt.value());
    return opt.value();
}

bool Behavior::Receive(tamaf::messaging::ACLMessage& outMsg, bool blockOnEmpty) {
    if (myAgent) {
        bool res = myAgent->Receive(outMsg);
        if (!res && blockOnEmpty) {
            Block();
        }
        return res;
    }
    if (blockOnEmpty) Block();
    return false;
}

std::optional<tamaf::messaging::ACLMessage> Behavior::ReceiveOptional(const tamaf::messaging::ACLMessageTemplate& tmpl) {
    if (myAgent) {
        return myAgent->Receive(tmpl);
    }
    return std::nullopt;
}

void Behavior::PutBack(const tamaf::messaging::ACLMessage& msg) {
    if (myAgent) {
        myAgent->PutBack(msg);
    }
}

void Behavior::setAgent(tamaf::core::Agent* agent) {
    this->myAgent = agent;
}

tamaf::core::Agent* Behavior::getAgent() const {
    return this->myAgent;
}

} // namespace behaviors
} // namespace tamaf
