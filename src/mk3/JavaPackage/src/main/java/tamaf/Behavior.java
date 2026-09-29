package tamaf;

import tamaf.defines.EmptyReceiveException;
import java.util.ArrayList;
import java.util.List;

public abstract class Behavior {
    protected Agent agent;
    private boolean isBlocked = false;
    private boolean isStarted = false;
    protected List<ACLMessage> messagesToPutBack = new ArrayList<>();
    protected EMAInteraction emaInteraction;

    public Behavior() {
        this.emaInteraction = new EMAInteraction(this);
    }
    public Behavior(Agent agent) {
        this.agent = agent;
        this.emaInteraction = new EMAInteraction(this);
    }

    public void setAgent(Agent agent) { this.agent = agent; }
    public Agent getAgent() { return agent; }

    public void onStart() {}
    public abstract void action();
    public abstract boolean done();
    public int onEnd() { return 0; }

    public void block() { this.isBlocked = true; }
    public void unblock() { this.isBlocked = false; }
    public boolean isBlocked() { return isBlocked; }

    public void setStarted(boolean started) { this.isStarted = started; }
    public boolean isStarted() { return isStarted; }

    public ACLMessage receive(ACLMessageTemplate template) {
        ACLMessage msg = this.agent.receive(template);
        if (msg == null) {
            throw new EmptyReceiveException();
        } else {
            messagesToPutBack.add(0, msg);
        }
        return msg;
    }

    public void execute() {
        messagesToPutBack.clear();
        try {
            this.unblock();
            this.action();
        } catch (EmptyReceiveException e) {
            this.block();
            for (ACLMessage msg : messagesToPutBack) {
                this.putBack(msg);
            }
        }
    }

    public boolean send(ACLMessage msg) {
        return this.agent.send(msg);
    }

    public void putBack(ACLMessage msg) {
        this.agent.putBack(msg);
    }
}
