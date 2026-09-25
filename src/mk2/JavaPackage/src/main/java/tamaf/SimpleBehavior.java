package tamaf;

public abstract class SimpleBehavior extends Behavior {

    public SimpleBehavior(Agent agent) {
        super(agent);
    }

    @Override
    public abstract void action();

    @Override
    public abstract boolean done();
}
