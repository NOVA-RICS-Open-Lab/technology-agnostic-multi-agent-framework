#ifndef TAMAF_ACLMESSAGETEMPLATE_H
#define TAMAF_ACLMESSAGETEMPLATE_H

#include "ACLMessage.h"

namespace tamaf {
namespace messaging {

class ACLMessageTemplate {
private:
    bool matchPerformative;
    tamaf::enums::Performative performativeToMatch;
    
    bool matchConversationId;
    String conversationIdToMatch;

    bool matchSender;
    tamaf::types::AgentID senderToMatch;

    bool matchOntology;
    String ontologyToMatch;

public:
    ACLMessageTemplate();
    
    void setPerformative(tamaf::enums::Performative perf);
    void setConversationId(const String& convId);
    void addSender(const tamaf::types::AgentID& sender);
    void setOntology(const String& ontology);
    
    bool Match(const ACLMessage& msg) const;

    // Static helpers typical of JADE
    static ACLMessageTemplate MatchPerformative(tamaf::enums::Performative perf);
};

} // namespace messaging
} // namespace tamaf

#endif // TAMAF_ACLMESSAGETEMPLATE_H
