package org.cts.modules.product;

import org.cts.utilities.Constants;

import tamaf.*;

public class NextSkillBehaviour extends OneShotBehavior {

    @Override
    public void action() {
        //GetNextSkill
        String nextSkill = ((ProdutAgent) this.agent).currentSkill();
        //LaunchCFP

            System.out.println("/////////////////////////\nAGORA VOU FAZER A SKILL: " + nextSkill + "\n/////////////////////////");

            AgentDescription searchAD = new AgentDescription();

            ServiceDescription service = new ServiceDescription(nextSkill);

            searchAD.addService(service);

            //Create message
            AgentDescription[] dfAgentDescriptions = this.emaInteraction.search(searchAD).toArray(new AgentDescription[0]);

        try {
            ACLMessage msg = new ACLMessage(Performative.CFP);
            msg.setContent(nextSkill);
            if(dfAgentDescriptions.length!=0){
                for (int i = 0; i < dfAgentDescriptions.length; i++) {
                    msg.addReceiver(dfAgentDescriptions[i].getAgentid());
                }
                msg.setOntology(Constants.ONTOLOGY_NEGOTIATE_NEXT_RESOURCE);
                //Add behaviour
                ((ProdutAgent) this.agent).addBehavior(new CNETProposalInitiator(this.agent,msg));
            } else {
                ((ProdutAgent) this.agent).addBehavior(new NextSkillBehaviour());
            }

        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
            throw new RuntimeException(e);
        }
    }
}
