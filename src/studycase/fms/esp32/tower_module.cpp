#include "tower_module.hpp"

// CNETSkilledResponder
CNETSkilledResponder::CNETSkilledResponder(TowerAgent* agent, const tamaf::messaging::ACLMessageTemplate& tmpl)
    : FIPAContractNetResponder(tmpl), towerAgent(agent) {}

tamaf::messaging::ACLMessage CNETSkilledResponder::handleCfp(const tamaf::messaging::ACLMessage& cfpMessage) {
    tamaf::messaging::ACLMessage propose(tamaf::enums::Performative::PROPOSE);
    propose.addReceiver(cfpMessage.getSender());
    propose.setConversationId(cfpMessage.getConversationId());
    propose.setOntology(cfpMessage.getOntology());
    propose.setContent("1"); // Proposal cost/score
    return propose;
}

tamaf::messaging::ACLMessage CNETSkilledResponder::handleAcceptProposal(const tamaf::messaging::ACLMessage& cfpMessage, const tamaf::messaging::ACLMessage& proposeMessage, const tamaf::messaging::ACLMessage& acceptMessage) {
    tamaf::messaging::ACLMessage inform(tamaf::enums::Performative::INFORM);
    inform.addReceiver(acceptMessage.getSender());
    inform.setConversationId(acceptMessage.getConversationId());
    inform.setOntology(acceptMessage.getOntology());
    if (towerAgent) {
        inform.setContent(locationToString(towerAgent->getMyLocation()).c_str());
    } else {
        inform.setContent("D");
    }
    return inform;
}

// REQExecuteSkillResponder
REQExecuteSkillResponder::REQExecuteSkillResponder(TowerAgent* agent, const tamaf::messaging::ACLMessageTemplate& tpl)
    : FIPARequestResponder(tpl), towerAgent(agent) {}

tamaf::messaging::ACLMessage REQExecuteSkillResponder::PrepareResponse(const tamaf::messaging::ACLMessage& request) {
    tamaf::messaging::ACLMessage agree(tamaf::enums::Performative::AGREE);
    agree.addReceiver(request.getSender());
    agree.setConversationId(request.getConversationId());
    agree.setOntology(request.getOntology());
    return agree;
}

tamaf::messaging::ACLMessage REQExecuteSkillResponder::PrepareResultNotification(const tamaf::messaging::ACLMessage& request, const tamaf::messaging::ACLMessage& response) {
    String skillName = request.getContent().c_str();
    Serial.print("[TowerAgent] Executing skill: ");
    Serial.println(skillName);
    
    if (towerAgent) {
        towerAgent->executeSkill(skillName);
    } else {
        delay(500); // Simulate skill execution delay
    }
    
    tamaf::messaging::ACLMessage inform(tamaf::enums::Performative::INFORM);
    inform.addReceiver(request.getSender());
    inform.setConversationId(request.getConversationId());
    inform.setOntology(request.getOntology());
    inform.setContent("SUCCESS");
    return inform;
}

// TowerAgent
TowerAgent::TowerAgent(const String& name, const std::vector<SkillInfo>& skills, const String& resType, Location loc, const String& ip)
    : Agent(formatAgentName(name).c_str(), tamaf::types::Address(ip, 4001)),
      mySkills(skills), resourceType(resType), myLocation(loc) {
    for (const auto& skill : mySkills) {
        tamaf::types::ServiceDescription sd(skill.name.c_str(), skill.serviceType.c_str());
        agentDescription.addService(sd);
    }
}

void TowerAgent::Setup(tamaf::ema::EMAInteraction* emaInteraction) {
    // ContractNet Responder
    tamaf::messaging::ACLMessageTemplate cnetTpl;
    cnetTpl.setPerformative(tamaf::enums::Performative::CFP);
    cnetTpl.setOntology(ONTOLOGY_NEGOTIATE_NEXT_RESOURCE);
    AddBehavior(new CNETSkilledResponder(this, cnetTpl));

    // Request Responder
    tamaf::messaging::ACLMessageTemplate reqTpl;
    reqTpl.setPerformative(tamaf::enums::Performative::REQUEST);
    reqTpl.setOntology(ONTOLOGY_REQUEST_EXECUTE_SKILL);
    AddBehavior(new REQExecuteSkillResponder(this, reqTpl));
}

bool TowerAgent::executeSkill(const String& skillName) {
    String skillUrl = "";
    for (const auto& skill : mySkills) {
        if (skill.name == skillName) {
            skillUrl = skill.url;
            break;
        }
    }
    
    if (skillUrl.length() > 0 && WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(skillUrl);
        int httpCode = http.GET();
        http.end();
        return (httpCode > 0);
    }
    
    delay(500);
    return true;
}
