from .behavior import Behavior

DEFAULT = "DEFAULT"

class FSMBehavior(Behavior):
    
    def __init__(self, agent):
        super().__init__(agent)
        self.states: dict[str, Behavior] = {}
        self.initialState: str = None
        self.currentState: str = None
        self.finalStates: set[str] = set()
        self.transitions: dict[tuple[str, int], str] = {}
        self.sharedData: dict = {}

    def AddState(self, behavior: Behavior, stateName: str):
        behavior.agent = self.agent
        self.states[stateName] = behavior
    
    def AddInitialState(self, behavior: Behavior, stateName: str):
        behavior.agent = self.agent
        self.initialState = stateName
        self.currentState = stateName
        self.states[stateName] = behavior
    
    def AddFinalState(self, behavior: Behavior, stateName: str):
        behavior.agent = self.agent
        self.finalStates.add(stateName)
        self.states[stateName] = behavior

    def AddTransition(self, sourceStateName: str, destinationStateName: str, transitionCode: int):
        self.transitions[(sourceStateName, transitionCode)] = destinationStateName

    def AddDefaultTransition(self, sourceStateName: str, destinationStateName: str):
        self.transitions[(sourceStateName, DEFAULT)] = destinationStateName

    def OnStart(self):
        if self.currentState in self.states:
            self.states[self.currentState].OnStart()
    
    def Action(self):
        if self.currentState in self.states:
            self.states[self.currentState].Execute()
            
            if self.states[self.currentState].IsBlocked():
                self.Block()

    def Done(self):
        if self.currentState not in self.states:
            return True
            
        if not self.states[self.currentState].Done():
            return False

        if self.currentState in self.finalStates:
            self.states[self.currentState].OnEnd()
            return True
        
        
        transitionCode = self.states[self.currentState].OnEnd()

        nextState = self.transitions.get((self.currentState, transitionCode))

        if nextState is not None:
            self.currentState = nextState
            self.isStarted = False
            return False
        
        nextState = self.transitions.get((self.currentState, DEFAULT))
        if nextState is not None:
            self.currentState = nextState
            self.isStarted = False
            return False
        
        return True

    def OnEnd(self):
        self.sharedData = {}
        self.currentState = self.initialState
        self.isStarted = False
        return 0