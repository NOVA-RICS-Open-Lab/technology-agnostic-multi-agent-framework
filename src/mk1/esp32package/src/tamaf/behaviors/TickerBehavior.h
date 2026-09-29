#ifndef TAMAF_TICKERBEHAVIOR_H
#define TAMAF_TICKERBEHAVIOR_H

#include "Behavior.h"

namespace tamaf {
namespace behaviors {

class TickerBehavior : public Behavior {
private:
    uint32_t period;
    uint32_t lastExecutionTime;

public:
    TickerBehavior(uint32_t periodMillis);
    virtual ~TickerBehavior() = default;

    void Execute() override; // Overrides base to check time
    bool Done() override;
    
    virtual void OnTick() = 0; // The actual logic, implemented by subclass
    void Action() override;    // Maps Action to OnTick
};

} // namespace behaviors
} // namespace tamaf

#endif // TAMAF_TICKERBEHAVIOR_H
