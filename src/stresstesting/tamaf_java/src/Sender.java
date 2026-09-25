package tamaf.stresstest;
import tamaf.*;
import java.util.Collections;

public class Sender extends Agent {
    public long triggerTime;
    public AgentID receiverAgentId;
    public Long rtt = null;
    public volatile boolean isRegistered = false;
    
    public Sender(String name, long triggerTime, String receiverIp, int receiverPort) {
        super(name);
        this.triggerTime = triggerTime;
        this.receiverAgentId = new AgentID("receiver", new Address(receiverIp, receiverPort));
    }
    
    private class SendBehavior extends SimpleBehavior {
        private boolean sent = false;
        private long sendTime = 0;
        
        public SendBehavior(Agent agent) {
            super(agent);
        }
        
        @Override
        public void action() {
            if (!sent) {
                long waitTime = ((Sender)agent).triggerTime - System.currentTimeMillis();
                if (waitTime > 0) {
                    ((Sender)agent).ams.scheduleWakeUp(waitTime);
                    return;
                }
                
                sendTime = System.currentTimeMillis();
                ACLMessage msg = new ACLMessage();
                msg.setSender(agent.getAgentDescription().getAgentid());
                msg.setPerformative(Performative.REQUEST);
                msg.addReceiver(((Sender)agent).receiverAgentId);
                msg.setContent(String.valueOf(sendTime));
                
                agent.send(msg);
                sent = true;
            }
            
            ACLMessageTemplate template = new ACLMessageTemplate();
            template.setPerformatives(Collections.singleton(Performative.INFORM));
            
            ACLMessage reply = receive(template); // throws EmptyReceiveException cooperatively
            
            long receiveTime = System.currentTimeMillis();
            ((Sender)agent).rtt = receiveTime - sendTime;
        }
        
        @Override
        public boolean done() {
            return ((Sender)agent).rtt != null;
        }
    }
    
    @Override
    public void setup(EMAInteraction emaInteraction) {
        this.isRegistered = true;
        addBehavior(new SendBehavior(this));
    }

    @Override
    public void stop() {
        super.stop();
        if (mts != null) {
            mts.stopMessageServer();
        }
    }
}
