#ifndef TAMAF_WAKERBEHAVIOR_H
#define TAMAF_WAKERBEHAVIOR_H

#include "Behavior.h"

namespace tamaf {
namespace behaviors {

class WakerBehavior : public Behavior {
private:
    unsigned long wakeupTime;
    unsigned long timeoutPeriod;
    bool waken;

protected:
    virtual void OnWake() = 0;

public:
    WakerBehavior(unsigned long timeout_ms);
    WakerBehavior(tamaf::core::Agent* agent, unsigned long timeout_ms);
    virtual ~WakerBehavior() = default;

    void OnStart() override;
    void Action() override;
    bool Done() override;
};

} // namespace behaviors
} // namespace tamaf

#endif // TAMAF_WAKERBEHAVIOR_H
