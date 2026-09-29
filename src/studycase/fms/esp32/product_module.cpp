#include "product_module.hpp"

NextSkillBehavior::NextSkillBehavior(ProductAgent* agent)
    : SimpleBehavior(), productAgent(agent), requestSent(false), searchCompleted(false) {
    emaInteraction = new tamaf::ema::EMAInteraction(this);
}

NextSkillBehavior::~NextSkillBehavior() {
    delete emaInteraction;
}

void NextSkillBehavior::Action() {
    if (!productAgent) {
        searchCompleted = true;
        return;
    }

    String skill = productAgent->currentSkill();
    if (skill.length() == 0) {
        Serial.print("[");
        Serial.print(productAgent->GetAgentID().getName());
        Serial.println("] All skills completed.");
        searchCompleted = true;
        return;
    }

    if (!requestSent) {
        Serial.print("[");
        Serial.print(productAgent->GetAgentID().getName());
        Serial.print("] Searching for skill: ");
        Serial.println(skill);

        tamaf::types::ServiceDescription sd(skill.c_str(), "");
        searchAD.addService(sd);
        requestSent = true;
    }

    auto result = emaInteraction->Search(searchAD);
    if (result.has_value()) {
        searchCompleted = true;
        if (result.value().empty()) {
            Serial.print("[");
            Serial.print(productAgent->GetAgentID().getName());
            Serial.println("] No agents found for skill. Retrying in 1s...");
            delay(1000);
            productAgent->AddBehavior(new NextSkillBehavior(productAgent));
        } else {
            tamaf::messaging::ACLMessage cfp(tamaf::enums::Performative::CFP);
            for (const auto& ad : result.value()) {
                cfp.addReceiver(ad.getAgentId());
            }
            cfp.setOntology(ONTOLOGY_NEGOTIATE_NEXT_RESOURCE);
            cfp.setContent(skill.c_str());

            productAgent->AddBehavior(new CNETProposalInitiator(productAgent, cfp));
        }
    }
}

// CNETProposalInitiator
CNETProposalInitiator::CNETProposalInitiator(ProductAgent* agent, const tamaf::messaging::ACLMessage& cfpMsg)
    : FIPAContractNetInitiator(cfpMsg), productAgent(agent) {
    emaInteraction = new tamaf::ema::EMAInteraction(this);
}

CNETProposalInitiator::~CNETProposalInitiator() {
    delete emaInteraction;
}

void CNETProposalInitiator::handleProposals(const std::vector<tamaf::messaging::ACLMessage>& proposals, std::vector<tamaf::messaging::ACLMessage>& responses) {
    if (proposals.empty()) {
        Serial.print("[");
        Serial.print(productAgent->GetAgentID().getName());
        Serial.println("] No proposals received.");
        return;
    }

    // Select the first proposal as best
    const auto& bestProposal = proposals[0];
    tamaf::messaging::ACLMessage accept(tamaf::enums::Performative::ACCEPT_PROPOSAL);
    accept.addReceiver(bestProposal.getSender());
    accept.setConversationId(bestProposal.getConversationId());
    accept.setOntology(bestProposal.getOntology());
    responses.push_back(accept);

    // Reject others
    for (size_t i = 1; i < proposals.size(); ++i) {
        tamaf::messaging::ACLMessage reject(tamaf::enums::Performative::REJECT_PROPOSAL);
        reject.addReceiver(proposals[i].getSender());
        reject.setConversationId(proposals[i].getConversationId());
        reject.setOntology(proposals[i].getOntology());
        responses.push_back(reject);
    }
}

void CNETProposalInitiator::handleInform(const tamaf::messaging::ACLMessage& informMsg) {
    if (!productAgent) return;

    productAgent->setResourceExecuter(informMsg.getSender());
    Location nextLoc = stringToLocation(informMsg.getContent().c_str());
    productAgent->setNextLocation(nextLoc);

    Serial.print("[");
    Serial.print(productAgent->GetAgentID().getName());
    Serial.print("] Proposal accepted. Next location: ");
    Serial.println(locationToString(nextLoc));

    // Search for transport agent
    tamaf::types::AgentDescription searchAD;
    tamaf::types::ServiceDescription sd(DF_SERVICE_NAME_TRANSPORT, DF_SERVICE_TYPE_TRANSPORT);
    searchAD.addService(sd);

    auto result = emaInteraction->Search(searchAD);
    if (!result.has_value() || result.value().empty()) {
        Serial.print("[");
        Serial.print(productAgent->GetAgentID().getName());
        Serial.println("] Transport agent not found!");
        return;
    }

    tamaf::messaging::ACLMessage req(tamaf::enums::Performative::REQUEST);
    req.addReceiver(result.value()[0].getAgentId());
    req.setOntology(ONTOLOGY_REQUEST_TRANSPORT);

    String content = "{\"a\":\"" + locationToString(productAgent->getMyLocation()) + "\",\"b\":\"" + locationToString(nextLoc) + "\"}";
    req.setContent(content.c_str());

    productAgent->AddBehavior(new REQUESTExecuteTransportInitiator(productAgent, req));
}

// REQUESTExecuteTransportInitiator
REQUESTExecuteTransportInitiator::REQUESTExecuteTransportInitiator(ProductAgent* agent, const tamaf::messaging::ACLMessage& request)
    : FIPARequestInitiator(request), productAgent(agent) {}

void REQUESTExecuteTransportInitiator::HandleInform(const tamaf::messaging::ACLMessage& inform) {
    if (!productAgent) return;

    productAgent->setMyLocation(productAgent->getNextLocation());
    Serial.print("[");
    Serial.print(productAgent->GetAgentID().getName());
    Serial.print("] Transport completed. Arrived at ");
    Serial.println(locationToString(productAgent->getMyLocation()));

    tamaf::messaging::ACLMessage req(tamaf::enums::Performative::REQUEST);
    req.addReceiver(productAgent->getResourceExecuter());
    req.setOntology(ONTOLOGY_REQUEST_EXECUTE_SKILL);
    req.setContent(productAgent->currentSkill().c_str());

    productAgent->AddBehavior(new REQUESTExecuteSkillInitiator(productAgent, req));
}

void REQUESTExecuteTransportInitiator::HandleRefuse(const tamaf::messaging::ACLMessage& refuse) {
    if (!productAgent) return;
    Serial.print("[");
    Serial.print(productAgent->GetAgentID().getName());
    Serial.println("] Transport refused. Retrying...");
    productAgent->AddBehavior(new NextSkillBehavior(productAgent));
}

// REQUESTExecuteSkillInitiator
REQUESTExecuteSkillInitiator::REQUESTExecuteSkillInitiator(ProductAgent* agent, const tamaf::messaging::ACLMessage& request)
    : FIPARequestInitiator(request), productAgent(agent) {}

void REQUESTExecuteSkillInitiator::HandleInform(const tamaf::messaging::ACLMessage& inform) {
    if (!productAgent) return;

    Serial.print("[");
    Serial.print(productAgent->GetAgentID().getName());
    Serial.print("] Skill ");
    Serial.print(productAgent->currentSkill());
    Serial.println(" completed.");

    if (productAgent->hasNextSkill()) {
        productAgent->setNextSkill();
        productAgent->AddBehavior(new NextSkillBehavior(productAgent));
    } else {
        Serial.print("[");
        Serial.print(productAgent->GetAgentID().getName());
        Serial.println("] All skills completed. Returning to location A.");

        tamaf::messaging::ACLMessage req(tamaf::enums::Performative::REQUEST);
        req.setOntology(ONTOLOGY_REQUEST_TRANSPORT);
        String content = "{\"a\":\"" + locationToString(productAgent->getMyLocation()) + "\",\"b\":\"A\"}";
        req.setContent(content.c_str());

        productAgent->AddBehavior(new TransportBackBehavior(productAgent, req));
    }
}

void REQUESTExecuteSkillInitiator::HandleRefuse(const tamaf::messaging::ACLMessage& refuse) {
    if (!productAgent) return;
    Serial.print("[");
    Serial.print(productAgent->GetAgentID().getName());
    Serial.println("] Skill execution refused. Retrying...");
    productAgent->AddBehavior(new NextSkillBehavior(productAgent));
}

// TransportBackBehavior
TransportBackBehavior::TransportBackBehavior(ProductAgent* agent, const tamaf::messaging::ACLMessage& request)
    : FIPARequestInitiator(request), productAgent(agent) {}

void TransportBackBehavior::HandleInform(const tamaf::messaging::ACLMessage& inform) {
    if (!productAgent) return;
    Serial.print("[");
    Serial.print(productAgent->GetAgentID().getName());
    Serial.println("] Back at A. Work completed successfully!");
}

void TransportBackBehavior::HandleFailure(const tamaf::messaging::ACLMessage& failure) {
    if (!productAgent) return;
    Serial.print("[");
    Serial.print(productAgent->GetAgentID().getName());
    Serial.println("] Failed to return to A.");
}

// ProductAgent
ProductAgent::ProductAgent(const String& name, const std::vector<String>& process, Location loc, const String& ip)
    : Agent(formatAgentName(name).c_str(), tamaf::types::Address(ip, 4001)),
      myProcess(process), myLocation(loc), nextLocation(loc), currentSkillIndex(0),
      resourceExecuter("", tamaf::types::Address("0.0.0.0", 0)) {}

void ProductAgent::Setup(tamaf::ema::EMAInteraction* emaInteraction) {
    AddBehavior(new NextSkillBehavior(this));
}

String ProductAgent::currentSkill() const {
    if (currentSkillIndex < myProcess.size()) {
        return myProcess[currentSkillIndex];
    }
    return "";
}

bool ProductAgent::hasNextSkill() const {
    return (currentSkillIndex + 1) < myProcess.size();
}

void ProductAgent::setNextSkill() {
    currentSkillIndex++;
}
