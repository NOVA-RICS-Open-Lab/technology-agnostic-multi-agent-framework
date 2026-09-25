#ifndef TAMAF_TRANSPORTTYPE_H
#define TAMAF_TRANSPORTTYPE_H

#include <Arduino.h>

namespace tamaf {
namespace enums {

/**
 * @brief Defines the physical transport layer for the Message Transport Service.
 */
enum class TransportType {
    HTTP,
    UDP,
    TCP,
    UNKNOWN
};

inline String TransportTypeToString(TransportType type) {
    switch (type) {
        case TransportType::HTTP: return "HTTP";
        case TransportType::UDP: return "UDP";
        case TransportType::TCP: return "TCP";
        default: return "UNKNOWN";
    }
}

inline TransportType StringToTransportType(const String& type) {
    if (type == "HTTP") return TransportType::HTTP;
    if (type == "UDP") return TransportType::UDP;
    if (type == "TCP") return TransportType::TCP;
    return TransportType::UNKNOWN;
}

} // namespace enums
} // namespace tamaf

#endif // TAMAF_TRANSPORTTYPE_H
