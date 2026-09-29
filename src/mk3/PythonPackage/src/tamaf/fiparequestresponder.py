from typing import TYPE_CHECKING
from .fsmbehavior import FSMBehavior
from .simplebehavior import SimpleBehavior
from .performative import Performative

if TYPE_CHECKING:
    from .agent import Agent
    from .aclmessage import ACLMessage
    from .aclmessagetemplate import ACLMessageTemplate

class FIPARequestResponder(FSMBehavior):
    
    def __init__(self, agent: 'Agent', requestTemplate: 'ACLMessageTemplate'):
        super().__init__(agent)
        self.requestTemplate = requestTemplate

        class HandleRequestBehavior(SimpleBehavior):
            def __init__(self, fsm: 'FIPARequestResponder'):
                super().__init__(fsm.agent)
                self.fsm = fsm
                self.transitionCode: int
                self.is_done: bool

            def OnStart(self):
                super().OnStart()
                self.is_done = False
                self.transitionCode = -1

            def Action(self):
                requestMsg = self.Receive(self.fsm.requestTemplate)
                reply = self.fsm.handleRequest(requestMsg)

                if reply is None:
                    return

                if getattr(requestMsg, 'conversationid', None):
                    reply.conversationid = requestMsg.conversationid
                
                self.agent.Send(reply)
                
                if reply.performative == Performative.AGREE:
                    self.fsm.sharedData['request'] = requestMsg
                    self.fsm.sharedData['agreeMessage'] = reply
                    self.transitionCode = 0
                else:
                    self.transitionCode = 1 
                
                self.is_done = True

            def Done(self):
                return self.is_done

            def OnEnd(self):
                return self.transitionCode

        class PrepareResultBehavior(SimpleBehavior):
            def __init__(self, fsm: 'FIPARequestResponder'):
                super().__init__(fsm.agent)
                self.fsm = fsm
                self.is_done: bool

            def OnStart(self):
                super().OnStart()
                self.is_done = False

            def Action(self):
                requestMsg = self.fsm.sharedData['request']
                agreeMsg = self.fsm.sharedData['agreeMessage']
                resultMsg = self.fsm.prepareResultNotification(requestMsg, agreeMsg)

                if resultMsg is None:
                    return

                if getattr(requestMsg, 'conversationid', None):
                    resultMsg.conversationid = requestMsg.conversationid
                self.agent.Send(resultMsg)
                self.is_done = True

            def Done(self):
                return self.is_done

            def OnEnd(self):
                return 0 

        self.AddInitialState(HandleRequestBehavior(self), "handleRequest")
        self.AddState(PrepareResultBehavior(self), "prepareResult")
        
        self.AddTransition("handleRequest", "prepareResult", 0) 
        self.AddTransition("handleRequest", "handleRequest", 1)   
        self.AddTransition("prepareResult", "handleRequest", 0)     

    def handleRequest(self, requestMessage: 'ACLMessage') -> 'ACLMessage':
        pass
        
    def prepareResultNotification(self, requestMessage: 'ACLMessage', responseMessage: 'ACLMessage') -> 'ACLMessage':
        pass
