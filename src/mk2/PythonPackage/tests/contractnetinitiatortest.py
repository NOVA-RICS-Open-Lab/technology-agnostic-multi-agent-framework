from tamaf import Agent, Address, AgentID, ACLMessage, Performative, FIPAContractNetInitiator, GetLocalIP
import time

class SimpleClientAgent(Agent):
    def Setup(self, emainteraction):
        
        # 1. Prepare the initial Call For Proposal (CFP)
        target_agent = AgentID(name="Responder", address=Address(GetLocalIP(), int(input("PORTO DO RECETOR: ")))) 
        # Target the ESP32 Responder
        # target_agent = AgentID(name="ResponderESP32", address=Address("192.168.0.104", 4000))

        cfp_msg = ACLMessage(
            sender=self.agentDescription.agentid,
            performative=Performative.CFP,
            receiver=[target_agent],
            content="Auction: Who can calculate this data the fastest?"
        )
        
        # 2. Instantiate and customize the Contract Net Initiator
        class MyContractNetInitiator(FIPAContractNetInitiator):
            
            def handlePropose(self, msg):
                print(f"[{self.agent.agentDescription.agentid.name}] Received a PROPOSE from {msg.sender.name}: {msg.content}")
                return True
                
            def handleRefuse(self, msg):
                print(f"[{self.agent.agentDescription.agentid.name}] {msg.sender.name} refused the CFP.")
                return True

            def handleProposals(self, proposals: list['ACLMessage'], responses: list['ACLMessage']) -> None:
                print(f"[{self.agent.agentDescription.agentid.name}] Deadline reached! Evaluating {len(proposals)} bids...")
                
                accepted_one = False
                accept_count = 0
                reject_count = 0
                
                # Simple Logic: Accept the first valid PROPOSE, reject everyone else
                for prop in proposals:
                    if prop.performative == Performative.PROPOSE:
                        if not accepted_one:
                            # We pick this one!
                            accept_msg = prop.CreateReply(Performative.ACCEPT_PROPOSAL)
                            accept_msg.content = "You won the contract! Begin execution."
                            responses.append(accept_msg)
                            accepted_one = True
                            accept_count += 1
                        else:
                            # We already picked a winner, reject this one
                            reject_msg = prop.CreateReply(Performative.REJECT_PROPOSAL)
                            reject_msg.content = "Sorry, another agent had a better bid."
                            responses.append(reject_msg)
                            reject_count += 1
                            
                print(f"\n[{self.agent.agentDescription.agentid.name}] Accepting {accept_count} proposal(s) and rejecting {reject_count} proposal(s).\n")
                return responses
                
            def handleInform(self, msg):
                print(f"[{self.agent.agentDescription.agentid.name}] Success! Winner finished the task: {msg.content}")
                return True
                
            def handleFailure(self, msg):
                print(f"[{self.agent.agentDescription.agentid.name}] The winning agent failed to complete the task!")
                return True

        # 3. Add the behavior to the kernel with a 5-second deadline
        self.AddBehavior(MyContractNetInitiator(self, cfp_msg))

initiator = SimpleClientAgent("Initiator")
initiator.Start()

while True:
    time.sleep(1)