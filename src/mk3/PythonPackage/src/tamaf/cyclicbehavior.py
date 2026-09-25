from .simplebehavior import SimpleBehavior

class CyclicBehavior(SimpleBehavior):
    def Done(self):
        return False
