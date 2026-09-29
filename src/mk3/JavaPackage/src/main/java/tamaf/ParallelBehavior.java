package tamaf;

import java.util.ArrayList;
import java.util.List;

public class ParallelBehavior extends Behavior {
    private final List<Behavior> subBehaviors = new ArrayList<>();
    private boolean finishWhenAnyDone;
    protected java.util.Map<String, Object> sharedData = new java.util.HashMap<>();

    public ParallelBehavior(Agent agent, boolean finishWhenAnyDone) {
        super(agent);
        this.finishWhenAnyDone = finishWhenAnyDone;
    }
    
    public ParallelBehavior(boolean finishWhenAnyDone) {
        super();
        this.finishWhenAnyDone = finishWhenAnyDone;
    }

    public void addSubBehavior(Behavior b) {
        b.setAgent(this.agent);
        subBehaviors.add(b);
    }

    @Override
    public void onStart() {
        for (Behavior b : subBehaviors) {
            b.onStart();
        }
    }

    @Override
    public void action() {
        boolean allChildrenBlocked = true;
        for (Behavior b : subBehaviors) {
            if (b.done()) continue;

            if (!b.isBlocked()) {
                b.execute();
                allChildrenBlocked = false;
            }
        }

        if (allChildrenBlocked) {
            this.block();
        }
    }

    @Override
    public boolean done() {
        if (finishWhenAnyDone) {
            for (Behavior b : subBehaviors) {
                if (b.done()) {
                    for (Behavior sub : subBehaviors) sub.onEnd();
                    return true;
                }
            }
            return false;
        } else {
            for (Behavior b : subBehaviors) {
                if (!b.done()) return false;
            }
            for (Behavior b : subBehaviors) b.onEnd();
            return true;
        }
    }

    @Override
    public int onEnd() {
        this.sharedData.clear();
        this.setStarted(false);
        return 0;
    }
}
