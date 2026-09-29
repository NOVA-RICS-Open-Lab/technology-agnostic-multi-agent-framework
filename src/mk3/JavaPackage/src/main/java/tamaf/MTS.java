package tamaf;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousServerSocketChannel;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.datatype.jsr310.JavaTimeModule;

import tamaf.defines.Defines;

public class MTS {
    private final Agent agent;
    private final List<ACLMessage> messageQueue = new ArrayList<>();
    private final Object messageQueueLock = new Object();
    private volatile AsynchronousServerSocketChannel serverSocket;
    private volatile boolean running = true;
    private volatile boolean newMessage = false;
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
    
    private static void writeFully(AsynchronousSocketChannel channel, ByteBuffer buf, Runnable onComplete, Runnable onError) {
        channel.write(buf, null, new CompletionHandler<Integer, Void>() {
            @Override
            public void completed(Integer result, Void attachment) {
                if (result < 0) {
                    onError.run();
                    return;
                }
                if (buf.hasRemaining()) {
                    channel.write(buf, null, this);
                } else {
                    onComplete.run();
                }
            }

            @Override
            public void failed(Throwable exc, Void attachment) {
                onError.run();
            }
        });
    }

    private static void readFully(AsynchronousSocketChannel channel, ByteBuffer buf, Runnable onComplete, Runnable onError) {
        channel.read(buf, null, new CompletionHandler<Integer, Void>() {
            @Override
            public void completed(Integer result, Void attachment) {
                if (result < 0) {
                    onError.run();
                    return;
                }
                if (buf.hasRemaining()) {
                    channel.read(buf, null, this);
                } else {
                    onComplete.run();
                }
            }

            @Override
            public void failed(Throwable exc, Void attachment) {
                onError.run();
            }
        });
    }

    public boolean send(ACLMessage msg) {
        if (msg.getSender() == null) {
            msg.setSender(agent.getAgentDescription().getAgentid());
        }
        for (AgentID receiver : msg.getReceiver()) {
            String targetIp = receiver.getAddress().getIp();
            if (targetIp != null && targetIp.equals(Utils.getLocalIP())) {
                targetIp = Defines.LOCALHOST;
            }
            
            final String finalTargetIp = targetIp;
            try {
                AsynchronousSocketChannel client = AsynchronousSocketChannel.open();
                client.connect(new InetSocketAddress(finalTargetIp, receiver.getAddress().getPort()), null, new CompletionHandler<Void, Void>() {
                    @Override
                    public void completed(Void result, Void attachment) {
                        try {
                            TransportMessage tm = new TransportMessage(msg, agent.getAgentDescription().getAgentid().getAddress(), receiver.getAddress());
                            byte[] data = mapper.writeValueAsBytes(tm.toMap());
                            
                            ByteBuffer buf = ByteBuffer.allocate(4 + data.length);
                            buf.putInt(data.length);
                            buf.put(data);
                            buf.flip();
                            
                            writeFully(client, buf, () -> {
                                agent.logDebug(1, "Sent message to " + (receiver.getName() != null ? receiver.getName() : "target") + " (Performative: " + msg.getPerformative() + ")");
                                agent.logDebug(2, "NEW MESSAGE SENT: " + tm.toMap());
                                
                                ByteBuffer respSizeBuf = ByteBuffer.allocate(4);
                                readFully(client, respSizeBuf, () -> {
                                    respSizeBuf.flip();
                                    int respLen = respSizeBuf.getInt();
                                    ByteBuffer respDataBuf = ByteBuffer.allocate(respLen);
                                    readFully(client, respDataBuf, () -> {
                                        try { client.close(); } catch (IOException e) {}
                                    }, () -> {
                                        try { client.close(); } catch (IOException e) {}
                                    });
                                }, () -> {
                                    try { client.close(); } catch (IOException e) {}
                                });
                            }, () -> {
                                try { client.close(); } catch (IOException e) {}
                            });
                        } catch (Exception e) {
                            try { client.close(); } catch (IOException ex) {}
                        }
                    }

                    @Override
                    public void failed(Throwable exc, Void attachment) {
                        try { client.close(); } catch (IOException e) {}
                    }
                });
            } catch (IOException e) {
                // Ignore
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
        this.running = true;
        
        try {
            serverSocket = AsynchronousServerSocketChannel.open();
            serverSocket.bind(new InetSocketAddress(port), 500);
            serverOnline = true;
            agent.logDebug(1, "Agent active at: " + agent.getAgentDescription().getAgentid().getAddress().getString());
            
            serverSocket.accept(null, new CompletionHandler<AsynchronousSocketChannel, Void>() {
                @Override
                public void completed(AsynchronousSocketChannel client, Void attachment) {
                    if (running && serverSocket != null && serverSocket.isOpen()) {
                        serverSocket.accept(null, this);
                        processIncomingAsync(client);
                    }
                }

                @Override
                public void failed(Throwable exc, Void attachment) {
                    if (exc instanceof java.nio.channels.AsynchronousCloseException || 
                        exc instanceof java.nio.channels.ClosedChannelException || 
                        !running) {
                        return;
                    }
                    serverOnline = false;
                    if (port == registerAddress.getPort()) {
                        System.err.println(agent.getAgentDescription().getAgentid().getFullID() + ": Port " + port + " busy. An Agent already exists there");
                    } else if (port == emaAddress.getPort()) {
                        System.err.println(agent.getAgentDescription().getAgentid().getFullID() + ": Port " + port + " busy. EMA already exists in this environment");
                        agent.stop();
                    } else {
                        System.err.println(agent.getAgentDescription().getAgentid().getFullID() + ": Port " + port + " busy. Retrying...");
                        agent.stop();
                    }
                }
            });
        } catch (java.net.BindException e) {
            serverOnline = false;
            if (port == registerAddress.getPort()) {
                System.err.println(agent.getAgentDescription().getAgentid().getFullID() + ": Port " + port + " busy. An Agent already exists there");
            } else if (port == emaAddress.getPort()) {
                System.err.println(agent.getAgentDescription().getAgentid().getFullID() + ": Port " + port + " busy. EMA already exists in this environment");
                agent.stop();
            } else {
                System.err.println(agent.getAgentDescription().getAgentid().getFullID() + ": Port " + port + " busy. Retrying...");
                agent.stop();
            }
        } catch (IOException e) {
            serverOnline = false;
        }
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

    private void processIncomingAsync(AsynchronousSocketChannel client) {
        ByteBuffer sizeBuf = ByteBuffer.allocate(4);
        readFully(client, sizeBuf, () -> {
            sizeBuf.flip();
            int len = sizeBuf.getInt();
            
            ByteBuffer dataBuf = ByteBuffer.allocate(len);
            readFully(client, dataBuf, () -> {
                try {
                    dataBuf.flip();
                    byte[] data = new byte[len];
                    dataBuf.get(data);
                    
                    @SuppressWarnings("unchecked")
                    Map<String, Object> map = mapper.readValue(data, Map.class);
                    
                    agent.logDebug(2, "NEW MESSAGE ARRIVED: " + map);
                    
                    TransportMessage tm = TransportMessage.fromMap(map);
                    ACLMessage msg = tm.getAclMessage();
                    agent.logDebug(1, "Received message from " + (msg.getSender() != null ? msg.getSender().getName() : "unknown") + " (Performative: " + msg.getPerformative() + ")");

                    synchronized (messageQueueLock) {
                        messageQueue.add(msg);
                        newMessage = true;
                    }
                    if (agent.ams != null) {
                        agent.ams.wakeUp();
                    }

                    byte[] resp = "200".getBytes(StandardCharsets.UTF_8);
                    ByteBuffer respBuf = ByteBuffer.allocate(4 + resp.length);
                    respBuf.putInt(resp.length);
                    respBuf.put(resp);
                    respBuf.flip();
                    
                    writeFully(client, respBuf, () -> {
                        try { client.close(); } catch (IOException e) {}
                    }, () -> {
                        try { client.close(); } catch (IOException e) {}
                    });
                } catch (Exception e) {
                    try { client.close(); } catch (IOException ex) {}
                }
            }, () -> {
                try { client.close(); } catch (IOException e) {}
            });
        }, () -> {
            try { client.close(); } catch (IOException e) {}
        });
    }
}
