package org.cts.modules.skilled;

import tamaf.*;

public class CNETSkilledResponder extends FIPAContractNetResponder {

    public CNETSkilledResponder(Agent a, ACLMessageTemplate mt) {
        super(a, mt);
    }

    @Override
    public ACLMessage handleCfp(ACLMessage cfp) {
        System.out.println("DEBUG: " + this.agent.getName() + " Creating proposal");
        ACLMessage reply = cfp.createReply();
        reply.setPerformative(Performative.PROPOSE);
        reply.setContent("1");
        return reply;
    }

    @Override
    public ACLMessage handleAcceptProposal(ACLMessage cfp, ACLMessage propose, ACLMessage accept) {
        System.out.println("DEBUG: " + this.agent.getName() + " Proposal Accepted");
        ACLMessage reply = accept.createReply();
        reply.setPerformative(Performative.INFORM);
        reply.setContent(((SkilledAgent)this.agent).myLocation);
        return reply;
    }
}