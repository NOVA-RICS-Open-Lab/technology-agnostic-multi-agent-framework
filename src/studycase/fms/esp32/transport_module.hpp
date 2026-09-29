#ifndef TRANSPORT_MODULE_HPP
#define TRANSPORT_MODULE_HPP

#include <tamaf.h>
#include "constants.h"
#include <HTTPClient.h>

class TransportAgent;

class REQExecuteTransportResponder : public tamaf::protocols::FIPARequestResponder {
private:
    TransportAgent* transportAgent;
    Location targetLocation;
    bool isMoving;

protected:
    tamaf::messaging::ACLMessage PrepareResponse(const tamaf::messaging::ACLMessage& request) override;
    tamaf::messaging::ACLMessage PrepareResultNotification(const tamaf::messaging::ACLMessage& request, const tamaf::messaging::ACLMessage& response) override;

public:
    REQExecuteTransportResponder(TransportAgent* agent, const tamaf::messaging::ACLMessageTemplate& tpl);
};

class TransportAgent : public tamaf::core::Agent {
private:
    std::vector<SkillInfo> mySkills;
    String resourceType;
    Location myLocation;
    Location currentProductLocation;

public:
    TransportAgent(const String& name, const std::vector<SkillInfo>& skills, const String& resourceType, Location loc);

    void Setup(tamaf::ema::EMAInteraction* emaInteraction) override;

    Location getCurrentProductLocation() const { return currentProductLocation; }
    void setCurrentProductLocation(Location loc) { currentProductLocation = loc; }
    Location getMyLocation() const { return myLocation; }
    
    bool executeTransportStep(Location fromLoc, Location toLoc);
};

#endif // TRANSPORT_MODULE_HPP
