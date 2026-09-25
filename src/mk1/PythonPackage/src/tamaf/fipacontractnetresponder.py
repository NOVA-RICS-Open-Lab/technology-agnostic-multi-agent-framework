from typing import TYPE_CHECKING
from .fsmbehavior import FSMBehavior
from .simplebehavior import SimpleBehavior
from .oneshotbehavior import OneShotBehavior
from .aclmessagetemplate import ACLMessageTemplate
from .performative import Performative

if TYPE_CHECKING:
    from .agent import Agent
    from .aclmessage import ACLMessage

class FIPAContractNetResponder(FSMBehavior):
    
    def __init__(self, agent: 'Agent', cfpTemplate: 'ACLMessageTemplate'):
        super().__init__(agent)
        self.cfpTemplate = cfpTemplate

        class WaitCfpBehavior(OneShotBehavior):
            def __init__(self, fsm: 'FIPAContractNetResponder'):
                super().__init__(fsm.agent)
                self.fsm = fsm

            def Action(self):
                cfpMsg = self.Receive(self.fsm.cfpTemplate)
                self.fsm.sharedData['cfp'] = cfpMsg
                
                self.fsm.sharedData['reply_template'] = ACLMessageTemplate(
                    conversationID=cfpMsg.conversationid
                )

            def OnEnd(self):
                return 0

        class PrepareResponseBehavior(SimpleBehavior):
            def __init__(self, fsm: 'FIPAContractNetResponder'):
                super().__init__(fsm.agent)
                self.fsm = fsm
                self.transitionCode: int
                self.is_done: bool

            def OnStart(self):
                super().OnStart()
                self.is_done = False
                self.transitionCode = -1

            def Action(self):
                cfpMsg = self.fsm.sharedData['cfp']
                reply = self.fsm.handleCfp(cfpMsg)
                if reply is None:
                    return

                if getattr(cfpMsg, 'conversationid', None):
                    reply.conversationid = cfpMsg.conversationid
                
                self.fsm.sharedData['propose_msg'] = reply
                self.agent.Send(reply)
                
                if reply.performative == Performative.PROPOSE:
                    self.transitionCode = 0 
                else:
                    self.transitionCode = 1 
                self.is_done = True

            def Done(self):
                return self.is_done

            def OnEnd(self):
                return self.transitionCode

        class WaitAcceptRejectBehavior(SimpleBehavior):
            def __init__(self, fsm: 'FIPAContractNetResponder'):
                super().__init__(fsm.agent)
                self.fsm = fsm
                self.transitionCode: int
                self.is_done: bool
            
            def OnStart(self):
                super().OnStart()
                self.is_done = False
                self.transitionCode = -1

            def Action(self):
                reply = self.Receive(self.fsm.sharedData['reply_template'])
                if reply is None:
                    return

                cfpMsg = self.fsm.sharedData['cfp']
                proposeMsg = self.fsm.sharedData['propose_msg']

                if reply.performative == Performative.ACCEPT_PROPOSAL:
                    self.fsm.sharedData['accept_msg'] = reply
                    self.transitionCode = 0 
                else:
                    res = self.fsm.handleRejectProposal(cfpMsg, proposeMsg, reply)
                    if res is None or res is False:
                        return
                    self.transitionCode = 1 
                self.is_done = True

            def Done(self):
                return self.is_done

            def OnEnd(self):
                return self.transitionCode

        class ExecuteTaskBehavior(SimpleBehavior):
            def __init__(self, fsm: 'FIPAContractNetResponder'):
                super().__init__(fsm.agent)
                self.fsm = fsm
                self.is_done: bool

            def OnStart(self):
                super().OnStart()
                self.is_done = False

            def Action(self):
                cfpMsg = self.fsm.sharedData['cfp']
                proposeMsg = self.fsm.sharedData['propose_msg']
                acceptMsg = self.fsm.sharedData['accept_msg']
                resultMsg = self.fsm.handleAcceptProposal(cfpMsg, proposeMsg, acceptMsg)
                if resultMsg is None:
                    return
                
                if getattr(cfpMsg, 'conversationid', None):
                    resultMsg.conversationid = cfpMsg.conversationid
                self.agent.Send(resultMsg)
                self.is_done = True

            def Done(self):
                return self.is_done

            def OnEnd(self):
                return 0 

        self.AddInitialState(WaitCfpBehavior(self), "waitCfp")
        self.AddState(PrepareResponseBehavior(self), "prepareResponse")
        self.AddState(WaitAcceptRejectBehavior(self), "waitAcceptReject")
        self.AddState(ExecuteTaskBehavior(self), "executeTask")

        self.AddTransition("waitCfp", "prepareResponse", 0)
        self.AddTransition("prepareResponse", "waitAcceptReject", 0) 
        self.AddTransition("prepareResponse", "waitCfp", 1)           
        self.AddTransition("waitAcceptReject", "executeTask", 0)     
        self.AddTransition("waitAcceptReject", "waitCfp", 1)         
        self.AddTransition("executeTask", "waitCfp", 0)               

    def handleCfp(self, cfpMessage: 'ACLMessage') -> 'ACLMessage':
        pass
    def handleAcceptProposal(self, cfpMessage: 'ACLMessage', proposeMessage: 'ACLMessage', acceptMessage: 'ACLMessage') -> 'ACLMessage':
        pass
    def handleRejectProposal(self, cfpMessage: 'ACLMessage', proposeMessage: 'ACLMessage', rejectMessage: 'ACLMessage') -> bool:
        return True