#ifndef TAMAF_INITIALIZATION_BEHAVIOR_H
#define TAMAF_INITIALIZATION_BEHAVIOR_H

#include "tamaf/behaviors/Behavior.h"
#include "tamaf/ema/EMAInteraction.h"
#include "tamaf/behaviors/HeartBeatBehavior.h"

namespace tamaf {
namespace behaviors {

class InitializationBehavior : public Behavior {
private:
    tamaf::ema::EMAInteraction emaInteraction;
    bool isRegistered;
    bool initializationDone;
    bool serverStarted;
    unsigned long startTime;
    
public:
    InitializationBehavior(tamaf::core::Agent* agent);
    
    void Action() override;
    bool Done() override;
};

} // namespace behaviors
} // namespace tamaf

#endif // TAMAF_INITIALIZATION_BEHAVIOR_H
