#ifndef TAMAF_FIPACONTRACTNETRESPONDER_H
#define TAMAF_FIPACONTRACTNETRESPONDER_H

#include "tamaf/behaviors/CyclicBehavior.h"
#include "tamaf/messaging/ACLMessage.h"
#include "tamaf/messaging/ACLMessageTemplate.h"
#include <map>
#include <string>

namespace tamaf {
namespace protocols {

class FIPAContractNetResponder : public tamaf::behaviors::CyclicBehavior {
protected:
    tamaf::messaging::ACLMessageTemplate msgTemplate;
    std::map<std::string, tamaf::messaging::ACLMessage> activeConversations;

    virtual tamaf::messaging::ACLMessage handleCfp(const tamaf::messaging::ACLMessage& cfpMessage);
    virtual void handleRejectProposal(const tamaf::messaging::ACLMessage& cfpMessage, const tamaf::messaging::ACLMessage& proposeMessage, const tamaf::messaging::ACLMessage& rejectMessage);
    virtual tamaf::messaging::ACLMessage handleAcceptProposal(const tamaf::messaging::ACLMessage& cfpMessage, const tamaf::messaging::ACLMessage& proposeMessage, const tamaf::messaging::ACLMessage& acceptMessage);

public:
    FIPAContractNetResponder(const tamaf::messaging::ACLMessageTemplate& tmpl);
    FIPAContractNetResponder(tamaf::core::Agent* agent, const tamaf::messaging::ACLMessageTemplate& tmpl);
    virtual ~FIPAContractNetResponder() = default;

    void Action() override;
};

} // namespace protocols
} // namespace tamaf

#endif // TAMAF_FIPACONTRACTNETRESPONDER_H
