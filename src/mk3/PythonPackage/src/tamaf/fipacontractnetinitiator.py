import time
import uuid
from typing import TYPE_CHECKING
from .fsmbehavior import FSMBehavior
from .simplebehavior import SimpleBehavior
from .oneshotbehavior import OneShotBehavior
from .aclmessagetemplate import ACLMessageTemplate
from .performative import Performative

from .defines import DEFAULT_CONTRACTNETINITIATOR_RECEIVE_CFPS_TIMEOUT

if TYPE_CHECKING:
    from .agent import Agent
    from .aclmessage import ACLMessage

class FIPAContractNetInitiator(FSMBehavior):
    def __init__(self, agent: 'Agent', cfpMessage: 'ACLMessage'):
        super().__init__(agent)
        self.cfpMessage = cfpMessage
        self.deadline = time.time() + DEFAULT_CONTRACTNETINITIATOR_RECEIVE_CFPS_TIMEOUT
        
        if not getattr(self.cfpMessage, 'conversationid', None):
            self.cfpMessage.conversationid = f"cnet-{uuid.uuid4()}"
            
        self.replyTemplate = ACLMessageTemplate(
            conversationID=self.cfpMessage.conversationid
        )
        
        self.expectedCFPs = 0
        self.sharedData['proposals'] = []
        self.sharedData['accepts'] = []
        self.sharedData['results'] = []

        class SendCfpBehavior(SimpleBehavior):
            def __init__(self, fsm: 'FIPAContractNetInitiator'):
                super().__init__(fsm.agent)
                self.fsm = fsm
                self.is_done: bool
                            
            def OnStart(self):
                super().OnStart()
                self.is_done = False

            def Action(self):
                cfps = self.fsm.prepareCfps(self.fsm.cfpMessage)
                if cfps is None:
                    return

                for msg in cfps:
                    self.fsm.expectedCFPs += len(msg.receiver)
                    msg.conversationid = self.fsm.cfpMessage.conversationid
                    self.agent.Send(msg)
                self.is_done = True
            
            def Done(self): return self.is_done
            def OnEnd(self): return 0

        class CollectProposalsBehavior(SimpleBehavior):
            def __init__(self, fsm: 'FIPAContractNetInitiator'):
                super().__init__(fsm.agent)
                self.fsm = fsm
                self.cfpsReceived: int = 0
                self.transitionCode: int
                self.is_done: bool
            
            def OnStart(self):
                super().OnStart()
                self.transitionCode = -1
                self.is_done = False

            def Action(self):
                if self.cfpsReceived >= self.fsm.expectedCFPs or time.time() > self.fsm.deadline:
                    self.transitionCode = 2
                    self.is_done = True
                    return         

                reply = self.Receive(self.fsm.replyTemplate)
                if reply is None:
                    return

                self.fsm.sharedData['current_reply'] = reply
                self.cfpsReceived += 1

                if reply.performative == Performative.PROPOSE:
                    self.transitionCode = 0
                elif reply.performative == Performative.REFUSE:
                    self.transitionCode = 1
                else:
                    self.transitionCode = 1

                self.is_done = True
            
            def Done(self): return self.is_done
            def OnEnd(self): 
                
                if self.transitionCode == 2:
                    self.cfpsReceived = 0
                return self.transitionCode

        class HandleProposeBehavior(SimpleBehavior):
            def __init__(self, fsm: 'FIPAContractNetInitiator'):
                super().__init__(fsm.agent)
                self.fsm = fsm
                self.is_done: bool
            
            def OnStart(self):
                super().OnStart()
                self.is_done = False

            def Action(self):
                msg = self.fsm.sharedData['current_reply']
                res = self.fsm.handlePropose(msg)
                if res is None or res is False:
                    return
                self.fsm.sharedData['proposals'].append(msg)
                self.is_done = True
            def Done(self): return self.is_done
            def OnEnd(self): return 0

        class HandleRefuseBehavior(SimpleBehavior):
            def __init__(self, fsm: 'FIPAContractNetInitiator'):
                super().__init__(fsm.agent)
                self.fsm = fsm
                self.is_done: bool
            
            def OnStart(self):
                super().OnStart()
                self.is_done = False

            def Action(self):
                msg = self.fsm.sharedData['current_reply']
                res = self.fsm.handleRefuse(msg)
                if res is None or res is False:
                    return
                self.fsm.sharedData['proposals'].append(msg)
                self.is_done = True
            def Done(self): return self.is_done
            def OnEnd(self): return 0

        class HandleProposalsBehavior(SimpleBehavior):
            def __init__(self, fsm: 'FIPAContractNetInitiator'):
                super().__init__(fsm.agent)
                self.fsm = fsm
                self.transitionCode: int
                self.is_done: bool
                self.responses: list['ACLMessage']
            
            def OnStart(self):
                super().OnStart()
                self.is_done = False
                self.transitionCode = -1
                self.responses = []

            def Action(self):
                proposals = self.fsm.sharedData['proposals']
                if not self.fsm.handleProposals(proposals, self.responses):
                    return

                self.fsm.sharedData['responses'] = self.responses
                accepts = []
                for msg in self.responses:
                    msg.conversationid = self.fsm.cfpMessage.conversationid
                    if msg.performative == Performative.ACCEPT_PROPOSAL:
                        accepts.append(msg)
                    self.agent.Send(msg)
                    
                self.fsm.sharedData['accepts'] = accepts
                self.transitionCode = 0 if len(self.fsm.sharedData['accepts']) > 0 else 1
                self.is_done = True
            
            def Done(self): return self.is_done
            def OnEnd(self): return self.transitionCode

        class WaitResultsBehavior(SimpleBehavior):
            def __init__(self, fsm: 'FIPAContractNetInitiator'):
                super().__init__(fsm.agent)
                self.fsm = fsm
                self.transitionCode: int
                self.is_done: bool
                
            def OnStart(self):
                super().OnStart()
                self.is_done = False
                self.transitionCode = -1

            def Action(self):
                results = self.fsm.sharedData['results']
                accepts = self.fsm.sharedData['accepts']

                if len(results) >= len(accepts):
                    self.transitionCode = 2
                    self.is_done = True
                    return

                reply = self.Receive(self.fsm.replyTemplate)
                if reply is None:
                    return

                self.fsm.sharedData['current_reply'] = reply
                if reply.performative == Performative.INFORM:
                    self.transitionCode = 0
                elif reply.performative == Performative.FAILURE:
                    self.transitionCode = 1
                else:
                    self.transitionCode = 1

                self.is_done = True
            
            def Done(self): return self.is_done
            def OnEnd(self): return self.transitionCode

        class HandleInformBehavior(SimpleBehavior):
            def __init__(self, fsm: 'FIPAContractNetInitiator'):
                super().__init__(fsm.agent)
                self.fsm = fsm
                self.is_done: bool
            
            def OnStart(self):
                super().OnStart()
                self.is_done = False

            def Action(self):
                msg = self.fsm.sharedData['current_reply']
                res = self.fsm.handleInform(msg)
                if res is None or res is False:
                    return
                self.fsm.sharedData['results'].append(msg)
                self.is_done = True
            def Done(self): return self.is_done
            def OnEnd(self): return 0

        class HandleFailureBehavior(SimpleBehavior):
            def __init__(self, fsm: 'FIPAContractNetInitiator'):
                super().__init__(fsm.agent)
                self.fsm = fsm
                self.is_done: bool
            
            def OnStart(self):
                super().OnStart()
                self.is_done = False

            def Action(self):
                msg = self.fsm.sharedData['current_reply']
                res = self.fsm.handleFailure(msg)
                if res is None or res is False:
                    return
                self.fsm.sharedData['results'].append(msg)
                self.is_done = True
            def Done(self): return self.is_done
            def OnEnd(self): return 0

        class EnderBehavior(OneShotBehavior):
            def Action(self): pass

        self.AddInitialState(SendCfpBehavior(self), "sendCFP")
        self.AddState(CollectProposalsBehavior(self), "collect")
        self.AddState(HandleProposeBehavior(self), "handlePropose")
        self.AddState(HandleRefuseBehavior(self), "handleRefuse")
        self.AddState(HandleProposalsBehavior(self), "handleProposals")
        self.AddState(WaitResultsBehavior(self), "waitResults")
        self.AddState(HandleInformBehavior(self), "handleInform")
        self.AddState(HandleFailureBehavior(self), "handleFailure")
        self.AddFinalState(EnderBehavior(self), "end")

        self.AddTransition("sendCFP", "collect", 0)
        self.AddTransition("collect", "handlePropose", 0)
        self.AddTransition("collect", "handleRefuse", 1)
        self.AddTransition("collect", "handleProposals", 2)
        
        self.AddTransition("handlePropose", "collect", 0)
        self.AddTransition("handleRefuse", "collect", 0)

        self.AddTransition("handleProposals", "waitResults", 0)
        self.AddTransition("handleProposals", "end", 1)

        self.AddTransition("waitResults", "handleInform", 0)
        self.AddTransition("waitResults", "handleFailure", 1)
        self.AddTransition("waitResults", "end", 2)

        self.AddTransition("handleInform", "waitResults", 0)
        self.AddTransition("handleFailure", "waitResults", 0)

    def prepareCfps(self, cfpMessage: 'ACLMessage') -> list['ACLMessage']:
        return [cfpMessage]
    def handlePropose(self, msg: 'ACLMessage') -> bool:
        return True
    def handleRefuse(self, msg: 'ACLMessage') -> bool:
        return True
    def handleProposals(self, proposals: list['ACLMessage'], responses: list['ACLMessage']) -> bool:
        return True
    def handleInform(self, msg: 'ACLMessage') -> bool:
        return True
    def handleFailure(self, msg: 'ACLMessage') -> bool:
        return True
