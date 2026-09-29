package org.cts.modules.launch;

import org.cts.modules.product.ProdutAgent;
import org.cts.modules.skilled.SkilledAgent;
import org.cts.modules.transport.TransportAgent;
import org.cts.utilities.Constants;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Scanner;

import tamaf.*;

public class AgentLauncher {
    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);
        Constants constants = new Constants();

        if (args.length > 0) {
            String input = args[0];
            switch (input) {
                case "1":
                    launchResource1(constants);
                    return;
                case "2":
                    launchResource2(constants);
                    return;
                case "3":
                    launchTransport(constants);
                    return;
                case "4":
                    launchProduct();
                    return;
                case "5":
                    launchFMSTower(constants);
                    return;
                default:
                    System.out.println("Unknown argument: " + input);
            }
        }
        
        System.out.println("Agent Launcher Started.");
        System.out.println("1 - Launch Resource 1 (Drill)");
        System.out.println("2 - Launch Resource 2 (Screw)");
        System.out.println("3 - Launch Transport Agent");
        System.out.println("4 - Launch Product Agent");
        System.out.println("5 - FMS Tower Agent");
        System.out.println("0 - Exit");

        while (true) {
            System.out.print("> ");
            String input = scanner.nextLine();

            switch (input) {
                case "1":
                    launchResource1(constants);
                    break;
                case "2":
                    launchResource2(constants);
                    break;
                case "3":
                    launchTransport(constants);
                    break;
                case "4":
                    launchProduct();
                    break;
                case "5":
                    launchFMSTower(constants);
                    break;
                case "0":
                    System.out.println("Exiting...");
                    System.exit(0);
                default:
                    System.out.println("Invalid input.");
            }
        }
    }

    private static void launchResource1(Constants constants) {
        String[][] skill = {constants.skills[0]}; // Drill
        Object[] args = new Object[]{skill, "Station", "A"};
        SkilledAgent agent = new SkilledAgent("Resource_1", args);
        agent.start();
        System.out.println("Resource 1 (Drill) launched.");
    }

    private static void launchResource2(Constants constants) {
        String[][] skill = {constants.skills[1]}; // Screw
        Object[] args = new Object[]{skill, "Station", "B"};
        SkilledAgent agent = new SkilledAgent("Resource_2", args);
        agent.start();
        System.out.println("Resource 2 (Screw) launched.");
    }

    private static void launchTransport(Constants constants) {
        String[][] skills = {constants.skills[2], constants.skills[3]}; // GoToAB, GoToBC
        Object[] args = new Object[]{skills, "Robot", "A"};
        TransportAgent agent = new TransportAgent("Transport_1", args);
        agent.start();
        System.out.println("Transport Agent launched.");
    }

    private static void launchProduct() {
        ArrayList<String> process = new ArrayList<>(Arrays.asList("Drill", "Screw"));
        Object[] args = new Object[]{process, "A"};
        ProdutAgent agent = new ProdutAgent("Product_1", args);
        agent.start();
        System.out.println("Product Agent launched.");
    }

    private static void launchFMSTower(Constants constants) {
        String[][] skills = {constants.skills[0], constants.skills[1]};
        Object[] args = new Object[]{skills, "Station", "D"};
        SkilledAgent agent = new SkilledAgent("FMS_Tower", args);
        agent.start();
        System.out.println("FMS_Tower launched.");
    }
}
