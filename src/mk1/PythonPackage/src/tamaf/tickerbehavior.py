import time
from abc import abstractmethod
from .behavior import Behavior

class TickerBehavior(Behavior):
    def __init__(self, agent, tickPeriod: int):
        super().__init__(agent)
        self.period = tickPeriod / 1000.0
        self.lastTick = time.time()

    def Action(self):
        now = time.time()
        if now >= self.lastTick + self.period:
            self.OnTick()
            self.lastTick = now

    @abstractmethod
    def OnTick(self):
        pass
    
    def Done(self):
        return False
