#include "AgentID.h"

namespace tamaf {
namespace types {

AgentID::AgentID() : name(""), address(Address()) {}

AgentID::AgentID(const String& name, const Address& address) : name(name), address(address) {}

String AgentID::getName() const { return name; }
void AgentID::setName(const String& name) { this->name = name; }

Address AgentID::getAddress() const { return address; }
void AgentID::setAddress(const Address& address) { this->address = address; }
void AgentID::setPort(int port) { this->address.setPort(port); }

String AgentID::GetFullID() const {
    if (name.length() == 0 && address.getIp().length() == 0) return "";
    if (address.getIp() == "" || address.getIp() == "0.0.0.0") return name + "@None:None";
    return name + "@" + address.GetString();
}

String AgentID::GetIDNoPort() const {
    if (name.length() == 0 && address.getIp().length() == 0) return "";
    if (address.getIp() == "" || address.getIp() == "0.0.0.0") return name + "@None";
    return name + "@" + address.getIp();
}

bool AgentID::operator==(const AgentID& other) const {
    return (name == other.name && address == other.address);
}

AgentID AgentID::FromString(const String& aidString) {
    if (aidString.length() == 0) return AgentID();
    
    int atIndex = aidString.indexOf('@');
    if (atIndex > 0) {
        String namePart = aidString.substring(0, atIndex);
        String addressPart = aidString.substring(atIndex + 1);
        return AgentID(namePart, Address::FromString(addressPart));
    }
    return AgentID(aidString, Address());
}

bool AgentID::Matches(const AgentID& templateID, const AgentID& targetID) {
    if (templateID.getName().length() > 0 && templateID.getName() != targetID.getName()) return false;
    
    if (templateID.getAddress().getIp().length() > 0 && 
        templateID.getAddress().getIp() != targetID.getAddress().getIp()) return false;
        
    if (templateID.getAddress().getPort() > 0 && 
        templateID.getAddress().getPort() != targetID.getAddress().getPort()) return false;

    return true;
}

} // namespace types
} // namespace tamaf
