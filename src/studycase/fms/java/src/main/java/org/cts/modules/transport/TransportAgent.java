package org.cts.modules.transport;

import org.cts.utilities.Constants;
import tamaf.*;

import java.util.ArrayList;

public class TransportAgent extends Agent {

    protected String currentProductLocation;

    protected String[][] mySkills;
    protected String myResourceType;
    protected String myLocation;

    public TransportAgent(String name, Object[] arguments) {
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

    @Override
    public void setup(EMAInteraction emailInteraction) {

        //Launch Responder Behaviours
        //Negotiation

        //ACLMessageTemplate CNETResourceResponderMT = new ACLMessageTemplate();
        //CNETResourceResponderMT.addPerformative(Performative.CFP);
        //CNETResourceResponderMT.setOntology(Constants.ONTOLOGY_NEGOTIATE_NEXT_RESOURCE);
        //this.addBehavior(new CNETResourceResponder(this, CNETResourceResponderMT));

        ACLMessageTemplate REQUESTExecuteTransportMT = new ACLMessageTemplate();
        REQUESTExecuteTransportMT.addPerformative(Performative.REQUEST);
        REQUESTExecuteTransportMT.setOntology(Constants.ONTOLOGY_REQUEST_TRANSPORT);
        this.addBehavior(new REQExecuteTransportResponder(this, REQUESTExecuteTransportMT));
    }

}