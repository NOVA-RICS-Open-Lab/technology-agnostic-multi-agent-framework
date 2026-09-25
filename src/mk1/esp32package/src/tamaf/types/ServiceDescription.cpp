#include "ServiceDescription.h"

namespace tamaf {
namespace types {

ServiceDescription::ServiceDescription() : name(""), type(""), ownership("") {}

ServiceDescription::ServiceDescription(const String& name, const String& type) 
    : name(name), type(type), ownership("") {}

ServiceDescription::ServiceDescription(const String& name, const String& type, const String& ownership, 
                                       const std::vector<String>& protocols, const std::vector<String>& ontologies, 
                                       const std::map<String, String>& properties)
    : name(name), type(type), ownership(ownership), protocols(protocols), ontologies(ontologies), properties(properties) {}

String ServiceDescription::getName() const { return name; }
void ServiceDescription::setName(const String& name) { this->name = name; }

String ServiceDescription::getType() const { return type; }
void ServiceDescription::setType(const String& type) { this->type = type; }

String ServiceDescription::getOwnership() const { return ownership; }
void ServiceDescription::setOwnership(const String& ownership) { this->ownership = ownership; }

std::vector<String> ServiceDescription::getProtocols() const { return protocols; }
void ServiceDescription::addProtocol(const String& protocol) { protocols.push_back(protocol); }

std::vector<String> ServiceDescription::getOntologies() const { return ontologies; }
void ServiceDescription::addOntology(const String& ontology) { ontologies.push_back(ontology); }

std::map<String, String> ServiceDescription::getProperties() const { return properties; }
void ServiceDescription::addProperty(const String& key, const String& value) { properties[key] = value; }

bool ServiceDescription::Matches(const ServiceDescription& templateDesc) const {
    if (templateDesc.getName().length() > 0 && templateDesc.getName() != name) return false;
    if (templateDesc.getType().length() > 0 && templateDesc.getType() != type) return false;
    if (templateDesc.getOwnership().length() > 0 && templateDesc.getOwnership() != ownership) return false;
    
    // Check if target contains all protocols from template
    for (const String& p : templateDesc.getProtocols()) {
        bool found = false;
        for (const String& myP : protocols) {
            if (p == myP) { found = true; break; }
        }
        if (!found) return false;
    }

    // Check if target contains all ontologies from template
    for (const String& o : templateDesc.getOntologies()) {
        bool found = false;
        for (const String& myO : ontologies) {
            if (o == myO) { found = true; break; }
        }
        if (!found) return false;
    }

    // Check if target contains all properties from template
    for (const auto& pair : templateDesc.getProperties()) {
        if (properties.find(pair.first) == properties.end() || properties.at(pair.first) != pair.second) {
            return false;
        }
    }

    return true;
}

void ServiceDescription::ToJson(JsonVariant doc) const {
    if (name.length() > 0) doc["name"] = name;
    if (type.length() > 0) doc["type"] = type;
    if (ownership.length() > 0) doc["ownership"] = ownership;

    if (!protocols.empty()) {
        JsonArray arr = doc["protocols"].to<JsonArray>();
        for (const String& p : protocols) arr.add(p);
    }
    
    if (!ontologies.empty()) {
        JsonArray arr = doc["ontologies"].to<JsonArray>();
        for (const String& o : ontologies) arr.add(o);
    }
    
    if (!properties.empty()) {
        JsonObject obj = doc["properties"].to<JsonObject>();
        for (const auto& kv : properties) obj[kv.first] = kv.second;
    }
}

ServiceDescription ServiceDescription::FromJson(const JsonVariant doc) {
    ServiceDescription sd;
    
    if (doc["name"].is<String>()) sd.setName(doc["name"].as<String>());
    if (doc["type"].is<String>()) sd.setType(doc["type"].as<String>());
    if (doc["ownership"].is<String>()) sd.setOwnership(doc["ownership"].as<String>());
    
    if (doc["protocols"].is<JsonArray>()) {
        JsonArray arr = doc["protocols"].as<JsonArray>();
        for (JsonVariant v : arr) sd.addProtocol(v.as<String>());
    }
    
    if (doc["ontologies"].is<JsonArray>()) {
        JsonArray arr = doc["ontologies"].as<JsonArray>();
        for (JsonVariant v : arr) sd.addOntology(v.as<String>());
    }
    
    if (doc["properties"].is<JsonObject>()) {
        JsonObject obj = doc["properties"].as<JsonObject>();
        for (JsonPair kv : obj) sd.addProperty(kv.key().c_str(), kv.value().as<String>());
    }
    
    return sd;
}

} // namespace types
} // namespace tamaf
