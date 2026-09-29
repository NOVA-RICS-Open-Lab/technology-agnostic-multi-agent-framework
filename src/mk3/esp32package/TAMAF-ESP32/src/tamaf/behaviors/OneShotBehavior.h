#ifndef TAMAF_ONESHOTBEHAVIOR_H
#define TAMAF_ONESHOTBEHAVIOR_H

#include "Behavior.h"

namespace tamaf {
namespace behaviors {

class OneShotBehavior : public Behavior {
public:
    OneShotBehavior();
    OneShotBehavior(tamaf::core::Agent* agent);
    virtual ~OneShotBehavior() = default;

    bool Done() override;
};

} // namespace behaviors
} // namespace tamaf

#endif // TAMAF_ONESHOTBEHAVIOR_H
