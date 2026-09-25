from tamaf import address
from typing import TYPE_CHECKING
from .agentid import AgentID
from .address import Address
from .utils import GetLocalIP

if TYPE_CHECKING:
    from .agentdescription import AgentDescription
    from .ams import AMS
    from .mts import MTS
    from .emainteraction import EMAInteraction
    from .aclmessage import ACLMessage
    from .aclmessagetemplate import ACLMessageTemplate
    from .behavior import Behavior

from .oneshotbehavior import OneShotBehavior

class ModifyAgentEMABehavior(OneShotBehavior):
    def __init__(self, agent: 'Agent' = None):
        super().__init__(agent)
    def Action(self):
        self.emaInteraction.ModifyAgent()

class Agent:

    def __init__(self, name: str, specificRegisterAddress: Address = None, specificEMAAddress: Address = None):
        from .agentdescription import AgentDescription
        from .ams import AMS
        from .mts import MTS

        self.agentDescription = AgentDescription(AgentID(name, Address(GetLocalIP())))
        self.mts = MTS(agent=self, registerAddress=specificRegisterAddress, emaAddress=specificEMAAddress)
        self.ams = AMS(agent=self)

        print(f"New Agent was born: {self.agentDescription.agentid.GetFullID()}")

    def getName(self) -> str :
        return self.agentDescription.agentid.GetFullID()

    def Setup(self, emaInteraction: 'EMAInteraction'):
        pass

    def Suspend(self):
        self.ams.Suspend()

    def Resume(self):
        self.ams.Resume()

    def Start(self):
        self.ams.Start()

    def TakeDown(self, emaInteraction: 'EMAInteraction'):
        pass

    def Stop(self):
        self.ams.Shutdown()

    def Send(self, aclMessage: 'ACLMessage'):
        return self.ams.Send(aclMessage)

    def Receive(self, aclMessageTemplate: 'ACLMessageTemplate'):
        return self.ams.Receive(aclMessageTemplate)

    def AddBehavior(self, behavior : 'Behavior'):
        behavior.agent = self
        self.ams.AddBehavior(behavior)
        
    def RemoveBehavior(self, behavior : 'Behavior'):
        self.ams.RemoveBehavior(behavior)
        
    def UpdateAgentDescription(self, agentDescription: 'AgentDescription'):
        self.agentDescription.services.clear()
        for service in agentDescription.services:
            self.agentDescription.AddService(service)
        
        self.AddBehavior(ModifyAgentEMABehavior())

    def GetAgentDescription(self) -> 'AgentDescription':
        return self.agentDescription.copy()
    
    def GetAgentDescriptionTemplate(self) -> 'AgentDescription':
        from .agentdescription import AgentDescription
        return AgentDescription(self.agentDescription.agentid)

    def IsAlive(self) -> bool:
        return self.ams.IsAlive()
