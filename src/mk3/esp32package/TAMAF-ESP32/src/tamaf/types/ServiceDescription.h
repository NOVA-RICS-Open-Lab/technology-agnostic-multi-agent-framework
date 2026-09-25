#ifndef TAMAF_SERVICEDESCRIPTION_H
#define TAMAF_SERVICEDESCRIPTION_H

#include <Arduino.h>
#include <vector>
#include <map>
#include <ArduinoJson.h>

namespace tamaf {
namespace types {

class ServiceDescription {
private:
    String name;
    String type;
    String ownership;
    std::vector<String> protocols;
    std::vector<String> ontologies;
    std::map<String, String> properties;

public:
    ServiceDescription();
    ServiceDescription(const String& name, const String& type);
    ServiceDescription(const String& name, const String& type, const String& ownership, 
                       const std::vector<String>& protocols, const std::vector<String>& ontologies, 
                       const std::map<String, String>& properties);

    String getName() const;
    void setName(const String& name);

    String getType() const;
    void setType(const String& type);

    String getOwnership() const;
    void setOwnership(const String& ownership);

    std::vector<String> getProtocols() const;
    void addProtocol(const String& protocol);

    std::vector<String> getOntologies() const;
    void addOntology(const String& ontology);

    std::map<String, String> getProperties() const;
    void addProperty(const String& key, const String& value);

    bool Matches(const ServiceDescription& templateDesc) const;

    // Serialization
    void ToJson(JsonVariant doc) const;
    static ServiceDescription FromJson(const JsonVariant doc);
};

} // namespace types
} // namespace tamaf

#endif // TAMAF_SERVICEDESCRIPTION_H
