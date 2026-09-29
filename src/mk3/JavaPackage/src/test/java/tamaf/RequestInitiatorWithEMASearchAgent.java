package tamaf;

import java.util.ArrayList;
import java.util.List;

public class RequestInitiatorWithEMASearchAgent extends Agent {

    public RequestInitiatorWithEMASearchAgent(String name) {
        super(name);
    }

    @Override
    public void setup(EMAInteraction emaInteraction) {
        System.out.println("Initiator with EMA Search starting...");

        addBehavior(new OneShotBehavior(this) {
            @Override
            public void action() {
                // Search for an agent named "Responder"
                // AgentID templateId = new AgentID("Responder", new Address("192.168.0.10"));
                AgentID templateId = new AgentID("Responder");
                AgentDescription template = new AgentDescription(templateId);
                
                System.out.println("Searching for Responder in EMA...");
                List<AgentDescription> results = emaInteraction.search(template);
                
                if (results == null || results.isEmpty()) {
                    System.out.println("No Responder found. Will retry in next loop if this was a CyclicBehavior, but this is OneShot.");
                    // In the Python version, it's a SimpleBehavior that sets done=True. 
                    // If we want it to wait, we could use a Ticker or Waker.
                    // For now, let's just try once.
                    return;
                }

                System.out.println("Found " + results.size() + " responders: " + results);

                List<AgentID> receivers = new ArrayList<>();
                for (AgentDescription ad : results) {
                    receivers.add(ad.getAgentid());
                }

                ACLMessage requestMsg = new ACLMessage(Performative.REQUEST);
                requestMsg.setReceiver(receivers);
                requestMsg.setContent("Please calculate the data!");

                getAgent().addBehavior(new FIPARequestInitiator(getAgent(), requestMsg) {
                    @Override
                    public Boolean handleAgree(ACLMessage msg) {
                        System.out.println("[" + getAgent().getAgentDescription().getAgentid().getName() + "] Server agreed! Waiting for results...");
                        return true;
                    }

                    @Override
                    public Boolean handleRefuse(ACLMessage msg) {
                        System.out.println("[" + getAgent().getAgentDescription().getAgentid().getName() + "] Server refused the request.");
                        return true;
                    }

                    @Override
                    public Boolean handleInform(ACLMessage msg) {
                        System.out.println("[" + getAgent().getAgentDescription().getAgentid().getName() + "] Success! Server says: " + msg.getContent());
                        return true;
                    }

                    @Override
                    public Boolean handleFailure(ACLMessage msg) {
                        System.out.println("[" + getAgent().getAgentDescription().getAgentid().getName() + "] Server failed to complete the task.");
                        return true;
                    }
                });
            }
        });
    }

    public static void main(String[] args) {
        RequestInitiatorWithEMASearchAgent initiator = new RequestInitiatorWithEMASearchAgent("Initiator");
        initiator.start();

        while (true) {
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                break;
            }
        }
    }
}
