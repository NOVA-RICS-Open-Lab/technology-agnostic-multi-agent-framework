package tamaf;

import java.util.Collections;
import java.util.Map;

public class InteropResponder {

    private static class MyFIPARequestResponder extends FIPARequestResponder {
        public MyFIPARequestResponder(Agent agent, ACLMessageTemplate template) {
            super(agent, template);
        }

        @Override
        public ACLMessage handleRequest(ACLMessage request) {
            System.out.println("Java: Received REQUEST from " + request.getSender().getFullID());
            return request.createReply(Performative.AGREE);
        }

        @Override
        public ACLMessage prepareResultNotification(ACLMessage request, ACLMessage response) {
            System.out.println("Java: Preparing result for " + request.getSender().getFullID());
            ACLMessage reply = request.createReply(Performative.INFORM);
            reply.setContent(Map.of("status", "success", "platform", "java"));
            return reply;
        }
    }

    public static void main(String[] args) {
        Agent agent = new Agent("JavaResponder") {
            @Override
            public void setup(EMAInteraction ema) {
                System.out.println("Java Responder starting...");
                ACLMessageTemplate template = new ACLMessageTemplate();
                template.addPerformative(Performative.REQUEST);
                addBehavior(new MyFIPARequestResponder(this, template));
            }
        };
        agent.start();
        
        while (true) {
            try { Thread.sleep(1000); } catch (InterruptedException e) {}
        }
    }
}
