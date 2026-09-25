#include "ACLMessage.h"

namespace tamaf {
namespace messaging {

ACLMessage::ACLMessage() : performative(enums::Performative::UNKNOWN) {}

ACLMessage::ACLMessage(enums::Performative perf) : performative(perf) {}

enums::Performative ACLMessage::getPerformative() const { return performative; }
void ACLMessage::setPerformative(enums::Performative perf) { performative = perf; }

types::AgentID ACLMessage::getSender() const { return sender; }
void ACLMessage::setSender(const types::AgentID& sender) { this->sender = sender; }

std::vector<types::AgentID> ACLMessage::getReceivers() const { return receivers; }
void ACLMessage::addReceiver(const types::AgentID& receiver) { receivers.push_back(receiver); }
void ACLMessage::clearReceivers() { receivers.clear(); }

std::vector<types::AgentID> ACLMessage::getReplyTo() const { return replyTo; }
void ACLMessage::addReplyTo(const types::AgentID& agentId) { replyTo.push_back(agentId); }

String ACLMessage::getContent() const { return content; }
void ACLMessage::setContent(const String& content) { this->content = content; }

String ACLMessage::getReplyWith() const { return replyWith; }
void ACLMessage::setReplyWith(const String& replyWith) { this->replyWith = replyWith; }

String ACLMessage::getInReplyTo() const { return inReplyTo; }
void ACLMessage::setInReplyTo(const String& inReplyTo) { this->inReplyTo = inReplyTo; }

String ACLMessage::getEnvelope() const { return envelope; }
void ACLMessage::setEnvelope(const String& envelope) { this->envelope = envelope; }

String ACLMessage::getLanguage() const { return language; }
void ACLMessage::setLanguage(const String& language) { this->language = language; }

String ACLMessage::getOntology() const { return ontology; }
void ACLMessage::setOntology(const String& ontology) { this->ontology = ontology; }

String ACLMessage::getReplyBy() const { return replyBy; }
void ACLMessage::setReplyBy(const String& replyBy) { this->replyBy = replyBy; }

String ACLMessage::getProtocol() const { return protocol; }
void ACLMessage::setProtocol(const String& protocol) { this->protocol = protocol; }

String ACLMessage::getConversationId() const { return conversationId; }
void ACLMessage::setConversationId(const String& conversationId) { this->conversationId = conversationId; }

void ACLMessage::addUserDefinedParameter(const String& key, const String& value) { userDefinedParameters[key] = value; }
String ACLMessage::getUserDefinedParameter(const String& key) const {
    auto it = userDefinedParameters.find(key);
    if (it != userDefinedParameters.end()) return it->second;
    return "";
}
std::map<String, String> ACLMessage::getUserDefinedParameters() const { return userDefinedParameters; }

ACLMessage ACLMessage::createReply() const {
    ACLMessage reply(enums::Performative::UNKNOWN); // To be set by behavior
    reply.addReceiver(this->sender);
    if (this->replyWith.length() > 0) {
        reply.setInReplyTo(this->replyWith);
    }
    reply.setConversationId(this->conversationId);
    reply.setLanguage(this->language);
    reply.setOntology(this->ontology);
    reply.setProtocol(this->protocol);
    return reply;
}

void ACLMessage::ToJson(JsonVariant doc) const {
    doc["performative"] = enums::PerformativeToString(performative);
    doc["sender"] = sender.GetFullID();
    
    if (!receivers.empty()) {
        JsonArray rArr = doc["receiver"].to<JsonArray>();
        for (const auto& r : receivers) {
            rArr.add(r.GetFullID());
        }
    }
    
    if (!replyTo.empty()) {
        JsonArray rtArr = doc["reply-to"].to<JsonArray>();
        for (const auto& rt : replyTo) {
            rtArr.add(rt.GetFullID());
        }
    }
    
    if (content.length() > 0) {
        if (content.startsWith("{") || content.startsWith("[")) {
            deserializeJson(doc["content"], content);
        } else {
            doc["content"] = content;
        }
    }
    if (replyWith.length() > 0) doc["reply-with"] = replyWith;
    if (inReplyTo.length() > 0) doc["in-reply-to"] = inReplyTo;
    if (envelope.length() > 0) doc["envelope"] = envelope;
    if (language.length() > 0) doc["language"] = language;
    if (ontology.length() > 0) doc["ontology"] = ontology;
    if (replyBy.length() > 0) doc["reply-by"] = replyBy;
    if (protocol.length() > 0) doc["protocol"] = protocol;
    if (conversationId.length() > 0) doc["conversation-id"] = conversationId;
    
    if (!userDefinedParameters.empty()) {
        JsonObject paramsObj = doc["userDefinedParameters"].to<JsonObject>();
        for (const auto& kv : userDefinedParameters) {
            paramsObj[kv.first] = kv.second;
        }
    }
}

ACLMessage ACLMessage::FromJson(const JsonVariant doc) {
    ACLMessage msg;
    
    if (doc["performative"].is<String>()) {
        msg.setPerformative(enums::StringToPerformative(doc["performative"].as<String>()));
    }
    
    if (doc["sender"].is<String>()) {
        msg.setSender(types::AgentID::FromString(doc["sender"].as<String>()));
    }
    
    if (doc["receiver"].is<JsonArray>()) {
        JsonArray rArr = doc["receiver"].as<JsonArray>();
        for (JsonVariant v : rArr) {
            msg.addReceiver(types::AgentID::FromString(v.as<String>()));
        }
    }
    
    if (doc["reply-to"].is<JsonArray>()) {
        JsonArray rtArr = doc["reply-to"].as<JsonArray>();
        for (JsonVariant v : rtArr) {
            msg.addReplyTo(types::AgentID::FromString(v.as<String>()));
        }
    }
    
    if (doc["content"].is<JsonObject>() || doc["content"].is<JsonArray>()) {
        String contentStr;
        serializeJson(doc["content"], contentStr);
        msg.setContent(contentStr);
    } else if (doc["content"].is<String>()) {
        msg.setContent(doc["content"].as<String>());
    }
    if (doc["reply-with"].is<String>()) msg.setReplyWith(doc["reply-with"].as<String>());
    if (doc["in-reply-to"].is<String>()) msg.setInReplyTo(doc["in-reply-to"].as<String>());
    if (doc["envelope"].is<String>()) msg.setEnvelope(doc["envelope"].as<String>());
    if (doc["language"].is<String>()) msg.setLanguage(doc["language"].as<String>());
    if (doc["ontology"].is<String>()) msg.setOntology(doc["ontology"].as<String>());
    if (doc["reply-by"].is<String>()) msg.setReplyBy(doc["reply-by"].as<String>());
    if (doc["protocol"].is<String>()) msg.setProtocol(doc["protocol"].as<String>());
    if (doc["conversation-id"].is<String>()) msg.setConversationId(doc["conversation-id"].as<String>());
    
    if (doc["userDefinedParameters"].is<JsonObject>()) {
        JsonObject obj = doc["userDefinedParameters"].as<JsonObject>();
        for (JsonPair kv : obj) {
            msg.addUserDefinedParameter(kv.key().c_str(), kv.value().as<String>());
        }
    }
    
    return msg;
}

} // namespace messaging
} // namespace tamaf
