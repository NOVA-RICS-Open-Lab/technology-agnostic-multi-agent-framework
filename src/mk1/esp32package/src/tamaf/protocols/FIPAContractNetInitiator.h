#ifndef TAMAF_FIPACONTRACTNETINITIATOR_H
#define TAMAF_FIPACONTRACTNETINITIATOR_H

#include "tamaf/behaviors/Behavior.h"
#include "tamaf/messaging/ACLMessage.h"
#include <vector>

namespace tamaf {
namespace protocols {

class FIPAContractNetInitiator : public tamaf::behaviors::Behavior {
public:
    enum State {
        SEND_CFP,
        WAIT_PROPOSALS,
        EVALUATE_PROPOSALS,
        WAIT_INFORMS,
        FINISHED
    };

private:
    tamaf::messaging::ACLMessage cfp;
    std::vector<tamaf::messaging::ACLMessage> receivedProposals;
    size_t expectedProposals;
    
protected:
    State currentState;
    virtual void handlePropose(const tamaf::messaging::ACLMessage& proposeMsg);
    virtual void handleRefuse(const tamaf::messaging::ACLMessage& refuseMsg);
    virtual void handleProposals(const std::vector<tamaf::messaging::ACLMessage>& proposals, std::vector<tamaf::messaging::ACLMessage>& responses);
    virtual void handleInform(const tamaf::messaging::ACLMessage& informMsg);
    virtual void handleFailure(const tamaf::messaging::ACLMessage& failureMsg);

public:
    FIPAContractNetInitiator(const tamaf::messaging::ACLMessage& cfpMsg);
    virtual ~FIPAContractNetInitiator() = default;

    void Action() override;
    bool Done() override;
};

} // namespace protocols
} // namespace tamaf

#endif // TAMAF_FIPACONTRACTNETINITIATOR_H
