package org.cts.modules.product;

import org.cts.utilities.Constants;

import tamaf.*;

public class REQUESTExecuteTransportInitiator extends FIPARequestInitiator {

    public REQUESTExecuteTransportInitiator(Agent a, ACLMessage msg) {
        super(a, msg);
    }

    @Override
    public Boolean handleAgree(ACLMessage agree) {
        System.out.println("DEBUG: " + this.agent.getName() + " Received AGREE execute transport");
        return true;
    }

    @Override
    public Boolean handleRefuse(ACLMessage refuse) {
        System.out.println("DEBUG: " + this.agent.getName() + " Received REFUSE execute transport");
        ((ProdutAgent) this.agent).addBehavior(new NextSkillBehaviour());
        return true;
    }

    @Override
    public Boolean handleInform(ACLMessage inform) {
        System.out.println("DEBUG: " + this.agent.getName() + " Received INFORM execute transport");
        if(((ProdutAgent) this.agent).hasNextSkill()){
            ((ProdutAgent) this.agent).myLocation = inform.getContent().toString();
            //Execute Skill
            //Add Execute Skill Behaviour
            //Create message
            ACLMessage msg = new ACLMessage(Performative.REQUEST);
            msg.addReceiver(((ProdutAgent) this.agent).resourceExecuter);
            msg.setContent(((ProdutAgent) this.agent).currentSkill());
            msg.setOntology(Constants.ONTOLOGY_REQUEST_EXECUTE_SKILL);
            //Add behaviour
            ((ProdutAgent) this.agent).addBehavior(new REQUESTExecuteSkillInitiator(this.agent,msg));
        }else{
            System.out.println("DEBUG: " + this.agent.getName() + " LEAVING SYSTEM");
            this.agent.stop();
        }
        return true;
    }
}
