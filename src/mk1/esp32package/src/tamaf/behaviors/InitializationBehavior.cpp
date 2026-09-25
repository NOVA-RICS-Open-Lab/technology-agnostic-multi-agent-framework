#include "InitializationBehavior.h"
#include "tamaf/core/Agent.h"
#include "tamaf/ema/AgentManagementOntology.h"
#include "defines.h"

namespace tamaf {
namespace behaviors {

InitializationBehavior::InitializationBehavior(tamaf::core::Agent* agent)
    : emaInteraction(this), isRegistered(false), initializationDone(false), startTime(0) {
    setAgent(agent);
}

void InitializationBehavior::Action() {
    if (startTime == 0) {
        startTime = millis();
    }

    if (!isRegistered) {
        if (millis() - startTime > DEFAULT_EMA_CONNECTION_TIMEOUT) {
            Serial.println("Registration Failed: Unable to connect to EMA within timeout. Restarting...");
            delay(1000);
            ESP.restart();
        }

        if (getAgent()->GetAgentID().getName() == DEFAULT_EMA_NAME) {
            getAgent()->Setup(&emaInteraction);
            initializationDone = true;
            return;
        }

        std::optional<tamaf::messaging::ACLMessage> agree = emaInteraction.RegisterAgent();
        
        if (agree.has_value()) {
            String concept = tamaf::ema::AgentManagementOntology::GetConcept(agree.value().getContent());
            
            if (concept == DEFAULT_AGENTMANAGEMENT_ONTOLOGY_INFORM_CONCEPT) {
                int inform = tamaf::ema::AgentManagementOntology::GetInformFromAttributes(agree.value().getContent());
                
                if (inform == DEFAULT_AGENT_ALREADY_EXISTS) {
                    Serial.println("Registration Failed: Agent Already Exists");
                    delay(1000);
                    ESP.restart();
                    return;
                }
                if (inform == DEFAULT_EMA_HAS_NO_MORE_PORTS) {
                    Serial.println("Registration Failed: EMA Has No More Ports");
                    delay(1000);
                    ESP.restart();
                    return;
                }
                if (inform == DEFAULT_OUTSIDE_AGENT) {
                    isRegistered = true;
                }
            }
            
            if (concept == DEFAULT_AGENTMANAGEMENT_ONTOLOGY_AGENTADDRESS_CONCEPT) {
                tamaf::types::Address newAddress = tamaf::ema::AgentManagementOntology::GetAgentAddressFromAttributes(agree.value().getContent());
                // The StopMessageServer and StartTCPServer need to be mapped properly.
                // In C++ MTS, we have Stop() and SetPort() and Start().
                getAgent()->getMTS().Stop();
                getAgent()->getMTS().SetPort(newAddress.getPort());
                getAgent()->getMTS().Start();
                isRegistered = true;
            }
        }
    }

    if (isRegistered) {
        HeartBeatBehavior* heartBeat = new HeartBeatBehavior(getAgent());
        getAgent()->AddBehavior(heartBeat);
        getAgent()->Setup(&emaInteraction);
        initializationDone = true;
    }
}

bool InitializationBehavior::Done() {
    return initializationDone;
}

} // namespace behaviors
} // namespace tamaf
