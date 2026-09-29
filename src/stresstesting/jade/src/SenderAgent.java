import jade.core.Agent;
import jade.core.AID;
import jade.core.behaviours.SimpleBehaviour;
import jade.lang.acl.ACLMessage;
import jade.lang.acl.MessageTemplate;

public class SenderAgent extends Agent {
    private long triggerTime;
    private String receiverName;
    private boolean sent = false;
    private Long rtt = null;
    private long sendTime;

    protected void setup() {
        Object[] args = getArguments();
        if (args != null && args.length == 2) {
            triggerTime = Long.parseLong((String) args[0]);
            receiverName = (String) args[1];
        }

        addBehaviour(new SimpleBehaviour(this) {
            public void action() {
                if (!sent) {
                    while (System.currentTimeMillis() < triggerTime) {
                        try {
                            Thread.sleep(1);
                        } catch (InterruptedException e) { }
                    }
                    sendTime = System.currentTimeMillis();
                    ACLMessage msg = new ACLMessage(ACLMessage.REQUEST);
                    msg.addReceiver(new AID(receiverName, AID.ISLOCALNAME));
                    msg.setContent(String.valueOf(sendTime));
                    myAgent.send(msg);
                    sent = true;
                }

                MessageTemplate mt = MessageTemplate.MatchPerformative(ACLMessage.INFORM);
                ACLMessage reply = myAgent.receive(mt);
                if (reply != null) {
                    long receiveTime = System.currentTimeMillis();
                    rtt = receiveTime - sendTime;
                } else {
                    block();
                }
            }

            public boolean done() {
                return rtt != null;
            }
            
            public int onEnd() {
                Main.reportRTT(myAgent.getLocalName(), rtt);
                return super.onEnd();
            }
        });
    }
}
