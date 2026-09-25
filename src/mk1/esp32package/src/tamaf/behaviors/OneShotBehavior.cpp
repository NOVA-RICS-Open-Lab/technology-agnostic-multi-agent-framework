#include "OneShotBehavior.h"

namespace tamaf {
namespace behaviors {

OneShotBehavior::OneShotBehavior() : Behavior() {}

bool OneShotBehavior::Done() {
    return true; // Always finishes after one execution
}

} // namespace behaviors
} // namespace tamaf
