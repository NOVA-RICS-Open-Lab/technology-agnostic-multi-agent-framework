#include "ACLMessageTemplate.h"

namespace tamaf {
namespace messaging {

ACLMessageTemplate::ACLMessageTemplate() 
    : matchPerformative(false), matchConversationId(false), matchSender(false), matchOntology(false) {}

void ACLMessageTemplate::setPerformative(tamaf::enums::Performative perf) {
    performativeToMatch = perf;
    matchPerformative = true;
}

void ACLMessageTemplate::setConversationId(const String& convId) {
    conversationIdToMatch = convId;
    matchConversationId = true;
}

void ACLMessageTemplate::addSender(const tamaf::types::AgentID& sender) {
    senderToMatch = sender;
    matchSender = true;
}

void ACLMessageTemplate::setOntology(const String& ontology) {
    ontologyToMatch = ontology;
    matchOntology = true;
}

bool ACLMessageTemplate::Match(const ACLMessage& msg) const {
    if (matchPerformative && msg.getPerformative() != performativeToMatch) return false;
    if (matchConversationId && msg.getConversationId() != conversationIdToMatch) return false;
    if (matchSender && msg.getSender().getName() != senderToMatch.getName()) return false;
    if (matchOntology && msg.getOntology() != ontologyToMatch) return false;
    return true; // Matches everything not explicitly filtered
}

ACLMessageTemplate ACLMessageTemplate::MatchPerformative(tamaf::enums::Performative perf) {
    ACLMessageTemplate tpl;
    tpl.setPerformative(perf);
    return tpl;
}

} // namespace messaging
} // namespace tamaf
