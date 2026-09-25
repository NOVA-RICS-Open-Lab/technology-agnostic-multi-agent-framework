#ifndef TAMAF_ADDRESS_H
#define TAMAF_ADDRESS_H

#include <Arduino.h>

namespace tamaf {
namespace types {

class Address {
private:
    String ip;
    int port;

public:
    Address();
    Address(const String& ip, int port);
    
    String getIp() const;
    void setIp(const String& ip);
    
    int getPort() const;
    void setPort(int port);

    // Formats:
    String GetTuple() const;       // Returns ["IP", port] equivalent for JSON arrays (or just IP:port for simplicity if needed, but python backend uses list)
    String GetString() const;      // Returns IP:port
    String GetHTTPLink() const;    // Returns http://IP:port
    
    bool operator==(const Address& other) const;
    
    static Address FromString(const String& addressString);
};

} // namespace types
} // namespace tamaf

#endif // TAMAF_ADDRESS_H
