from typing import TYPE_CHECKING
from .fsmbehavior import FSMBehavior
from .simplebehavior import SimpleBehavior
from .performative import Performative

if TYPE_CHECKING:
    from .agent import Agent
    from .aclmessage import ACLMessage

class FIPARequestResponderForDispatcher(FSMBehavior):
    
    TRANSITION_AGREED = 0
    TRANSITION_REFUSED = 1
    TRANSITION_DONE = 0
    
    def __init__(self, agent: 'Agent', requestMessage: 'ACLMessage'):
        super().__init__(agent)
        self.requestMessage = requestMessage

        class HandleRequestBehavior(SimpleBehavior):
            def __init__(self, fsm: 'FIPARequestResponderForDispatcher'):
                super().__init__(fsm.agent)
                self.fsm = fsm
                self.transitionCode = -1
                self.is_done = False

            def Action(self):
                reply = self.fsm.handleRequest(self.fsm.requestMessage)
                if reply is None:
                    return

                if getattr(self.fsm.requestMessage, 'conversationid', None):
                    reply.conversationid = self.fsm.requestMessage.conversationid
                
                self.fsm.sharedData['agreeMessage'] = reply
                self.agent.Send(reply)
                
                if reply.performative == Performative.AGREE:
                    self.transitionCode = self.fsm.TRANSITION_AGREED
                else:
                    self.transitionCode = self.fsm.TRANSITION_REFUSED
                
                self.is_done = True

            def Done(self):
                return self.is_done

            def OnEnd(self):
                return self.transitionCode

        class PrepareResultBehavior(SimpleBehavior):
            def __init__(self, fsm: 'FIPARequestResponderForDispatcher'):
                super().__init__(fsm.agent)
                self.fsm = fsm
                self.is_done = False

            def Action(self):
                resultMsg = self.fsm.prepareResultNotification(
                    self.fsm.requestMessage, 
                    self.fsm.sharedData['agreeMessage']
                )
                if resultMsg is None:
                    return

                if getattr(self.fsm.requestMessage, 'conversationid', None):
                    resultMsg.conversationid = self.fsm.requestMessage.conversationid
                self.agent.Send(resultMsg)
                self.is_done = True

            def Done(self):
                return self.is_done

            def OnEnd(self):
                return self.fsm.TRANSITION_DONE

        class EndResponderBehavior(OneShotBehavior):
            def Action(self): pass 

        self.AddInitialState(HandleRequestBehavior(self), "handleRequest")
        self.AddState(PrepareResultBehavior(self), "prepareResult")
        self.AddFinalState(EndResponderBehavior(self), "end")

        self.AddTransition("handleRequest", "prepareResult", self.TRANSITION_AGREED)
        self.AddTransition("handleRequest", "end", self.TRANSITION_REFUSED)
        self.AddTransition("prepareResult", "end", self.TRANSITION_DONE)

    def handleRequest(self, requestMessage: 'ACLMessage') -> 'ACLMessage':
        pass
    def prepareResultNotification(self, requestMessage: 'ACLMessage', responseMessage: 'ACLMessage') -> 'ACLMessage':
        pass
