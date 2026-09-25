#include <Arduino.h>
#include <WiFi.h>
#include <tamaf.h>

#define WIFI_SSID "your_ssid_here"
#define WIFI_PASSWORD "your_password_here"

// Define a custom Request Initiator Behavior
class MyRequestInitiator : public tamaf::protocols::FIPARequestInitiator {
public:
    MyRequestInitiator(tamaf::core::Agent* agent, const tamaf::messaging::ACLMessage& request)
        : tamaf::protocols::FIPARequestInitiator(agent, request) {}

protected:
    void handleAgree(const tamaf::messaging::ACLMessage& agree) override {
        Serial.println("[ESP32 Request Initiator] Responder AGREED to perform the task.");
    }

    void handleRefuse(const tamaf::messaging::ACLMessage& refuse) override {
        Serial.println("[ESP32 Request Initiator] Responder REFUSED to perform the task.");
    }

    void handleInform(const tamaf::messaging::ACLMessage& inform) override {
        Serial.print("[ESP32 Request Initiator] Task completed successfully. Result: ");
        Serial.println(inform.getContent().c_str());
    }

    void handleFailure(const tamaf::messaging::ACLMessage& failure) override {
        Serial.println("[ESP32 Request Initiator] Responder FAILED during execution.");
    }
};

class InitiatorAgent : public tamaf::core::Agent {
public:
    InitiatorAgent(const String& name, const String& ip) 
        : Agent(name.c_str(), tamaf::types::Address(ip, 4001)) {}

    void Setup(tamaf::ema::EMAInteraction* emaInteraction) override {
        // Empty
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
            
            tamaf::messaging::ACLMessage request(tamaf::enums::Performative::REQUEST);
            request.addReceiver(tamaf::types::AgentID("ResponderPython@192.168.0.10:4000", tamaf::types::Address("192.168.0.10", 4000)));
            request.setContent("Please calculate the meaning of life.");
            
            agent1->AddBehavior(new MyRequestInitiator(agent1, request));
            Serial.println("Added Request Initiator Behavior.");
        }
    }
    delay(10);
}
