package tamaf;

import java.net.DatagramSocket;
import java.net.InetAddress;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;

public class Utils {
    private static final BlockingQueue<String> logQueue = new LinkedBlockingQueue<>();
    static {
        Thread logThread = new Thread(() -> {
            try {
                while (true) {
                    String msg = logQueue.take();
                    System.out.println(msg);
                }
            } catch (InterruptedException ignored) {}
        }, "TAMAF-Async-Logger");
        logThread.setDaemon(true);
        logThread.start();
    }

    public static void logAsync(String message) {
        logQueue.offer(message);
    }

    public static String getLocalIP() {
        try (final DatagramSocket socket = new DatagramSocket()) {
            socket.connect(InetAddress.getByName("8.8.8.8"), 10002);
            return socket.getLocalAddress().getHostAddress();
        } catch (Exception e) {
            return "127.0.0.1";
        }
    }
}
