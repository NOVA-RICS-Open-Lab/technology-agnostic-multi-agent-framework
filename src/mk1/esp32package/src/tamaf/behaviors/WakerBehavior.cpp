#include "WakerBehavior.h"

namespace tamaf {
namespace behaviors {

WakerBehavior::WakerBehavior(unsigned long timeout_ms) : timeoutPeriod(timeout_ms), waken(false) {}

WakerBehavior::WakerBehavior(tamaf::core::Agent* agent, unsigned long timeout_ms) : timeoutPeriod(timeout_ms), waken(false) {
    this->myAgent = agent;
}

void WakerBehavior::OnStart() {
    wakeupTime = millis() + timeoutPeriod;
    waken = false;
}

void WakerBehavior::Action() {
    if (millis() >= wakeupTime && !waken) {
        OnWake();
        waken = true;
    }
}

bool WakerBehavior::Done() {
    return waken;
}

} // namespace behaviors
} // namespace tamaf
