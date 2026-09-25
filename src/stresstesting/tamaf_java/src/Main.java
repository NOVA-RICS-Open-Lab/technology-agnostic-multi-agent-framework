package tamaf.stresstest;
import tamaf.*;
import java.io.File;
import java.io.FileWriter;
import java.util.ArrayList;
import java.util.List;

public class Main {
    public static void main(String[] args) throws Exception {
        System.setProperty("DEFAULT_EMA_HEARTBEAT_INTERVAL", "60000");

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
                System.out.println("Running test for scale " + scale + " (Run " + i + ") in a new process...");
                ProcessBuilder pb = new ProcessBuilder(
                    "java", "-cp", System.getProperty("java.class.path"),
                    "tamaf.stresstest.Main", "--execute-test", String.valueOf(scale), String.valueOf(i)
                );
                pb.environment().put("DEFAULT_EMA_HEARTBEAT_INTERVAL", "60000");
                pb.inheritIO();
                Process process = pb.start();
                int exitCode = process.waitFor();
                if (exitCode != 0) {
                    System.err.println("Warning: Test process exited with code " + exitCode);
                }
                System.out.println("Waiting 1 seconds for ports to be fully released before the next test...");
                Thread.sleep(1000);
            }
        }
        System.exit(0);
    }
    
    public static void runTest(int numAgents, int numTest) throws Exception {
        System.out.println("==================================================");
        System.out.println("Starting TAMAF Java test with " + numAgents + " sender agents... test number " + numTest);
        System.out.println("==================================================");
        
        // Launch Python EMA process
        // Must use absolute path to PythonPackage/src based on current working directory
        String pythonScript = "import sys; import os; import time; " +
                              "os.environ['AVAILABLE_PORTS_END'] = '5000'; " +
                              "os.environ['DEFAULT_EMA_HEARTBEAT_INTERVAL'] = '60000'; " +
                              "sys.path.append(os.path.abspath('../../mk3/PythonPackage/src')); " +
                              "from tamaf import LaunchEMA; " +
                              "ema = LaunchEMA(debug=False); " +
                              "time.sleep(10000)";
        ProcessBuilder pb = new ProcessBuilder("py", "-c", pythonScript);
        pb.inheritIO(); // Pipe output so we can see if it fails
        Process emaProcess = pb.start();
        
        Thread shutdownHook = new Thread(() -> {
            if (emaProcess.isAlive()) {
                emaProcess.destroyForcibly();
            }
        });
        Runtime.getRuntime().addShutdownHook(shutdownHook);

        Receiver receiver = null;
        List<Sender> senders = new ArrayList<>();

        try {
            Thread.sleep(2000); // Give Python EMA time to boot
            
            receiver = new Receiver("receiver");
            receiver.start();
            
            // Wait for receiver to register
            while (!receiver.isRegistered && receiver.isAlive()) {
                Thread.sleep(50);
            }
            
            String receiverIp = receiver.getAgentDescription().getAgentid().getAddress().getIp();
            if (receiverIp == null || receiverIp.isEmpty()) {
                receiverIp = Utils.getLocalIP();
            }
            int receiverPort = receiver.getAgentDescription().getAgentid().getAddress().getPort();
            
            long startRegistration = System.currentTimeMillis();
            long triggerTime = System.currentTimeMillis() + 10000 + (long)(numAgents * 100); // 10s + buffer for seq reg
            
            for (int i = 0; i < numAgents; i++) {
                long t1 = System.currentTimeMillis();
                Sender sender = new Sender("sender" + (i+1), triggerTime, receiverIp, receiverPort);
                sender.start();
                senders.add(sender);
                
                // Wait for this sender to be fully registered in EMA
                while (!sender.isRegistered && sender.isAlive()) {
                    Thread.sleep(10);
                }
                long t2 = System.currentTimeMillis();
                System.out.printf("sender%d took %.4f seconds to register.%n", (i+1), (t2 - t1) / 1000.0);
            }
            
            long endRegistration = System.currentTimeMillis();
            double registrationTime = (endRegistration - startRegistration) / 1000.0;
            
            System.out.println("SUCCESS: All " + numAgents + " agents are fully registered and ACTIVE in the EMA.");
            System.out.println("Registration took " + registrationTime + " seconds. Waiting for trigger time...");
            
            long waitTime = triggerTime - System.currentTimeMillis();
            if (waitTime > 0) {
                Thread.sleep(waitTime + 2000);
            } else {
                Thread.sleep(2000);
            }
            
            System.out.println("Collecting results...");
            boolean allDone = false;
            long timeout = System.currentTimeMillis() + 20000;
            while (!allDone && System.currentTimeMillis() < timeout) {
                allDone = true;
                for (Sender sender : senders) {
                    if (sender.rtt == null) {
                        allDone = false;
                        break;
                    }
                }
                if (!allDone) {
                    Thread.sleep(500);
                }
            }
            
            if (System.currentTimeMillis() >= timeout) {
                System.out.println("WARNING: Timeout reached while waiting for ACKs.");
            } else {
                System.out.println("Test number " + numTest + " with " + numAgents + " agents completed successfully.");
            }
            
            File resultsDir = new File("results");
            if (!resultsDir.exists()) resultsDir.mkdirs();
            
            FileWriter writer = new FileWriter(new File(resultsDir, numTest + "_test_" + numAgents + "_agent_testresults.txt"));
            writer.write("Registration Time: " + registrationTime + "\n");
            writer.write("Round Trip Times:\n");
            
            for (Sender sender : senders) {
                String rttVal = sender.rtt != null ? String.valueOf(sender.rtt / 1000.0) : "TIMEOUT";
                writer.write(sender.getAgentDescription().getAgentid().getName() + ": " + rttVal + "\n");
            }
            writer.close();
        } finally {
            for (Sender sender : senders) {
                try { sender.stop(); } catch (Exception ignored) {}
            }
            if (receiver != null) {
                try { receiver.stop(); } catch (Exception ignored) {}
            }
            if (emaProcess.isAlive()) {
                emaProcess.destroy();
                if (!emaProcess.waitFor(5, java.util.concurrent.TimeUnit.SECONDS)) {
                    emaProcess.destroyForcibly();
                }
            }
            try {
                Runtime.getRuntime().removeShutdownHook(shutdownHook);
            } catch (IllegalStateException ignored) {}
            Thread.sleep(2000);
        }
    }
}
