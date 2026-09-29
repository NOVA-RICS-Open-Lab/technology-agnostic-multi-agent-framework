package tamaf;

public class FIPARequestResponderForDispatcher extends FSMBehavior {
    protected ACLMessage requestMessage;

    public static final int TRANSITION_AGREED = 0;
    public static final int TRANSITION_REFUSED = 1;
    public static final int TRANSITION_DONE = 0;

    public FIPARequestResponderForDispatcher(Agent agent, ACLMessage requestMessage) {
        super(agent);
        this.requestMessage = requestMessage;

        addInitialState(new HandleRequestBehavior(this), "handleRequest");
        addState(new PrepareResultBehavior(this), "prepareResult");
        addFinalState(new OneShotBehavior(agent) { @Override public void action() {} }, "end");

        addTransition("handleRequest", "prepareResult", TRANSITION_AGREED);
        addTransition("handleRequest", "end", TRANSITION_REFUSED);
        addTransition("prepareResult", "end", TRANSITION_DONE);
    }

    public ACLMessage handleRequest(ACLMessage request) { return null; }
    public ACLMessage prepareResultNotification(ACLMessage request, ACLMessage response) { return null; }

    private class HandleRequestBehavior extends SimpleBehavior {
        FIPARequestResponderForDispatcher fsm;
        int transitionCode = -1;
        boolean isDone = false;

        HandleRequestBehavior(FIPARequestResponderForDispatcher fsm) { super(fsm.getAgent()); this.fsm = fsm; }
        @Override
        public void action() {
            ACLMessage reply = fsm.handleRequest(fsm.requestMessage);
            if (reply == null) return;

            if (fsm.requestMessage.getConversationId() != null) reply.setConversationId(fsm.requestMessage.getConversationId());
            fsm.sharedData.put("agreeMessage", reply);
            fsm.getAgent().send(reply);
            transitionCode = (reply.getPerformative() == Performative.AGREE) ? TRANSITION_AGREED : TRANSITION_REFUSED;
            isDone = true;
        }
        @Override
        public boolean done() { return isDone; }
        @Override
        public int onEnd() { return transitionCode; }
    }

    private class PrepareResultBehavior extends SimpleBehavior {
        FIPARequestResponderForDispatcher fsm;
        boolean isDone = false;

        PrepareResultBehavior(FIPARequestResponderForDispatcher fsm) { super(fsm.getAgent()); this.fsm = fsm; }
        @Override
        public void action() {
            ACLMessage resultMsg = fsm.prepareResultNotification(fsm.requestMessage, (ACLMessage) fsm.sharedData.get("agreeMessage"));
            if (resultMsg == null) return;

            if (fsm.requestMessage.getConversationId() != null) resultMsg.setConversationId(fsm.requestMessage.getConversationId());
            fsm.getAgent().send(resultMsg);
            isDone = true;
        }
        @Override
        public boolean done() { return isDone; }
    }
}
