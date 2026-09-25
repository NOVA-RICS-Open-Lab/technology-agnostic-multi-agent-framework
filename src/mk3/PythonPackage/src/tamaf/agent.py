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

    def __init__(self, name: str, specificRegisterAddress: Address = None, specificEMAAddress: Address = None, debugLevel: int = 1):
        from .agentdescription import AgentDescription
        from .ams import AMS
        from .mts import MTS

        self.debugLevel = debugLevel
        self.agentDescription = AgentDescription(AgentID(name, Address(GetLocalIP())))
        self._mts = MTS(agent=self, registerAddress=specificRegisterAddress, emaAddress=specificEMAAddress)
        self._ams = AMS(agent=self)

        self.log_debug(1, f"Agent Born: {self.agentDescription.agentid.GetFullID()}")

    @property
    def mts(self) -> 'MTS':
        return self._mts

    @property
    def ams(self) -> 'AMS':
        return self._ams

    def log_debug(self, level_or_msg, message: str = None):
        if message is None:
            level = 2
            msg = level_or_msg
        else:
            level = level_or_msg
            msg = message

        if self.debugLevel >= level:
            id_name = self.agentDescription.agentid.name
            sufix = "[DEBUG-L1]" if level == 1 else "[DEBUG-L2]"
            print(f"[{id_name}]{sufix} {msg}")

    def getName(self) -> str :
        return self.agentDescription.agentid.GetFullID()

    def Setup(self, emaInteraction: 'EMAInteraction'):
        pass

    def Suspend(self):
        self._ams.Suspend()

    def Resume(self):
        self._ams.Resume()

    def Start(self):
        self._ams.Start()

    def TakeDown(self, emaInteraction: 'EMAInteraction'):
        pass

    def Stop(self):
        self._ams.Shutdown()

    def Send(self, aclMessage: 'ACLMessage'):
        return self._mts.Send(aclMessage)

    def Receive(self, aclMessageTemplate: 'ACLMessageTemplate'):
        return self._mts.Receive(aclMessageTemplate)

    def PutBack(self, aclMessage: 'ACLMessage'):
        self._mts.PutBack(aclMessage)

    def AddBehavior(self, behavior : 'Behavior'):
        behavior.agent = self
        self._ams.AddBehavior(behavior)
        
    def RemoveBehavior(self, behavior : 'Behavior'):
        self._ams.RemoveBehavior(behavior)
        
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
        return self._ams.IsAlive()
