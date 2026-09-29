package tamaf;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

import tamaf.defines.AgentAlreadyExistsException;
import tamaf.defines.EMANoMorePortsException;
import tamaf.defines.Defines;
import tamaf.defines.EMAFailureException;

public class AMS {
    private final Agent agent;
    private final List<Behavior> activeBehaviors = new CopyOnWriteArrayList<>();
    private final List<Behavior> pendingAdditions = new ArrayList<>();
    private final List<Behavior> pendingRemovals = new ArrayList<>();
    private final List<Behavior> blockedBehaviors = new ArrayList<>();
    
    private LifeCycleState lifeCycleState = LifeCycleState.INITIATED;
    private final Object stateLock = new Object();
    private boolean running = true;
    private final Thread kernelThread;

    public AMS(Agent agent) {
        this.agent = agent;
        this.kernelThread = new Thread(this::kernel, "AgentKernel-" + agent.getAgentDescription().getAgentid().getName());
        this.kernelThread.setDaemon(true);
    }

    public void start() {
        kernelThread.start();
    }

    public void addBehavior(Behavior behavior) {
        synchronized (pendingAdditions) {
            pendingAdditions.add(behavior);
        }
    }

    public void removeBehavior(Behavior behavior) {
        synchronized (pendingRemovals) {
            pendingRemovals.add(behavior);
        }
    }

    public void unblockBehaviors() {
        synchronized (blockedBehaviors) {
            for (Behavior behavior : blockedBehaviors) {
                behavior.unblock();
            }
            activeBehaviors.addAll(blockedBehaviors);
            blockedBehaviors.clear();
        }
    }

    public void blockBehavior(Behavior behavior) {
        activeBehaviors.remove(behavior);
        synchronized (blockedBehaviors) {
            blockedBehaviors.add(behavior);
        }
    }

    public boolean send(ACLMessage msg) {
        if (msg.getSender() == null) {
            msg.setSender(this.agent.agentDescription.getAgentid());
        }
        return this.agent.mts.send(msg);
    }

    public ACLMessage receive(ACLMessageTemplate template) {
        return this.agent.mts.receive(template);
    }

    public void putBack(ACLMessage msg) {
        this.agent.mts.putBack(msg);
    }

    private void kernel() {
        InitializationBehavior initializationBehavior = new InitializationBehavior(agent);
        ClosingBehavior closingBehavior = new ClosingBehavior(agent);

        while (running) {
            try {
                LifeCycleState currentState;
                synchronized (stateLock) {
                    currentState = lifeCycleState;
                }
                
                switch (currentState) {
                    case SUSPENDED -> {
                        for (Behavior behavior : activeBehaviors) {
                            if (behavior instanceof HeartBeatBehavior) {
                                if (!behavior.isStarted()) {
                                    behavior.onStart();
                                    behavior.setStarted(true);
                                }
                                behavior.execute();
                            }
                        }
                        Thread.sleep((long) (Defines.DEFAULT_KERNEL_STABILIZATION_TIME * 1000));
                        continue;
                    }
                    case INITIATED -> {
                        if (agent.mts.hasNewMessage()) {
                            if (initializationBehavior.isBlocked()) {
                                initializationBehavior.unblock();
                            }
                            agent.mts.resetNewMessage();
                        }       
                        
                        if (!initializationBehavior.isStarted()) {
                            initializationBehavior.onStart();
                            initializationBehavior.setStarted(true);
                        }
                        
                        initializationBehavior.execute();
                        
                        if (initializationBehavior.isBlocked()) {
                            Thread.sleep((long) (Defines.DEFAULT_KERNEL_STABILIZATION_TIME * 1000));
                            continue;
                        }
                        
                        if (initializationBehavior.done()) {
                            initializationBehavior.onEnd();
                            synchronized (stateLock) {
                                lifeCycleState = LifeCycleState.ACTIVE;
                            }
                        }
                    }
                    case ACTIVE -> {
                        if (agent.mts.hasNewMessage()) {
                            unblockBehaviors();
                            agent.mts.resetNewMessage();
                        }       
                        
                        synchronized (pendingRemovals) {
                            activeBehaviors.removeAll(pendingRemovals);
                            pendingRemovals.clear();
                        }
                        synchronized (pendingAdditions) {
                            activeBehaviors.addAll(pendingAdditions);
                            pendingAdditions.clear();
                        }
                        
                        for (Behavior behavior : activeBehaviors) {
                            synchronized (stateLock) {
                                if (lifeCycleState != LifeCycleState.ACTIVE) break;
                            }
                            
                            if (!behavior.isStarted()) {
                                behavior.onStart();
                                behavior.setStarted(true);
                            }
                            
                            behavior.execute();
                            
                            if (behavior.isBlocked()) {
                                blockBehavior(behavior);
                                continue;
                            }
                            
                            if (behavior.done()) {
                                behavior.onEnd();
                                activeBehaviors.remove(behavior);
                            }
                        }
                    }
                    case CLOSING -> {
                        if (agent.mts.hasNewMessage()) {
                            if (closingBehavior.isBlocked()) {
                                closingBehavior.unblock();
                            }
                            agent.mts.resetNewMessage();
                        }       
                        
                        if (!closingBehavior.isStarted()) {
                            closingBehavior.onStart();
                            closingBehavior.setStarted(true);
                        }
                        
                        closingBehavior.execute();
                        
                        if (closingBehavior.isBlocked()) {
                            Thread.sleep((long) (Defines.DEFAULT_KERNEL_STABILIZATION_TIME * 1000));
                            continue;
                        }
                        
                        if (closingBehavior.done()) {
                            closingBehavior.onEnd();
                            agent.mts.stopMessageServer();
                            System.out.println(agent.getAgentDescription().getAgentid().getFullID() + ": Agent Taken Down");
                            running = false;
                            break;
                        }
                    }
                }
                Thread.sleep((long) (Defines.DEFAULT_KERNEL_STABILIZATION_TIME * 1000));
            } catch (AgentAlreadyExistsException | EMANoMorePortsException | EMAFailureException e) {
                System.err.println(agent.getAgentDescription().getAgentid().getFullID() + ": Agent Exception: " + e.getMessage());
                closingBehavior.setDeRegister(false);
                synchronized (stateLock) {
                    lifeCycleState = LifeCycleState.CLOSING;
                }
            } catch (Exception e) {
                System.err.println(agent.getAgentDescription().getAgentid().getFullID() + ": Agent Exception: " + e.getMessage());
                synchronized (stateLock) {
                    lifeCycleState = LifeCycleState.CLOSING;
                }
            }
        }
    }

    private class InitializationBehavior extends SimpleBehavior {
        private boolean isRegistered = false;
        private boolean initializationDone = false;
        private boolean ServerStarted = false;

        public InitializationBehavior(Agent agent) { super(agent); }

        @Override
        public void action() {
            if (!isRegistered) {
                if (!agent.mts.isServerOnline()) {

                    if (ServerStarted) {
                        try {
                            Thread.sleep((long) (Defines.DEFAULT_LOOP_STABILIZATION_TIME * 1000));
                        } catch (InterruptedException e) {
                            return;
                        }
                    }

                    if (Defines.DEFAULT_EMA_NAME.equals(agent.getAgentDescription().getAgentid().getName())) {
                        int emaPort = agent.mts.getEmaAddress().getPort();
                        agent.getAgentDescription().getAgentid().setPort(emaPort);
                        agent.mts.startServer(emaPort);

                        this.ServerStarted = true;
                        
                        if (!agent.mts.isServerOnline()) {
                            return;
                        }
                        agent.setup(emaInteraction);
                        return;
                    }
                    
                    int regPort = agent.mts.getRegisterAddress().getPort();
                    agent.getAgentDescription().getAgentid().setPort(regPort);
                    agent.mts.startServer(regPort);
                    this.ServerStarted = true;
                    if (!agent.mts.isServerOnline()) {
                        return;
                    }
                }

                ACLMessage agree = emaInteraction.registerAgent();
                if (agree != null) {

                    agent.mts.stopMessageServer();
                    @SuppressWarnings("unchecked")
                    java.util.Map<String, Object> contentMap = (java.util.Map<String, Object>) agree.getContent();
                    String concept = AgentManagementOntology.getConcept(contentMap);
                    
                    if (Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_INFORM_CONCEPT.equals(concept)) {
                        int inform = (int) AgentManagementOntology.getAttributes(contentMap);
                        if (inform == Defines.DEFAULT_AGENT_ALREADY_EXISTS) {
                            throw new AgentAlreadyExistsException();
                        }
                        if (inform == Defines.DEFAULT_EMA_HAS_NO_MORE_PORTS) {
                            throw new EMANoMorePortsException();
                        }
                        if (inform == Defines.DEFAULT_OUTSIDE_AGENT) {
                            isRegistered = true;
                        }
                    } else if (Defines.DEFAULT_AGENTMANAGEMENT_ONTOLOGY_AGENTADDRESS_CONCEPT.equals(concept)) {
                        Address newAgentAddress = AgentManagementOntology.getAgentAddressFromAttributes(contentMap);
                        if (newAgentAddress != null) {
                            agent.getAgentDescription().getAgentid().setPort(newAgentAddress.getPort());
                            agent.mts.setServerOnline(false);
                            agent.mts.startServer(newAgentAddress.getPort());
                            
                            isRegistered = true;
                        }
                    } else {
                        System.out.println(agent.getAgentDescription().getAgentid().getFullID() + ": Unknown concept from EMA registration response");
                        return;
                    }
                } else {
                    System.out.println(agent.getAgentDescription().getAgentid().getFullID() + ": Failed to register with EMA");
                    return;
                }
            }
            agent.addBehavior(new HeartBeatBehavior(agent));
            agent.setup(emaInteraction);
            initializationDone = true;
        }

        @Override
        public boolean done() {
            return initializationDone;
        }
    }

    private class HeartBeatBehavior extends TickerBehavior {
        public HeartBeatBehavior(Agent agent) { super(agent, Defines.DEFAULT_EMA_HEARTBEAT_INTERVAL); }

        @Override
        public void onTick() {
            emaInteraction.keepAliveAgent();
        }
    }

    private class ClosingBehavior extends OneShotBehavior {
        private boolean deRegister = true;
        public ClosingBehavior(Agent agent) { super(agent); }

        public void setDeRegister(boolean deRegister) { this.deRegister = deRegister; }

        @Override
        public void action() {
            agent.takeDown(emaInteraction);
            if (!Defines.DEFAULT_EMA_NAME.equals(agent.getAgentDescription().getAgentid().getName())) {
                if (deRegister) {
                    try {
                        emaInteraction.deRegisterAgent();
                    } catch (EMAFailureException e) {
                        // Ignore failure on shutdown
                    }
                }
            }
        }
    }

    public void shutdown() {
        synchronized (stateLock) {
            lifeCycleState = LifeCycleState.CLOSING;
        }
    }
    
    public void suspend() {
        synchronized (stateLock) {
            lifeCycleState = LifeCycleState.SUSPENDED;
        }
    }
    
    public void resume() {
        synchronized (stateLock) {
            if (lifeCycleState == LifeCycleState.SUSPENDED) {
                lifeCycleState = LifeCycleState.ACTIVE;
            }
        }
    }

    public boolean isAlive() {
        return kernelThread.isAlive();
    }
}
