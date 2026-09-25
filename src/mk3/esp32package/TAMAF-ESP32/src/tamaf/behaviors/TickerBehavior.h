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

    bool Done() override;
    
    virtual void OnTick() = 0; // The actual logic, implemented by subclass
    void Action() override;    // Checks period and calls OnTick
};

} // namespace behaviors
} // namespace tamaf

#endif // TAMAF_TICKERBEHAVIOR_H
