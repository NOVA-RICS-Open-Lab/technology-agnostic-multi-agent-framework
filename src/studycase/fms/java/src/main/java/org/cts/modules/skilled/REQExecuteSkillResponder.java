package org.cts.modules.skilled;

import org.cts.modules.resource.libraries.kitt_library;
import org.cts.modules.skilled.libraries.executionLibrary;
import org.cts.utilities.Constants;
import tamaf.*;

import java.io.IOException;
import java.util.Objects;

public class REQExecuteSkillResponder extends FIPARequestResponder {

    Thread executinonThread = null;

    public REQExecuteSkillResponder(Agent a, ACLMessageTemplate mt) {
        super(a, mt);
    }

    @Override
    public ACLMessage handleRequest(ACLMessage request) {
        System.out.println("DEBUG: " + this.agent.getName() + " Received REQUEST execute skill");
        ACLMessage reply = request.createReply();
        reply.setPerformative(Performative.AGREE);
        return reply;
    }

    @Override
    public ACLMessage prepareResultNotification(ACLMessage request, ACLMessage response) {
        if (executinonThread == null) {
            try {
                this.executinonThread = executeSkill((String)request.getContent());
            } catch (IOException e) {
                throw new RuntimeException(e);
            }
        }

        if (this.executinonThread.isAlive()) {
            return null;
        }

        this.executinonThread = null;

        ACLMessage reply = request.createReply();
        reply.setPerformative(Performative.INFORM);
        System.err.println("INFO: " + this.agent.getName() + "Skill executed: " + request.getContent());
        return reply;

    }

    private Thread executeSkill(String content) throws IOException {

        String [][] availableSkills = ((SkilledAgent)this.agent).getSkills();

        String urlSkill = "";

        for (int i = 0; i < availableSkills.length; i++) {
            if (Objects.equals(availableSkills[i][0], content)) {
                urlSkill = availableSkills[i][2];
                break;
            }
        }

        String finalUrlSkill = urlSkill;

        Thread thread = new Thread(() -> {
            try {
                executionLibrary.executeSkill(finalUrlSkill);
            } catch (IOException e) {
                System.out.println("ERROR: " + this.agent.getName() + "Exception: " + e.getMessage());
                throw new RuntimeException(e);
            }
        });

        thread.start();

        return thread;
    }

}