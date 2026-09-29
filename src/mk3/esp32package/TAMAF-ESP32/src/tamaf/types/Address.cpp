#include "Address.h"
#include "../../defines.h"

namespace tamaf {
namespace types {

Address::Address() : ip(LOCALHOST), port(DEFAULT_EMA_PORT) {}

Address::Address(const String& ip, int port) : ip(ip), port(port) {}

String Address::getIp() const { return ip; }
void Address::setIp(const String& ip) { this->ip = ip; }

int Address::getPort() const { return port; }
void Address::setPort(int port) { this->port = port; }

String Address::GetTuple() const {
    // Return standard JSON array string formatting
    return "[\"" + ip + "\", " + String(port) + "]";
}

String Address::GetString() const {
    return ip + ":" + String(port);
}

String Address::GetHTTPLink() const {
    return "http://" + GetString();
}

bool Address::operator==(const Address& other) const {
    return (ip == other.ip && port == other.port);
}

Address Address::FromString(const String& addressString) {
    int colonIndex = addressString.indexOf(':');
    if (colonIndex > 0) {
        String ipPart = addressString.substring(0, colonIndex);
        int portPart = addressString.substring(colonIndex + 1).toInt();
        return Address(ipPart, portPart);
    }
    return Address(addressString, DEFAULT_EMA_PORT);
}

} // namespace types
} // namespace tamaf
