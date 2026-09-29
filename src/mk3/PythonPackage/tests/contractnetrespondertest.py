import time
from tamaf import Agent, Address, AgentID, ACLMessage, ACLMessageTemplate, Performative, FIPAContractNetResponder, GetLocalIP

class SimpleServerAgent(Agent):
    def Setup(self, emainteraction):
        
        # 1. Listen for any Call For Proposal (CFP) message
        template = ACLMessageTemplate(performatives={Performative.CFP})
        
        # 2. Instantiate and customize the Contract Net Responder
        class MyContractNetResponder(FIPAContractNetResponder):
            
            def handleCfp(self, cfpMessage):
                print(f"[{self.agent.agentDescription.agentid.name}] Received CFP: '{cfpMessage.content}' from {cfpMessage.sender.name}")
                print(f"[{self.agent.agentDescription.agentid.name}] Formulating a bid...")
                
                # To test a refusal, you would use Performative.REFUSE
                # For this test, we will submit a valid PROPOSE
                reply = cfpMessage.CreateReply(Performative.PROPOSE)
                reply.content = "I can calculate the data in 5 seconds for $10!"
                return reply

            def handleRejectProposal(self, cfpMessage, proposeMessage, rejectMessage):
                print(f"[{self.agent.agentDescription.agentid.name}] Darn, I lost the auction. Message: {rejectMessage.content}")
                print(f"[{self.agent.agentDescription.agentid.name}] Freeing up resources and going back to sleep.")
                return True
                
            def handleAcceptProposal(self, cfpMessage, proposeMessage, acceptMessage):
                print(f"[{self.agent.agentDescription.agentid.name}] WOOHOO! I won the auction! Message: {acceptMessage.content}")
                print(f"[{self.agent.agentDescription.agentid.name}] Allocating resources to begin the task...")
                print(f"[{self.agent.agentDescription.agentid.name}] Doing the heavy lifting...")
                
                # Simulate work
                time.sleep(5)

                # Send the final Inform
                print(f"[{self.agent.agentDescription.agentid.name}] Task complete. Sending results back to the initiator.")
                result = cfpMessage.CreateReply(Performative.INFORM)
                result.content = "Here is your calculated data: 42"
                return result

        # 3. Add it to the kernel
        self.AddBehavior(MyContractNetResponder(self, template))

responder = SimpleServerAgent("Responder")
responder.Start()

while True:
    time.sleep(1)