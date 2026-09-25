package org.cts.modules.product;

import org.cts.utilities.Constants;
import org.json.JSONObject;
import tamaf.*;

import java.util.List;

public class REQUESTExecuteSkillInitiator extends FIPARequestInitiator {

    public REQUESTExecuteSkillInitiator(Agent a, ACLMessage msg) {
        super(a, msg);
    }

    @Override
    public Boolean handleAgree(ACLMessage agree) {
        System.out.println("DEBUG: " + this.agent.getName() + " Received AGREE execute skill");
        return true;
    }

    @Override
    public Boolean handleRefuse(ACLMessage refuse) {
        System.out.println("DEBUG: " + this.agent.getName() + " Received REFUSE execute skill");
        this.agent.addBehavior(new NextSkillBehaviour());
        return true;
    }

    @Override
    public Boolean handleInform(ACLMessage inform) {
        System.out.println("DEBUG: " + this.agent.getName() + " Received INFORM execute skill");
        if (((ProdutAgent) this.agent).hasNextSkill()) {
            ((ProdutAgent) this.agent).setNextSkill();
            this.agent.addBehavior(new NextSkillBehaviour());
        }else{
            //Leave system (Transport to A)
            ((ProdutAgent)this.agent).nextLocation = Constants.Locations.A.name();
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

        }
        return true;
    }
}
