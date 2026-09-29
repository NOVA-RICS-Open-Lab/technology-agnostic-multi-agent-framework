#include "FIPAContractNetInitiator.h"
#include "tamaf/core/Agent.h"

namespace tamaf {
namespace protocols {

FIPAContractNetInitiator::FIPAContractNetInitiator(const tamaf::messaging::ACLMessage& cfpMsg)
    : Behavior(), cfp(cfpMsg), expectedProposals(0), currentState(SEND_CFP) {
    if (cfp.getReceivers().size() > 0) {
        expectedProposals = cfp.getReceivers().size();
    }
}

FIPAContractNetInitiator::FIPAContractNetInitiator(tamaf::core::Agent* agent, const tamaf::messaging::ACLMessage& cfpMsg)
    : Behavior(), cfp(cfpMsg), expectedProposals(0), currentState(SEND_CFP) {
    setAgent(agent);
    if (cfp.getReceivers().size() > 0) {
        expectedProposals = cfp.getReceivers().size();
    }
}

void FIPAContractNetInitiator::Action() {
    if (!myAgent) return;

    switch (currentState) {
        case SEND_CFP:
            Send(cfp);
            currentState = WAIT_PROPOSALS;
            break;
            
        case WAIT_PROPOSALS: {
            auto reply = Receive(tamaf::messaging::ACLMessageTemplate::MatchConversationId(cfp.getConversationId()));
            if (reply.getPerformative() == tamaf::enums::Performative::PROPOSE) {
                handlePropose(reply);
                receivedProposals.push_back(reply);
            } else if (reply.getPerformative() == tamaf::enums::Performative::REFUSE) {
                handleRefuse(reply);
                receivedProposals.push_back(reply);
            } else {
                receivedProposals.push_back(reply);
            }
            
            if (receivedProposals.size() >= expectedProposals) {
                currentState = EVALUATE_PROPOSALS;
            }
            break;
        }
            
        case EVALUATE_PROPOSALS: {
            std::vector<tamaf::messaging::ACLMessage> responses;
            handleProposals(receivedProposals, responses);
            
            bool hasAccept = false;
            for (const auto& response : responses) {
                Send(response);
                if (response.getPerformative() == tamaf::enums::Performative::ACCEPT_PROPOSAL) {
                    hasAccept = true;
                }
            }
            
            currentState = hasAccept ? WAIT_INFORMS : FINISHED;
            break;
        }
            
        case WAIT_INFORMS: {
            auto reply = Receive(tamaf::messaging::ACLMessageTemplate::MatchConversationId(cfp.getConversationId()));
            if (reply.getPerformative() == tamaf::enums::Performative::INFORM) {
                handleInform(reply);
                currentState = FINISHED;
            } else if (reply.getPerformative() == tamaf::enums::Performative::FAILURE) {
                handleFailure(reply);
                currentState = FINISHED;
            }
            break;
        }
            
        case FINISHED:
            break;
    }
}

bool FIPAContractNetInitiator::Done() {
    return currentState == FINISHED;
}

void FIPAContractNetInitiator::handlePropose(const tamaf::messaging::ACLMessage& proposeMsg) {}
void FIPAContractNetInitiator::handleRefuse(const tamaf::messaging::ACLMessage& refuseMsg) {}
void FIPAContractNetInitiator::handleProposals(const std::vector<tamaf::messaging::ACLMessage>& proposals, std::vector<tamaf::messaging::ACLMessage>& responses) {}
void FIPAContractNetInitiator::handleInform(const tamaf::messaging::ACLMessage& informMsg) {}
void FIPAContractNetInitiator::handleFailure(const tamaf::messaging::ACLMessage& failureMsg) {}

} // namespace protocols
} // namespace tamaf
