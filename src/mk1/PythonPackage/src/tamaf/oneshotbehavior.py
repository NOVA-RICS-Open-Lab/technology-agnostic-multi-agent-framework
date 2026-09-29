from .simplebehavior import SimpleBehavior

class OneShotBehavior(SimpleBehavior):
    def Done(self):
        return True
