package tamaf;

public abstract class WakerBehavior extends Behavior {
    private long wakeUpTime;
    private boolean finished = false;

    public WakerBehavior(long timeoutMs) {
        super();
        this.wakeUpTime = System.currentTimeMillis() + timeoutMs;
    }

    public WakerBehavior(Agent agent, long timeoutMs) {
        super(agent);
        this.wakeUpTime = System.currentTimeMillis() + timeoutMs;
    }

    @Override
    public void action() {
        if (System.currentTimeMillis() >= wakeUpTime) {
            onWake();
            finished = true;
        }
    }

    public abstract void onWake();

    @Override
    public boolean done() {
        return finished;
    }
}
