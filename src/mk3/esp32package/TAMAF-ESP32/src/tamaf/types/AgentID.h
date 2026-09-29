#ifndef TAMAF_AGENTID_H
#define TAMAF_AGENTID_H

#include <Arduino.h>
#include "Address.h"

namespace tamaf {
namespace types {

class AgentID {
private:
    String name;
    Address address;

public:
    AgentID();
    AgentID(const String& name, const Address& address);

    String getName() const;
    void setName(const String& name);

    Address getAddress() const;
    void setAddress(const Address& address);
    void setPort(int port);

    String GetFullID() const;
    String GetIDNoPort() const;

    bool operator==(const AgentID& other) const;

    static AgentID FromString(const String& aidString);
    static bool Matches(const AgentID& templateID, const AgentID& targetID);
};

} // namespace types
} // namespace tamaf

#endif // TAMAF_AGENTID_H
