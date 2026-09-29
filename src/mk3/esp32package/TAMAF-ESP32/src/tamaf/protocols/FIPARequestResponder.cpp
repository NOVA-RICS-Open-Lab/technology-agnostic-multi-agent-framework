#include "FIPARequestResponder.h"
#include "tamaf/core/Agent.h"

namespace tamaf {
namespace protocols {

FIPARequestResponder::FIPARequestResponder(const tamaf::messaging::ACLMessageTemplate& tpl)
    : CyclicBehavior(), msgTemplate(tpl) {}

FIPARequestResponder::FIPARequestResponder(tamaf::core::Agent* agent, const tamaf::messaging::ACLMessageTemplate& tpl)
    : CyclicBehavior(agent), msgTemplate(tpl) {}

void FIPARequestResponder::Action() {
    if (!myAgent) return;

    tamaf::messaging::ACLMessage request = Receive(msgTemplate);
    
    // Prepare response (AGREE, REFUSE, NOT_UNDERSTOOD)
    tamaf::messaging::ACLMessage response = PrepareResponse(request);
    Send(response);
    
    // If agreed, perform action and send result
    if (response.getPerformative() == tamaf::enums::Performative::AGREE) {
        tamaf::messaging::ACLMessage result = PrepareResultNotification(request, response);
        Send(result);
    }
}

tamaf::messaging::ACLMessage FIPARequestResponder::PrepareResponse(const tamaf::messaging::ACLMessage& request) {
    return handleRequest(request);
}

tamaf::messaging::ACLMessage FIPARequestResponder::handleRequest(const tamaf::messaging::ACLMessage& request) {
    // Default implementation: just agree
    tamaf::messaging::ACLMessage agree(tamaf::enums::Performative::AGREE);
    agree.addReceiver(request.getSender());
    agree.setConversationId(request.getConversationId());
    return agree;
}

tamaf::messaging::ACLMessage FIPARequestResponder::PrepareResultNotification(const tamaf::messaging::ACLMessage& request, const tamaf::messaging::ACLMessage& response) {
    return prepareResultNotification(request, response);
}

tamaf::messaging::ACLMessage FIPARequestResponder::prepareResultNotification(const tamaf::messaging::ACLMessage& request, const tamaf::messaging::ACLMessage& response) {
    // Default implementation: inform success
    tamaf::messaging::ACLMessage inform(tamaf::enums::Performative::INFORM);
    inform.addReceiver(request.getSender());
    inform.setConversationId(request.getConversationId());
    return inform;
}

} // namespace protocols
} // namespace tamaf
