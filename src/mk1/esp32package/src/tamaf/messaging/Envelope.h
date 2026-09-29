#ifndef TAMAF_ENVELOPE_H
#define TAMAF_ENVELOPE_H

#include <Arduino.h>
#include <ArduinoJson.h>

namespace tamaf {
namespace messaging {

class Envelope {
private:
    String sender;
    String receiver; // Simplified for transport level

public:
    Envelope();
    Envelope(const String& sender, const String& receiver);

    String getSender() const;
    void setSender(const String& sender);

    String getReceiver() const;
    void setReceiver(const String& receiver);

    // Serialization
    void ToJson(JsonVariant doc) const;
    static Envelope FromJson(const JsonVariant doc);
};

} // namespace messaging
} // namespace tamaf

#endif // TAMAF_ENVELOPE_H
