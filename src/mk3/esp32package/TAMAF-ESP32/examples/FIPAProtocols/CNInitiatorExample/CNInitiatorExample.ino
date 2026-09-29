#include <Arduino.h>
#include <WiFi.h>
#include <tamaf.h>

#define WIFI_SSID "your_ssid_here"
#define WIFI_PASSWORD "your_password_here"

// Define a custom Contract Net Initiator Behavior
class MyCNInitiator : public tamaf::protocols::FIPAContractNetInitiator {
public:
    MyCNInitiator(tamaf::core::Agent* agent, const tamaf::messaging::ACLMessage& cfp)
        : tamaf::protocols::FIPAContractNetInitiator(agent, cfp) {}

protected:
    void handlePropose(const tamaf::messaging::ACLMessage& propose) override {
        Serial.print("[ESP32 CN Initiator] Received PROPOSE from ");
        Serial.print(propose.getSender().getName().c_str());
        Serial.print(": ");
        Serial.println(propose.getContent().c_str());
    }

    void handleRefuse(const tamaf::messaging::ACLMessage& refuse) override {
        Serial.println("[ESP32 CN Initiator] Received REFUSE.");
    }

    void handleProposals(const std::vector<tamaf::messaging::ACLMessage>& proposals, std::vector<tamaf::messaging::ACLMessage>& responses) override {
        Serial.print("[ESP32 CN Initiator] Deadline reached! Evaluating ");
        Serial.print(proposals.size());
        Serial.println(" bids...");

        for (const auto& propose : proposals) {
            tamaf::messaging::ACLMessage reply = propose.createReply();
            reply.setPerformative(tamaf::enums::Performative::ACCEPT_PROPOSAL);
            reply.setContent("You won the contract! Begin execution.");
            responses.push_back(reply);
        }
    }

    void handleInform(const tamaf::messaging::ACLMessage& inform) override {
        Serial.print("[ESP32 CN Initiator] Success! Winner finished the task: ");
        Serial.println(inform.getContent().c_str());
    }

    void handleFailure(const tamaf::messaging::ACLMessage& failure) override {
        Serial.println("[ESP32 CN Initiator] Task failed.");
    }
};

class InitiatorAgent : public tamaf::core::Agent {
public:
    InitiatorAgent(const String& name, const String& ip) 
        : Agent(name.c_str(), tamaf::types::Address(ip, 4001)) {}

    void Setup(tamaf::ema::EMAInteraction* emaInteraction) override {
        // Wait for registration before adding complex behaviors
    }
};

InitiatorAgent* agent1;
bool behaviorAdded = false;

void setup() {
    Serial.begin(115200);
    delay(1000);
    WiFi.mode(WIFI_STA); 
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
    Serial.println();
    Serial.println(WiFi.localIP());
    
    agent1 = new InitiatorAgent("InitiatorESP32", WiFi.localIP().toString());
    agent1->setEMAAddress(tamaf::types::Address("192.168.0.10", 4000));
    agent1->Start();
}

void loop() {
    if (agent1) {
        agent1->Loop();
        
        if (!behaviorAdded && millis() > 15000) {
            behaviorAdded = true;
            
            tamaf::messaging::ACLMessage cfp(tamaf::enums::Performative::CFP);
            cfp.addReceiver(tamaf::types::AgentID("ResponderPython@192.168.0.10:4000", tamaf::types::Address("192.168.0.10", 4000)));
            cfp.setContent("Auction: Who can calculate this data the fastest?");
            
            agent1->AddBehavior(new MyCNInitiator(agent1, cfp));
            Serial.println("Added CN Initiator Behavior.");
        }
    }
    delay(10);
}
