from typing import TYPE_CHECKING
from .performative import Performative

if TYPE_CHECKING:
    from .core import AgentID
    from .aclmessage import ACLMessage

class ACLMessageTemplate:
    
    def __init__(self, sender: 'AgentID' = None,  performatives: set[Performative] = None, conversationID: str = None, protocol: str = None, ontology: str = None):
        self.sender = sender
        self.performatives = performatives
        self.conversationID = conversationID
        self.protocol = protocol
        self.ontology = ontology

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
        
        if self.conversationID is not None:
            if message.conversationid != self.conversationID:
                return False
        
        if self.protocol is not None:
            if message.protocol != self.protocol:
                return False
        
        if self.ontology is not None:
            if message.ontology != self.ontology:
                return False
                
        return True
