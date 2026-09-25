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

void SequentialBehavior::setAgent(tamaf::core::Agent* agent) {
    Behavior::setAgent(agent);
    for (auto b : subBehaviors) {
        if (b) b->setAgent(agent);
    }
}

void SequentialBehavior::AddSubBehavior(Behavior* b) {
    if (myAgent) b->setAgent(myAgent);
    subBehaviors.push_back(b);
}

void SequentialBehavior::OnStart() {
    if (currentIndex < subBehaviors.size()) {
        if (!subBehaviors[currentIndex]->getAgent() && myAgent) {
             subBehaviors[currentIndex]->setAgent(myAgent);
        }
        subBehaviors[currentIndex]->OnStart();
        subBehaviors[currentIndex]->Started();
    }
}

void SequentialBehavior::Action() {
    if (currentIndex < subBehaviors.size()) {
        Behavior* current = subBehaviors[currentIndex];
        
        if (!current->getAgent() && myAgent) {
             current->setAgent(myAgent);
        }
        
        if (!current->IsStarted()) {
            current->OnStart();
            current->Started();
        }

        current->Execute();

        if (current->IsBlocked()) {
            Block();
        }

        if (current->Done()) {
            current->OnEnd();
            currentIndex++;
        }
    }
}

bool SequentialBehavior::Done() {
    return currentIndex >= subBehaviors.size();
}

int SequentialBehavior::OnEnd() {
    currentIndex = 0;
    SetStarted(false);
    return Behavior::OnEnd();
}

} // namespace behaviors
} // namespace tamaf
