#ifndef TAMAF_SIMPLEBEHAVIOR_H
#define TAMAF_SIMPLEBEHAVIOR_H

#include "Behavior.h"

namespace tamaf {
namespace behaviors {

class SimpleBehavior : public Behavior {
public:
    SimpleBehavior() = default;
    virtual ~SimpleBehavior() = default;

    // Subclasses must implement Action() and Done()
};

} // namespace behaviors
} // namespace tamaf

#endif // TAMAF_SIMPLEBEHAVIOR_H
