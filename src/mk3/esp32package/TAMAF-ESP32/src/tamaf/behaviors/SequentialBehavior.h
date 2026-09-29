#ifndef TAMAF_SEQUENTIALBEHAVIOR_H
#define TAMAF_SEQUENTIALBEHAVIOR_H

#include "Behavior.h"
#include <vector>

namespace tamaf {
namespace behaviors {

class SequentialBehavior : public Behavior {
private:
    std::vector<Behavior*> subBehaviors;
    size_t currentIndex;

public:
    SequentialBehavior();
    SequentialBehavior(tamaf::core::Agent* agent);
    virtual ~SequentialBehavior();

    void AddSubBehavior(Behavior* b);
    
    void OnStart() override;
    void Action() override;
    bool Done() override;
    int OnEnd() override;
    void setAgent(tamaf::core::Agent* agent) override;
};

} // namespace behaviors
} // namespace tamaf

#endif // TAMAF_SEQUENTIALBEHAVIOR_H
