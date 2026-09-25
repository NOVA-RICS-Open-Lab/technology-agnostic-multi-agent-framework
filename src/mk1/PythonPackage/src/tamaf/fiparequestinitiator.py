import uuid
from typing import TYPE_CHECKING
from .fsmbehavior import FSMBehavior
from .simplebehavior import SimpleBehavior
from .oneshotbehavior import OneShotBehavior
from .aclmessagetemplate import ACLMessageTemplate
from .performative import Performative

if TYPE_CHECKING:
    from .agent import Agent
    from .aclmessage import ACLMessage

class FIPARequestInitiator(FSMBehavior):
    
    def __init__(self, agent: 'Agent', requestMessage: 'ACLMessage'):
        super().__init__(agent)
        self.requestMessage = requestMessage
        
        if not getattr(self.requestMessage, 'conversationid', None):
            self.requestMessage.conversationid = f"req-{uuid.uuid4()}"
            
        self.replyTemplate = ACLMessageTemplate(
            conversationID=self.requestMessage.conversationid
        )

        class SendRequestBehavior(SimpleBehavior):
            def __init__(self, fsm: 'FIPARequestInitiator'):
                super().__init__(fsm.agent)
                self.fsm = fsm
                self.is_done: bool

            def OnStart(self):
                super().OnStart()
                self.is_done = False

            def Action(self):
                requests_to_send = self.fsm.prepareRequests(self.fsm.requestMessage)
                if requests_to_send is None:
                    return

                for req in requests_to_send:
                    if not getattr(req, 'conversationid', None):
                        req.conversationid = self.fsm.requestMessage.conversationid
                    self.agent.Send(req)
                self.is_done = True

            def Done(self):
                return self.is_done

            def OnEnd(self):
                return 0

        class WaitFirstResponseBehavior(OneShotBehavior):
            def __init__(self, fsm: 'FIPARequestInitiator'):
                super().__init__(fsm.agent)
                self.fsm = fsm
                self.transitionCode: int
            
            def OnStart(self):
                super().OnStart()
                self.transitionCode = -1

            def Action(self):
                reply = self.Receive(self.fsm.replyTemplate)
                self.fsm.sharedData['reply'] = reply
                
                if reply.performative == Performative.AGREE:
                    self.transitionCode = 0
                elif reply.performative == Performative.REFUSE:
                    self.transitionCode = 1
                elif reply.performative == Performative.NOT_UNDERSTOOD:
                    self.transitionCode = 2
                elif reply.performative == Performative.INFORM:
                    self.transitionCode = 3
                elif reply.performative == Performative.FAILURE:
                    self.transitionCode = 4
                else:
                    self.transitionCode = 2

            def OnEnd(self):
                return self.transitionCode

        class WaitResultBehavior(OneShotBehavior):
            def __init__(self, fsm: 'FIPARequestInitiator'):
                super().__init__(fsm.agent)
                self.fsm = fsm
                self.transitionCode: int
            
            def OnStart(self):
                super().OnStart()
                self.transitionCode = -1

            def Action(self):
                reply = self.Receive(self.fsm.replyTemplate)
                self.fsm.sharedData['reply'] = reply
                
                if reply.performative == Performative.INFORM:
                    self.transitionCode = 0
                else:
                    self.transitionCode = 1 

            def OnEnd(self):
                return self.transitionCode

        class HandleAgreeBehavior(SimpleBehavior):
            def __init__(self, fsm: 'FIPARequestInitiator'):
                super().__init__(fsm.agent)
                self.fsm = fsm
                self.is_done: bool
            
            def OnStart(self):
                super().OnStart()
                self.is_done = False

            def Action(self):
                res = self.fsm.handleAgree(self.fsm.sharedData['reply'])
                if res is None or res is False:
                    return
                self.is_done = True
            def Done(self):
                return self.is_done
            def OnEnd(self):
                return 0

        class HandleRefuseBehavior(SimpleBehavior):
            def __init__(self, fsm: 'FIPARequestInitiator'):
                super().__init__(fsm.agent)
                self.fsm = fsm
                self.is_done: bool
            
            def OnStart(self):
                super().OnStart()
                self.is_done = False

            def Action(self):
                res = self.fsm.handleRefuse(self.fsm.sharedData['reply'])
                if res is None or res is False:
                    return
                self.is_done = True
            def Done(self):
                return self.is_done

        class HandleNotUnderstoodBehavior(SimpleBehavior):
            def __init__(self, fsm: 'FIPARequestInitiator'):
                super().__init__(fsm.agent)
                self.fsm = fsm
                self.is_done: bool
            
            def OnStart(self):
                super().OnStart()
                self.is_done = False

            def Action(self):
                res = self.fsm.handleNotUnderstood(self.fsm.sharedData['reply'])
                if res is None or res is False:
                    return
                self.is_done = True
            def Done(self):
                return self.is_done

        class HandleInformBehavior(SimpleBehavior):
            def __init__(self, fsm: 'FIPARequestInitiator'):
                super().__init__(fsm.agent)
                self.fsm = fsm
                self.is_done: bool
            
            def OnStart(self):
                super().OnStart()
                self.is_done = False

            def Action(self):
                res = self.fsm.handleInform(self.fsm.sharedData['reply'])
                if res is None or res is False:
                    return
                self.is_done = True
            def Done(self):
                return self.is_done

        class HandleFailureBehavior(SimpleBehavior):
            def __init__(self, fsm: 'FIPARequestInitiator'):
                super().__init__(fsm.agent)
                self.fsm = fsm
                self.is_done: bool
            
            def OnStart(self):
                super().OnStart()
                self.is_done = False

            def Action(self):
                res = self.fsm.handleFailure(self.fsm.sharedData['reply'])
                if res is None or res is False:
                    return
                self.is_done = True
            def Done(self):
                return self.is_done

        self.AddInitialState(SendRequestBehavior(self), "sendRequest")
        self.AddState(WaitFirstResponseBehavior(self), "waitFirstResponse")
        self.AddState(HandleAgreeBehavior(self), "handleAgree")
        self.AddState(WaitResultBehavior(self), "waitResult")
        
        self.AddFinalState(HandleRefuseBehavior(self), "handleRefuse")
        self.AddFinalState(HandleNotUnderstoodBehavior(self), "handleNotUnderstood")
        self.AddFinalState(HandleInformBehavior(self), "handleInform")
        self.AddFinalState(HandleFailureBehavior(self), "handleFailure")

        self.AddTransition("sendRequest", "waitFirstResponse", 0)
        self.AddTransition("waitFirstResponse", "handleAgree", 0)
        self.AddTransition("waitFirstResponse", "handleRefuse", 1)
        self.AddTransition("waitFirstResponse", "handleNotUnderstood", 2)
        self.AddTransition("waitFirstResponse", "handleInform", 3)
        self.AddTransition("waitFirstResponse", "handleFailure", 4)
        self.AddTransition("handleAgree", "waitResult", 0)
        self.AddTransition("waitResult", "handleInform", 0)
        self.AddTransition("waitResult", "handleFailure", 1)

    def handleAgree(self, aclMessage: 'ACLMessage') -> bool:
        return True
    def handleRefuse(self, aclMessage: 'ACLMessage') -> bool:
        return True
    def handleInform(self, aclMessage: 'ACLMessage') -> bool:
        return True
    def handleFailure(self, aclMessage: 'ACLMessage') -> bool:
        return True
    def handleNotUnderstood(self, aclMessage: 'ACLMessage') -> bool:
        return True
    def prepareRequests(self, requestMessage: 'ACLMessage') -> list['ACLMessage']:
        return [requestMessage]
