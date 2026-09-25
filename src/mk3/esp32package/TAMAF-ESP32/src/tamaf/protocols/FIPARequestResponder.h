#ifndef TAMAF_FIPAREQUESTRESPONDER_H
#define TAMAF_FIPAREQUESTRESPONDER_H

#include "tamaf/behaviors/CyclicBehavior.h"
#include "tamaf/messaging/ACLMessage.h"
#include "tamaf/messaging/ACLMessageTemplate.h"

namespace tamaf {
namespace protocols {

class FIPARequestResponder : public tamaf::behaviors::CyclicBehavior {
private:
    tamaf::messaging::ACLMessageTemplate msgTemplate;
    
protected:
    // User overrides these to provide business logic
    virtual tamaf::messaging::ACLMessage PrepareResponse(const tamaf::messaging::ACLMessage& request);
    virtual tamaf::messaging::ACLMessage PrepareResultNotification(const tamaf::messaging::ACLMessage& request, const tamaf::messaging::ACLMessage& response);

    virtual tamaf::messaging::ACLMessage handleRequest(const tamaf::messaging::ACLMessage& request);
    virtual tamaf::messaging::ACLMessage prepareResultNotification(const tamaf::messaging::ACLMessage& request, const tamaf::messaging::ACLMessage& response);

public:
    FIPARequestResponder(const tamaf::messaging::ACLMessageTemplate& tpl);
    FIPARequestResponder(tamaf::core::Agent* agent, const tamaf::messaging::ACLMessageTemplate& tpl);
    virtual ~FIPARequestResponder() = default;

    void Action() override;
};

} // namespace protocols
} // namespace tamaf

#endif // TAMAF_FIPAREQUESTRESPONDER_H
