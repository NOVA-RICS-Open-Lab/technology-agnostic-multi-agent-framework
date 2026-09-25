#include "FSMBehavior.h"
#include "tamaf/core/Agent.h"

namespace tamaf {
namespace behaviors {

FSMBehavior::FSMBehavior() : isDone(false) {}

FSMBehavior::FSMBehavior(tamaf::core::Agent* agent) : isDone(false) {
    this->myAgent = agent;
}

FSMBehavior::~FSMBehavior() {
    for (auto& pair : states) {
        delete pair.second.behavior;
    }
}

void FSMBehavior::AddInitialState(Behavior* b, const std::string& name) {
    if (myAgent) b->setAgent(myAgent);
    states[name] = {b, {}, "", false};
    initialState = name;
}

void FSMBehavior::AddState(Behavior* b, const std::string& name) {
    if (myAgent) b->setAgent(myAgent);
    states[name] = {b, {}, "", false};
}

void FSMBehavior::AddFinalState(Behavior* b, const std::string& name) {
    if (myAgent) b->setAgent(myAgent);
    states[name] = {b, {}, "", true};
}

void FSMBehavior::AddTransition(const std::string& source, const std::string& dest, int eventValue) {
    states[source].transitions[eventValue] = dest;
}

void FSMBehavior::AddDefaultTransition(const std::string& source, const std::string& dest) {
    states[source].defaultTransition = dest;
}

void FSMBehavior::OnStart() {
    currentState = initialState;
    isDone = false;
    if (states.find(currentState) != states.end()) {
        if (!states[currentState].behavior->getAgent() && myAgent) {
             states[currentState].behavior->setAgent(myAgent);
        }
        states[currentState].behavior->OnStart();
    } else {
        isDone = true;
    }
}

void FSMBehavior::Action() {
    if (isDone || states.find(currentState) == states.end()) {
        isDone = true;
        return;
    }

    StateInfo& info = states[currentState];
    if (!info.behavior->getAgent() && myAgent) {
         info.behavior->setAgent(myAgent);
    }
    
    info.behavior->Execute();

    if (info.behavior->Done()) {
        int eventValue = info.behavior->OnEnd();
        
        if (info.isFinal) {
            isDone = true;
            return;
        }

        std::string nextState = "";
        auto it = info.transitions.find(eventValue);
        if (it != info.transitions.end()) {
            nextState = it->second;
        } else if (!info.defaultTransition.empty()) {
            nextState = info.defaultTransition;
        }

        if (!nextState.empty() && states.find(nextState) != states.end()) {
            currentState = nextState;
            if (!states[currentState].behavior->getAgent() && myAgent) {
                 states[currentState].behavior->setAgent(myAgent);
            }
            states[currentState].behavior->OnStart();
        } else {
            // Invalid transition, finish
            isDone = true;
        }
    }
}

bool FSMBehavior::Done() {
    return isDone;
}

} // namespace behaviors
} // namespace tamaf
