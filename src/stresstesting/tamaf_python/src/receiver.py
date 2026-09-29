from tamaf import Agent, CyclicBehavior, ACLMessageTemplate, Performative
import time

class ReceiverAgent(Agent):
    class ReceiveBehavior(CyclicBehavior):
        def Action(self):
            # Match only REQUEST messages
            template = ACLMessageTemplate(performatives={Performative.REQUEST})
            msg = self.Receive(template)
            
            receive_time = time.time()
            sender_name = msg.sender.name if msg.sender else "Unknown"
            print(f"[RECEIVER] Received message from {sender_name} at {receive_time}")
            
            # Send acknowledgement
            reply = msg.CreateReply(Performative.INFORM)
            reply.content = "ACK"
            self.agent.Send(reply)

    def Setup(self, emainteraction):
        self.AddBehavior(self.ReceiveBehavior())
