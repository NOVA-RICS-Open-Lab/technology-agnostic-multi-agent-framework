package tamaf;

public class RequestInitiatorAgent extends Agent {

    public RequestInitiatorAgent(String name) {
        super(name);
    }

    @Override
    public void setup(EMAInteraction emaInteraction) {
        // We wait a bit to ensure responder is ready
        addBehavior(new WakerBehavior(this, 1000) {
            @Override
            public void onWake() {
                ACLMessage request = new ACLMessage(Performative.REQUEST);
                request.addReceiver(new AgentID("Responder", new Address("192.168.0.10", 4010)));
                request.setContent("Calculate 2+2");

                addBehavior(new FIPARequestInitiator(getAgent(), request) {
                    @Override
                    public Boolean handleAgree(ACLMessage msg) {
                        System.out.println("Initiator: Responder agreed: " + msg.getContent());
                        return true;
                    }

                    @Override
                    public Boolean handleInform(ACLMessage msg) {
                        System.out.println("Initiator: Received result: " + msg.getContent());
                        return true;
                    }

                    @Override
                    public Boolean handleRefuse(ACLMessage msg) {
                        System.out.println("Initiator: Responder refused.");
                        return true;
                    }

                    @Override
                    public Boolean handleFailure(ACLMessage msg) {
                        System.out.println("Initiator: Protocol failure.");
                        return true;
                    }
                });
            }
        });
    }

    public static void main(String[] args) {
        RequestInitiatorAgent initiator = new RequestInitiatorAgent("InitiatorAgent");
        initiator.start();

        while (true) {
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                break;
            }
        }
    }
}
