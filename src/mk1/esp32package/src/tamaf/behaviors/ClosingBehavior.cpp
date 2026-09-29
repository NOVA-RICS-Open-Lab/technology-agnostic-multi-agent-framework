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
    if (deRegister) {
        if (getAgent()->GetAgentID().getName() != DEFAULT_EMA_NAME) {
            emaInteraction.DeRegisterAgent();
        }
    }
}

} // namespace behaviors
} // namespace tamaf
