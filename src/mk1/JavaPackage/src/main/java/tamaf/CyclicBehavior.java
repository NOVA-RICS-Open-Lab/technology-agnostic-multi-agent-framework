package tamaf;

public abstract class CyclicBehavior extends Behavior {
    
    public CyclicBehavior() {
        super();
    }

    public CyclicBehavior(Agent agent) {
        super(agent);
    }
    
    @Override
    public boolean done() { return false; }
}
