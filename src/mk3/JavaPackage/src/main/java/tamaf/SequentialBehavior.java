package tamaf;

import java.util.ArrayList;
import java.util.List;

public class SequentialBehavior extends Behavior {
    private List<Behavior> subBehaviors = new ArrayList<>();
    private int currentIndex = 0;
    protected java.util.Map<String, Object> sharedData = new java.util.HashMap<>();

    public SequentialBehavior() {
        super();
    }
    
    public SequentialBehavior(Agent agent) {
        super(agent);
    }

    public void addSubBehavior(Behavior b) {
        b.setAgent(this.agent);
        subBehaviors.add(b);
    }

    @Override
    public void onStart() {
        if (!subBehaviors.isEmpty()) {
            subBehaviors.get(currentIndex).onStart();
        }
    }

    @Override
    public void action() {
        if (currentIndex < subBehaviors.size()) {
            Behavior current = subBehaviors.get(currentIndex);
            current.execute();
            if (current.isBlocked()) {
                this.block();
            }
        }
    }

    @Override
    public boolean done() {
        if (subBehaviors.isEmpty()) return true;
        if (currentIndex >= subBehaviors.size()) return true;

        Behavior current = subBehaviors.get(currentIndex);
        if (current.done()) {
            current.onEnd();
            currentIndex++;
            if (currentIndex >= subBehaviors.size()) {
                return true;
            } else {
                setStarted(false); // Reset to trigger onStart of next
                return false;
            }
        }
        return false;
    }

    @Override
    public int onEnd() {
        this.sharedData.clear();
        this.currentIndex = 0;
        this.setStarted(false);
        return 0;
    }
}
