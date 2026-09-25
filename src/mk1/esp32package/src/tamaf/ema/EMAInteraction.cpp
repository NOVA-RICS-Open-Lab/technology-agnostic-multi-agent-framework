#include "EMAInteraction.h"
#include "AgentManagementOntology.h"
#include "tamaf/behaviors/Behavior.h"
#include "tamaf/core/Agent.h"
#include "tamaf/messaging/ACLMessageTemplate.h"
#include "defines.h"
#include <Arduino.h>

namespace tamaf {
namespace ema {

EMAInteraction::EMAInteraction(tamaf::behaviors::Behavior* behavior) 
    : behavior(behavior), messageSent(false) {
    // Generate simple pseudo-random conversation ID
    conversationID = "ema-" + String(esp_random());
}

bool EMAInteraction::SendToEMA(const tamaf::messaging::ACLMessage& aclMessage) {
    return behavior->getAgent()->Send(aclMessage);
}

void EMAInteraction::KeepAliveAgent() {
    std::vector<tamaf::types::AgentDescription> descriptions = { behavior->getAgent()->GetAgentDescription() };
    
    tamaf::messaging::ACLMessage aclMessage;
    aclMessage.setSender(behavior->getAgent()->GetAgentID());
    aclMessage.addReceiver(tamaf::types::AgentID(DEFAULT_EMA_NAME, behavior->getAgent()->GetEMAAddress()));
    aclMessage.setPerformative(tamaf::enums::Performative::KEEPALIVE);
    aclMessage.setOntology(DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME);
    aclMessage.setContent(AgentManagementOntology::CreateAgentDescriptionMessageContent(tamaf::actions::KEEPALIVE, descriptions));
    
    SendToEMA(aclMessage);
}

std::optional<tamaf::messaging::ACLMessage> EMAInteraction::RegisterAgent() {
    if (!messageSent) {
        std::vector<tamaf::types::AgentDescription> descriptions = { behavior->getAgent()->GetAgentDescription() };
        
        tamaf::messaging::ACLMessage registerACLMessage;
        registerACLMessage.setSender(behavior->getAgent()->GetAgentID());
        registerACLMessage.addReceiver(tamaf::types::AgentID(DEFAULT_EMA_NAME, behavior->getAgent()->GetEMAAddress()));
        registerACLMessage.setPerformative(tamaf::enums::Performative::REQUEST);
        registerACLMessage.setOntology(DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME);
        registerACLMessage.setContent(AgentManagementOntology::CreateAgentDescriptionMessageContent(tamaf::actions::REGISTER, descriptions));
        registerACLMessage.setConversationId(conversationID);

        Serial.println("EMAInteraction: Sending Register Message to EMA...");
        if (SendToEMA(registerACLMessage)) {
            Serial.println("EMAInteraction: SendToEMA returned true.");
            messageSent = true;
        } else {
            Serial.println("EMAInteraction: SendToEMA returned false.");
        }
    }

    if (messageSent) {
        tamaf::messaging::ACLMessageTemplate tmpl;
        tmpl.addSender(tamaf::types::AgentID(DEFAULT_EMA_NAME, behavior->getAgent()->GetEMAAddress()));
        tmpl.setPerformative(tamaf::enums::Performative::INFORM);
        tmpl.setOntology(DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME);
        tmpl.setConversationId(conversationID);

        std::optional<tamaf::messaging::ACLMessage> responseMessage = behavior->getAgent()->Receive(tmpl); // Wait! In python behavior.Receive(), in C++ Agent::Receive() or we wait. Ah, behavior->getAgent()->Receive()
        if (responseMessage.has_value()) {
            if (AgentManagementOntology::Validate(responseMessage.value().getContent())) {
                messageSent = false;
                return responseMessage;
            }
        }
    }
    return std::nullopt;
}

std::optional<tamaf::messaging::ACLMessage> EMAInteraction::DeRegisterAgent() {
    if (!messageSent) {
        std::vector<tamaf::types::AgentDescription> descriptions = { behavior->getAgent()->GetAgentDescription() };
        
        tamaf::messaging::ACLMessage deregisterACLMessage;
        deregisterACLMessage.setSender(behavior->getAgent()->GetAgentID());
        deregisterACLMessage.addReceiver(tamaf::types::AgentID(DEFAULT_EMA_NAME, behavior->getAgent()->GetEMAAddress()));
        deregisterACLMessage.setPerformative(tamaf::enums::Performative::REQUEST);
        deregisterACLMessage.setOntology(DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME);
        deregisterACLMessage.setContent(AgentManagementOntology::CreateAgentDescriptionMessageContent(tamaf::actions::DEREGISTER, descriptions));
        deregisterACLMessage.setConversationId(conversationID);

        if (SendToEMA(deregisterACLMessage)) {
            messageSent = true;
        }
    }

    if (messageSent) {
        tamaf::messaging::ACLMessageTemplate tmpl;
        tmpl.addSender(tamaf::types::AgentID(DEFAULT_EMA_NAME, behavior->getAgent()->GetEMAAddress()));
        tmpl.setPerformative(tamaf::enums::Performative::INFORM);
        tmpl.setOntology(DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME);
        tmpl.setConversationId(conversationID);

        std::optional<tamaf::messaging::ACLMessage> responseMessage = behavior->getAgent()->Receive(tmpl);
        if (responseMessage.has_value()) {
            if (AgentManagementOntology::Validate(responseMessage.value().getContent())) {
                messageSent = false;
                return responseMessage;
            }
        }
    }
    return std::nullopt;
}

std::optional<tamaf::messaging::ACLMessage> EMAInteraction::ModifyAgent() {
    if (!messageSent) {
        std::vector<tamaf::types::AgentDescription> descriptions = { behavior->getAgent()->GetAgentDescription() };
        
        tamaf::messaging::ACLMessage modifyACLMessage;
        modifyACLMessage.setSender(behavior->getAgent()->GetAgentID());
        modifyACLMessage.addReceiver(tamaf::types::AgentID(DEFAULT_EMA_NAME, behavior->getAgent()->GetEMAAddress()));
        modifyACLMessage.setPerformative(tamaf::enums::Performative::REQUEST);
        modifyACLMessage.setOntology(DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME);
        modifyACLMessage.setContent(AgentManagementOntology::CreateAgentDescriptionMessageContent(tamaf::actions::MODIFY, descriptions));
        modifyACLMessage.setConversationId(conversationID);

        if (SendToEMA(modifyACLMessage)) {
            messageSent = true;
        }
    }

    if (messageSent) {
        tamaf::messaging::ACLMessageTemplate tmpl;
        tmpl.addSender(tamaf::types::AgentID(DEFAULT_EMA_NAME, behavior->getAgent()->GetEMAAddress()));
        tmpl.setPerformative(tamaf::enums::Performative::INFORM);
        tmpl.setOntology(DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME);
        tmpl.setConversationId(conversationID);

        std::optional<tamaf::messaging::ACLMessage> responseMessage = behavior->getAgent()->Receive(tmpl);
        if (responseMessage.has_value()) {
            if (AgentManagementOntology::Validate(responseMessage.value().getContent())) {
                messageSent = false;
                return responseMessage;
            }
        }
    }
    return std::nullopt;
}

std::optional<std::vector<tamaf::types::AgentDescription>> EMAInteraction::Search(const tamaf::types::AgentDescription& agentDescription) {
    if (!messageSent) {
        std::vector<tamaf::types::AgentDescription> descriptions = { agentDescription };
        
        tamaf::messaging::ACLMessage searchACLMessage;
        searchACLMessage.setSender(behavior->getAgent()->GetAgentID());
        searchACLMessage.addReceiver(tamaf::types::AgentID(DEFAULT_EMA_NAME, behavior->getAgent()->GetEMAAddress()));
        searchACLMessage.setPerformative(tamaf::enums::Performative::REQUEST);
        searchACLMessage.setOntology(DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME);
        searchACLMessage.setContent(AgentManagementOntology::CreateAgentDescriptionMessageContent(tamaf::actions::GLOBAL_SEARCH, descriptions));
        searchACLMessage.setConversationId(conversationID);

        if (SendToEMA(searchACLMessage)) {
            messageSent = true;
        }
    }

    if (messageSent) {
        tamaf::messaging::ACLMessageTemplate tmpl;
        tmpl.addSender(tamaf::types::AgentID(DEFAULT_EMA_NAME, behavior->getAgent()->GetEMAAddress()));
        tmpl.setPerformative(tamaf::enums::Performative::INFORM);
        tmpl.setOntology(DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME);
        tmpl.setConversationId(conversationID);

        std::optional<tamaf::messaging::ACLMessage> responseMessage = behavior->getAgent()->Receive(tmpl);
        if (responseMessage.has_value()) {
            if (AgentManagementOntology::Validate(responseMessage.value().getContent())) {
                messageSent = false;
                return AgentManagementOntology::GetAgentDescriptionsFromAttributes(responseMessage.value().getContent());
            }
        }
    }
    return std::nullopt;
}

std::optional<std::vector<tamaf::types::AgentDescription>> EMAInteraction::LocalSearch(const tamaf::types::AgentDescription& agentDescription) {
    if (!messageSent) {
        std::vector<tamaf::types::AgentDescription> descriptions = { agentDescription };
        
        tamaf::messaging::ACLMessage searchACLMessage;
        searchACLMessage.setSender(behavior->getAgent()->GetAgentID());
        searchACLMessage.addReceiver(tamaf::types::AgentID(DEFAULT_EMA_NAME, behavior->getAgent()->GetEMAAddress()));
        searchACLMessage.setPerformative(tamaf::enums::Performative::REQUEST);
        searchACLMessage.setOntology(DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME);
        searchACLMessage.setContent(AgentManagementOntology::CreateAgentDescriptionMessageContent(tamaf::actions::LOCAL_SEARCH, descriptions));
        searchACLMessage.setConversationId(conversationID);

        if (SendToEMA(searchACLMessage)) {
            messageSent = true;
        }
    }

    if (messageSent) {
        tamaf::messaging::ACLMessageTemplate tmpl;
        tmpl.addSender(tamaf::types::AgentID(DEFAULT_EMA_NAME, behavior->getAgent()->GetEMAAddress()));
        tmpl.setPerformative(tamaf::enums::Performative::INFORM);
        tmpl.setOntology(DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME);
        tmpl.setConversationId(conversationID);

        std::optional<tamaf::messaging::ACLMessage> responseMessage = behavior->getAgent()->Receive(tmpl);
        if (responseMessage.has_value()) {
            if (AgentManagementOntology::Validate(responseMessage.value().getContent())) {
                messageSent = false;
                return AgentManagementOntology::GetAgentDescriptionsFromAttributes(responseMessage.value().getContent());
            }
        }
    }
    return std::nullopt;
}

std::optional<std::vector<tamaf::types::AgentDescription>> EMAInteraction::ExternalSearch(const tamaf::types::AgentDescription& agentDescription) {
    if (!messageSent) {
        std::vector<tamaf::types::AgentDescription> descriptions = { agentDescription };
        
        tamaf::messaging::ACLMessage searchACLMessage;
        searchACLMessage.setSender(behavior->getAgent()->GetAgentID());
        searchACLMessage.addReceiver(tamaf::types::AgentID(DEFAULT_EMA_NAME, behavior->getAgent()->GetEMAAddress()));
        searchACLMessage.setPerformative(tamaf::enums::Performative::REQUEST);
        searchACLMessage.setOntology(DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME);
        searchACLMessage.setContent(AgentManagementOntology::CreateAgentDescriptionMessageContent(tamaf::actions::EXTERNAL_SEARCH, descriptions));
        searchACLMessage.setConversationId(conversationID);

        if (SendToEMA(searchACLMessage)) {
            messageSent = true;
        }
    }

    if (messageSent) {
        tamaf::messaging::ACLMessageTemplate tmpl;
        tmpl.addSender(tamaf::types::AgentID(DEFAULT_EMA_NAME, behavior->getAgent()->GetEMAAddress()));
        tmpl.setPerformative(tamaf::enums::Performative::INFORM);
        tmpl.setOntology(DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME);
        tmpl.setConversationId(conversationID);

        std::optional<tamaf::messaging::ACLMessage> responseMessage = behavior->getAgent()->Receive(tmpl);
        if (responseMessage.has_value()) {
            if (AgentManagementOntology::Validate(responseMessage.value().getContent())) {
                messageSent = false;
                return AgentManagementOntology::GetAgentDescriptionsFromAttributes(responseMessage.value().getContent());
            }
        }
    }
    return std::nullopt;
}

} // namespace ema
} // namespace tamaf
