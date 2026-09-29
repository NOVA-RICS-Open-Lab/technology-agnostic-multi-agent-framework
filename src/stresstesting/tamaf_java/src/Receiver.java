package tamaf.stresstest;
import tamaf.*;
import java.util.Collections;

public class Receiver extends Agent {
    public volatile boolean isRegistered = false;

    public Receiver(String name) {
        super(name);
    }

    private class ReceiveBehavior extends CyclicBehavior {
        public ReceiveBehavior(Agent agent) {
            super(agent);
        }

        @Override
        public void action() {
            ACLMessageTemplate template = new ACLMessageTemplate();
            template.setPerformatives(Collections.singleton(Performative.REQUEST));
            
            while (true) {
                ACLMessage msg = receive(template); // throws EmptyReceiveException cooperatively
                
                long receiveTime = System.currentTimeMillis();
                String senderName = msg.getSender() != null ? msg.getSender().getName() : "Unknown";
                Utils.logAsync("[RECEIVER] Received message from " + senderName + " at " + receiveTime);
                
                ACLMessage reply = msg.createReply();
                reply.setPerformative(Performative.INFORM);
                reply.setContent("ACK");
                agent.send(reply);
            }
        }
    }

    @Override
    public void setup(EMAInteraction emaInteraction) {
        this.isRegistered = true;
        addBehavior(new ReceiveBehavior(this));
    }

    @Override
    public void stop() {
        super.stop();
        if (mts != null) {
            mts.stopMessageServer();
        }
    }
}
