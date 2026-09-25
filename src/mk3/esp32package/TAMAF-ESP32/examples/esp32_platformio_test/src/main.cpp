#include <Arduino.h>
#include <WiFi.h>
#include <tamaf.h>
#include "secrets.h"

// Define a custom Request Initiator Behavior
class MyInitiator : public tamaf::protocols::FIPARequestInitiator {
public:
    MyInitiator(const tamaf::messaging::ACLMessage& request)
        : tamaf::protocols::FIPARequestInitiator(request) {}

protected:
    void HandleAgree(const tamaf::messaging::ACLMessage& agree) override {
        Serial.println("[ESP32 Request Initiator] Server agreed! Waiting for results...");
    }

    void HandleRefuse(const tamaf::messaging::ACLMessage& refuse) override {
        Serial.println("[ESP32 Request Initiator] Server refused the request.");
    }

    void HandleInform(const tamaf::messaging::ACLMessage& inform) override {
        Serial.print("[ESP32 Request Initiator] Success! Server says: ");
        Serial.println(inform.getContent().c_str());
    }

    void HandleFailure(const tamaf::messaging::ACLMessage& failure) override {
        Serial.println("[ESP32 Request Initiator] Server failed to complete the task.");
    }
};

class StarterBehavior : public tamaf::behaviors::SimpleBehavior {
private:
    bool isDone = false;
    tamaf::ema::EMAInteraction* emaInteraction;
public:
    StarterBehavior() {
        emaInteraction = new tamaf::ema::EMAInteraction(this);
    }
    
    ~StarterBehavior() {
        delete emaInteraction;
    }

    void Action() override {
        // Build the target we are searching for
        tamaf::types::AgentID targetAgent("Responder", tamaf::types::Address("0.0.0.0", 0)); // The EMA will match by name
        
        // Use EMAInteraction to perform the search (non-blocking)
        std::optional<std::vector<tamaf::types::AgentDescription>> descriptions = emaInteraction->Search(tamaf::types::AgentDescription(targetAgent));
        
        if (descriptions.has_value()) {
            Serial.print("Found ");
            Serial.print(descriptions.value().size());
            Serial.println(" agents matching 'Responder'.");

            std::vector<tamaf::types::AgentID> receivers;
            for (const auto& desc : descriptions.value()) {
                receivers.push_back(desc.getAgentId());
            }
            
            if (!receivers.empty()) {
                tamaf::messaging::ACLMessage requestMsg(tamaf::enums::Performative::REQUEST);
                for(auto r : receivers) { requestMsg.addReceiver(r); }
                requestMsg.setContent("Please calculate the data!");
                
                myAgent->AddBehavior(new MyInitiator(requestMsg));
                Serial.println("Sent REQUEST to found agent.");
            } else {
                Serial.println("No responder found. Retrying later maybe?");
            }
            isDone = true;
        } else {
            // Still waiting for EMA's search results
            Block();
        }
    }
    
    bool Done() override {
        return isDone;
    }
};

class InitiatorAgent : public tamaf::core::Agent {
public:
    InitiatorAgent(const String& name, const String& ip) 
        : Agent(name.c_str(), tamaf::types::Address(ip, 4001)) {}

    void Setup(tamaf::ema::EMAInteraction* emaInteraction) override {
        AddBehavior(new StarterBehavior());
    }
};

InitiatorAgent* agent1;

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
    }
    delay(10);
    
    static unsigned long startTime = millis();
    if (millis() - startTime > 15000) {
        // Just for logging if it takes too long
    }
}
