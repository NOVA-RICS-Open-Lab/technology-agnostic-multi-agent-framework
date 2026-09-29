#ifndef PRODUCT_MODULE_HPP
#define PRODUCT_MODULE_HPP

#include <tamaf.h>
#include "constants.h"
#include <vector>

class ProductAgent;

class NextSkillBehavior : public tamaf::behaviors::SimpleBehavior {
private:
    ProductAgent* productAgent;
    tamaf::ema::EMAInteraction* emaInteraction;
    tamaf::types::AgentDescription searchAD;
    bool requestSent = false;
    bool searchCompleted = false;

public:
    NextSkillBehavior(ProductAgent* agent);
    ~NextSkillBehavior();

    void Action() override;
    bool Done() override { return searchCompleted; }
};

class CNETProposalInitiator : public tamaf::protocols::FIPAContractNetInitiator {
private:
    ProductAgent* productAgent;
    tamaf::ema::EMAInteraction* emaInteraction;

protected:
    void handleProposals(const std::vector<tamaf::messaging::ACLMessage>& proposals, std::vector<tamaf::messaging::ACLMessage>& responses) override;
    void handleInform(const tamaf::messaging::ACLMessage& informMsg) override;

public:
    CNETProposalInitiator(ProductAgent* agent, const tamaf::messaging::ACLMessage& cfpMsg);
    ~CNETProposalInitiator();
};

class REQUESTExecuteTransportInitiator : public tamaf::protocols::FIPARequestInitiator {
private:
    ProductAgent* productAgent;

protected:
    void HandleInform(const tamaf::messaging::ACLMessage& inform) override;
    void HandleRefuse(const tamaf::messaging::ACLMessage& refuse) override;

public:
    REQUESTExecuteTransportInitiator(ProductAgent* agent, const tamaf::messaging::ACLMessage& request);
};

class REQUESTExecuteSkillInitiator : public tamaf::protocols::FIPARequestInitiator {
private:
    ProductAgent* productAgent;

protected:
    void HandleInform(const tamaf::messaging::ACLMessage& inform) override;
    void HandleRefuse(const tamaf::messaging::ACLMessage& refuse) override;

public:
    REQUESTExecuteSkillInitiator(ProductAgent* agent, const tamaf::messaging::ACLMessage& request);
};

class TransportBackBehavior : public tamaf::protocols::FIPARequestInitiator {
private:
    ProductAgent* productAgent;

protected:
    void HandleInform(const tamaf::messaging::ACLMessage& inform) override;
    void HandleFailure(const tamaf::messaging::ACLMessage& failure) override;

public:
    TransportBackBehavior(ProductAgent* agent, const tamaf::messaging::ACLMessage& request);
};

class ProductAgent : public tamaf::core::Agent {
private:
    std::vector<String> myProcess;
    Location myLocation;
    Location nextLocation;
    size_t currentSkillIndex;
    tamaf::types::AgentID resourceExecuter;

public:
    ProductAgent(const String& name, const std::vector<String>& process, Location loc, const String& ip);

    void Setup(tamaf::ema::EMAInteraction* emaInteraction) override;

    String currentSkill() const;
    bool hasNextSkill() const;
    void setNextSkill();

    Location getMyLocation() const { return myLocation; }
    void setMyLocation(Location loc) { myLocation = loc; }
    Location getNextLocation() const { return nextLocation; }
    void setNextLocation(Location loc) { nextLocation = loc; }
    
    const tamaf::types::AgentID& getResourceExecuter() const { return resourceExecuter; }
    void setResourceExecuter(const tamaf::types::AgentID& executer) { resourceExecuter = executer; }
};

#endif // PRODUCT_MODULE_HPP
