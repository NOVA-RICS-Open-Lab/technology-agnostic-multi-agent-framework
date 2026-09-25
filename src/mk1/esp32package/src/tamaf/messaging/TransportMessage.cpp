#include "TransportMessage.h"

namespace tamaf {
namespace messaging {

TransportMessage::TransportMessage() : envelope(Envelope()), payload("") {}

TransportMessage::TransportMessage(const Envelope& envelope, const String& payload) 
    : envelope(envelope), payload(payload) {}

Envelope TransportMessage::getEnvelope() const { return envelope; }
void TransportMessage::setEnvelope(const Envelope& envelope) { this->envelope = envelope; }

String TransportMessage::getPayload() const { return payload; }
void TransportMessage::setPayload(const String& payload) { this->payload = payload; }

void TransportMessage::ToJson(JsonVariant doc) const {
    JsonObject envObj = doc["envelope"].to<JsonObject>();
    envelope.ToJson(envObj);
    
    if (payload.length() > 0) {
        deserializeJson(doc["aclmessage"], payload);
    }
}

TransportMessage TransportMessage::FromJson(const JsonVariant doc) {
    TransportMessage msg;
    
    if (doc["envelope"].is<JsonObject>()) {
        msg.setEnvelope(Envelope::FromJson(doc["envelope"]));
    }
    
    if (doc["aclmessage"].is<JsonObject>()) {
        String payloadStr;
        serializeJson(doc["aclmessage"], payloadStr);
        msg.setPayload(payloadStr);
    }
    
    return msg;
}

} // namespace messaging
} // namespace tamaf
