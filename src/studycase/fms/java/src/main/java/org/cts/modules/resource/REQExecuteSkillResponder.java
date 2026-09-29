package org.cts.modules.resource;

import org.cts.modules.resource.libraries.kitt_library;
import org.cts.utilities.Constants;
import tamaf.*;

import java.io.IOException;

public class REQExecuteSkillResponder extends FIPARequestResponder {

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

    private void executeSkill(String content) throws IOException {
        if(((ResourceAgent) this.agent).myResourceType.matches(Constants.ResourceType.Station.name())){
            //Call arduino service
            if(content.matches("Skill_Screw")){
                kitt_library.executeStation("0");
            } else {
                kitt_library.executeStation("1");
            }
        } else if (((ResourceAgent) this.agent).myResourceType.matches(Constants.ResourceType.Robot.name())) {
            //Call ROS service
            // robot_library.executeSkill(content, ((ResourceAgent) this.agent).myLocation);
        } else if (((ResourceAgent) this.agent).myResourceType.matches(Constants.ResourceType.Human.name())) {
            //Call human service
            // human_library.executeSkill(content);
        }
    }

    @Override
    public ACLMessage prepareResultNotification(ACLMessage request, ACLMessage response) {
        System.out.println("DEBUG: " + this.agent.getName() + " Executing skill");
        ACLMessage reply = request.createReply();
        reply.setPerformative(Performative.INFORM);

        try {
            executeSkill((String)request.getContent());
        } catch (IOException e) {
            throw new RuntimeException(e);
        }

        //System.err.println("Executei skill: " + request.getContent());
        return reply;
    }
}