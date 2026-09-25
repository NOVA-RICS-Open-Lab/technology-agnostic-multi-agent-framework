package tamaf;

import java.util.HashSet;
import java.util.Set;

public class ACLMessageTemplate {
    private AgentID sender;
    private Set<Performative> performatives;
    private String conversationId;
    private String protocol;
    private String ontology;

    public ACLMessageTemplate() {
        this.performatives = new HashSet<>();
    }

    public ACLMessageTemplate(AgentID sender, Set<Performative> performatives, String conversationId, String protocol, String ontology) {
        this.sender = sender;

        if (performatives == null) {
            this.performatives = new HashSet<>();
        } else {
            this.performatives = performatives;
        }

        this.conversationId = conversationId;
        this.protocol = protocol;
        this.ontology = ontology;
    }

    public boolean matches(ACLMessage msg) {
        if (sender != null) {
            if (msg.getSender() == null || !AgentID.matches(sender, msg.getSender())) {
                
                System.out.println("Sender mismatch: expected " + sender.getFullID() + ", got " + (msg.getSender() != null ? msg.getSender().getFullID() : "null"));
                
                return false;
            }
        }
        
        if (performatives != null && !performatives.isEmpty()) {
            if (!performatives.contains(msg.getPerformative())) {
                
                System.out.println("Performative mismatch: expected one of " + performatives + ", got " + msg.getPerformative());
                
                return false;
            }
        }
        
        if (conversationId != null && !conversationId.equals(msg.getConversationId())) {
            
            System.out.println("Conversation ID mismatch: expected " + conversationId + ", got " + msg.getConversationId());
            
            return false;

        }
        
        if (protocol != null && !protocol.equals(msg.getProtocol())) {

            System.out.println("Protocol mismatch: expected " + protocol + ", got " + msg.getProtocol());

            return false;
        }
        
        if (ontology != null && !ontology.equals(msg.getOntology())) {
            
            System.out.println("Ontology mismatch: expected " + ontology + ", got " + msg.getOntology());
            
            return false;
        }
        
        System.out.println("Message matches template");

        return true;
    }

    // Getters and Setters
    public AgentID getSender() { return sender; }
    public void setSender(AgentID sender) { this.sender = sender; }
    public Set<Performative> getPerformatives() { return performatives; }
    public void setPerformatives(Set<Performative> performatives) { this.performatives = performatives; }
    public void addPerformative(Performative performative) { this.performatives.add(performative); }
    public String getConversationId() { return conversationId; }
    public void setConversationId(String conversationId) { this.conversationId = conversationId; }
    public String getProtocol() { return protocol; }
    public void setProtocol(String protocol) { this.protocol = protocol; }
    public String getOntology() { return ontology; }
    public void setOntology(String ontology) { this.ontology = ontology; }
}
