package tamaf;

import java.util.Scanner;

public class ContractNetInitiatorAgent extends Agent {

    public ContractNetInitiatorAgent(String name) {
        super(name);
    }

    @Override
    public void setup(EMAInteraction emaInteraction) {
        System.out.println("Contract Net Initiator starting...");
        
        // Use Scanner to get the port from console like the Python version
        Scanner scanner = new Scanner(System.in);
        System.out.print("PORTO DO RECETOR: ");
        int port = scanner.nextInt();

        ACLMessage cfp = new ACLMessage(Performative.CFP);
        cfp.addReceiver(new AgentID("Responder", new Address(tamaf.defines.Defines.getLocalIP(), port)));
        cfp.setContent("Auction: Who can calculate this data the fastest?");

        addBehavior(new FIPAContractNetInitiator(this, cfp) {
            @Override
            public Boolean handlePropose(ACLMessage msg) {
                System.out.println("[" + getAgent().getAgentDescription().getAgentid().getName() + "] Received a PROPOSE from " + msg.getSender().getName() + ": " + msg.getContent());
                return true;
            }

            @Override
            public Boolean handleRefuse(ACLMessage msg) {
                System.out.println("[" + getAgent().getAgentDescription().getAgentid().getName() + "] " + msg.getSender().getName() + " refused the CFP.");
                return true;
            }

            @Override
            public boolean handleProposals(java.util.List<ACLMessage> proposals, java.util.List<ACLMessage> responses) {
                System.out.println("[" + getAgent().getAgentDescription().getAgentid().getName() + "] Deadline reached! Evaluating " + proposals.size() + " bids...");
                
                boolean acceptedOne = false;
                int acceptCount = 0;
                int rejectCount = 0;

                for (ACLMessage prop : proposals) {
                    if (prop.getPerformative() == Performative.PROPOSE) {
                        if (!acceptedOne) {
                            ACLMessage accept = prop.createReply(Performative.ACCEPT_PROPOSAL);
                            accept.setContent("You won the contract! Begin execution.");
                            responses.add(accept);
                            acceptedOne = true;
                            acceptCount++;
                        } else {
                            ACLMessage reject = prop.createReply(Performative.REJECT_PROPOSAL);
                            reject.setContent("Sorry, another agent had a better bid.");
                            responses.add(reject);
                            rejectCount++;
                        }
                    }
                }
                
                System.out.println("\n[" + getAgent().getAgentDescription().getAgentid().getName() + "] Accepting " + acceptCount + " proposal(s) and rejecting " + rejectCount + " proposal(s).\n");
                return true;
            }

            @Override
            public Boolean handleInform(ACLMessage msg) {
                System.out.println("[" + getAgent().getAgentDescription().getAgentid().getName() + "] Success! Winner finished the task: " + msg.getContent());
                return true;
            }

            @Override
            public Boolean handleFailure(ACLMessage msg) {
                System.out.println("[" + getAgent().getAgentDescription().getAgentid().getName() + "] The winning agent failed to complete the task!");
                return true;
            }
        });
    }

    public static void main(String[] args) {
        ContractNetInitiatorAgent agent = new ContractNetInitiatorAgent("Initiator");
        agent.start();
        
        while (true) {
            try { Thread.sleep(1000); } catch (InterruptedException e) { break; }
        }
    }
}
