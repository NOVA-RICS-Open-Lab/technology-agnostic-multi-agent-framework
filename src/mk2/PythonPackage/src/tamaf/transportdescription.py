from .transporttype import TransportType

class TransportDescription:
    
    def __init__(self, transportSpecificAddress: str, transportType: TransportType = TransportType.HTTP):
        self.transportType = transportType
        self.transportSpecificAddress = transportSpecificAddress
    
    @classmethod
    def FromDict(cls, data: dict):
        t_type = TransportType(data.get("transport-type", TransportType.HTTP.value))
        transportSpecificAddress = data.get("transport-specific-address")
        return cls(transportSpecificAddress, t_type)
    
    def ToDict(self):
        return {
            "transport-type": self.transportType.value,
            "transport-specific-address": self.transportSpecificAddress
        }
