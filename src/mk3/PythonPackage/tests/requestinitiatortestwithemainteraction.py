from tamaf import Agent, Address, AgentID, ACLMessage, Performative, FIPARequestInitiator, GetLocalIP, AgentDescription, SimpleBehavior
import time

class SimpleClientAgent(Agent):
    def Setup(self, emainteraction):
        
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

        class starterbehavior(SimpleBehavior):
            def __init__(self):
                super().__init__()
                self.done = False

            def Action(self):
                
                targetAgent = AgentID(name="Responder", address=Address(GetLocalIP()))
                
                agentDescriptions = self.emaInteraction.Search(AgentDescription(targetAgent))
                print(f"vou mostrar as descriptions que me apareceram: {agentDescriptions}")

                agents = []

                for description in agentDescriptions:
                    agents.append(description.agentid)
                
                requestMsg = ACLMessage(
                    performative=Performative.REQUEST,
                    receiver=agents,
                    content="Please calculate the data!"
                )

                self.agent.AddBehavior(MyInitiator(self, requestMsg))
                self.done = True
            
            def Done(self):
                return self.done

        self.AddBehavior(starterbehavior())

initiator = SimpleClientAgent("Initiator")
initiator.Start()

while True:
    time.sleep(1)