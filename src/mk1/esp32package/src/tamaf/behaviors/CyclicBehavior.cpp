#include "CyclicBehavior.h"

namespace tamaf {
namespace behaviors {

CyclicBehavior::CyclicBehavior() : Behavior() {}

bool CyclicBehavior::Done() {
    return false; // Never finishes automatically
}

} // namespace behaviors
} // namespace tamaf
