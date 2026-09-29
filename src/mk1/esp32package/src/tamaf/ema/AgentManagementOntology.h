#ifndef TAMAF_AGENT_MANAGEMENT_ONTOLOGY_H
#define TAMAF_AGENT_MANAGEMENT_ONTOLOGY_H

#include <ArduinoJson.h>
#include <vector>
#include "defines.h"
#include "tamaf/types/AgentDescription.h"
#include "tamaf/types/Address.h"

namespace tamaf {
namespace ema {

class AgentManagementOntology {
public:
    static bool Validate(const String& contentStr);
    
    static String GetAction(const String& contentStr);
    static String GetConcept(const String& contentStr);
    
    // Deserialization
    static std::vector<tamaf::types::AgentDescription> GetAgentDescriptionsFromAttributes(const String& contentStr);
    static tamaf::types::Address GetAgentAddressFromAttributes(const String& contentStr);
    static int GetInformFromAttributes(const String& contentStr);

    // Serialization
    static String CreateAgentDescriptionMessageContent(const String& action, const std::vector<tamaf::types::AgentDescription>& agentDescriptions);
    static String CreateAgentAddressMessageContent(const String& action, const tamaf::types::Address& agentAddress);
    static String CreateResultMessageContent(const String& action, const String& concept, int inform);
};

} // namespace ema
} // namespace tamaf

#endif // TAMAF_AGENT_MANAGEMENT_ONTOLOGY_H
