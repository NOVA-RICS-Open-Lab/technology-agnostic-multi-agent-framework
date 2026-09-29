#include "ModifyAgentEMABehavior.h"
#include "tamaf/core/Agent.h"

namespace tamaf {
namespace behaviors {

ModifyAgentEMABehavior::ModifyAgentEMABehavior() 
    : OneShotBehavior(), emaInteraction(this) {}

ModifyAgentEMABehavior::ModifyAgentEMABehavior(tamaf::core::Agent* agent) 
    : OneShotBehavior(agent), emaInteraction(this) {}

void ModifyAgentEMABehavior::Action() {
    emaInteraction.ModifyAgent();
}

} // namespace behaviors
} // namespace tamaf
