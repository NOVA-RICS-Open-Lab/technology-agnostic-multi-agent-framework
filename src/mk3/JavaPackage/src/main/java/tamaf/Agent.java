package tamaf;

import tamaf.defines.Defines;

public class Agent {
    protected AgentDescription agentDescription;
    protected AMS ams;
    protected MTS mts;
    protected final int debugLevel;

    public Agent(String name) {
        this(name, null, null, 1);
    }

    public Agent(String name, int debugLevel) {
        this(name, null, null, debugLevel);
    }

    public Agent(String name, boolean debug) {
        this(name, null, null, debug ? 2 : 0);
    }

    public Agent(String name, Address specificRegisterAddress, Address specificEMAAddress) {
        this(name, specificRegisterAddress, specificEMAAddress, 1);
    }

    public Agent(String name, Address specificRegisterAddress, Address specificEMAAddress, boolean debug) {
        this(name, specificRegisterAddress, specificEMAAddress, debug ? 2 : 0);
    }

    public Agent(String name, Address specificRegisterAddress, Address specificEMAAddress, int debugLevel) {
        this.debugLevel = debugLevel;
        this.agentDescription = new AgentDescription(new AgentID(name, new Address(Utils.getLocalIP(), null)));
        this.ams = new AMS(this);
        this.mts = new MTS(this, specificRegisterAddress, specificEMAAddress);
        logDebug(1, "Agent Born: " + name + "@" + this.agentDescription.getAgentid().getAddress().getIp());
        
        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            try {
                if (this.ams != null && this.ams.isAlive()) {
                    this.stop();
                }
            } catch (Exception ignored) {}
        }));
    }

    public int getDebugLevel() {
        return this.debugLevel;
    }

    public boolean isDebug() {
        return this.debugLevel > 0;
    }

    public void logDebug(String message) {
        logDebug(2, message);
    }

    public void logDebug(int level, String message) {
        if (this.debugLevel >= level) {
            String id = (this.agentDescription != null && this.agentDescription.getAgentid() != null)
                    ? this.agentDescription.getAgentid().getName()
                    : "Agent";
            String prefix = (level == 1) ? "[DEBUG-L1]" : "[DEBUG-L2]";
            Utils.logAsync(prefix + "[" + id + "] " + message);
        }
    }

    public String getName() {
        return this.agentDescription.getAgentid().getFullID();
    }

    public void setup(EMAInteraction emaInteraction) {}
    public void takeDown(EMAInteraction emaInteraction) {}

    public void start() {
        ams.start();
    }

    public void suspend() {
        ams.suspend();
    }

    public void resume() {
        ams.resume();
    }

    public void stop() {
        ams.shutdown();
    }

    public boolean send(ACLMessage msg) {
        return mts.send(msg);
    }

    public ACLMessage receive(ACLMessageTemplate template) {
        return mts.receive(template);
    }

    public void addBehavior(Behavior b) {
        b.setAgent(this);
        ams.addBehavior(b);
    }
    
    public void removeBehavior(Behavior b) {
        b.setAgent(this);
        ams.removeBehavior(b);
    }

    public void putBack(ACLMessage msg) {
        mts.putBack(msg);
    }

    public void updateAgentDescription(AgentDescription agentDescription) {
        this.agentDescription.getServices().clear();
        for (ServiceDescription service : agentDescription.getServices()) {
            this.agentDescription.addService(service);
        }
        this.addBehavior(new ModifyAgentEMABehavior(this));
    }

    public AgentDescription getAgentDescription() { return agentDescription; }

    public AgentDescription getAgentDescriptionTemplate() { return new AgentDescription(this.agentDescription.getAgentid()); }
    
    public boolean isAlive() {
        return ams.isAlive();
    }
}
