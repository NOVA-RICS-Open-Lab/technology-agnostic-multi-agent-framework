#ifndef TAMAF_FSMBEHAVIOR_H
#define TAMAF_FSMBEHAVIOR_H

#include "Behavior.h"
#include <map>
#include <string>

namespace tamaf {
namespace behaviors {

class FSMBehavior : public Behavior {
private:
    struct StateInfo {
        Behavior* behavior;
        std::map<int, std::string> transitions;
        std::string defaultTransition;
        bool isFinal;
    };

    std::map<std::string, StateInfo> states;
    std::string currentState;
    std::string initialState;
    bool isDone;

public:
    FSMBehavior();
    FSMBehavior(tamaf::core::Agent* agent);
    virtual ~FSMBehavior();

    void AddInitialState(Behavior* b, const std::string& name);
    void AddState(Behavior* b, const std::string& name);
    void AddFinalState(Behavior* b, const std::string& name);
    
    void AddTransition(const std::string& source, const std::string& dest, int eventValue);
    void AddDefaultTransition(const std::string& source, const std::string& dest);

    void OnStart() override;
    void Action() override;
    bool Done() override;
};

} // namespace behaviors
} // namespace tamaf

#endif // TAMAF_FSMBEHAVIOR_H
