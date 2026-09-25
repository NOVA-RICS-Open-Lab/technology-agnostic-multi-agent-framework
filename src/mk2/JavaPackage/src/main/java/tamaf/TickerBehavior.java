package tamaf;

public abstract class TickerBehavior extends Behavior {
    private long period;
    private long lastTick;

    public TickerBehavior(long tickPeriod) {
        super();
        this.period = tickPeriod;
        this.lastTick = System.currentTimeMillis();
    }

    public TickerBehavior(Agent agent, long tickPeriod) {
        super(agent);
        this.period = tickPeriod;
        this.lastTick = System.currentTimeMillis();
    }

    @Override
    public void action() {
        long now = System.currentTimeMillis();
        if (now >= lastTick + period) {
            onTick();
            lastTick = now;
        }
    }

    public abstract void onTick();

    @Override
    public boolean done() {
        return false;
    }
}
