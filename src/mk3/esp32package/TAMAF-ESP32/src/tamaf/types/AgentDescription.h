#ifndef TAMAF_AGENTDESCRIPTION_H
#define TAMAF_AGENTDESCRIPTION_H

#include <Arduino.h>
#include <vector>
#include <ArduinoJson.h>
#include "AgentID.h"
#include "ServiceDescription.h"

namespace tamaf {
namespace types {

class AgentDescription {
private:
    AgentID agentId;
    std::vector<ServiceDescription> services;

public:
    AgentDescription();
    AgentDescription(const AgentID& agentId);

    const AgentID& getAgentId() const;
    AgentID& getAgentId();
    void setAgentId(const AgentID& agentId);
    void setPort(int port);
    void setAddress(const Address& addr);

    const std::vector<ServiceDescription>& getServices() const;
    std::vector<ServiceDescription>& getServices();
    void addService(const ServiceDescription& service);
    void clearServices();

    bool Matches(const AgentDescription& templateDesc) const;
    AgentDescription copy() const;

    // Serialization
    void ToJson(JsonVariant doc) const;
    static AgentDescription FromJson(const JsonVariant doc);
};

} // namespace types
} // namespace tamaf

#endif // TAMAF_AGENTDESCRIPTION_H
