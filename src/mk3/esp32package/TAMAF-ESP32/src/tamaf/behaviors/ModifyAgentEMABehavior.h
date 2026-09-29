#ifndef TAMAF_MODIFY_AGENT_EMA_BEHAVIOR_H
#define TAMAF_MODIFY_AGENT_EMA_BEHAVIOR_H

#include "tamaf/behaviors/OneShotBehavior.h"
#include "tamaf/ema/EMAInteraction.h"

namespace tamaf {
namespace behaviors {

class ModifyAgentEMABehavior : public OneShotBehavior {
private:
    tamaf::ema::EMAInteraction emaInteraction;

public:
    ModifyAgentEMABehavior();
    ModifyAgentEMABehavior(tamaf::core::Agent* agent);
    virtual ~ModifyAgentEMABehavior() = default;

    void Action() override;
};

} // namespace behaviors
} // namespace tamaf

#endif // TAMAF_MODIFY_AGENT_EMA_BEHAVIOR_H
