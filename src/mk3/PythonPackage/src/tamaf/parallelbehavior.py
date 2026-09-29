from .behavior import Behavior

class ParallelBehavior(Behavior):
    def __init__(self, agent, finishWhenAnyDone: bool = True):
        super().__init__(agent)
        self.subBehaviors: list[Behavior] = []
        self.finishWhenAnyDone = finishWhenAnyDone
        self.sharedData: dict = {}
    
    def AddSubBehavior(self, behavior: Behavior):
        behavior.agent = self.agent
        self.subBehaviors.append(behavior)

    def OnStart(self):
        for behavior in self.subBehaviors:
            behavior.OnStart()
    
    def Action(self):
        all_children_blocked = True
        
        for behavior in self.subBehaviors:
            if behavior.Done():
                continue
                
            if not behavior.IsBlocked():
                behavior.Execute()
                all_children_blocked = False

        if all_children_blocked:
            self.Block()

    def Done(self):
        if self.finishWhenAnyDone:
            for behavior in self.subBehaviors:
                if behavior.Done():
                    for b in self.subBehaviors:
                        b.OnEnd()
                    return True
            return False
        else:
            for behavior in self.subBehaviors:
                if not behavior.Done():
                    return False
            for behavior in self.subBehaviors:
                behavior.OnEnd()
            return True

    def OnEnd(self):
        self.sharedData = {}
        self.isStarted = False
        return 0