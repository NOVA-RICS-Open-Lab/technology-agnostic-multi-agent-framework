#include "TickerBehavior.h"

namespace tamaf {
namespace behaviors {

TickerBehavior::TickerBehavior(uint32_t periodMillis) 
    : Behavior(), period(periodMillis), lastExecutionTime(0) {}

void TickerBehavior::Execute() {
    if (!blockedFlag && !doneFlag) {
        uint32_t now = millis();
        if (now - lastExecutionTime >= period) {
            lastExecutionTime = now;
            Action(); // Calls OnTick() internally
            if (Done()) {
                doneFlag = true;
            }
        }
    }
}

void TickerBehavior::Action() {
    OnTick();
}

bool TickerBehavior::Done() {
    return false; // Ticker runs forever until manually stopped or removed
}

} // namespace behaviors
} // namespace tamaf
