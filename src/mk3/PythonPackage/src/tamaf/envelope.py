from .transportdescription import TransportDescription

class Envelope:

    def __init__(self, senderTransportDescription: TransportDescription, receiverTransportDescription: TransportDescription):
        self.senderTransportDescription = senderTransportDescription
        self.receiverTransportDescription = receiverTransportDescription

    def ToDict(self) -> dict:
        return {
            "sender-transport-description": self.senderTransportDescription.ToDict(),
            "receiver-transport-description": self.receiverTransportDescription.ToDict()
        }
    
    @classmethod
    def FromDict(cls, data: dict):
        senderTransportDescription = TransportDescription.FromDict(data.get("sender-transport-description"))
        receiverTransportDescription = TransportDescription.FromDict(data.get("receiver-transport-description"))
        return cls(senderTransportDescription, receiverTransportDescription)
