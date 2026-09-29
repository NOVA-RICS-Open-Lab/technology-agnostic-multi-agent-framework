import jade.core.Profile;
import jade.core.ProfileImpl;
import jade.core.Runtime;
import jade.wrapper.AgentController;
import jade.wrapper.ContainerController;

import java.io.File;
import java.io.FileWriter;
import java.util.concurrent.ConcurrentHashMap;

public class Main {
    private static ConcurrentHashMap<String, Long> rttResults = new ConcurrentHashMap<>();

    public static void reportRTT(String agentName, long rtt) {
        rttResults.put(agentName, rtt);
    }

    public static void runTest(int numAgents, int numTest) {
        System.out.println("==================================================");
        System.out.println("Starting JADE test with " + numAgents + " sender agents... test number " + numTest);
        System.out.println("==================================================");
        
        rttResults.clear();
        
        Runtime rt = Runtime.instance();
        Profile p = new ProfileImpl();
        p.setParameter(Profile.MAIN_HOST, "localhost");
        p.setParameter(Profile.GUI, "false");
        ContainerController cc = rt.createMainContainer(p);

        try {
            long startRegistration = System.currentTimeMillis();
            
            AgentController receiver = cc.createNewAgent("receiver", "ReceiverAgent", null);
            receiver.start();

            // 10 seconds in the future
            long triggerTime = System.currentTimeMillis() + 10000;
            String receiverName = "receiver";

            AgentController[] senders = new AgentController[numAgents];
            for (int i = 0; i < numAgents; i++) {
                String name = "sender" + (i + 1);
                Object[] args = new Object[]{String.valueOf(triggerTime), receiverName};
                senders[i] = cc.createNewAgent(name, "SenderAgent", args);
                senders[i].start();
            }
            
            long endRegistration = System.currentTimeMillis();
            long registrationTime = endRegistration - startRegistration;
            
            System.out.println("Registration took " + (registrationTime / 1000.0) + " seconds. Waiting for trigger time...");
            
            long waitTime = triggerTime - System.currentTimeMillis();
            if (waitTime > 0) {
                Thread.sleep(waitTime + 2000); // 2 extra seconds buffer
            } else {
                Thread.sleep(2000);
            }
            
            System.out.println("Collecting results...");
            long timeout = System.currentTimeMillis() + 20000; // max wait 20 seconds for acks
            while (rttResults.size() < numAgents && System.currentTimeMillis() < timeout) {
                Thread.sleep(500);
            }
            
            if (rttResults.size() < numAgents) {
                System.out.println("WARNING: Timeout reached while waiting for ACKs.");
            } else {
                System.out.println("Test number " + numTest + " with " + numAgents + " agents completed successfully.");
            }
            
            File resultsDir = new File("../results");
            if (!resultsDir.exists()) {
                resultsDir.mkdirs();
            }
            
            File resultFile = new File(resultsDir, numTest + "_test_" + numAgents + "_agent_testresults.txt");
            try (FileWriter writer = new FileWriter(resultFile)) {
                writer.write("Registration Time: " + (registrationTime / 1000.0) + "\n");
                writer.write("Round Trip Times:\n");
                for (int i = 0; i < numAgents; i++) {
                    String name = "sender" + (i + 1);
                    Long rtt = rttResults.get(name);
                    String rttStr = rtt != null ? String.valueOf(rtt / 1000.0) : "TIMEOUT";
                    writer.write(name + ": " + rttStr + "\n");
                }
            }
            
            // Cleanup
            cc.kill();
            Thread.sleep(2000); // Give container time to shutdown properly
            
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void main(String[] args) {
        if (args.length >= 3 && "--execute-test".equals(args[0])) {
            int scale = Integer.parseInt(args[1]);
            int testNum = Integer.parseInt(args[2]);
            runTest(scale, testNum);
            System.exit(0);
        }

        int[] testScales;
        if (args.length > 0) {
            testScales = new int[args.length];
            for (int i = 0; i < args.length; i++) {
                testScales[i] = Integer.parseInt(args[i]);
            }
        } else {
            testScales = new int[]{1, 5, 10, 15, 20, 30, 40, 50, 75, 100, 200};
        }

        int numberOfTests = 10;

        for (int i = 1; i <= numberOfTests; i++) {
            System.out.println("\n=== Starting Test Loop " + i + "/" + numberOfTests + " ===");
            for (int scale : testScales) {
                System.out.println("Running JADE test for scale " + scale + " (Run " + i + ") in a new JVM...");
                try {
                    ProcessBuilder pb = new ProcessBuilder(
                        "java", "-cp", ".;D:\\jade\\lib\\jade.jar",
                        "Main", "--execute-test", String.valueOf(scale), String.valueOf(i)
                    );
                    pb.inheritIO();
                    Process p = pb.start();
                    int exitCode = p.waitFor();
                    if (exitCode != 0) {
                        System.err.println("Warning: Test process exited with code " + exitCode);
                    }
                    System.out.println("Waiting 1 seconds for ports to be fully released before the next test...");
                    Thread.sleep(1000);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }

        System.out.println("\nAll stress tests completed successfully.");
        System.exit(0);
    }
}
