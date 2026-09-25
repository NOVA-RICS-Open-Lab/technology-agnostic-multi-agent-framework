from .behavior import Behavior

class SequentialBehavior(Behavior):
    def __init__(self, agent):
        super().__init__(agent)
        self.subBehaviors: list[Behavior] = []
        self.currentIndex: int = 0
        self.sharedData: dict = {}

    def AddSubBehavior(self, behavior: Behavior):
        behavior.agent = self.agent
        self.subBehaviors.append(behavior)

    def OnStart(self):
        if self.currentIndex < len(self.subBehaviors):
            self.subBehaviors[self.currentIndex].OnStart()
    
    def Action(self):
        if self.currentIndex < len(self.subBehaviors):
            self.subBehaviors[self.currentIndex].Execute()
            if self.subBehaviors[self.currentIndex].IsBlocked():
                self.Block()

    def Done(self):
        if self.currentIndex >= len(self.subBehaviors):
            return True
            
        if not self.subBehaviors[self.currentIndex].Done():
            return False
        else:
            self.subBehaviors[self.currentIndex].OnEnd()
            self.currentIndex += 1
            if self.currentIndex >= len(self.subBehaviors):
                return True
            else:
                self.isStarted = False
                return False

    def OnEnd(self):
        self.sharedData = {}
        self.currentIndex = 0
        self.isStarted = False
        return 0