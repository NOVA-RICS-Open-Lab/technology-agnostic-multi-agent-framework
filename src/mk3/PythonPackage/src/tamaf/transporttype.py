from enum import Enum

class TransportType(str, Enum):
    HTTP = "http"
    UDP = "udp"
    TCP = "tcp"
