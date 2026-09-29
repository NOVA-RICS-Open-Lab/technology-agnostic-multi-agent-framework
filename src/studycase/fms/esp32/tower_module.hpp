#ifndef TOWER_MODULE_HPP
#define TOWER_MODULE_HPP

#include <tamaf.h>
#include "constants.h"
#include <HTTPClient.h>

class TowerAgent;

class CNETSkilledResponder : public tamaf::protocols::FIPAContractNetResponder {
private:
    TowerAgent* towerAgent;

protected:
    tamaf::messaging::ACLMessage handleCfp(const tamaf::messaging::ACLMessage& cfpMessage) override;
    tamaf::messaging::ACLMessage handleAcceptProposal(const tamaf::messaging::ACLMessage& cfpMessage, const tamaf::messaging::ACLMessage& proposeMessage, const tamaf::messaging::ACLMessage& acceptMessage) override;

public:
    CNETSkilledResponder(TowerAgent* agent, const tamaf::messaging::ACLMessageTemplate& tmpl);
};

class REQExecuteSkillResponder : public tamaf::protocols::FIPARequestResponder {
private:
    TowerAgent* towerAgent;

protected:
    tamaf::messaging::ACLMessage PrepareResponse(const tamaf::messaging::ACLMessage& request) override;
    tamaf::messaging::ACLMessage PrepareResultNotification(const tamaf::messaging::ACLMessage& request, const tamaf::messaging::ACLMessage& response) override;

public:
    REQExecuteSkillResponder(TowerAgent* agent, const tamaf::messaging::ACLMessageTemplate& tpl);
};

class TowerAgent : public tamaf::core::Agent {
private:
    std::vector<SkillInfo> mySkills;
    String resourceType;
    Location myLocation;

public:
    TowerAgent(const String& name, const std::vector<SkillInfo>& skills, const String& resType, Location loc, const String& ip = "0.0.0.0");

    void Setup(tamaf::ema::EMAInteraction* emaInteraction) override;

    Location getMyLocation() const { return myLocation; }
    const std::vector<SkillInfo>& getMySkills() const { return mySkills; }
    
    bool executeSkill(const String& skillName);
};

#endif // TOWER_MODULE_HPP
