#include "Envelope.h"

namespace tamaf {
namespace messaging {

Envelope::Envelope() : sender(""), receiver("") {}

Envelope::Envelope(const String& sender, const String& receiver) : sender(sender), receiver(receiver) {}

String Envelope::getSender() const { return sender; }
void Envelope::setSender(const String& sender) { this->sender = sender; }

String Envelope::getReceiver() const { return receiver; }
void Envelope::setReceiver(const String& receiver) { this->receiver = receiver; }

void Envelope::ToJson(JsonVariant doc) const {
    JsonObject senderDesc = doc["sender-transport-description"].to<JsonObject>();
    senderDesc["transport-type"] = "http";
    int atPosSender = sender.indexOf('@');
    String senderAddress = (atPosSender != -1) ? sender.substring(atPosSender + 1) : sender;
    senderDesc["transport-specific-address"] = "http://" + senderAddress;
    
    JsonObject receiverDesc = doc["receiver-transport-description"].to<JsonObject>();
    receiverDesc["transport-type"] = "http";
    int atPosReceiver = receiver.indexOf('@');
    String receiverAddress = (atPosReceiver != -1) ? receiver.substring(atPosReceiver + 1) : receiver;
    receiverDesc["transport-specific-address"] = "http://" + receiverAddress;
}

Envelope Envelope::FromJson(const JsonVariant doc) {
    Envelope env;
    if (doc["sender"].is<String>()) env.setSender(doc["sender"].as<String>());
    if (doc["receiver"].is<String>()) env.setReceiver(doc["receiver"].as<String>());
    return env;
}

} // namespace messaging
} // namespace tamaf
