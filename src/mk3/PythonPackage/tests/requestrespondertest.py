from tamaf import Agent, ACLMessageTemplate, Performative, FIPARequestResponder
import time

class SimpleServerAgent(Agent):
    def Setup(self, emainteraction):
        
        # 1. Listen for any REQUEST message
        template = ACLMessageTemplate(performatives={Performative.REQUEST})
        
        # 2. Instantiate and customize the responder
        class MyResponder(FIPARequestResponder):
            def handleRequest(self, requestMsg):
                print(f"[{self.agent.agentDescription.agentid.name}] Received Request: '{requestMsg.content}' from {requestMsg.sender.name}")
                
                # Agree to the request!
                # reply = requestMsg.CreateReply(Performative.REFUSE)
                reply = requestMsg.CreateReply(Performative.AGREE)
                return reply
                
            def prepareResultNotification(self, requestMsg, responseMsg):
                print(f"[{self.agent.agentDescription.agentid.name}] Doing the work...")
                
                time.sleep(5)

                # Send the final Inform
                result = requestMsg.CreateReply(Performative.INFORM)
                result.content = "Task completed successfully!"

                return result

        # 3. Add it to the kernel
        self.AddBehavior(MyResponder(self, template))

responder = SimpleServerAgent("Responder")
responder.Start()

while True:
    time.sleep(1)