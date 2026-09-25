package tamaf;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.datatype.jsr310.JavaTimeModule;

import tamaf.defines.Defines;

public class MTS {
    private final Agent agent;
    private final List<ACLMessage> messageQueue = new ArrayList<>();
    private final Object messageQueueLock = new Object();
    private volatile ServerSocket serverSocket;
    private volatile boolean running = true;
    private volatile boolean newMessage = false;
    private volatile long serverSessionId = 0;
    private Address emaAddress;
    private Address registerAddress;
    private boolean serverOnline = false;

    private static final ObjectMapper mapper = new ObjectMapper().registerModule(new JavaTimeModule());

    public MTS(Agent agent) {
        this(agent, null, null);
    }

    public MTS(Agent agent, Address specificRegisterAddress, Address specificEMAAddress) {
        this.agent = agent;
        String localIp = Utils.getLocalIP();
        
        if (specificEMAAddress != null) {
            this.emaAddress = specificEMAAddress;
        } else {
            this.emaAddress = new Address(localIp, Defines.DEFAULT_EMA_PORT);
        }
        
        if (specificRegisterAddress != null) {
            this.registerAddress = specificRegisterAddress;
        } else {
            this.registerAddress = new Address(localIp, Defines.DEFAULT_EMA_REGISTER_PORT);
        }
    }

    public Address getEmaAddress() { return emaAddress; }
    public void setEmaAddress(Address emaAddress) { this.emaAddress = emaAddress; }
    public Address getRegisterAddress() { return registerAddress; }
    public void setRegisterAddress(Address registerAddress) { this.registerAddress = registerAddress; }
    public boolean isServerOnline() { return serverOnline; }
    public void setServerOnline(boolean serverOnline) { this.serverOnline = serverOnline; }
    
    public void setNewMessage(boolean value) { this.newMessage = value; }
    
    public boolean send(ACLMessage msg) {
        for (AgentID receiver : msg.getReceiver()) {
            String targetIp = receiver.getAddress().getIp();
            if (targetIp != null && targetIp.equals(Utils.getLocalIP())) {
                targetIp = Defines.LOCALHOST;
            }
            try (Socket socket = new Socket(targetIp, receiver.getAddress().getPort())) {
                socket.setSoTimeout(Defines.DEFAULT_MESSAGE_TIMEOUT_TIME * 1000);
                DataOutputStream out = new DataOutputStream(socket.getOutputStream());
                DataInputStream in = new DataInputStream(socket.getInputStream());
                
                TransportMessage tm = new TransportMessage(msg, agent.getAgentDescription().getAgentid().getAddress(), receiver.getAddress());
                byte[] data = mapper.writeValueAsBytes(tm.toMap());
                
                out.writeInt(data.length);
                out.write(data);
                out.flush();
                
                int respLen = in.readInt();
                byte[] respData = new byte[respLen];
                in.readFully(respData);
                String resp = new String(respData, StandardCharsets.UTF_8).trim();
                
                System.out.println("\nNEW MESSAGE SENT: " + tm.toMap());

                try {
                    if (Integer.parseInt(resp) > 299) return false;
                } catch (NumberFormatException e) {
                    if (!resp.equals("200")) return false;
                }
                
            } catch (IOException e) {
                return false;
            }
        }
        return true;
    }
    
    public ACLMessage receive(ACLMessageTemplate template) {
        synchronized (messageQueueLock) {
            for (ACLMessage msg : messageQueue) {
                if (template.matches(msg)) {
                    messageQueue.remove(msg);
                    return msg;
                }
            }
        }
        return null;
    }
    
    public void putBack(ACLMessage msg) {
        synchronized (messageQueueLock) {
            messageQueue.add(0, msg);
        }
    }
    
    public boolean hasNewMessage() {
        return newMessage;
    }
    
    public void resetNewMessage() { this.newMessage = false; }
    
    public synchronized void startServer(int port) {
        final long currentSession = ++serverSessionId;
        this.running = true;
        new Thread(() -> {
            while (running && serverSessionId == currentSession) {
                try (ServerSocket socket = new ServerSocket(port)) {
                    this.serverSocket = socket;
                    serverOnline = true;
                    System.out.println(agent.getAgentDescription().getAgentid().getFullID() + ": Agent Port Setup");
                    while (running && serverSessionId == currentSession) {
                        Socket client = socket.accept();
                        processIncoming(client);
                    }
                } catch (java.net.BindException e) {
                    serverOnline = false;
                    serverSocket = null;
                    if (running && serverSessionId == currentSession) {
                        if (port == registerAddress.getPort()) {
                            System.err.println(agent.getAgentDescription().getAgentid().getFullID() + ": Port " + port + " busy. An Agent already exists there \n ErrorCode: " + e.getMessage());
                        } else if (port == emaAddress.getPort()) {
                            System.err.println(agent.getAgentDescription().getAgentid().getFullID() + ": Port " + port + " busy. EMA already exists in this environment \n ErrorCode: " + e.getMessage());
                            agent.stop();
                        } else {
                            System.err.println(agent.getAgentDescription().getAgentid().getFullID() + ": Port " + port + " busy. Retrying... \n ErrorCode: " + e.getMessage());
                            agent.stop();
                        }
                        
                        try {
                            Thread.sleep((long) (Defines.DEFAULT_REGISTRATION_PORT_RETRY_TIME * 1000));
                        } catch (InterruptedException ie) {
                            Thread.currentThread().interrupt();
                            break;
                        }
                    }
                } catch (IOException e) {
                    if (running && serverSessionId == currentSession) {
                        System.err.println("MTS Server on port: " + port + " Error: " + e.getMessage());
                    }
                    break;
                } finally {
                    if (serverSessionId == currentSession) {
                        serverOnline = false;
                        serverSocket = null;
                    }
                }
            }
        }, "MTSServer-" + agent.getAgentDescription().getAgentid().getName()).start();
    }

    public synchronized void stopMessageServer() {
        running = false;
        if (serverSocket != null) {
            try {
                serverSocket.close();
            } catch (IOException e) {
                // ignore
            }
        }
    }

    private void processIncoming(Socket client) {
        new Thread(() -> {
            try (client) {
                DataInputStream in = new DataInputStream(client.getInputStream());
                DataOutputStream out = new DataOutputStream(client.getOutputStream());

                int len = in.readInt();
                byte[] data = new byte[len];
                in.readFully(data);

                @SuppressWarnings("unchecked")
                java.util.Map<String, Object> map = mapper.readValue(data, java.util.Map.class);
                
                System.out.println("\nNEW MESSAGE ARRIVED: " + map);
                
                TransportMessage tm = TransportMessage.fromMap(map);
                ACLMessage msg = tm.getAclMessage();

                synchronized (messageQueueLock) {
                    messageQueue.add(msg);
                    newMessage = true;
                }

                byte[] resp = "200".getBytes(StandardCharsets.UTF_8);
                out.writeInt(resp.length);
                out.write(resp);
                out.flush();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }).start();
    }
}
