package tamaf;

import java.util.Collections;

public class FIPARequestDispatcher extends CyclicBehavior {
    protected ACLMessageTemplate template;

    public FIPARequestDispatcher(Agent agent) {
        this(agent, null);
    }

    public FIPARequestDispatcher(Agent agent, ACLMessageTemplate template) {
        super(agent);
        if (template != null) {
            this.template = template;
        } else {
            this.template = new ACLMessageTemplate();
            this.template.setPerformatives(Collections.singleton(Performative.REQUEST));
        }
    }

    @Override
    public void action() {
        ACLMessage incomingRequest = receive(template);
        FIPARequestResponderForDispatcher responder = createResponder(incomingRequest);
        if (responder != null) {
            agent.addBehavior(responder);
        }
    }

    public FIPARequestResponderForDispatcher createResponder(ACLMessage request) {
        return new FIPARequestResponderForDispatcher(agent, request);
    }
}
