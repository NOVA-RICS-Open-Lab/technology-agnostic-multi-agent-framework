#include "AgentDescription.h"

namespace tamaf {
namespace types {

AgentDescription::AgentDescription() : agentId(AgentID()) {}

AgentDescription::AgentDescription(const AgentID& agentId) : agentId(agentId) {}

AgentID AgentDescription::getAgentId() const { return agentId; }
void AgentDescription::setAgentId(const AgentID& agentId) { this->agentId = agentId; }

std::vector<ServiceDescription> AgentDescription::getServices() const { return services; }
void AgentDescription::addService(const ServiceDescription& service) { services.push_back(service); }

bool AgentDescription::Matches(const AgentDescription& templateDesc) const {
    if (!AgentID::Matches(templateDesc.getAgentId(), agentId)) return false;

    // Check if target contains all services from template
    for (const ServiceDescription& templateSvc : templateDesc.getServices()) {
        bool found = false;
        for (const ServiceDescription& mySvc : services) {
            if (mySvc.Matches(templateSvc)) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }

    return true;
}

AgentDescription AgentDescription::copy() const {
    AgentDescription clone(this->agentId);
    for (const ServiceDescription& svc : this->services) {
        clone.addService(svc); // Copies implicitly
    }
    return clone;
}

void AgentDescription::ToJson(JsonVariant doc) const {
    // Note: Python implementation structures it slightly differently, 
    // typically agentID is nested or formatted as string depending on ontology.
    // For now, we serialize exactly to match Python.
    // Python usually has {"name": agentId.GetFullID(), "addresses": [...], "services": [...]}
    // But since AgentID now holds address, we'll map properly.
    
    doc["agentid"] = agentId.GetFullID();
    
    if (agentId.getAddress().getIp() != "" && agentId.getAddress().getIp() != "0.0.0.0") {
        JsonArray addresses = doc["addresses"].to<JsonArray>();
        addresses.add(agentId.getAddress().GetHTTPLink());
    }

    if (!services.empty()) {
        JsonArray svcs = doc["services"].to<JsonArray>();
        for (const ServiceDescription& svc : services) {
            JsonVariant svcObj = svcs.add<JsonVariant>();
            svc.ToJson(svcObj);
        }
    }
}

AgentDescription AgentDescription::FromJson(const JsonVariant doc) {
    AgentDescription ad;
    
    if (doc["agentid"].is<String>()) {
        ad.setAgentId(AgentID::FromString(doc["agentid"].as<String>()));
    } else if (doc["name"].is<String>()) {
        ad.setAgentId(AgentID::FromString(doc["name"].as<String>()));
    }
    
    // In strict python TAMAF, addresses list might override the agentID's address
    if (doc["addresses"].is<JsonArray>()) {
        JsonArray arr = doc["addresses"].as<JsonArray>();
        if (arr.size() > 0) {
            String addrStr = arr[0].as<String>();
            // Remove "http://" if present
            if (addrStr.startsWith("http://")) addrStr = addrStr.substring(7);
            ad.getAgentId().setAddress(Address::FromString(addrStr));
        }
    }
    
    if (doc["services"].is<JsonArray>()) {
        JsonArray arr = doc["services"].as<JsonArray>();
        for (JsonVariant v : arr) {
            ad.addService(ServiceDescription::FromJson(v));
        }
    }
    
    return ad;
}

} // namespace types
} // namespace tamaf
