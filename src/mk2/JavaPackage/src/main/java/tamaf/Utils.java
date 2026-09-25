package tamaf;

import java.net.DatagramSocket;
import java.net.InetAddress;

public class Utils {
    public static String getLocalIP() {
        try (final DatagramSocket socket = new DatagramSocket()) {
            socket.connect(InetAddress.getByName("8.8.8.8"), 10002);
            return socket.getLocalAddress().getHostAddress();
        } catch (Exception e) {
            return "127.0.0.1";
        }
    }
}
