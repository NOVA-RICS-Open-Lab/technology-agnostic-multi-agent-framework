import time
from abc import abstractmethod
from .behavior import Behavior

class WakerBehavior(Behavior):
    def __init__(self, agent, timeout_ms: int):
        super().__init__(agent)
        self.wake_up_time = time.time() + (timeout_ms / 1000.0)
        self.done_flag = False

    def Action(self):
        if time.time() >= self.wake_up_time:
            self.OnWake()
            self.done_flag = True

    @abstractmethod
    def OnWake(self):
        pass

    def Done(self):
        return self.done_flag
