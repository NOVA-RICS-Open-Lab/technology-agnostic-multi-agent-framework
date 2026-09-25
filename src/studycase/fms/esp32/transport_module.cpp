#include "transport_module.hpp"

REQExecuteTransportResponder::REQExecuteTransportResponder(TransportAgent* agent, const tamaf::messaging::ACLMessageTemplate& tpl)
    : FIPARequestResponder(tpl), transportAgent(agent), targetLocation(Location::UNKNOWN), isMoving(false) {}

tamaf::messaging::ACLMessage REQExecuteTransportResponder::PrepareResponse(const tamaf::messaging::ACLMessage& request) {
    String content = request.getContent().c_str();
    
    // Parse target location 'b' from content JSON or raw string (e.g. {"a":"A","b":"B"} or "B")
    String targetStr = "";
    int bIndex = content.indexOf("\"b\":");
    if (bIndex != -1) {
        int startQuote = content.indexOf("\"", bIndex + 4);
        int endQuote = content.indexOf("\"", startQuote + 1);
        if (startQuote != -1 && endQuote != -1) {
            targetStr = content.substring(startQuote + 1, endQuote);
        }
    } else {
        targetStr = content;
        targetStr.trim();
    }
    
    Location loc = stringToLocation(targetStr);
    if (loc != Location::UNKNOWN) {
        targetLocation = loc;
        Serial.print("[TransportAgent] Received transport request to target location: ");
        Serial.print(targetStr);
        if (transportAgent) {
            Serial.print(" (Current location: ");
            Serial.print(locationToString(transportAgent->getCurrentProductLocation()));
            Serial.println(")");
        } else {
            Serial.println();
        }
        tamaf::messaging::ACLMessage agree(tamaf::enums::Performative::AGREE);
        agree.addReceiver(request.getSender());
        agree.setConversationId(request.getConversationId());
        agree.setOntology(request.getOntology());
        return agree;
    } else {
        Serial.print("[TransportAgent] Refused transport request, unknown target location: ");
        Serial.println(targetStr);
        tamaf::messaging::ACLMessage refuse(tamaf::enums::Performative::REFUSE);
        refuse.addReceiver(request.getSender());
        refuse.setConversationId(request.getConversationId());
        refuse.setOntology(request.getOntology());
        return refuse;
    }
}

tamaf::messaging::ACLMessage REQExecuteTransportResponder::PrepareResultNotification(const tamaf::messaging::ACLMessage& request, const tamaf::messaging::ACLMessage& response) {
    if (transportAgent && targetLocation != Location::UNKNOWN) {
        if (transportAgent->getCurrentProductLocation() == targetLocation) {
            Serial.print("[TransportAgent] Product is already at target location: ");
            Serial.println(locationToString(targetLocation));
        }
        while (transportAgent->getCurrentProductLocation() != targetLocation) {
            Location currentLoc = transportAgent->getCurrentProductLocation();
            Location nextLoc = getNextLocation(currentLoc);
            
            Serial.print("[TransportAgent] Moving product from ");
            Serial.print(locationToString(currentLoc));
            Serial.print(" to ");
            Serial.println(locationToString(nextLoc));
            
            bool success = transportAgent->executeTransportStep(currentLoc, nextLoc);
            if (success) {
                transportAgent->setCurrentProductLocation(nextLoc);
                Serial.print("[TransportAgent] Arrived at ");
                Serial.println(locationToString(nextLoc));
            } else {
                Serial.println("[TransportAgent] Hardware offline/unreachable, step simulation completed.");
                transportAgent->setCurrentProductLocation(nextLoc);
            }
            delay(200);
        }
    }
    
    tamaf::messaging::ACLMessage inform(tamaf::enums::Performative::INFORM);
    inform.addReceiver(request.getSender());
    inform.setConversationId(request.getConversationId());
    inform.setOntology(request.getOntology());
    if (transportAgent) {
        inform.setContent(locationToString(transportAgent->getCurrentProductLocation()).c_str());
    } else {
        inform.setContent(locationToString(targetLocation).c_str());
    }
    return inform;
}

TransportAgent::TransportAgent(const String& name, const std::vector<SkillInfo>& skills, const String& resType, Location loc)
    : Agent(formatAgentName(name).c_str(), tamaf::types::Address("0.0.0.0", 4001)),
      mySkills(skills), resourceType(resType), myLocation(loc), currentProductLocation(loc) {
    for (const auto& skill : mySkills) {
        tamaf::types::ServiceDescription sd(skill.name.c_str(), skill.serviceType.c_str());
        agentDescription.addService(sd);
    }
}

void TransportAgent::Setup(tamaf::ema::EMAInteraction* emaInteraction) {
    tamaf::messaging::ACLMessageTemplate tpl;
    tpl.setPerformative(tamaf::enums::Performative::REQUEST);
    tpl.setOntology(ONTOLOGY_REQUEST_TRANSPORT);

    AddBehavior(new REQExecuteTransportResponder(this, tpl));
}

bool TransportAgent::executeTransportStep(Location fromLoc, Location toLoc) {
    String stepId = locationToString(fromLoc) + locationToString(toLoc);
    String url = "http://" + String(CONTROLLER_IP) + "/passadeiras?skill=" + stepId;
    
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.setTimeout(30000); // 30s timeout matching Python requests
        http.begin(url);
        http.addHeader("User-Agent", "python-requests/2.31.0");
        http.addHeader("Connection", "close");
        int httpCode = http.GET();
        http.end();
        return (httpCode >= 200 && httpCode < 300);
    }
    return false;
}
