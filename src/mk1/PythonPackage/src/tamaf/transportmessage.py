from typing import TYPE_CHECKING
from .aclmessage import ACLMessage
from .envelope import Envelope
from .transportdescription import TransportDescription

if TYPE_CHECKING:
    from .address import Address

class TransportMessage:

    def __init__(self, aclMessage: ACLMessage, senderAddress: 'Address' = None, receiverAddress: 'Address' = None):
        self.aclmessage = aclMessage
        if senderAddress and receiverAddress:
            self.envelope = Envelope(
                TransportDescription(senderAddress.GetHTTPLink()), 
                TransportDescription(receiverAddress.GetHTTPLink())
            )

    def ToDict(self) -> dict:
        toReturn = {
            "aclmessage": self.aclmessage.ToDict()
        }

        if hasattr(self, 'envelope'):
            toReturn["envelope"] = self.envelope.ToDict()
        
        return toReturn 

    @classmethod
    def FromDict(cls, data: dict):
        aclData = data.get("aclmessage")
        aclMessage = ACLMessage.FromDict(aclData) if aclData else None
        
        envData = data.get("envelope")
        envelope = Envelope.FromDict(envData) if envData else None
        
        transportMessageInstance = cls(aclMessage=aclMessage)
        transportMessageInstance.envelope = envelope
        
        return transportMessageInstance
