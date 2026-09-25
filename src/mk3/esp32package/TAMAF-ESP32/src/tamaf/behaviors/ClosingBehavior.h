#ifndef TAMAF_CLOSING_BEHAVIOR_H
#define TAMAF_CLOSING_BEHAVIOR_H

#include "tamaf/behaviors/OneShotBehavior.h"
#include "tamaf/ema/EMAInteraction.h"

namespace tamaf {
namespace behaviors {

class ClosingBehavior : public OneShotBehavior {
private:
    tamaf::ema::EMAInteraction emaInteraction;
    bool deRegister;

public:
    ClosingBehavior(tamaf::core::Agent* agent, bool deRegister = true);
    
    void SetDeRegisterFalse();
    void Action() override;
};

} // namespace behaviors
} // namespace tamaf

#endif // TAMAF_CLOSING_BEHAVIOR_H
