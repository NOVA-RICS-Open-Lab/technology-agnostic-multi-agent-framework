#ifndef TAMAF_FIPAREQUESTINITIATOR_H
#define TAMAF_FIPAREQUESTINITIATOR_H

#include "tamaf/behaviors/Behavior.h"
#include "tamaf/messaging/ACLMessage.h"
#include <functional>

namespace tamaf {
namespace protocols {

class FIPARequestInitiator : public tamaf::behaviors::Behavior {
public:
    enum State {
        SEND_REQUEST,
        WAIT_AGREE,
        WAIT_INFORM,
        FINISHED
    };

private:
    tamaf::messaging::ACLMessage request;
    
protected:
    State currentState;
    virtual void HandleAgree(const tamaf::messaging::ACLMessage& agreeMsg);
    virtual void HandleRefuse(const tamaf::messaging::ACLMessage& refuseMsg);
    virtual void HandleInform(const tamaf::messaging::ACLMessage& informMsg);
    virtual void HandleFailure(const tamaf::messaging::ACLMessage& failureMsg);

    virtual void handleAgree(const tamaf::messaging::ACLMessage& agreeMsg) {}
    virtual void handleRefuse(const tamaf::messaging::ACLMessage& refuseMsg) {}
    virtual void handleInform(const tamaf::messaging::ACLMessage& informMsg) {}
    virtual void handleFailure(const tamaf::messaging::ACLMessage& failureMsg) {}

public:
    FIPARequestInitiator(const tamaf::messaging::ACLMessage& requestMsg);
    FIPARequestInitiator(tamaf::core::Agent* agent, const tamaf::messaging::ACLMessage& requestMsg);
    virtual ~FIPARequestInitiator() = default;

    void Action() override;
    bool Done() override;
};

} // namespace protocols
} // namespace tamaf

#endif // TAMAF_FIPAREQUESTINITIATOR_H
