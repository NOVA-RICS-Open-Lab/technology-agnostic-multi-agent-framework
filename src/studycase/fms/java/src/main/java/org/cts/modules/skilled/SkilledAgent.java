package org.cts.modules.skilled;

import org.cts.utilities.Constants;
import tamaf.*;

import java.util.ArrayList;

public class SkilledAgent extends Agent {

    protected String[][] mySkills;
    protected String myResourceType;
    protected String myLocation;

    public SkilledAgent(String name, Object[] arguments) {
        super(Constants.formatAgentName(name));

        mySkills = (String[][]) arguments[0];
        myResourceType = (String) arguments[1];
        myLocation = (String) arguments[2];

        for (int i = 0; i < mySkills.length; i++) {
            ServiceDescription sd = new ServiceDescription(mySkills[i][0]);
            sd.setType(mySkills[i][1]);
            this.agentDescription.addService(sd);
        }

    }

    public String[][] getSkills() {
        return this.mySkills;
    }

    @Override
    public void setup(EMAInteraction emailInteraction) {

        //Launch Responder Behaviours
        //Negotiation

        ACLMessageTemplate CNETResourceResponderMT = new ACLMessageTemplate();
        CNETResourceResponderMT.addPerformative(Performative.CFP);
        CNETResourceResponderMT.setOntology(Constants.ONTOLOGY_NEGOTIATE_NEXT_RESOURCE);
        this.addBehavior(new CNETSkilledResponder(this, CNETResourceResponderMT));

        ACLMessageTemplate REQExecuteSkillResponderMT = new ACLMessageTemplate();
        REQExecuteSkillResponderMT.addPerformative(Performative.REQUEST);
        REQExecuteSkillResponderMT.setOntology(Constants.ONTOLOGY_REQUEST_EXECUTE_SKILL);
        this.addBehavior(new REQExecuteSkillResponder(this, REQExecuteSkillResponderMT));

        //Request Update Configs
        //ACLMessageTemplate REQUESTUpdateConfigsResponderMT = new ACLMessageTemplate();
        //REQUESTUpdateConfigsResponderMT.addPerformative(Performative.REQUEST);
        //REQUESTUpdateConfigsResponderMT.setOntology(Constants.ONTOLOGY_REQUEST_UPDATE_CONFIG);
        //this.addBehavior(new REQUESTUpdateConfigsResponder(this, REQUESTUpdateConfigsResponderMT));
    }

    public void updateSkills(String[][] arrayList) {
        this.mySkills = arrayList;

        AgentDescription ad = this.getAgentDescriptionTemplate();

        for (int i = 0; i < mySkills.length; i++) {
            ServiceDescription sd = new ServiceDescription(mySkills[i][0]);
            sd.setType(Constants.DF_SERVICE_TYPE_SKILL);
            ad.addService(sd);
        }

        this.updateAgentDescription(ad);
    }
}