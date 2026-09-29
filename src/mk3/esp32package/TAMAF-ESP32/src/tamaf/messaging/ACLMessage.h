#ifndef TAMAF_ACLMESSAGE_H
#define TAMAF_ACLMESSAGE_H

#include <Arduino.h>
#include <vector>
#include <map>
#include <ArduinoJson.h>
#include "../enums/Performative.h"
#include "../types/AgentID.h"

namespace tamaf {
namespace messaging {

class ACLMessage {
private:
    enums::Performative performative;
    types::AgentID sender;
    std::vector<types::AgentID> receivers;
    std::vector<types::AgentID> replyTo;
    String content;
    String replyWith;
    String inReplyTo;
    String envelope;
    String language;
    String ontology;
    String replyBy;
    String protocol;
    String conversationId;
    std::map<String, String> userDefinedParameters;

public:
    ACLMessage();
    ACLMessage(enums::Performative perf);

    // Getters and Setters
    enums::Performative getPerformative() const;
    void setPerformative(enums::Performative perf);

    types::AgentID getSender() const;
    void setSender(const types::AgentID& sender);

    std::vector<types::AgentID> getReceivers() const;
    void addReceiver(const types::AgentID& receiver);
    void clearReceivers();

    std::vector<types::AgentID> getReplyTo() const;
    void addReplyTo(const types::AgentID& agentId);

    String getContent() const;
    void setContent(const String& content);

    String getReplyWith() const;
    void setReplyWith(const String& replyWith);

    String getInReplyTo() const;
    void setInReplyTo(const String& inReplyTo);

    String getEnvelope() const;
    void setEnvelope(const String& envelope);

    String getLanguage() const;
    void setLanguage(const String& language);

    String getOntology() const;
    void setOntology(const String& ontology);

    String getReplyBy() const;
    void setReplyBy(const String& replyBy);

    String getProtocol() const;
    void setProtocol(const String& protocol);

    String getConversationId() const;
    void setConversationId(const String& conversationId);

    void addUserDefinedParameter(const String& key, const String& value);
    String getUserDefinedParameter(const String& key) const;
    std::map<String, String> getUserDefinedParameters() const;

    ACLMessage createReply() const;

    // Serialization
    void ToJson(JsonVariant doc) const;
    static ACLMessage FromJson(const JsonVariant doc);
};

} // namespace messaging
} // namespace tamaf

#endif // TAMAF_ACLMESSAGE_H
