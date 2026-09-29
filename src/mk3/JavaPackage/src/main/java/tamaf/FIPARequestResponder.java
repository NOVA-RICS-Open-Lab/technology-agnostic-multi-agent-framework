package tamaf;

public class FIPARequestResponder extends FSMBehavior {
    protected ACLMessageTemplate requestTemplate;

    public FIPARequestResponder(Agent agent, ACLMessageTemplate requestTemplate) {
        super(agent);
        this.requestTemplate = requestTemplate;

        addInitialState(new HandleRequestBehavior(this), "handleRequest");
        addState(new PrepareResultBehavior(this), "prepareResult");
        
        addTransition("handleRequest", "prepareResult", 0); // Agreed
        addTransition("handleRequest", "handleRequest", 1);   // Refused
        addTransition("prepareResult", "handleRequest", 0);     // Done
    }

    public ACLMessage handleRequest(ACLMessage request) { return null; }
    public ACLMessage prepareResultNotification(ACLMessage request, ACLMessage response) { return null; }

    private class HandleRequestBehavior extends SimpleBehavior {
        FIPARequestResponder fsm;
        int transitionCode;
        boolean isDone;

        @Override
        public void onStart() {
            super.onStart();
            this.transitionCode = -1;
            this.isDone = false;
        }

        HandleRequestBehavior(FIPARequestResponder fsm) { super(fsm.getAgent()); this.fsm = fsm; }
        @Override
        public void action() {
            ACLMessage requestMsg = receive(fsm.requestTemplate);
            if (requestMsg == null) {
                return;
            }
            ACLMessage reply = fsm.handleRequest(requestMsg);

            if (reply == null) {
                return;
            }

            if (requestMsg.getConversationId() != null) {
                reply.setConversationId(requestMsg.getConversationId());
            }
            fsm.getAgent().send(reply);
            if (reply.getPerformative() == Performative.AGREE) {
                fsm.sharedData.put("request", requestMsg);
                fsm.sharedData.put("agreeMessage", reply);
                transitionCode = 0;
            } else {
                transitionCode = 1;
            }
            isDone = true;
        }

        @Override
        public boolean done() { return this.isDone; }

        @Override
        public int onEnd() { return transitionCode; }
    }

    private class PrepareResultBehavior extends SimpleBehavior {
        FIPARequestResponder fsm;
        boolean isDone;

        PrepareResultBehavior(FIPARequestResponder fsm) {
            super(fsm.getAgent());
            this.fsm = fsm;
        }

        @Override
        public void onStart() {
            super.onStart();
            this.isDone = false;
        }

        @Override
        public void action() {
            ACLMessage requestMsg = (ACLMessage) fsm.sharedData.get("request");
            ACLMessage agreeMsg = (ACLMessage) fsm.sharedData.get("agreeMessage");
            ACLMessage resultMsg = fsm.prepareResultNotification(requestMsg, agreeMsg);

            if (resultMsg == null) {
                return;
            }

            if (requestMsg.getConversationId() != null) {
                resultMsg.setConversationId(requestMsg.getConversationId());
            }
            fsm.getAgent().send(resultMsg);
            this.isDone = true;
        }

        @Override
        public boolean done(){
            return this.isDone;
        }

    }
}
