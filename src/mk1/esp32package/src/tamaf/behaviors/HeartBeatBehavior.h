#ifndef TAMAF_HEART_BEAT_BEHAVIOR_H
#define TAMAF_HEART_BEAT_BEHAVIOR_H

#include "tamaf/behaviors/TickerBehavior.h"
#include "tamaf/ema/EMAInteraction.h"

namespace tamaf {
namespace behaviors {

class HeartBeatBehavior : public TickerBehavior {
private:
    tamaf::ema::EMAInteraction emaInteraction;

public:
    HeartBeatBehavior(tamaf::core::Agent* agent);
    
    void OnTick() override;
};

} // namespace behaviors
} // namespace tamaf

#endif // TAMAF_HEART_BEAT_BEHAVIOR_H
