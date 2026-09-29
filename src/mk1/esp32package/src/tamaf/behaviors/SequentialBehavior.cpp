#include "SequentialBehavior.h"
#include "tamaf/core/Agent.h"

namespace tamaf {
namespace behaviors {

SequentialBehavior::SequentialBehavior() : currentIndex(0) {}

SequentialBehavior::SequentialBehavior(tamaf::core::Agent* agent) : currentIndex(0) {
    this->myAgent = agent;
}

SequentialBehavior::~SequentialBehavior() {
    for (auto b : subBehaviors) {
        delete b;
    }
}

void SequentialBehavior::AddSubBehavior(Behavior* b) {
    if (myAgent) b->setAgent(myAgent);
    subBehaviors.push_back(b);
}

void SequentialBehavior::OnStart() {
    currentIndex = 0;
    if (!subBehaviors.empty()) {
        if (!subBehaviors[currentIndex]->getAgent() && myAgent) {
             subBehaviors[currentIndex]->setAgent(myAgent);
        }
        subBehaviors[currentIndex]->OnStart();
    }
}

void SequentialBehavior::Action() {
    if (currentIndex < subBehaviors.size()) {
        Behavior* current = subBehaviors[currentIndex];
        
        if (!current->getAgent() && myAgent) {
             current->setAgent(myAgent);
        }
        
        current->Execute();

        if (current->Done()) {
            current->OnEnd();
            currentIndex++;
            if (currentIndex < subBehaviors.size()) {
                if (!subBehaviors[currentIndex]->getAgent() && myAgent) {
                     subBehaviors[currentIndex]->setAgent(myAgent);
                }
                subBehaviors[currentIndex]->OnStart();
            }
        }
    }
}

bool SequentialBehavior::Done() {
    return currentIndex >= subBehaviors.size();
}

int SequentialBehavior::OnEnd() {
    return Behavior::OnEnd();
}

} // namespace behaviors
} // namespace tamaf
