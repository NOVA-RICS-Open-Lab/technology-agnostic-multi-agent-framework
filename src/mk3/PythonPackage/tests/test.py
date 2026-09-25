from tamaf import Agent, SimpleBehavior, OneShotBehavior, WakerBehavior, CyclicBehavior, SequentialBehavior, LaunchEMA, FSMBehavior
import time

class PingerAgent(Agent):
    class ThreePingsBehavior(SimpleBehavior):
        def __init__(self):
            super().__init__()
            self.ping_count = 0
        def Action(self):
            print(f"Ping: {self.ping_count}")
            self.ping_count += 1
        def Done(self):
            return self.ping_count >= 3
        
    class PimbaBehavior(OneShotBehavior):
        def Action(self):
            print(f"PIMBA")
    
    class TauasBehavior(CyclicBehavior):
        def Action(self):
            print(f"Tauas")

    def Setup(self, emaInteraction):
        # Test basic
        # self.AddBehavior(self.PimbaBehavior())
        # self.AddBehavior(self.ThreePingsBehavior())
        # self.AddBehavior(self.TauasBehavior())

        # Test Sequencial
        sequencialBehavior = SequentialBehavior(self)
        sequencialBehavior.AddSubBehavior(self.ThreePingsBehavior())
        sequencialBehavior.AddSubBehavior(self.PimbaBehavior())    
        sequencialBehavior.AddSubBehavior(self.ThreePingsBehavior())
        self.AddBehavior(sequencialBehavior)

class IamBoundToDieAgent(Agent):
    class suicideBehavior(WakerBehavior):
        def __init__(self, agent):
            super().__init__(agent, 30000)
        def OnWake(self):
            print("I die!!!")
            self.agent.Stop()
            
    def Setup(self, emaInteraction):
        self.AddBehavior(self.suicideBehavior(self))

class FSMTestAgent(Agent):
    
    class KeyboardInputBehavior(OneShotBehavior):

        def __init__(self, agent = None):
            self.number = 0
            super().__init__(agent)

        def Action(self):
            self.number = int(input("Numero a dar no OnEnd: "))

        def OnEnd(self):
            return self.number
        
    def Setup(self, emaInteraction):
        # Test FSM
        fsmBehavior = FSMBehavior(self)
        fsmBehavior.AddInitialState(self.KeyboardInputBehavior(), "primeiro")
        fsmBehavior.AddState(self.KeyboardInputBehavior(), "segundo")
        fsmBehavior.AddState(self.KeyboardInputBehavior(), "terceiro")
        fsmBehavior.AddFinalState(self.KeyboardInputBehavior(), "quarto")
        
        fsmBehavior.AddTransition("primeiro", "segundo", 0)
        fsmBehavior.AddDefaultTransition("primeiro", "primeiro")
        fsmBehavior.AddTransition("segundo", "terceiro", 0)
        fsmBehavior.AddTransition("segundo", "quarto", 2)
        fsmBehavior.AddDefaultTransition("terceiro", "segundo")
        self.AddBehavior(fsmBehavior)

#LaunchEMA()
agent1 = PingerAgent("Test1Agent")
agent1.Start()
agent2 = PingerAgent("Test2Agent")
agent2.Start()
# agent3 = FSMTestAgent("Test3Agent")
# agent3.Start()
# agent4 = IamBoundToDieAgent("Test4Agent")
# agent4.Start()

while agent1.IsAlive() or agent2.IsAlive():
    time.sleep(1)