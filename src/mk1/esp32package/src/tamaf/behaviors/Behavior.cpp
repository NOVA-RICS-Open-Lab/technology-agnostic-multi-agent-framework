#include "Behavior.h"

namespace tamaf {
namespace behaviors {

Behavior::Behavior() : myAgent(nullptr), doneFlag(false), blockedFlag(false) {}

void Behavior::OnStart() {
    // Default empty implementation
}

int Behavior::OnEnd() {
    // Default implementation returning 0 (success)
    return 0;
}

bool Behavior::IsBlocked() const {
    return blockedFlag;
}

void Behavior::Block() {
    blockedFlag = true;
}

void Behavior::Restart() {
    blockedFlag = false;
    doneFlag = false;
}

void Behavior::Execute() {
    if (!blockedFlag && !doneFlag) {
        Action();
        if (Done()) {
            doneFlag = true;
        }
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
