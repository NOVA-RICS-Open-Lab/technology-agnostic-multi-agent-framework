#include "ClosingBehavior.h"
#include "tamaf/core/Agent.h"
#include "defines.h"

namespace tamaf {
namespace behaviors {

ClosingBehavior::ClosingBehavior(tamaf::core::Agent* agent, bool deRegister)
    : emaInteraction(this), deRegister(deRegister) {
    setAgent(agent);
}

void ClosingBehavior::SetDeRegisterFalse() {
    deRegister = false;
}

void ClosingBehavior::Action() {
    if (getAgent()) {
        getAgent()->TakeDown(&emaInteraction);
    }
    if (deRegister) {
        if (getAgent() && getAgent()->GetAgentID().getName() != DEFAULT_EMA_NAME) {
            try {
                emaInteraction.DeRegisterAgent();
            } catch (const std::exception& e) {
                // Best-effort teardown: ignore communication failures during deregistration
            } catch (...) {
                // Best-effort teardown
            }
        }
    }
}

} // namespace behaviors
} // namespace tamaf
