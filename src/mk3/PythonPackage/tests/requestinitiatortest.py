from tamaf import Agent, Address, AgentID, ACLMessage, Performative, FIPARequestInitiator, GetLocalIP
import time

class SimpleClientAgent(Agent):
    def Setup(self, emainteraction):
        
        # 1. Prepare the initial request message
        target_agent = AgentID(name="Responder", address=Address(GetLocalIP(), int(input("PORTO DO RECETOR: ")))) # Adjust based on your ID structure
        # target_agent = AgentID(name="ResponderESP32", address=Address("192.168.0.104", 4000))
        
        request_msg = ACLMessage(
            sender=self.agentDescription.agentid,
            performative=Performative.REQUEST,
            receiver=[target_agent],
            content="Please calculate the data!"
            # content="Please calculate the data ESP32!"
        )
        
        # 2. Instantiate and customize the initiator
        class MyInitiator(FIPARequestInitiator):
            def handleAgree(self, msg):
                print(f"[{self.agent.agentDescription.agentid.name}] Server agreed! Waiting for results...")
                return True
                
            def handleRefuse(self, msg):
                print(f"[{self.agent.agentDescription.agentid.name}] Server refused the request.")
                return True
                
            def handleInform(self, msg):
                print(f"[{self.agent.agentDescription.agentid.name}] Success! Server says: {msg.content}")
                return True
                
            def handleFailure(self, msg):
                print(f"[{self.agent.agentDescription.agentid.name}] Server failed to complete the task.")
                return True

        self.AddBehavior(MyInitiator(self, request_msg))

initiator = SimpleClientAgent("Initiator")
initiator.Start()

while True:
    time.sleep(1)