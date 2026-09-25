#ifndef TAMAF_ACLMESSAGETEMPLATE_H
#define TAMAF_ACLMESSAGETEMPLATE_H

#include "ACLMessage.h"
#include <functional>
#include <type_traits>

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

    std::function<bool(const ACLMessage&)> customPredicate;

public:
    ACLMessageTemplate();
    ACLMessageTemplate(std::function<bool(const ACLMessage&)> predicate);

    template<typename F, typename = typename std::enable_if<!std::is_same<typename std::decay<F>::type, ACLMessageTemplate>::value>::type>
    ACLMessageTemplate(F&& predicate)
        : matchPerformative(false), matchConversationId(false), matchSender(false), matchOntology(false),
          customPredicate(std::forward<F>(predicate)) {}
    
    void setPerformative(tamaf::enums::Performative perf);
    void setConversationId(const String& convId);
    void addSender(const tamaf::types::AgentID& sender);
    void setOntology(const String& ontology);
    
    bool Match(const ACLMessage& msg) const;
    bool Matches(const ACLMessage& msg) const { return Match(msg); }

    // Static helpers typical of JADE
    static ACLMessageTemplate MatchPerformative(tamaf::enums::Performative perf);
    static ACLMessageTemplate MatchConversationId(const String& convId);
    static ACLMessageTemplate MatchSender(const tamaf::types::AgentID& sender);
};

} // namespace messaging
} // namespace tamaf

#endif // TAMAF_ACLMESSAGETEMPLATE_H
