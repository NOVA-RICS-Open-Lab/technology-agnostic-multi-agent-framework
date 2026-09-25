#include "AgentManagementOntology.h"

namespace tamaf {
namespace ema {

bool AgentManagementOntology::Validate(const String& contentStr) {
    if (contentStr.isEmpty()) return false;
    JsonDocument doc;
    if (deserializeJson(doc, contentStr)) return false; // Parse error
    
    if (doc["action"].isNull() || doc["concept"].isNull()) return false;
    
    String action = doc["action"].as<String>();
    String concept = doc["concept"].as<String>();
    
    // Simplified validation: as long as action and concept are present
    return true;
}

String AgentManagementOntology::GetAction(const String& contentStr) {
    if (!Validate(contentStr)) return "";
    JsonDocument doc;
    deserializeJson(doc, contentStr);
    return doc["action"].as<String>();
}

String AgentManagementOntology::GetConcept(const String& contentStr) {
    if (!Validate(contentStr)) return "";
    JsonDocument doc;
    deserializeJson(doc, contentStr);
    return doc["concept"].as<String>();
}

std::vector<tamaf::types::AgentDescription> AgentManagementOntology::GetAgentDescriptionsFromAttributes(const String& contentStr) {
    std::vector<tamaf::types::AgentDescription> results;
    if (!Validate(contentStr)) return results;
    
    JsonDocument doc;
    deserializeJson(doc, contentStr);
    
    if (doc["concept"].as<String>() != DEFAULT_AGENTMANAGEMENT_ONTOLOGY_AGENTDESCRIPTION_CONCEPT) {
        return results;
    }
    
    JsonArray attrs = doc["attributes"].as<JsonArray>();
    for (JsonObject attr : attrs) {
        tamaf::types::AgentDescription desc = tamaf::types::AgentDescription::FromJson(attr);
        results.push_back(desc);
    }
    return results;
}

tamaf::types::Address AgentManagementOntology::GetAgentAddressFromAttributes(const String& contentStr) {
    if (!Validate(contentStr)) return tamaf::types::Address();
    
    JsonDocument doc;
    deserializeJson(doc, contentStr);
    
    if (doc["concept"].as<String>() != DEFAULT_AGENTMANAGEMENT_ONTOLOGY_AGENTADDRESS_CONCEPT) {
        return tamaf::types::Address();
    }
    
    return tamaf::types::Address::FromString(doc["attributes"].as<String>());
}

int AgentManagementOntology::GetInformFromAttributes(const String& contentStr) {
    if (!Validate(contentStr)) return 0;
    
    JsonDocument doc;
    deserializeJson(doc, contentStr);
    
    if (doc["concept"].as<String>() != DEFAULT_AGENTMANAGEMENT_ONTOLOGY_INFORM_CONCEPT) {
        return 0;
    }
    
    return doc["attributes"].as<int>();
}

String AgentManagementOntology::CreateAgentDescriptionMessageContent(const String& action, const std::vector<tamaf::types::AgentDescription>& agentDescriptions) {
    JsonDocument doc;
    doc["action"] = action;
    doc["concept"] = DEFAULT_AGENTMANAGEMENT_ONTOLOGY_AGENTDESCRIPTION_CONCEPT;
    
    JsonArray attrs = doc["attributes"].to<JsonArray>();
    for (const auto& desc : agentDescriptions) {
        JsonObject obj = attrs.add<JsonObject>();
        desc.ToJson(obj);
    }
    
    String output;
    serializeJson(doc, output);
    return output;
}

String AgentManagementOntology::CreateAgentAddressMessageContent(const String& action, const tamaf::types::Address& agentAddress) {
    JsonDocument doc;
    doc["action"] = action;
    doc["concept"] = DEFAULT_AGENTMANAGEMENT_ONTOLOGY_AGENTADDRESS_CONCEPT;
    doc["attributes"] = agentAddress.GetString();
    
    String output;
    serializeJson(doc, output);
    return output;
}

String AgentManagementOntology::CreateResultMessageContent(const String& action, const String& concept, int inform) {
    JsonDocument doc;
    doc["action"] = action;
    doc["concept"] = concept;
    doc["attributes"] = inform;
    
    String output;
    serializeJson(doc, output);
    return output;
}

} // namespace ema
} // namespace tamaf
