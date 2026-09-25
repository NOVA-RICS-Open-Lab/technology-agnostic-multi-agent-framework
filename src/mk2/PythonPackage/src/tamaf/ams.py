import time
import threading
import asyncio
from typing import TYPE_CHECKING

from .lifecyclestate import LifeCycleState
from .address import Address
from .agentid import AgentID
from .performative import Performative
from .aclmessage import ACLMessage
from .aclmessagetemplate import ACLMessageTemplate
from .oneshotbehavior import OneShotBehavior
from .tickerbehavior import TickerBehavior
from .simplebehavior import SimpleBehavior
from .agentmanagementontology import AgentManagementOntology
from .defines import (
    DEFAULT_EMA_HAS_NO_MORE_PORTS,
    DEFAULT_EMA_NAME,
    DEFAULT_EMA_HEARTBEAT_INTERVAL,
    DEFAULT_KERNEL_STABILIZATION_TIME,
    DEFAULT_LOOP_STABILIZATION_TIME,
    ALL_INTERFACES,
    DEFAULT_AGENT_ALREADY_EXISTS,
    DEFAULT_OUTSIDE_AGENT,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_INFORM_CONCEPT,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_AGENTADDRESS_CONCEPT,
    AgentAlreadyExistsException,
    EMANoMorePortsException,
    EMAFailureException
)

if TYPE_CHECKING:
    from .agent import Agent
    from .behavior import Behavior
    from .simplebehavior import SimpleBehavior

class AMS:
    _shared_loop = None
    _shared_loop_thread = None
    _shared_loop_lock = threading.Lock()

    @classmethod
    def get_shared_loop(cls):
        with cls._shared_loop_lock:
            if cls._shared_loop is None:
                cls._shared_loop = asyncio.new_event_loop()
                def custom_exception_handler(loop, context):
                    exc = context.get('exception')
                    msg = context.get('message', '')
                    # Ignore known asyncio ProactorEventLoop bug on Windows when rapidly opening/closing servers
                    if isinstance(exc, AssertionError) and 'start_serving' in msg:
                        return
                    loop.default_exception_handler(context)
                cls._shared_loop.set_exception_handler(custom_exception_handler)
                
                def run_loop():
                    asyncio.set_event_loop(cls._shared_loop)
                    cls._shared_loop.run_forever()
                cls._shared_loop_thread = threading.Thread(target=run_loop, daemon=True, name="TAMAF-Async-EventLoop")
                cls._shared_loop_thread.start()
        return cls._shared_loop

    class InitializationBehavior(SimpleBehavior):
        def __init__(self, agent: 'Agent' = None):
            self.isRegistered = False
            self.initializationDone = False
            self.serverStarted = False
            super().__init__(agent)

        def Action(self):
            if self.agent.agentDescription.agentid.name == DEFAULT_EMA_NAME:
                if not self.serverStarted:
                    self.agent.mts.StartTCPServer(Address(ALL_INTERFACES, self.agent.mts.emaAddress.port))
                    self.serverStarted = True
                if not self.agent.mts.serverOnline:
                    return
                self.agent.Setup(self.emaInteraction)
                self.isRegistered = True
                self.initializationDone = True
                return

            if self.isRegistered is False:
                if self.agent.mts.serverOnline is False:
                    
                    if self.serverStarted:
                        return
                    
                    self.agent.mts.StartTCPServer(Address(ALL_INTERFACES, self.agent.mts.registeraddress.port))
                    self.serverStarted = True
                    if not self.agent.mts.serverOnline:
                        return

                agree = self.emaInteraction.RegisterAgent()
                
                concept = AgentManagementOntology.GetConcept(agree.content)

                if concept == DEFAULT_AGENTMANAGEMENT_ONTOLOGY_INFORM_CONCEPT:

                    inform = AgentManagementOntology.GetAttributes(agree.content)

                    if inform == DEFAULT_AGENT_ALREADY_EXISTS:
                        raise AgentAlreadyExistsException
                    if inform == DEFAULT_EMA_HAS_NO_MORE_PORTS:
                        raise EMANoMorePortsException
                    if inform == DEFAULT_OUTSIDE_AGENT:
                        self.isRegistered = True
                
                if concept == DEFAULT_AGENTMANAGEMENT_ONTOLOGY_AGENTADDRESS_CONCEPT:
                    self.agent.mts.StopMessageServer()
                    newAddress = AgentManagementOntology.GetAgentAddressFromAttributes(agree.content)

                    self.agent.mts.serverOnline = False
                    self.agent.mts.StartTCPServer(Address(ALL_INTERFACES, newAddress.port))

                    self.isRegistered = True
                    return # Wait for the new server to boot
    
            if not self.agent.mts.serverOnline:
                return # Block until the final assigned port is fully bound

            self.heartBeatBehavior = self.agent.ams.HeartBeatBehavior(self.agent)
            self.agent.AddBehavior(self.heartBeatBehavior)
            self.agent.Setup(self.emaInteraction)
            self.initializationDone = True

        def Done(self):
            return self.initializationDone

    class ClosingBehavior(OneShotBehavior):
        def __init__(self, agent: 'Agent' = None, deRegister: bool = True):
            self.deRegister = deRegister
            super().__init__(agent)

        def SetDeRegisterFalse(self):
            self.deRegister = False

        def Action(self):

            from .defines import EMAFailureException 

            self.agent.TakeDown(self.emaInteraction)
            if self.agent.agentDescription.agentid.name != DEFAULT_EMA_NAME:
                if self.deRegister:
                    self.emaInteraction.DeRegisterAgent()

    class HeartBeatBehavior(TickerBehavior):
        def __init__(self, agent: 'Agent'):
            super().__init__(agent, DEFAULT_EMA_HEARTBEAT_INTERVAL)

        def OnTick(self):
            self.emaInteraction.KeepAliveAgent()

    def __init__(self, agent: 'Agent'):
        self.agent = agent
        self.lifeCycleState = LifeCycleState.INITIATED
        self.lifeCycleStateLock = threading.Lock()
        self.activeBehaviorList: list['SimpleBehavior'] = []
        self.pendingAdditions: list['Behavior'] = []
        self.pendingRemovals: list['Behavior'] = []
        self.blockedBehaviorList: list['SimpleBehavior'] = []
        self.kernelTask = None
        self.heartBeatBehavior = None

    def GetLifeCycleState(self):
        with self.lifeCycleStateLock:
            return self.lifeCycleState

    def Shutdown(self):
        with self.lifeCycleStateLock:
            self.lifeCycleState = LifeCycleState.CLOSING

    def Suspend(self):
        with self.lifeCycleStateLock:
            if self.lifeCycleState == LifeCycleState.ACTIVE:
                self.lifeCycleState = LifeCycleState.SUSPENDED

    def Resume(self):
        with self.lifeCycleStateLock:
            if self.lifeCycleState == LifeCycleState.SUSPENDED:
                self.lifeCycleState = LifeCycleState.ACTIVE

    async def Kernel(self):
        # Ensure newMessageEvent is created in the event loop thread
        if self.agent.mts.newMessageEvent is None:
            self.agent.mts.newMessageEvent = asyncio.Event()
            
        initializationBehavior = self.InitializationBehavior(self.agent)
        closingBehavior = self.ClosingBehavior(self.agent)

        while True:
            try:
                with self.lifeCycleStateLock:
                    currentLifeCycleState = self.lifeCycleState

                if currentLifeCycleState == LifeCycleState.SUSPENDED:
                    if self.heartBeatBehavior:
                        if self.agent.mts.newMessageEvent.is_set():
                            if self.heartBeatBehavior.IsBlocked():
                                self.heartBeatBehavior.UnBlock()
                            self.agent.mts.newMessageEvent.clear()
                        
                        if not self.heartBeatBehavior.IsStarted():
                            self.heartBeatBehavior.OnStart()
                            self.heartBeatBehavior.Started()

                        self.heartBeatBehavior.Execute()
                        
                        if self.heartBeatBehavior.IsBlocked():
                            await asyncio.sleep(0)
                            continue
                            
                        if self.heartBeatBehavior.Done():
                            self.heartBeatBehavior.OnEnd()
                        
                    await asyncio.sleep(0)
                    continue
                
                elif currentLifeCycleState == LifeCycleState.INITIATED:
                    if self.agent.mts.newMessageEvent.is_set():
                        if initializationBehavior.IsBlocked():
                            initializationBehavior.UnBlock()
                        self.agent.mts.newMessageEvent.clear()
                    
                    if not initializationBehavior.IsStarted():
                        initializationBehavior.OnStart()
                        initializationBehavior.Started()

                    initializationBehavior.Execute()
                    
                    if initializationBehavior.IsBlocked():
                        await asyncio.sleep(0)
                        continue
                        
                    if initializationBehavior.Done():
                        initializationBehavior.OnEnd()
                        with self.lifeCycleStateLock:
                            self.lifeCycleState = LifeCycleState.ACTIVE
                
                elif currentLifeCycleState == LifeCycleState.ACTIVE:
                    if self.agent.mts.newMessageEvent.is_set():
                        self.UnblockBehaviors()
                        self.agent.mts.newMessageEvent.clear()
                    
                    for behavior in self.pendingRemovals:
                        if behavior in self.activeBehaviorList:
                            self.activeBehaviorList.remove(behavior)
                    self.pendingRemovals.clear()

                    if self.pendingAdditions:
                        self.activeBehaviorList.extend(self.pendingAdditions)
                        self.pendingAdditions.clear()
                    
                    for behavior in list(self.activeBehaviorList):
                        with self.lifeCycleStateLock:
                            if self.lifeCycleState != LifeCycleState.ACTIVE:
                                break
                        
                        if not behavior.IsStarted():
                            behavior.OnStart()
                            behavior.Started()

                        behavior.Execute()
                        
                        if behavior.IsBlocked():
                            self.BlockBehavior(behavior)
                            continue
                            
                        if behavior.Done():
                            behavior.OnEnd()
                            if behavior in self.activeBehaviorList:
                                self.activeBehaviorList.remove(behavior)
                                
                elif currentLifeCycleState == LifeCycleState.CLOSING:
                    if self.agent.mts.newMessageEvent.is_set():
                        if closingBehavior.IsBlocked():
                            closingBehavior.UnBlock()
                        self.agent.mts.newMessageEvent.clear()
                    
                    if not closingBehavior.IsStarted():
                        closingBehavior.OnStart()
                        closingBehavior.Started()

                    closingBehavior.Execute()
                    
                    if closingBehavior.IsBlocked():
                        await asyncio.sleep(0)
                        continue
                        
                    if closingBehavior.Done():
                        closingBehavior.OnEnd()
                        self.agent.mts.StopMessageServer()
                        print(f"{self.agent.agentDescription.agentid.GetFullID()}: Agent Taken Down")
                        break

                await asyncio.sleep(0)
            except (AgentAlreadyExistsException, EMANoMorePortsException, EMAFailureException) as e:
                print(f"{self.agent.agentDescription.agentid.GetFullID()}: Agent Exception: {e}")                
                closingBehavior.SetDeRegisterFalse()
            
                with self.lifeCycleStateLock:
                    self.lifeCycleState = LifeCycleState.CLOSING
                    
            except Exception as e:
                import traceback
                traceback.print_exc()
                print(f"{self.agent.agentDescription.agentid.GetFullID()}: Agent Exception: {e}")                
                
                with self.lifeCycleStateLock:
                    self.lifeCycleState = LifeCycleState.CLOSING

    def Receive(self, aclMessageTemplate: 'ACLMessageTemplate'):
        return self.agent.mts.Receive(aclMessageTemplate)

    def Send(self, aclMessage: ACLMessage):
        if aclMessage.sender is None:
            aclMessage.sender = self.agent.agentDescription.agentid
        return self.agent.mts.Send(aclMessage)

    def PutBack(self, aclMessage: ACLMessage):
        self.agent.mts.PutBack(aclMessage)

    def AddBehavior(self, behavior: 'Behavior'):
        self.pendingAdditions.append(behavior)
        
    def RemoveBehavior(self, behavior: 'Behavior'):
        self.pendingRemovals.append(behavior)
        
    def UnblockBehaviors(self):
        for behavior in self.blockedBehaviorList:
            behavior.UnBlock()
        self.activeBehaviorList.extend(self.blockedBehaviorList)
        self.blockedBehaviorList.clear()

    def BlockBehavior(self, behavior: 'SimpleBehavior'):
        if behavior in self.activeBehaviorList:
            self.activeBehaviorList.remove(behavior)
        self.blockedBehaviorList.append(behavior)

    def Start(self):
        loop = self.get_shared_loop()
        self.kernelTask = asyncio.run_coroutine_threadsafe(self.Kernel(), loop)

    def IsAlive(self):
        return self.kernelTask is not None and not self.kernelTask.done()
