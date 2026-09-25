package tamaf;

import org.junit.jupiter.api.Test;
import static org.junit.jupiter.api.Assertions.*;

import java.util.Map;

public class FSMBehaviorTest {

    private static class TestFSMBehavior extends FSMBehavior {
        public TestFSMBehavior() {
            super();
        }

        @Override
        public void action() {}

        @Override
        public boolean done() { return true; }

        // Helper to expose protected fields for testing
        public Map<String, Object> getSharedData() { return sharedData; }
        public String getCurrentState() { return currentState; }
        public String getInitialState() { return initialState; }
        
        public void setCurrentState(String state) { this.currentState = state; }
        public void setInitialState(String state) { this.initialState = state; }
    }

    @Test
    public void testOnEndResetsState() {
        TestFSMBehavior fsm = new TestFSMBehavior();
        fsm.setInitialState("START");
        fsm.setCurrentState("END");
        fsm.getSharedData().put("key", "value");
        fsm.setStarted(true);

        assertEquals("END", fsm.getCurrentState());
        assertFalse(fsm.getSharedData().isEmpty());
        assertTrue(fsm.isStarted());

        fsm.onEnd();

        // These should fail before implementation
        assertTrue(fsm.getSharedData().isEmpty(), "sharedData should be empty after onEnd");
        assertEquals("START", fsm.getCurrentState(), "currentState should be reset to initialState after onEnd");
        assertFalse(fsm.isStarted(), "isStarted should be false after onEnd");
    }
}
