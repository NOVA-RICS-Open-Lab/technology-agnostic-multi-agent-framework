#include "FIPARequestInitiator.h"
#include "tamaf/core/Agent.h"

namespace tamaf {
namespace protocols {

FIPARequestInitiator::FIPARequestInitiator(const tamaf::messaging::ACLMessage& requestMsg)
    : Behavior(), request(requestMsg), currentState(SEND_REQUEST) {}

void FIPARequestInitiator::Action() {
    if (!myAgent) return;

    switch (currentState) {
        case SEND_REQUEST:
            myAgent->Send(request);
            currentState = WAIT_AGREE;
            break;
            
        case WAIT_AGREE: {
            tamaf::messaging::ACLMessage reply;
            if (myAgent->Receive(reply)) {
                if (reply.getConversationId() == request.getConversationId()) {
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
                } else {
                    // Not for us, put it back or ignore (in real JADE this is handled by templates)
                    // For the sake of the C++ port, we just drop it or we should requeue it.
                    // A proper implementation would use ACLMessageTemplate.
                }
            } else {
                Block(); // Wait for new messages
            }
            break;
        }
            
        case WAIT_INFORM: {
            tamaf::messaging::ACLMessage reply;
            if (myAgent->Receive(reply)) {
                if (reply.getConversationId() == request.getConversationId()) {
                    if (reply.getPerformative() == tamaf::enums::Performative::INFORM) {
                        HandleInform(reply);
                        currentState = FINISHED;
                    } else if (reply.getPerformative() == tamaf::enums::Performative::FAILURE) {
                        HandleFailure(reply);
                        currentState = FINISHED;
                    }
                }
            } else {
                Block();
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

void FIPARequestInitiator::HandleAgree(const tamaf::messaging::ACLMessage& agreeMsg) {}
void FIPARequestInitiator::HandleRefuse(const tamaf::messaging::ACLMessage& refuseMsg) {}
void FIPARequestInitiator::HandleInform(const tamaf::messaging::ACLMessage& informMsg) {}
void FIPARequestInitiator::HandleFailure(const tamaf::messaging::ACLMessage& failureMsg) {}

} // namespace protocols
} // namespace tamaf
