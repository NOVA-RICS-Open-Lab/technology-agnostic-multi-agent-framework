#include <Arduino.h>
#include <WiFi.h>
#include <tamaf.h>

#define WIFI_SSID "your_ssid_here"
#define WIFI_PASSWORD "your_password_here"

// Define a custom Contract Net Responder Behavior
class MyCNResponder : public tamaf::protocols::FIPAContractNetResponder {
public:
    MyCNResponder(const tamaf::messaging::ACLMessageTemplate& tmpl)
        : tamaf::protocols::FIPAContractNetResponder(tmpl) {}

protected:
    tamaf::messaging::ACLMessage handleCfp(const tamaf::messaging::ACLMessage& cfp) override {
        Serial.print("[ESP32 CN Responder] Received CFP: ");
        Serial.println(cfp.getContent().c_str());
        
        tamaf::messaging::ACLMessage reply = cfp.createReply();
        reply.setPerformative(tamaf::enums::Performative::PROPOSE);
        reply.setContent("I can do it for 15 bucks.");
        Serial.println("[ESP32 CN Responder] Sending PROPOSE.");
        return reply;
    }

    void handleRejectProposal(const tamaf::messaging::ACLMessage& cfp,
                              const tamaf::messaging::ACLMessage& propose,
                              const tamaf::messaging::ACLMessage& reject) override {
        Serial.println("[ESP32 CN Responder] Proposal rejected.");
    }

    tamaf::messaging::ACLMessage handleAcceptProposal(const tamaf::messaging::ACLMessage& cfp,
                                                      const tamaf::messaging::ACLMessage& propose,
                                                      const tamaf::messaging::ACLMessage& accept) override {
        Serial.println("[ESP32 CN Responder] Proposal ACCEPTED! Doing work...");
        delay(1000); // Simulate work
        
        tamaf::messaging::ACLMessage reply = cfp.createReply();
        reply.setPerformative(tamaf::enums::Performative::INFORM);
        reply.setContent("Task completed successfully.");
        Serial.println("[ESP32 CN Responder] Work done, sending INFORM.");
        return reply;
    }
};

class CNAgent : public tamaf::core::Agent {
public:
    CNAgent(const String& name, const String& ip) 
        : Agent(name.c_str(), tamaf::types::Address(ip, 4000)) {}

    void Setup(tamaf::ema::EMAInteraction* emaInteraction) override {
        // Wait for registration before adding complex behaviors
    }
};

CNAgent* agent1;
bool behaviorAdded = false;

void setup() {
    Serial.begin(115200);
    delay(1000);
    WiFi.mode(WIFI_STA); 
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
    Serial.println();
    Serial.println(WiFi.localIP());
    
    agent1 = new CNAgent("ResponderESP32", WiFi.localIP().toString());
    agent1->setEMAAddress(tamaf::types::Address("192.168.0.10", 4000));
    agent1->Start();
}

void loop() {
    if (agent1) {
        agent1->Loop();
        
        // Add the CN responder after giving some time to register to the EMA
        if (!behaviorAdded && millis() > 15000) {
            behaviorAdded = true;
            tamaf::messaging::ACLMessageTemplate tmpl;
            tmpl.setPerformative(tamaf::enums::Performative::CFP);

            agent1->AddBehavior(new MyCNResponder(tmpl));
            Serial.println("Added CN Responder Behavior to listen for CFPs.");
        }
    }
    delay(10);
}
