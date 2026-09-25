package org.cts.modules.resource;

import org.cts.utilities.Constants;
import tamaf.*;

import java.util.ArrayList;

public class ResourceAgent extends Agent {

    protected ArrayList<String> mySkills;
    protected String myResourceType;
    protected String myLocation;

    public ResourceAgent(String name, Object[] arguments) {
        super(Constants.formatAgentName(name));
        mySkills = (ArrayList<String>) arguments[0];
        myResourceType = (String) arguments[1];
        myLocation = (String) arguments[2];

        for (int i = 0; i < mySkills.size(); i++) {
            ServiceDescription sd = new ServiceDescription(mySkills.get(i));
            sd.setType(Constants.DF_SERVICE_TYPE_SKILL);
            this.agentDescription.addService(sd);
        }

        ServiceDescription sd = new ServiceDescription(name);
        sd.setType(Constants.DF_SERVICE_TYPE_RESOURCE);
        this.agentDescription.addService(sd);

    }

    @Override
    public void setup(EMAInteraction emailInteraction) {

        //Launch Responder Behaviours
        //Negotiation

        ACLMessageTemplate CNETResourceResponderMT = new ACLMessageTemplate();
        CNETResourceResponderMT.addPerformative(Performative.CFP);
        CNETResourceResponderMT.setOntology(Constants.ONTOLOGY_NEGOTIATE_NEXT_RESOURCE);
        this.addBehavior(new CNETResourceResponder(this, CNETResourceResponderMT));

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

    public void updateSkills(ArrayList<String> arrayList) {
        this.mySkills = arrayList;

        AgentDescription ad = this.getAgentDescriptionTemplate();

        for (int i = 0; i < mySkills.size(); i++) {
            ServiceDescription sd = new ServiceDescription(mySkills.get(i));
            sd.setType(Constants.DF_SERVICE_TYPE_SKILL);
            ad.addService(sd);
        }

        ServiceDescription sd = new ServiceDescription(ad.getAgentid().getName());
        sd.setType(Constants.DF_SERVICE_TYPE_RESOURCE);
        ad.addService(sd);

        this.updateAgentDescription(ad);
    }
}