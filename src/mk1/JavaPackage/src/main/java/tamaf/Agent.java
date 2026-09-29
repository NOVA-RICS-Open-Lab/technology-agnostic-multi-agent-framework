package tamaf;

import tamaf.defines.Defines;

public class Agent {
    protected AgentDescription agentDescription;
    protected AMS ams;
    protected MTS mts;

    public Agent(String name) {
        this(name, null, null);
    }

    public Agent(String name, Address specificRegisterAddress, Address specificEMAAddress) {
        this.agentDescription = new AgentDescription(new AgentID(name, new Address(Utils.getLocalIP(), null)));
        this.ams = new AMS(this);
        this.mts = new MTS(this, specificRegisterAddress, specificEMAAddress);
        System.out.println("New Agent was born: " + name + "@" + this.agentDescription.getAgentid().getAddress().getIp());
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
        if (msg.getSender() == null) msg.setSender(agentDescription.getAgentid());
        return ams.send(msg);
    }

    public ACLMessage receive(ACLMessageTemplate template) {
        return ams.receive(template);
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
        ams.putBack(msg);
    }

    public void updateAgentDescription(AgentDescription agentDescription) {
        this.agentDescription.getServices().clear();
        for (ServiceDescription service : agentDescription.getServices()) {
            this.agentDescription.addService(service);
        }
        this.addBehavior(new ModifyAgentEMABehavior(this));
    }

    public AgentDescription getAgentDescription() { return agentDescription; }

    public AgentDescription getaAgentDescriptionTemplate() { return new AgentDescription(this.agentDescription.getAgentid()); }
    
    public boolean isAlive() {
        return ams.isAlive();
    }
}
