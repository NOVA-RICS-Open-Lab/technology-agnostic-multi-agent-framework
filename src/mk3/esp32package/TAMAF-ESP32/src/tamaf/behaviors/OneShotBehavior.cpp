#include "OneShotBehavior.h"

namespace tamaf {
namespace behaviors {

OneShotBehavior::OneShotBehavior() : Behavior() {}

OneShotBehavior::OneShotBehavior(tamaf::core::Agent* agent) : Behavior() {
    setAgent(agent);
}

bool OneShotBehavior::Done() {
    return true; // Always finishes after one execution
}

} // namespace behaviors
} // namespace tamaf
