package tamaf;

import java.util.Collections;

public class RequestResponderAgent extends Agent {

    public RequestResponderAgent(String name) {
        super(name);
    }

    @Override
    public void setup(EMAInteraction emaInteraction) {
        ACLMessageTemplate template = new ACLMessageTemplate();
        template.setPerformatives(Collections.singleton(Performative.REQUEST));

        addBehavior(new FIPARequestResponder(this, template) {
            @Override
            public ACLMessage handleRequest(ACLMessage request) {
                System.out.println("Responder: Received request: " + request.getContent());
                
                ACLMessage agree = request.createReply();
                agree.setPerformative(Performative.AGREE);
                agree.setContent("I will do it!");
                return agree;
            }

            @Override
            public ACLMessage prepareResultNotification(ACLMessage request, ACLMessage response) {
                System.out.println("Responder: Preparing result for: " + request.getContent());
                
                ACLMessage inform = request.createReply();
                inform.setPerformative(Performative.INFORM);
                inform.setContent("Task completed successfully!");
                return inform;
            }
        });
    }

    public static void main(String[] args) {
        RequestResponderAgent responder = new RequestResponderAgent("Responder");
        responder.start();
        
        // Prevent main from exiting immediately
        while (true) {
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                break;
            }
        }
    }
}
