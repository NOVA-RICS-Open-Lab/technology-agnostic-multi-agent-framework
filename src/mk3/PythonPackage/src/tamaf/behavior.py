from abc import ABC, abstractmethod
from typing import TYPE_CHECKING
from .defines import EmptyReceiveException

if TYPE_CHECKING:
    from .aclmessage import ACLMessage
    from .aclmessagetemplate import ACLMessageTemplate
    from .agent import Agent
    from .emainteraction import EMAInteraction

class Behavior(ABC):
    
    def __init__(self, agent: 'Agent' = None):
        from .emainteraction import EMAInteraction
        self.isBlocked = False
        self.isStarted = False
        self.agent = agent
        self.messagesToPutBack: list['ACLMessage'] = []
        self.emaInteraction = EMAInteraction(self)

    def OnStart(self):
        pass
    
    @abstractmethod
    def Action(self):
        pass

    @abstractmethod
    def Done(self):
        pass

    def OnEnd(self):
        return 0

    def Block(self):
        self.isBlocked = True
    
    def UnBlock(self):
        self.isBlocked = False

    def IsBlocked(self):
        return self.isBlocked

    def Started(self):
        self.isStarted = True
    
    def IsStarted(self):
        return self.isStarted
    
    def Receive(self, aclMessageTemplate : 'ACLMessageTemplate'):
        aclMessage = self.agent.Receive(aclMessageTemplate)

        if aclMessage is None:
            raise EmptyReceiveException
        else:
            self.messagesToPutBack.insert(0, aclMessage)
        
        return aclMessage

    def Execute(self):
        self.messagesToPutBack: list['ACLMessage'] = []

        self.UnBlock()
        
        try:
            self.Action()

        except EmptyReceiveException:
            self.Block()
            for aclMessage in self.messagesToPutBack:
                self.PutBack(aclMessage)

    def Send(self, aclMessage: 'ACLMessage'):
        return self.agent.Send(aclMessage)
    
    def PutBack(self, aclMessage: 'ACLMessage'):
            return self.agent.PutBack(aclMessage)
