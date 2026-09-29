package tamaf;

import java.util.Collections;
import java.util.List;
import java.util.UUID;

public class FIPARequestInitiator extends FSMBehavior {
    protected ACLMessage requestMessage;
    protected ACLMessageTemplate replyTemplate;

    public FIPARequestInitiator(Agent agent, ACLMessage requestMessage) {
        super(agent);
        this.requestMessage = requestMessage;
        if (this.requestMessage.getConversationId() == null) {
            this.requestMessage.setConversationId("req-" + UUID.randomUUID().toString());
        }
        this.replyTemplate = new ACLMessageTemplate();
        this.replyTemplate.setConversationId(this.requestMessage.getConversationId());

        addInitialState(new SendRequestBehavior(this), "sendRequest");
        addState(new WaitFirstResponseBehavior(this), "waitFirstResponse");
        addState(new HandleAgreeBehavior(this), "handleAgree");
        addState(new WaitResultBehavior(this), "waitResult");
        
        addFinalState(new HandleRefuseBehavior(this), "handleRefuse");
        addFinalState(new HandleNotUnderstoodBehavior(this), "handleNotUnderstood");
        addFinalState(new HandleInformBehavior(this), "handleInform");
        addFinalState(new HandleFailureBehavior(this), "handleFailure");

        addTransition("sendRequest", "waitFirstResponse", 0);
        addTransition("waitFirstResponse", "handleAgree", 0);
        addTransition("waitFirstResponse", "handleRefuse", 1);
        addTransition("waitFirstResponse", "handleNotUnderstood", 2);
        addTransition("waitFirstResponse", "handleInform", 3);
        addTransition("waitFirstResponse", "handleFailure", 4);

        addTransition("handleAgree", "waitResult", 0);
        addTransition("waitResult", "handleInform", 0);
        addTransition("waitResult", "handleFailure", 1);
    }

    public Boolean handleAgree(ACLMessage msg) { return true; }
    public Boolean handleRefuse(ACLMessage msg) { return true; }
    public Boolean handleInform(ACLMessage msg) { return true; }
    public Boolean handleFailure(ACLMessage msg) { return true; }
    public Boolean handleNotUnderstood(ACLMessage msg) { return true; }
    public List<ACLMessage> prepareRequests(ACLMessage msg) { return Collections.singletonList(msg); }

    private class SendRequestBehavior extends SimpleBehavior {
        FIPARequestInitiator fsm;
        boolean isDone;

        SendRequestBehavior(FIPARequestInitiator fsm) { super(fsm.getAgent()); this.fsm = fsm; }

        @Override
        public void onStart() {
            super.onStart();
            this.isDone = false;
        }

        @Override
        public void action() {
            List<ACLMessage> requests = fsm.prepareRequests(fsm.requestMessage);
            if (requests == null) return;

            for (ACLMessage req : requests) {
                if (req.getConversationId() == null) req.setConversationId(fsm.requestMessage.getConversationId());
                fsm.getAgent().send(req);
            }
            isDone = true;
        }
        @Override
        public boolean done() { return isDone; }
    }

    private class WaitFirstResponseBehavior extends OneShotBehavior {
        FIPARequestInitiator fsm;
        int transitionCode;
        WaitFirstResponseBehavior(FIPARequestInitiator fsm) { super(fsm.getAgent()); this.fsm = fsm; }
        @Override
        public void onStart() {
            super.onStart();
            transitionCode = -1;
        }
        @Override
        public void action() {
            ACLMessage reply = receive(fsm.replyTemplate);
            if (reply == null) return;
            fsm.sharedData.put("reply", reply);
            Performative perf = reply.getPerformative();
            if (perf == Performative.AGREE) transitionCode = 0;
            else if (perf == Performative.REFUSE) transitionCode = 1;
            else if (perf == Performative.NOT_UNDERSTOOD) transitionCode = 2;
            else if (perf == Performative.INFORM) transitionCode = 3;
            else if (perf == Performative.FAILURE) transitionCode = 4;
            else transitionCode = 2;
        }
        @Override
        public int onEnd() { return transitionCode; }
    }

    private class WaitResultBehavior extends OneShotBehavior {
        FIPARequestInitiator fsm;
        int transitionCode;
        WaitResultBehavior(FIPARequestInitiator fsm) { super(fsm.getAgent()); this.fsm = fsm; }
        @Override
        public void onStart() {
            super.onStart();
            transitionCode = -1;
        }
        @Override
        public void action() {
            ACLMessage reply = receive(fsm.replyTemplate);
            if (reply == null) return;
            fsm.sharedData.put("reply", reply);
            if (reply.getPerformative() == Performative.INFORM) transitionCode = 0;
            else transitionCode = 1;
        }
        @Override
        public int onEnd() { return transitionCode; }
    }

    private class HandleAgreeBehavior extends SimpleBehavior {
        FIPARequestInitiator fsm;
        boolean isDone;

        HandleAgreeBehavior(FIPARequestInitiator fsm) { super(fsm.getAgent()); this.fsm = fsm; }

        @Override
        public void onStart() {
            super.onStart();
            this.isDone = false;
        }

        @Override
        public void action() {
            Boolean res = fsm.handleAgree((ACLMessage) fsm.sharedData.get("reply"));
            if (res == null || !res) return;
            isDone = true;
        }
        @Override
        public boolean done() { return isDone; }
    }

    private class HandleRefuseBehavior extends SimpleBehavior {
        FIPARequestInitiator fsm;
        boolean isDone;

        HandleRefuseBehavior(FIPARequestInitiator fsm) { super(fsm.getAgent()); this.fsm = fsm; }

        @Override
        public void onStart() {
            super.onStart();
            this.isDone = false;
        }

        @Override
        public void action() {
            Boolean res = fsm.handleRefuse((ACLMessage) fsm.sharedData.get("reply"));
            if (res == null || !res) return;
            isDone = true;
        }
        @Override
        public boolean done() { return isDone; }
    }

    private class HandleNotUnderstoodBehavior extends SimpleBehavior {
        FIPARequestInitiator fsm;
        boolean isDone;

        HandleNotUnderstoodBehavior(FIPARequestInitiator fsm) { super(fsm.getAgent()); this.fsm = fsm; }

        @Override
        public void onStart() {
            super.onStart();
            this.isDone = false;
        }

        @Override
        public void action() {
            Boolean res = fsm.handleNotUnderstood((ACLMessage) fsm.sharedData.get("reply"));
            if (res == null || !res) return;
            isDone = true;
        }
        @Override
        public boolean done() { return isDone; }
    }

    private class HandleInformBehavior extends SimpleBehavior {
        FIPARequestInitiator fsm;
        boolean isDone;

        HandleInformBehavior(FIPARequestInitiator fsm) { super(fsm.getAgent()); this.fsm = fsm; }

        @Override
        public void onStart() {
            super.onStart();
            this.isDone = false;
        }

        @Override
        public void action() {
            Boolean res = fsm.handleInform((ACLMessage) fsm.sharedData.get("reply"));
            if (res == null || !res) return;
            isDone = true;
        }
        @Override
        public boolean done() { return isDone; }
    }

    private class HandleFailureBehavior extends SimpleBehavior {
        FIPARequestInitiator fsm;
        boolean isDone;

        HandleFailureBehavior(FIPARequestInitiator fsm) { super(fsm.getAgent()); this.fsm = fsm; }

        @Override
        public void onStart() {
            super.onStart();
            this.isDone = false;
        }

        @Override
        public void action() {
            Boolean res = fsm.handleFailure((ACLMessage) fsm.sharedData.get("reply"));
            if (res == null || !res) return;
            isDone = true;
        }
        @Override
        public boolean done() { return isDone; }
    }
}
