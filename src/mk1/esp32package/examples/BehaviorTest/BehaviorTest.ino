#include <Arduino.h>
#include <WiFi.h>
#include <tamaf.h>
#include "secrets.h"

// Note: WIFI_SSID and WIFI_PASSWORD are provided by secrets.h

class ThreePingsBehavior : public tamaf::behaviors::SimpleBehavior {
private:
    int pingCount = 0;
public:
    void Action() override {
        Serial.println(String("Ping: ") + String(pingCount));
        pingCount++;
    }
    bool Done() override {
        return pingCount >= 3;
    }
};

class PimbaBehavior : public tamaf::behaviors::OneShotBehavior {
public:
    void Action() override {
        Serial.println("PIMBA");
    }
};

class PingerAgent : public tamaf::core::Agent {
public:
    PingerAgent(const String& name, const String& ip) 
        : Agent(name.c_str(), tamaf::types::Address(ip, 4000)) {}

    void Setup(tamaf::ema::EMAInteraction* emaInteraction) override {
        tamaf::behaviors::SequentialBehavior* seq = new tamaf::behaviors::SequentialBehavior();
        seq->AddSubBehavior(new ThreePingsBehavior());
        seq->AddSubBehavior(new PimbaBehavior());
        seq->AddSubBehavior(new ThreePingsBehavior());
        AddBehavior(seq);
    }
};

class KeyboardInputBehavior : public tamaf::behaviors::OneShotBehavior {
private:
    int number;
public:
    KeyboardInputBehavior(int numToReturn) : number(numToReturn) {}
    void Action() override {
        Serial.println("State executed, returning: " + String(number));
    }
    int OnEnd() override {
        return number;
    }
};

class FSMTestAgent : public tamaf::core::Agent {
public:
    FSMTestAgent(const String& name, const String& ip) 
        : Agent(name.c_str(), tamaf::types::Address(ip, 4001)) {}

    void Setup(tamaf::ema::EMAInteraction* emaInteraction) override {
        tamaf::behaviors::FSMBehavior* fsm = new tamaf::behaviors::FSMBehavior();
        fsm->AddInitialState(new KeyboardInputBehavior(0), "primeiro");
        fsm->AddState(new KeyboardInputBehavior(0), "segundo");
        fsm->AddState(new KeyboardInputBehavior(0), "terceiro");
        fsm->AddFinalState(new KeyboardInputBehavior(0), "quarto");
        
        fsm->AddTransition("primeiro", "segundo", 0);
        fsm->AddTransition("segundo", "quarto", 0); 
        
        AddBehavior(fsm);
    }
};

PingerAgent* agent1;
FSMTestAgent* agent2;

void setup() {
    Serial.begin(115200);
    delay(1000);
    WiFi.mode(WIFI_STA); 
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
    Serial.println(WiFi.localIP());
    agent1 = new PingerAgent("Test1Agent", WiFi.localIP().toString());
    agent1->setEMAAddress(tamaf::types::Address("192.168.0.10", 4000));
    agent1->Start();
    
    agent2 = new FSMTestAgent("Test2Agent", WiFi.localIP().toString());
    agent2->setEMAAddress(tamaf::types::Address("192.168.0.10", 4000));
    agent2->Start();
}

void loop() {
    if (agent1) agent1->Loop();
    if (agent2) agent2->Loop();
    delay(10);
}
