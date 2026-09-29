package org.cts.modules.product;

import org.cts.utilities.Constants;
import tamaf.*;

import java.util.ArrayList;

public class ProdutAgent extends Agent {

    protected AgentID resourceExecuter;
    protected ArrayList<String> myProcess;
    protected String myLocation;
    protected String nextLocation;

    public ProdutAgent(String name, Object[] arguments) {
        super(Constants.formatAgentName(name));

        myProcess = (ArrayList<String>) arguments[0];
        myLocation = (String) arguments[1];
    }

    public boolean hasNextSkill() {
        System.out.println("/////////////////////////\nO PROCESSO ESTA ASSIM: " + this.myProcess + "\n/////////////////////////");

        return this.myProcess.size() > 1;
    }

    public void setNextSkill() {
        this.myProcess.remove(0);
    }

    public String currentSkill() {
        return this.myProcess.get(0);
    }

    @Override
    public void setup(EMAInteraction emaInteraction) {
        // Get Arguments
        //1 - List of Skills - ArrayList with Strings
        //2 - Priority - String (Robot, Human)
        //3 - Location - String

        //Get Next Skill behaviour
        this.addBehavior(new NextSkillBehaviour());
    }

}
