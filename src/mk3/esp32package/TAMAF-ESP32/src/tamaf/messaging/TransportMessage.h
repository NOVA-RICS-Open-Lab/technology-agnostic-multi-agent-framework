#ifndef TAMAF_TRANSPORTMESSAGE_H
#define TAMAF_TRANSPORTMESSAGE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "Envelope.h"

namespace tamaf {
namespace messaging {

class TransportMessage {
private:
    Envelope envelope;
    String payload; // Serialized ACLMessage JSON string

public:
    TransportMessage();
    TransportMessage(const Envelope& envelope, const String& payload);

    Envelope getEnvelope() const;
    void setEnvelope(const Envelope& envelope);

    String getPayload() const;
    void setPayload(const String& payload);

    // Serialization
    void ToJson(JsonVariant doc) const;
    static TransportMessage FromJson(const JsonVariant doc);
};

} // namespace messaging
} // namespace tamaf

#endif // TAMAF_TRANSPORTMESSAGE_H
