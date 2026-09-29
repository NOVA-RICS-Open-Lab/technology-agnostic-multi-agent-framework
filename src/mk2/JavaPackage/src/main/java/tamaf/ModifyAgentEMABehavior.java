package tamaf;

public class ModifyAgentEMABehavior extends OneShotBehavior {
    public ModifyAgentEMABehavior() {
        super();
    }

    public ModifyAgentEMABehavior(Agent agent) {
        super(agent);
    }

    @Override
    public void action() {
        emaInteraction.modifyAgent();
    }
}
