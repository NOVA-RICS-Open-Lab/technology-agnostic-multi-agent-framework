from typing import TYPE_CHECKING
from .cyclicbehavior import CyclicBehavior
from .aclmessagetemplate import ACLMessageTemplate
from .performative import Performative
from .fiparequestresponderfordispatcher import FIPARequestResponderForDispatcher

if TYPE_CHECKING:
    from .agent import Agent
    from .aclmessage import ACLMessage

class FIPARequestDispatcher(CyclicBehavior):
    def __init__(self, agent: 'Agent', template: ACLMessageTemplate = None):
        super().__init__(agent)
        self.template = template if template else ACLMessageTemplate(performatives={Performative.REQUEST})

    def Action(self):
        incomingRequest = self.Receive(self.template)
        if incomingRequest:
            responder = self.createResponder(incomingRequest)
            if responder is not None:
                self.agent.AddBehavior(responder)

    def createResponder(self, requestMessage: 'ACLMessage') -> FIPARequestResponderForDispatcher:
        return FIPARequestResponderForDispatcher(self.agent, requestMessage)
