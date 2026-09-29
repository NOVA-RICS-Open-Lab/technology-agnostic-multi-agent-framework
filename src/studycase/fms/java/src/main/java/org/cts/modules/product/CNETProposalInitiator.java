package org.cts.modules.product;

import org.cts.utilities.Constants;

import org.json.JSONObject;
import tamaf.*;

import java.util.List;

public class CNETProposalInitiator extends FIPAContractNetInitiator {
    public CNETProposalInitiator(Agent a, ACLMessage cfp) {
        super(a, cfp);
    }

    @Override
    public boolean handleProposals(List<ACLMessage> responses, List<ACLMessage> acceptances) {
        System.out.println("DEBUG: " + this.agent.getName() + " Processing proposals");

        int bestProposalIndex = 0;
        for (int i = 0; i < responses.size(); i++) {
            ACLMessage reply = ((ACLMessage) responses.get(i)).createReply();
            reply.setPerformative(Performative.REJECT_PROPOSAL);
            acceptances.add(reply);
        }

        ACLMessage boa = ((ACLMessage) acceptances.get(bestProposalIndex));
        boa.setPerformative(Performative.ACCEPT_PROPOSAL);
        acceptances.set(bestProposalIndex, boa);
        return true;
    }

    @Override
    public Boolean handleInform(ACLMessage inform) {
        System.out.println("DEBUG: " + this.agent.getName() + "Received inform from CNET");
        ((ProdutAgent)this.agent).nextLocation = inform.getContent().toString();
        ((ProdutAgent) this.agent).resourceExecuter = inform.getSender();

        //Add Transport Behaviour
        //Create message

        AgentDescription ad = new AgentDescription();
        ServiceDescription sd = new ServiceDescription(Constants.DF_SERVICE_NAME_TRANSPORT);
        sd.setType(Constants.DF_SERVICE_TYPE_TRANSPORT);
        ad.addService(sd);

        List<AgentDescription> agentDescriptions = this.emaInteraction.search(ad);

        if(agentDescriptions.isEmpty())
        {
            System.out.println("DEBUG: " + this.agent.getName() + "Transport not found in DF");
            return false;
        }

        ACLMessage msg = new ACLMessage(Performative.REQUEST);

        JSONObject myJson = new JSONObject();
        myJson.put("a", ((ProdutAgent) this.agent).myLocation);
        myJson.put("b", ((ProdutAgent) this.agent).nextLocation);

        msg.setContent(myJson.toString());

        msg.addReceiver(agentDescriptions.get(0).getAgentid());
        msg.setOntology(Constants.ONTOLOGY_REQUEST_TRANSPORT);
        //Add behaviour
        ((ProdutAgent) this.agent).addBehavior(new REQUESTExecuteTransportInitiator(this.agent,msg));
        return true;
    }
}
