#include <Arduino.h>
#include <WiFi.h>
#include <tamaf.h>

#define WIFI_SSID "your_ssid_here"
#define WIFI_PASSWORD "your_password_here"

// Define a custom Request Responder Behavior
class MyRequestResponder : public tamaf::protocols::FIPARequestResponder {
public:
    MyRequestResponder(const tamaf::messaging::ACLMessageTemplate& tmpl)
        : tamaf::protocols::FIPARequestResponder(tmpl) {}

protected:
    tamaf::messaging::ACLMessage handleRequest(const tamaf::messaging::ACLMessage& request) override {
        Serial.print("[ESP32 Request Responder] Received REQUEST: ");
        Serial.println(request.getContent().c_str());
        
        tamaf::messaging::ACLMessage reply = request.createReply();
        reply.setPerformative(tamaf::enums::Performative::AGREE);
        reply.setContent("I agree to perform the task.");
        Serial.println("[ESP32 Request Responder] Sending AGREE.");
        return reply;
    }

    tamaf::messaging::ACLMessage prepareResultNotification(const tamaf::messaging::ACLMessage& request, const tamaf::messaging::ACLMessage& response) override {
        Serial.println("[ESP32 Request Responder] Performing the task...");
        delay(1000); // Simulate work
        
        tamaf::messaging::ACLMessage reply = request.createReply();
        reply.setPerformative(tamaf::enums::Performative::INFORM);
        reply.setContent("42");
        Serial.println("[ESP32 Request Responder] Task completed. Sending INFORM.");
        return reply;
    }
};

class ResponderAgent : public tamaf::core::Agent {
public:
    ResponderAgent(const String& name, const String& ip) 
        : Agent(name.c_str(), tamaf::types::Address(ip, 4000)) {}

    void Setup(tamaf::ema::EMAInteraction* emaInteraction) override {
        // Empty
    }
};

ResponderAgent* agent1;
bool behaviorAdded = false;

void setup() {
    Serial.begin(115200);
    delay(1000);
    WiFi.mode(WIFI_STA); 
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
    Serial.println();
    Serial.println(WiFi.localIP());
    
    agent1 = new ResponderAgent("ResponderESP32", WiFi.localIP().toString());
    agent1->setEMAAddress(tamaf::types::Address("192.168.0.10", 4000));
    agent1->Start();
}

void loop() {
    if (agent1) {
        agent1->Loop();
        
        if (!behaviorAdded && millis() > 15000) {
            behaviorAdded = true;
            tamaf::messaging::ACLMessageTemplate tmpl;
            tmpl.setPerformative(tamaf::enums::Performative::REQUEST);

            agent1->AddBehavior(new MyRequestResponder(tmpl));
            Serial.println("Added Request Responder Behavior.");
        }
    }
    delay(10);
}
