from tamaf import Agent, SimpleBehavior, OneShotBehavior, WakerBehavior, CyclicBehavior, SequentialBehavior, LaunchEMA, FSMBehavior, AgentDescription, ServiceDescription
import time

class IamBoundToDieAgent(Agent):
    class suicideBehavior(WakerBehavior):
        def __init__(self, agent):
            super().__init__(agent, 30000)
        def OnWake(self):
            print("I die!!!")
            self.agent.Stop()
            
    def Setup(self, emaInteraction):
        self.AddBehavior(self.suicideBehavior(self))

class IChangeMyselfAgent(Agent):
    class addServiceBehavior(WakerBehavior):
        def __init__(self, agent):
            super().__init__(agent, 30000)
        def OnWake(self):
            print("I Change!!!")
            agentTemplate = self.agent.GetAgentDescriptionTemplate()
            
            service = ServiceDescription("gas", "do bom")

            agentTemplate.AddService(service)

            self.agent.UpdateAgentDescription(agentTemplate)

    def Setup(self, emaInteraction):
        self.AddBehavior(self.addServiceBehavior(self))
        
agent = IChangeMyselfAgent("Agent")
agent.Start()

while agent.IsAlive():
    time.sleep(1)