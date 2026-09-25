#ifndef TAMAF_CYCLICBEHAVIOR_H
#define TAMAF_CYCLICBEHAVIOR_H

#include "Behavior.h"

namespace tamaf {
namespace behaviors {

class CyclicBehavior : public Behavior {
public:
    CyclicBehavior();
    virtual ~CyclicBehavior() = default;

    bool Done() override;
};

} // namespace behaviors
} // namespace tamaf

#endif // TAMAF_CYCLICBEHAVIOR_H
