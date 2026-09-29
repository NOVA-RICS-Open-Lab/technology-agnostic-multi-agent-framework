package tamaf;

public abstract class OneShotBehavior extends Behavior {
    
    public OneShotBehavior(Agent agent) {
        super(agent);
    }

    public OneShotBehavior() {
        super();
    }
    
    @Override
    public boolean done() { return true; }
}
