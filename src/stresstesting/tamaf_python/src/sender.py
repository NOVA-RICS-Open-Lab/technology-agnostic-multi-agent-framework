from tamaf import Agent, SimpleBehavior, ACLMessage, Performative, AgentID, Address, ACLMessageTemplate
import time

class SenderAgent(Agent):
    def __init__(self, name, trigger_time, receiver_ip, receiver_port):
        super().__init__(name)
        self.trigger_time = trigger_time
        self.receiver_agentid = AgentID(name="receiver", address=Address(receiver_ip, receiver_port))
        self.rtt = None
        
    class SendBehavior(SimpleBehavior):
        def __init__(self, agent):
            super().__init__()
            self.my_agent = agent
            self.sent = False

        def Action(self):
            if not self.sent:
                # Wait for trigger time by yielding
                if time.time() < self.my_agent.trigger_time:
                    return
                
                self.send_time = time.time()
                msg = ACLMessage(
                    sender=self.my_agent.agentDescription.agentid,
                    performative=Performative.REQUEST,
                    receiver=[self.my_agent.receiver_agentid],
                    content=str(self.send_time)
                )
                self.my_agent.Send(msg)
                self.sent = True
            
            # Now block wait for ACK (will raise EmptyReceiveException if none yet)
            reply = self.Receive(ACLMessageTemplate(performatives={Performative.INFORM}))
            receive_time = time.time()
            self.my_agent.rtt = receive_time - self.send_time

        def Done(self):
            return self.my_agent.rtt is not None
            
    def Setup(self, emainteraction):
        self.AddBehavior(self.SendBehavior(self))
