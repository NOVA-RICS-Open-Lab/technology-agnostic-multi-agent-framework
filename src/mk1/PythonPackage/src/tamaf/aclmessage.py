from datetime import datetime
from typing import TYPE_CHECKING
from .performative import Performative

if TYPE_CHECKING:
    from .agentid import AgentID

class ACLMessage:
    
    def __init__(self, performative: Performative, sender: 'AgentID' = None, receiver: list['AgentID'] = None, content = None, language: str = None, ontology: str = None, protocol: str = None, conversationid: str = None, replyto: list['AgentID'] = None, replyby: datetime = None):
        self.sender = sender
        self.receiver = receiver if receiver else []
        self.performative = performative
        self.content = content
        self.language = language
        self.ontology = ontology
        self.protocol = protocol
        self.conversationid = conversationid
        self.replywith = None 
        self.inreplyto = None 
        self.replyto = replyto
        self.replyby = replyby
        self.encoding = None

    def CreateReply(self, performative: Performative = None) -> 'ACLMessage':
        if self.replyto and len(self.replyto) > 0:
            replyReceivers = list(self.replyto)
        else:
            replyReceivers = [self.sender] if self.sender else []

        reply = ACLMessage(
            sender=None,
            performative=performative,
            receiver=replyReceivers,
            language=self.language,
            ontology=self.ontology,
            protocol=self.protocol,
            conversationid=self.conversationid
        )

        if self.replywith:
            reply.inreplyto = self.replywith

        return reply

    def ToDict(self) -> dict:
        payload = {
            "performative": self.performative.value if self.performative else None,
            "sender": self.sender.GetFullID() if self.sender else None,
            "receiver": [receiver.GetFullID() for receiver in self.receiver],
            "reply-to": [replyto.GetFullID() for replyto in self.replyto] if self.replyto else None,
            "reply-by": self.replyby.isoformat() if self.replyby else None,
            "protocol": self.protocol,
            "content": self.content,
            "ontology": self.ontology,
            "language": self.language,            
            "conversation-id": self.conversationid,
            "reply-with": self.replywith,
            "in-reply-to": self.inreplyto,
            "encoding": self.encoding
        }

        clean_payload = {
            key: value for key, value in payload.items() 
            if value is not None and value != []
        }
        
        return clean_payload
    
    @classmethod
    def FromDict(cls, data: dict):
        from .agentid import AgentID
        
        perfStr = data.get("performative")
        performative = Performative(perfStr) if perfStr else None
        
        senderStr = data.get("sender")
        sender = AgentID.FromString(senderStr) if senderStr else None
        
        receiverStrs = data.get("receiver", [])
        receiver = [AgentID.FromString(r) for r in receiverStrs]
        
        replytoStrs = data.get("reply-to", [])
        replyto = [AgentID.FromString(r) for r in replytoStrs]
        
        replybyStr = data.get("reply-by")
        replyby = datetime.fromisoformat(replybyStr) if replybyStr else None
 
        message = cls(
            sender=sender,
            performative=performative,
            receiver=receiver,
            content=data.get("content"),
            ontology=data.get("ontology"),
            protocol=data.get("protocol"),
            language=data.get("language"),
            conversationid=data.get("conversation-id"),
            replyto=replyto,
            replyby=replyby
        )
        
        message.replywith = data.get("reply-with")
        message.inreplyto = data.get("in-reply-to")
        message.encoding = data.get("encoding")
        
        return message
