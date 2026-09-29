package org.cts.modules.resource;

import org.cts.utilities.Constants;
import tamaf.*;

public class CNETResourceResponder extends FIPAContractNetResponder {

    public CNETResourceResponder(Agent a, ACLMessageTemplate mt) {
        super(a, mt);
    }

    @Override
    public ACLMessage handleCfp(ACLMessage cfp) {
        System.out.println("DEBUG: " + this.agent.getName() + " Creating proposal");
        ACLMessage reply = cfp.createReply();
        reply.setPerformative(Performative.PROPOSE);
        if(String.valueOf(Constants.ResourceType.Human).equals(((ResourceAgent)this.agent).myResourceType))
            reply.setContent("2");
        else
            reply.setContent("1");
        return reply;
    }

    @Override
    public ACLMessage handleAcceptProposal(ACLMessage cfp, ACLMessage propose, ACLMessage accept) {
        System.out.println("DEBUG: " + this.agent.getName() + " Proposal Accepted");
        ACLMessage reply = accept.createReply();
        reply.setPerformative(Performative.INFORM);
        reply.setContent(((ResourceAgent)this.agent).myLocation);
        return reply;
    }
}