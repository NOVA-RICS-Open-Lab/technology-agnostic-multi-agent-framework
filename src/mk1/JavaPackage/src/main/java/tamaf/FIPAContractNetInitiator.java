package tamaf;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.UUID;

import tamaf.defines.Defines;

public class FIPAContractNetInitiator extends FSMBehavior {
    protected ACLMessage cfpMessage;
    protected ACLMessageTemplate replyTemplate;
    protected long deadline;
    protected int expectedCFPs = 0;

    public FIPAContractNetInitiator(Agent agent, ACLMessage cfpMessage) {
        super(agent);
        this.cfpMessage = cfpMessage;
        this.deadline = System.currentTimeMillis() + Defines.DEFAULT_CONTRACTNETINITIATOR_RECEIVE_CFPS_TIMEOUT * 1000;
        if (this.cfpMessage.getConversationId() == null) {
            this.cfpMessage.setConversationId("cnet-" + UUID.randomUUID().toString());
        }
        this.replyTemplate = new ACLMessageTemplate();
        this.replyTemplate.setConversationId(this.cfpMessage.getConversationId());

        addInitialState(new SendCfpBehavior(this), "sendCFP");
        addState(new CollectProposalsBehavior(this), "collect");
        addState(new HandleProposeBehavior(this), "handlePropose");
        addState(new HandleRefuseBehavior(this), "handleRefuse");
        addState(new HandleProposalsBehavior(this), "handleProposals");
        addState(new WaitResultsBehavior(this), "waitResults");
        addState(new HandleInformBehavior(this), "handleInform");
        addState(new HandleFailureBehavior(this), "handleFailure");
        addFinalState(new OneShotBehavior(agent) { @Override public void action() {} }, "end");

        addTransition("sendCFP", "collect", 0);
        
        // Collect loop
        addTransition("collect", "handlePropose", 0);
        addTransition("collect", "handleRefuse", 1);
        addTransition("collect", "handleProposals", 2); // Done collecting
        
        addTransition("handlePropose", "collect", 0);
        addTransition("handleRefuse", "collect", 0);

        addTransition("handleProposals", "waitResults", 0);
        addTransition("handleProposals", "end", 1);

        // Results loop
        addTransition("waitResults", "handleInform", 0);
        addTransition("waitResults", "handleFailure", 1);
        addTransition("waitResults", "end", 2); // Done results

        addTransition("handleInform", "waitResults", 0);
        addTransition("handleFailure", "waitResults", 0);
    }

    @Override
    public void onStart() {
        this.sharedData.put("proposals", new ArrayList<ACLMessage>());
        this.sharedData.put("accepts", new ArrayList<ACLMessage>());
        this.sharedData.put("results", new ArrayList<ACLMessage>());
        super.onStart();
    }

    public List<ACLMessage> prepareCfps(ACLMessage msg) { return Collections.singletonList(msg); }
    public Boolean handlePropose(ACLMessage msg) { return true; }
    public Boolean handleRefuse(ACLMessage msg) { return true; }
    public boolean handleProposals(List<ACLMessage> proposals, List<ACLMessage> responses) { return true; }
    public Boolean handleInform(ACLMessage msg) { return true; }
    public Boolean handleFailure(ACLMessage msg) { return true; }

    private class SendCfpBehavior extends SimpleBehavior {
        FIPAContractNetInitiator fsm;
        boolean isDone;

        SendCfpBehavior(FIPAContractNetInitiator fsm) { super(fsm.getAgent()); this.fsm = fsm; }

        @Override
        public void onStart() {
            super.onStart();
            this.isDone = false;
        }

        @Override
        public void action() {
            List<ACLMessage> cfps = fsm.prepareCfps(fsm.cfpMessage);
            if (cfps == null) return;

            for (ACLMessage msg : cfps) {
                fsm.expectedCFPs += msg.getReceiver().size();
                msg.setConversationId(fsm.cfpMessage.getConversationId());
                fsm.getAgent().send(msg);
            }
            isDone = true;
        }
        @Override
        public boolean done() { return isDone; }
    }

    private class CollectProposalsBehavior extends SimpleBehavior {
        FIPAContractNetInitiator fsm;
        int cfpsReceived = 0;
        int transitionCode;
        boolean isDone;

        CollectProposalsBehavior(FIPAContractNetInitiator fsm) { super(fsm.getAgent()); this.fsm = fsm; }

        @Override
        public void onStart() {
            super.onStart();
            this.transitionCode = -1;
            this.isDone = false;
        }

        @Override
        public void action() {
            if (cfpsReceived >= fsm.expectedCFPs || System.currentTimeMillis() > fsm.deadline) {
                transitionCode = 2;
                isDone = true;
                return;
            }

            ACLMessage reply = receive(fsm.replyTemplate);
            if (reply == null) return;

            fsm.sharedData.put("current_reply", reply);
            cfpsReceived++;

            if (reply.getPerformative() == Performative.PROPOSE) transitionCode = 0;
            else if (reply.getPerformative() == Performative.REFUSE) transitionCode = 1;
            else transitionCode = 1; // Treat unknown as refuse for safety or skip

            isDone = true;
        }
        @Override
        public boolean done() { return isDone; }
        @Override
        public int onEnd() {
            if (transitionCode == 2) {
                this.cfpsReceived = 0;
            }
            return transitionCode; }
    }

    private class HandleProposeBehavior extends SimpleBehavior {
        FIPAContractNetInitiator fsm;
        boolean isDone;

        HandleProposeBehavior(FIPAContractNetInitiator fsm) { super(fsm.getAgent()); this.fsm = fsm; }

        @Override
        public void onStart() {
            super.onStart();
            this.isDone = false;
        }

        @Override
        public void action() {
            ACLMessage msg = (ACLMessage) fsm.sharedData.get("current_reply");
            Boolean res = fsm.handlePropose(msg);
            if (res == null || !res) return;

            ((List<ACLMessage>) fsm.sharedData.get("proposals")).add(msg);
            isDone = true;
        }
        @Override
        public boolean done() { return isDone; }
        @Override
        public int onEnd() { return 0; }
    }

    private class HandleRefuseBehavior extends SimpleBehavior {
        FIPAContractNetInitiator fsm;
        boolean isDone = false;
        HandleRefuseBehavior(FIPAContractNetInitiator fsm) { super(fsm.getAgent()); this.fsm = fsm; }
        @Override
        public void action() {
            ACLMessage msg = (ACLMessage) fsm.sharedData.get("current_reply");
            Boolean res = fsm.handleRefuse(msg);
            if (res == null || !res) return;

            ((List<ACLMessage>) fsm.sharedData.get("proposals")).add(msg);
            isDone = true;
        }
        @Override
        public boolean done() { return isDone; }
        @Override
        public int onEnd() { return 0; }
    }

    private class HandleProposalsBehavior extends SimpleBehavior {
        FIPAContractNetInitiator fsm;
        int transitionCode;
        boolean isDone;
        List<ACLMessage> responses;
        List<ACLMessage> acceptances;

        HandleProposalsBehavior(FIPAContractNetInitiator fsm) {
            super(fsm.getAgent());
            this.fsm = fsm;
        }

        @Override
        public void onStart() {
            super.onStart();
            this.transitionCode = -1;
            this.isDone = false;
            this.responses = new ArrayList<>();
            this.acceptances = new ArrayList<>();
        }
        
        @Override
        public void action() {

            if(!fsm.handleProposals((List<ACLMessage>) fsm.sharedData.get("proposals"), responses)){
                return;
            }
            
            fsm.sharedData.put("responses", responses);

            for (ACLMessage msg : this.responses) {
                msg.setConversationId(fsm.cfpMessage.getConversationId());
                if (msg.getPerformative().equals(Performative.ACCEPT_PROPOSAL)) {
                    acceptances.add(msg);
                }
                fsm.getAgent().send(msg);
            }

            fsm.sharedData.put("acceptances", this.acceptances);
            transitionCode = (!this.acceptances.isEmpty()) ? 0 : 1;
            isDone = true;
        }
        @Override
        public boolean done() { return isDone; }
        @Override
        public int onEnd() { return transitionCode; }
    }

    private class WaitResultsBehavior extends SimpleBehavior {
        FIPAContractNetInitiator fsm;
        int transitionCode;
        boolean isDone;

        WaitResultsBehavior(FIPAContractNetInitiator fsm) { super(fsm.getAgent()); this.fsm = fsm; }

        @Override
        public void onStart() {
            super.onStart();
            this.transitionCode = -1;
            this.isDone = false;
        }
        @Override
        public void action() {
            List<ACLMessage> results = (List<ACLMessage>) fsm.sharedData.get("results");
            List<ACLMessage> acceptances = (List<ACLMessage>) fsm.sharedData.get("acceptances");

            if (results.size() >= acceptances.size()) {
                transitionCode = 2;
                isDone = true;
                return;
            }

            ACLMessage reply = receive(fsm.replyTemplate);
            if (reply == null) return;

            fsm.sharedData.put("current_reply", reply);
            if (reply.getPerformative() == Performative.INFORM) transitionCode = 0;
            else if (reply.getPerformative() == Performative.FAILURE) transitionCode = 1;
            else transitionCode = 1;

            isDone = true;
        }
        @Override
        public boolean done() { return isDone; }
        @Override
        public int onEnd() { return transitionCode; }
    }

    private class HandleInformBehavior extends SimpleBehavior {
        FIPAContractNetInitiator fsm;
        boolean isDone;

        HandleInformBehavior(FIPAContractNetInitiator fsm) { super(fsm.getAgent()); this.fsm = fsm; }

        @Override
        public void onStart() {
            super.onStart();
            this.isDone = false;
        }

        @Override
        public void action() {
            ACLMessage msg = (ACLMessage) fsm.sharedData.get("current_reply");
            Boolean res = fsm.handleInform(msg);
            if (res == null || !res) return;

            ((List<ACLMessage>) fsm.sharedData.get("results")).add(msg);
            isDone = true;
        }
        @Override
        public boolean done() { return isDone; }
        @Override
        public int onEnd() { return 0; }
    }

    private class HandleFailureBehavior extends SimpleBehavior {
        FIPAContractNetInitiator fsm;
        boolean isDone;

        HandleFailureBehavior(FIPAContractNetInitiator fsm) { super(fsm.getAgent()); this.fsm = fsm; }

        @Override
        public void onStart() {
            super.onStart();
            this.isDone = false;
        }

        @Override
        public void action() {
            ACLMessage msg = (ACLMessage) fsm.sharedData.get("current_reply");
            Boolean res = fsm.handleFailure(msg);
            if (res == null || !res) return;

            ((List<ACLMessage>) fsm.sharedData.get("results")).add(msg);
            isDone = true;
        }
        @Override
        public boolean done() { return isDone; }
        @Override
        public int onEnd() { return 0; }
    }
}
