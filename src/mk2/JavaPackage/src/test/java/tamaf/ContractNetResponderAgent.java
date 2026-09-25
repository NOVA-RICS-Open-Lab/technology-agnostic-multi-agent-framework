package tamaf;

import java.util.Collections;

public class ContractNetResponderAgent extends Agent {

    public ContractNetResponderAgent(String name) {
        super(name);
    }

    @Override
    public void setup(EMAInteraction emaInteraction) {
        System.out.println("Contract Net Responder starting...");

        ACLMessageTemplate template = new ACLMessageTemplate();
        template.setPerformatives(Collections.singleton(Performative.CFP));

        addBehavior(new FIPAContractNetResponder(this, template) {
            @Override
            public ACLMessage handleCfp(ACLMessage cfpMessage) {
                System.out.println("[" + getAgent().getAgentDescription().getAgentid().getName() + "] Received CFP: '" + cfpMessage.getContent() + "' from " + cfpMessage.getSender().getName());
            
                System.out.println("[" + getAgent().getAgentDescription().getAgentid().getName() + "] Formulating a bid...");
                
                ACLMessage reply = cfpMessage.createReply(Performative.PROPOSE);
                reply.setContent("I can calculate the data in 5 seconds for $10!");
                return reply;
            }

            @Override
            public Boolean handleRejectProposal(ACLMessage cfpMessage, ACLMessage proposeMessage, ACLMessage rejectMessage) {
                System.out.println("[" + getAgent().getAgentDescription().getAgentid().getName() + "] Darn, I lost the auction. Message: " + rejectMessage.getContent());
                System.out.println("[" + getAgent().getAgentDescription().getAgentid().getName() + "] Freeing up resources and going back to sleep.");
                return true;
            }

            @Override
            public ACLMessage handleAcceptProposal(ACLMessage cfpMessage, ACLMessage proposeMessage, ACLMessage acceptMessage) {
                System.out.println("[" + getAgent().getAgentDescription().getAgentid().getName() + "] WOOHOO! I won the auction! Message: " + acceptMessage.getContent());
                System.out.println("[" + getAgent().getAgentDescription().getAgentid().getName() + "] Allocating resources to begin the task...");
                System.out.println("[" + getAgent().getAgentDescription().getAgentid().getName() + "] Doing the heavy lifting...");
                
                try {
                    Thread.sleep(5000);
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                }

                System.out.println("[" + getAgent().getAgentDescription().getAgentid().getName() + "] Task complete. Sending results back to the initiator.");
                ACLMessage result = cfpMessage.createReply(Performative.INFORM);
                result.setContent("Here is your calculated data: 42");
                return result;
            }
        });
    }

    public static void main(String[] args) {
        ContractNetResponderAgent agent = new ContractNetResponderAgent("Responder");
        agent.start();

        while (true) {
            try { Thread.sleep(1000); } catch (InterruptedException e) { break; }
        }
    }
}
