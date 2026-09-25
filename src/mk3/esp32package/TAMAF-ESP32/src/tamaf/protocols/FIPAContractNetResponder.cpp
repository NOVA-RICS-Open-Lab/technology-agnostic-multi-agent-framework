#include "FIPAContractNetResponder.h"
#include "tamaf/core/Agent.h"

namespace tamaf {
namespace protocols {

FIPAContractNetResponder::FIPAContractNetResponder(const tamaf::messaging::ACLMessageTemplate& tmpl)
    : CyclicBehavior(), msgTemplate(tmpl) {}

FIPAContractNetResponder::FIPAContractNetResponder(tamaf::core::Agent* agent, const tamaf::messaging::ACLMessageTemplate& tmpl)
    : CyclicBehavior(agent), msgTemplate(tmpl) {}

void FIPAContractNetResponder::Action() {
    if (!myAgent) return;

    tamaf::messaging::ACLMessage msg = Receive(tamaf::messaging::ACLMessageTemplate([this](const tamaf::messaging::ACLMessage& m) {
        if (this->msgTemplate.Match(m)) return true;
        if (m.getPerformative() == tamaf::enums::Performative::ACCEPT_PROPOSAL || 
            m.getPerformative() == tamaf::enums::Performative::REJECT_PROPOSAL) {
            return this->activeConversations.find(m.getConversationId().c_str()) != this->activeConversations.end();
        }
        return false;
    }));

    if (msgTemplate.Match(msg)) {
        // It's a CFP
        tamaf::messaging::ACLMessage reply = handleCfp(msg);
        
        if (reply.getPerformative() == tamaf::enums::Performative::PROPOSE) {
            activeConversations[msg.getConversationId().c_str()] = msg;
            Send(reply);
        } else if (reply.getPerformative() == tamaf::enums::Performative::REFUSE) {
            Send(reply);
        }
    } else if (msg.getPerformative() == tamaf::enums::Performative::ACCEPT_PROPOSAL || 
               msg.getPerformative() == tamaf::enums::Performative::REJECT_PROPOSAL) {
        
        auto it = activeConversations.find(msg.getConversationId().c_str());
        if (it != activeConversations.end()) {
            tamaf::messaging::ACLMessage cfpMsg = it->second;
            tamaf::messaging::ACLMessage dummyPropose; 
            dummyPropose.setConversationId(msg.getConversationId());
            dummyPropose.setPerformative(tamaf::enums::Performative::PROPOSE);

            if (msg.getPerformative() == tamaf::enums::Performative::ACCEPT_PROPOSAL) {
                tamaf::messaging::ACLMessage informReply = handleAcceptProposal(cfpMsg, dummyPropose, msg);
                if (informReply.getPerformative() != tamaf::enums::Performative::UNKNOWN) {
                    Send(informReply);
                }
            } else if (msg.getPerformative() == tamaf::enums::Performative::REJECT_PROPOSAL) {
                handleRejectProposal(cfpMsg, dummyPropose, msg);
            }
            
            activeConversations.erase(it);
        }
    }
}

tamaf::messaging::ACLMessage FIPAContractNetResponder::handleCfp(const tamaf::messaging::ACLMessage& cfpMessage) {
    return tamaf::messaging::ACLMessage();
}

void FIPAContractNetResponder::handleRejectProposal(const tamaf::messaging::ACLMessage& cfpMessage, const tamaf::messaging::ACLMessage& proposeMessage, const tamaf::messaging::ACLMessage& rejectMessage) {}

tamaf::messaging::ACLMessage FIPAContractNetResponder::handleAcceptProposal(const tamaf::messaging::ACLMessage& cfpMessage, const tamaf::messaging::ACLMessage& proposeMessage, const tamaf::messaging::ACLMessage& acceptMessage) {
    return tamaf::messaging::ACLMessage();
}

} // namespace protocols
} // namespace tamaf
