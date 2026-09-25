#include "FIPARequestInitiator.h"
#include "tamaf/core/Agent.h"

namespace tamaf {
namespace protocols {

FIPARequestInitiator::FIPARequestInitiator(const tamaf::messaging::ACLMessage& requestMsg)
    : Behavior(), request(requestMsg), currentState(SEND_REQUEST) {}

FIPARequestInitiator::FIPARequestInitiator(tamaf::core::Agent* agent, const tamaf::messaging::ACLMessage& requestMsg)
    : Behavior(), request(requestMsg), currentState(SEND_REQUEST) {
    setAgent(agent);
}

void FIPARequestInitiator::Action() {
    if (!myAgent) return;

    switch (currentState) {
        case SEND_REQUEST:
            Send(request);
            currentState = WAIT_AGREE;
            break;
            
        case WAIT_AGREE: {
            auto reply = Receive(tamaf::messaging::ACLMessageTemplate::MatchConversationId(request.getConversationId()));
            if (reply.getPerformative() == tamaf::enums::Performative::AGREE) {
                HandleAgree(reply);
                currentState = WAIT_INFORM;
            } else if (reply.getPerformative() == tamaf::enums::Performative::REFUSE) {
                HandleRefuse(reply);
                currentState = FINISHED;
            } else if (reply.getPerformative() == tamaf::enums::Performative::FAILURE) {
                HandleFailure(reply);
                currentState = FINISHED;
            } else {
                // Protocol violation
                currentState = FINISHED;
            }
            break;
        }
            
        case WAIT_INFORM: {
            auto reply = Receive(tamaf::messaging::ACLMessageTemplate::MatchConversationId(request.getConversationId()));
            if (reply.getPerformative() == tamaf::enums::Performative::INFORM) {
                HandleInform(reply);
                currentState = FINISHED;
            } else if (reply.getPerformative() == tamaf::enums::Performative::FAILURE) {
                HandleFailure(reply);
                currentState = FINISHED;
            }
            break;
        }
            
        case FINISHED:
            break;
    }
}

bool FIPARequestInitiator::Done() {
    return currentState == FINISHED;
}

void FIPARequestInitiator::HandleAgree(const tamaf::messaging::ACLMessage& agreeMsg) { handleAgree(agreeMsg); }
void FIPARequestInitiator::HandleRefuse(const tamaf::messaging::ACLMessage& refuseMsg) { handleRefuse(refuseMsg); }
void FIPARequestInitiator::HandleInform(const tamaf::messaging::ACLMessage& informMsg) { handleInform(informMsg); }
void FIPARequestInitiator::HandleFailure(const tamaf::messaging::ACLMessage& failureMsg) { handleFailure(failureMsg); }

} // namespace protocols
} // namespace tamaf
