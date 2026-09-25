package tamaf;

public class FIPAContractNetResponder extends FSMBehavior {
    protected ACLMessageTemplate cfpTemplate;

    public FIPAContractNetResponder(Agent agent, ACLMessageTemplate cfpTemplate) {
        super(agent);
        this.cfpTemplate = cfpTemplate;

        addInitialState(new WaitCfpBehavior(this), "waitCfp");
        addState(new PrepareResponseBehavior(this), "prepareResponse");
        addState(new WaitAcceptRejectBehavior(this), "waitAcceptReject");
        addState(new ExecuteTaskBehavior(this), "executeTask");

        addTransition("waitCfp", "prepareResponse", 0);
        addTransition("prepareResponse", "waitAcceptReject", 0); // Bid sent
        addTransition("prepareResponse", "waitCfp", 1);           // Passed
        addTransition("waitAcceptReject", "executeTask", 0);     // Won auction
        addTransition("waitAcceptReject", "waitCfp", 1);         // Lost auction
        addTransition("executeTask", "waitCfp", 0);               // Task done
    }

    public ACLMessage handleCfp(ACLMessage cfp) { return null; }
    public ACLMessage handleAcceptProposal(ACLMessage cfp, ACLMessage propose, ACLMessage accept) { return null; }
    public Boolean handleRejectProposal(ACLMessage cfp, ACLMessage propose, ACLMessage reject) { return true; }

    private class WaitCfpBehavior extends OneShotBehavior {
        FIPAContractNetResponder fsm;
        WaitCfpBehavior(FIPAContractNetResponder fsm) { super(fsm.getAgent()); this.fsm = fsm; }
        @Override
        public void action() {
            ACLMessage cfpMsg = receive(fsm.cfpTemplate);
            if (cfpMsg == null) return;

            fsm.sharedData.put("cfp", cfpMsg);
            ACLMessageTemplate replyTemplate = new ACLMessageTemplate();
            replyTemplate.setConversationId(cfpMsg.getConversationId());
            fsm.sharedData.put("reply_template", replyTemplate);
        }
    }

    private class PrepareResponseBehavior extends SimpleBehavior {
        FIPAContractNetResponder fsm;
        int transitionCode;
        boolean isDone;

        @Override
        public void onStart() {
            super.onStart();
            this.transitionCode = -1;
            this.isDone = false;
        }

        PrepareResponseBehavior(FIPAContractNetResponder fsm) { super(fsm.getAgent()); this.fsm = fsm; }
        @Override
        public void action() {
            ACLMessage cfpMsg = (ACLMessage) fsm.sharedData.get("cfp");
            ACLMessage reply = fsm.handleCfp(cfpMsg);
            if (reply == null) return;

            if (cfpMsg.getConversationId() != null) reply.setConversationId(cfpMsg.getConversationId());
            fsm.sharedData.put("propose_msg", reply);
            fsm.getAgent().send(reply);
            transitionCode = (reply.getPerformative() == Performative.PROPOSE) ? 0 : 1;
            isDone = true;
        }
        @Override
        public boolean done() { return isDone; }
        @Override
        public int onEnd() { return transitionCode; }
    }

    private class WaitAcceptRejectBehavior extends SimpleBehavior {
        FIPAContractNetResponder fsm;
        int transitionCode;
        boolean isDone;

        @Override
        public void onStart() {
            super.onStart();
            this.transitionCode = -1;
            this.isDone = false;
        }

        WaitAcceptRejectBehavior(FIPAContractNetResponder fsm) { super(fsm.getAgent()); this.fsm = fsm; }
        @Override
        public void action() {
            ACLMessageTemplate template = (ACLMessageTemplate) fsm.sharedData.get("reply_template");
            ACLMessage reply = receive(template);
            if (reply == null) return;
            
            ACLMessage cfpMsg = (ACLMessage) fsm.sharedData.get("cfp");
            ACLMessage proposeMsg = (ACLMessage) fsm.sharedData.get("propose_msg");
            if (reply.getPerformative() == Performative.ACCEPT_PROPOSAL) {
                fsm.sharedData.put("accept_msg", reply);
                transitionCode = 0;
            } else {
                Boolean res = fsm.handleRejectProposal(cfpMsg, proposeMsg, reply);
                if (res == null || !res) return;
                transitionCode = 1;
            }
            isDone = true;
        }
        @Override
        public boolean done() { return isDone; }
        @Override
        public int onEnd() { return transitionCode; }
    }

    private class ExecuteTaskBehavior extends SimpleBehavior {
        FIPAContractNetResponder fsm;
        boolean isDone;

        @Override
        public void onStart() {
            super.onStart();
            this.isDone = false;
        }

        ExecuteTaskBehavior(FIPAContractNetResponder fsm) { super(fsm.getAgent()); this.fsm = fsm; }
        @Override
        public void action() {
            ACLMessage cfpMsg = (ACLMessage) fsm.sharedData.get("cfp");
            ACLMessage proposeMsg = (ACLMessage) fsm.sharedData.get("propose_msg");
            ACLMessage acceptMsg = (ACLMessage) fsm.sharedData.get("accept_msg");
            ACLMessage resultMsg = fsm.handleAcceptProposal(cfpMsg, proposeMsg, acceptMsg);
            if (resultMsg == null) return;

            if (cfpMsg.getConversationId() != null) resultMsg.setConversationId(cfpMsg.getConversationId());
            fsm.getAgent().send(resultMsg);
            isDone = true;
        }
        @Override
        public boolean done() { return isDone; }
    }
}
