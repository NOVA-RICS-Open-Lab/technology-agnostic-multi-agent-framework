#ifndef FMS_CONSTANTS_H
#define FMS_CONSTANTS_H

#include <Arduino.h>
#include <vector>
#include <string>

// Service Types
#define DF_SERVICE_TYPE_SKILL "SKILL_TYPE"
#define DF_SERVICE_TYPE_TRANSPORT "TRANSPORT_TYPE"
#define DF_SERVICE_TYPE_RESOURCE "RESOURCE_TYPE"
#define DF_SERVICE_NAME_TRANSPORT "TRANSPORT_NAME"

// Ontologies
#define ONTOLOGY_NEGOTIATE_NEXT_RESOURCE "CNET_NEXT_RESOURCE"
#define ONTOLOGY_REQUEST_TRANSPORT "REQ_TRANSPORT"
#define ONTOLOGY_REQUEST_EXECUTE_SKILL "REQ_EXECUTE_SKILL"
#define ONTOLOGY_REQUEST_UNPLUG_RESOURCE "REQ_UNPLUG_RESOURCE"
#define ONTOLOGY_REQUEST_UPDATE_CONFIG "REQ_UPDATE_CONFIG"

// Controller IP Configuration
#define CONTROLLER_IP ""

enum class Location {
    A, B, C, D, E, F, UNKNOWN
};

inline String locationToString(Location loc) {
    switch (loc) {
        case Location::A: return "A";
        case Location::B: return "B";
        case Location::C: return "C";
        case Location::D: return "D";
        case Location::E: return "E";
        case Location::F: return "F";
        default: return "UNKNOWN";
    }
}

inline Location stringToLocation(const String& str) {
    if (str == "A") return Location::A;
    if (str == "B") return Location::B;
    if (str == "C") return Location::C;
    if (str == "D") return Location::D;
    if (str == "E") return Location::E;
    if (str == "F") return Location::F;
    return Location::UNKNOWN;
}

inline Location getNextLocation(Location loc) {
    switch (loc) {
        case Location::A: return Location::B;
        case Location::B: return Location::C;
        case Location::C: return Location::D;
        case Location::D: return Location::E;
        case Location::E: return Location::F;
        case Location::F: return Location::A;
        default: return Location::A;
    }
}

struct SkillInfo {
    String name;
    String serviceType;
    String url;
};

inline String formatAgentName(const String& name) {
    if (!name.endsWith("_esp32")) {
        return name + "_esp32";
    }
    return name;
}

#endif // FMS_CONSTANTS_H
