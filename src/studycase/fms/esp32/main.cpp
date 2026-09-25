#include <Arduino.h>
#include <WiFi.h>
#include <tamaf.h>
#include "constants.h"
#include "transport_module.hpp"
#include "tower_module.hpp"
#include "product_module.hpp"

// WiFi Credentials (configure for target local network)
#ifndef WIFI_SSID
#define WIFI_SSID "TAMAF_NETWORK"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "TAMAF_PASSWORD"
#endif

#ifndef EMA_IP
#define EMA_IP "192.168.2.90"
#endif

#ifndef EMA_PORT
#define EMA_PORT 4000
#endif

// Select Agent Mode to run on this ESP32 board
// 1 = Resource 1 (Drill)
// 2 = Resource 2 (Screw)
// 3 = Transport Agent
// 4 = Product Agent
// 5 = FMS Tower Agent
#ifndef AGENT_MODE
#define AGENT_MODE 5
#endif

tamaf::core::Agent* currentAgent = nullptr;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("=========================================");
    Serial.println("   TAMAF ESP32 FMS Industrial Use Case   ");
    Serial.println("=========================================");

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.print("WiFi Connected! ESP32 IP: ");
    Serial.println(WiFi.localIP());

    String localIPStr = WiFi.localIP().toString();

    #if AGENT_MODE == 1
    Serial.println("Starting Resource 1 Agent (Drill)...");
    std::vector<SkillInfo> drillSkills = {
        {"Drill", DF_SERVICE_TYPE_RESOURCE, "http://" CONTROLLER_IP "/estacao?skill=0"}
    };
    TowerAgent* drillAgent = new TowerAgent("Resource_1", drillSkills, "Station", Location::A);
    drillAgent->setEMAAddress(tamaf::types::Address(EMA_IP, EMA_PORT));
    currentAgent = drillAgent;

    #elif AGENT_MODE == 2
    Serial.println("Starting Resource 2 Agent (Screw)...");
    std::vector<SkillInfo> screwSkills = {
        {"Screw", DF_SERVICE_TYPE_RESOURCE, "http://" CONTROLLER_IP "/estacao?skill=1"}
    };
    TowerAgent* screwAgent = new TowerAgent("Resource_2", screwSkills, "Station", Location::B);
    screwAgent->setEMAAddress(tamaf::types::Address(EMA_IP, EMA_PORT));
    currentAgent = screwAgent;

    #elif AGENT_MODE == 3
    Serial.println("Starting Transport Agent...");
    std::vector<SkillInfo> transportSkills = {
        {DF_SERVICE_NAME_TRANSPORT, DF_SERVICE_TYPE_TRANSPORT, "http://" CONTROLLER_IP "/passadeiras?skill=AB"},
        {"GoToAB", DF_SERVICE_TYPE_TRANSPORT, "http://" CONTROLLER_IP "/passadeiras?skill=AB"},
        {"GoToBC", DF_SERVICE_TYPE_TRANSPORT, "http://" CONTROLLER_IP "/passadeiras?skill=BC"},
        {"GoToCD", DF_SERVICE_TYPE_TRANSPORT, "http://" CONTROLLER_IP "/passadeiras?skill=CD"},
        {"GoToDE", DF_SERVICE_TYPE_TRANSPORT, "http://" CONTROLLER_IP "/passadeiras?skill=DE"},
        {"GoToEF", DF_SERVICE_TYPE_TRANSPORT, "http://" CONTROLLER_IP "/passadeiras?skill=EF"},
        {"GoToFA", DF_SERVICE_TYPE_TRANSPORT, "http://" CONTROLLER_IP "/passadeiras?skill=FA"}
    };
    TransportAgent* transportAgent = new TransportAgent("Transport_1", transportSkills, "Robot", Location::A);
    transportAgent->setEMAAddress(tamaf::types::Address(EMA_IP, EMA_PORT));
    currentAgent = transportAgent;

    #elif AGENT_MODE == 4
    Serial.println("Starting Product Agent...");
    std::vector<String> recipe = {"Drill", "Screw"};
    ProductAgent* productAgent = new ProductAgent("Product_1", recipe, Location::A, localIPStr);
    productAgent->setEMAAddress(tamaf::types::Address(EMA_IP, EMA_PORT));
    currentAgent = productAgent;

    #elif AGENT_MODE == 5
    Serial.println("Starting FMS Tower Agent...");
    std::vector<SkillInfo> towerSkills = {
        {"Drill", DF_SERVICE_TYPE_RESOURCE, "http://" CONTROLLER_IP "/estacao?skill=0"},
        {"Screw", DF_SERVICE_TYPE_RESOURCE, "http://" CONTROLLER_IP "/estacao?skill=1"}
    };
    TowerAgent* towerAgent = new TowerAgent("FMS_Tower", towerSkills, "Station", Location::D, localIPStr);
    towerAgent->setEMAAddress(tamaf::types::Address(EMA_IP, EMA_PORT));
    currentAgent = towerAgent;
    #endif

    if (currentAgent) {
        currentAgent->Start();
        Serial.println("Agent setup completed and started successfully.");
    }
}

void loop() {
    if (currentAgent) {
        currentAgent->Loop();
    }
    delay(10);
}
