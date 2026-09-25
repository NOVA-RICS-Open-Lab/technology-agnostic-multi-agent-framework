package tamaf;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

public abstract class FSMBehavior extends Behavior {
    protected Map<String, Behavior> states = new HashMap<>();
    protected String initialState;
    protected String currentState;
    protected Set<String> finalStates = new HashSet<>();
    protected Map<TransitionKey, String> transitions = new HashMap<>();
    protected Map<String, Object> sharedData = new HashMap<>();

    public static final int DEFAULT = -1;

    public FSMBehavior() {
        super();
    }

    public FSMBehavior(Agent agent) {
        super(agent);
    }

    public void addState(Behavior behavior, String stateName) {
        behavior.setAgent(this.agent);
        states.put(stateName, behavior);
    }

    public void addInitialState(Behavior behavior, String stateName) {
        behavior.setAgent(this.agent);
        this.initialState = stateName;
        this.currentState = stateName;
        states.put(stateName, behavior);
    }

    public void addFinalState(Behavior behavior, String stateName) {
        behavior.setAgent(this.agent);
        this.finalStates.add(stateName);
        states.put(stateName, behavior);
    }

    public void addTransition(String sourceStateName, String destinationStateName, int transitionCode) {
        transitions.put(new TransitionKey(sourceStateName, transitionCode), destinationStateName);
    }

    public void addDefaultTransition(String sourceStateName, String destinationStateName) {
        transitions.put(new TransitionKey(sourceStateName, DEFAULT), destinationStateName);
    }

    @Override
    public void onStart() {
        if (currentState != null && states.containsKey(currentState)) {
            states.get(currentState).onStart();
        }
    }

    @Override
    public void action() {
        Behavior b = states.get(currentState);
        b.execute();
        if (b.isBlocked()) {
            this.block();
        }
    }

    @Override
    public boolean done() {
        Behavior current = states.get(currentState);

        if (!current.done()) {
            return false;
        }

        if (finalStates.contains(currentState)) {
            current.onEnd();
            return true;
        }

        int transitionCode = current.onEnd();
        String nextState = transitions.get(new TransitionKey(currentState, transitionCode));

        if (nextState == null) {
            nextState = transitions.get(new TransitionKey(currentState, DEFAULT));
        }

        if (nextState != null) {
            currentState = nextState;
            setStarted(false);
            return false;
        }

        return true;
    }

    @Override
    public int onEnd() {
        this.sharedData.clear();
        this.currentState = this.initialState;
        this.setStarted(false);
        return 0;
    }

    protected static class TransitionKey {
        String state;
        int code;

        TransitionKey(String s, int c) {
            state = s;
            code = c;
        }

        @Override
        public boolean equals(Object o) {
            if (this == o) return true;
            if (o == null || getClass() != o.getClass()) return false;
            TransitionKey that = (TransitionKey) o;
            return code == that.code && state.equals(that.state);
        }

        @Override
        public int hashCode() {
            int result = state.hashCode();
            result = 31 * result + code;
            return result;
        }
    }
}
