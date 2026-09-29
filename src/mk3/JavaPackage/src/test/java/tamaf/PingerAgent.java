package tamaf;

public class PingerAgent extends Agent {

    public PingerAgent(String name) {
        super(name);
    }

    private static class ThreePingsBehavior extends Behavior {
        private int pingCount = 0;

        public ThreePingsBehavior(Agent agent) {
            super(agent);
        }

        @Override
        public void action() {
            System.out.println("Ping: " + pingCount);
            pingCount++;
        }

        @Override
        public boolean done() {
            return pingCount >= 3;
        }
    }

    private static class PimbaBehavior extends OneShotBehavior {
        public PimbaBehavior(Agent agent) {
            super(agent);
        }

        @Override
        public void action() {
            System.out.println("PIMBA");
        }
    }

    @Override
    public void setup(EMAInteraction emaInteraction) {
        SequentialBehavior sequentialBehavior = new SequentialBehavior(this);
        sequentialBehavior.addSubBehavior(new ThreePingsBehavior(this));
        sequentialBehavior.addSubBehavior(new PimbaBehavior(this));
        sequentialBehavior.addSubBehavior(new ThreePingsBehavior(this));
        addBehavior(sequentialBehavior);
    }

    public static void main(String[] args) {
        PingerAgent agent = new PingerAgent("Test2Agent");
        agent.start();

        while (true) {
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                break;
            }
        }
    }
}
