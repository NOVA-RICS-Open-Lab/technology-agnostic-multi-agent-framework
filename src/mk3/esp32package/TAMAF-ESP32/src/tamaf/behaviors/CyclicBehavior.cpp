#include "CyclicBehavior.h"

namespace tamaf {
namespace behaviors {

CyclicBehavior::CyclicBehavior() : Behavior() {}

CyclicBehavior::CyclicBehavior(tamaf::core::Agent* agent) : Behavior() {
    setAgent(agent);
}

bool CyclicBehavior::Done() {
    return false; // Never finishes automatically
}

} // namespace behaviors
} // namespace tamaf
