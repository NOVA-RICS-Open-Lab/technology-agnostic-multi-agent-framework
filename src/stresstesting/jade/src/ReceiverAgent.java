import jade.core.Agent;
import jade.core.behaviours.CyclicBehaviour;
import jade.lang.acl.ACLMessage;
import jade.lang.acl.MessageTemplate;

public class ReceiverAgent extends Agent {
    protected void setup() {
        addBehaviour(new CyclicBehaviour(this) {
            public void action() {
                MessageTemplate mt = MessageTemplate.MatchPerformative(ACLMessage.REQUEST);
                ACLMessage msg = myAgent.receive(mt);
                if (msg != null) {
                    long receiveTime = System.currentTimeMillis();
                    String senderName = msg.getSender().getLocalName();
                    System.out.println("[RECEIVER] Received message from " + senderName + " at " + receiveTime);
                    
                    ACLMessage reply = msg.createReply();
                    reply.setPerformative(ACLMessage.INFORM);
                    reply.setContent("ACK");
                    myAgent.send(reply);
                } else {
                    block();
                }
            }
        });
    }
}
