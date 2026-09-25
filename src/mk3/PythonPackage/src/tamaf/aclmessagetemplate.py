from typing import TYPE_CHECKING
from .performative import Performative

if TYPE_CHECKING:
    from .core import AgentID
    from .aclmessage import ACLMessage

class ACLMessageTemplate:
    
    def __init__(self, sender: 'AgentID' = None, performatives: set[Performative] = None, conversationid: str = None, protocol: str = None, ontology: str = None, conversationID: str = None):
        self.sender = sender
        self.performatives = performatives
        self.conversationid = conversationid if conversationid is not None else conversationID
        self.protocol = protocol
        self.ontology = ontology

    @property
    def conversationID(self):
        return self.conversationid

    @conversationID.setter
    def conversationID(self, value):
        self.conversationid = value

    def SetPerformative(self, performative: Performative):

        if self.performatives is None:
            self.performatives = set()

        self.performatives.add(performative)

    def SetOntology(self, ontology: str):
        self.ontology = ontology

    def Matches(self, message: 'ACLMessage') -> bool:
        from .agentid import AgentID

        if self.sender is not None:
            if not AgentID.Matches(self.sender, message.sender):
                return False
        
        if self.performatives is not None:
            if message.performative not in self.performatives:
                return False
        
        if self.conversationid is not None:
            if message.conversationid != self.conversationid:
                return False
        
        if self.protocol is not None:
            if message.protocol != self.protocol:
                return False
        
        if self.ontology is not None:
            if message.ontology != self.ontology:
                return False
                
        return True

    @classmethod
    def MatchPerformative(cls, performative: Performative) -> 'ACLMessageTemplate':
        tmpl = cls()
        tmpl.SetPerformative(performative)
        return tmpl

    @classmethod
    def MatchConversationId(cls, conversationid: str) -> 'ACLMessageTemplate':
        return cls(conversationid=conversationid)

    @classmethod
    def MatchSender(cls, sender: 'AgentID') -> 'ACLMessageTemplate':
        return cls(sender=sender)
