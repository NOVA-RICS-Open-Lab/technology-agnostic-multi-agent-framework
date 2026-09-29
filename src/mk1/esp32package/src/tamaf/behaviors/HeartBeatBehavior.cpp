#include "HeartBeatBehavior.h"
#include "defines.h"

namespace tamaf {
namespace behaviors {

HeartBeatBehavior::HeartBeatBehavior(tamaf::core::Agent* agent)
    : TickerBehavior(DEFAULT_EMA_HEARTBEAT_INTERVAL), emaInteraction(this) {
    setAgent(agent);
}

void HeartBeatBehavior::OnTick() {
    emaInteraction.KeepAliveAgent();
}

} // namespace behaviors
} // namespace tamaf
