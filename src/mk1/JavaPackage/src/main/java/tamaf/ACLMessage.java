package tamaf;

import java.time.OffsetDateTime;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

@com.fasterxml.jackson.annotation.JsonAutoDetect(fieldVisibility = com.fasterxml.jackson.annotation.JsonAutoDetect.Visibility.NONE, getterVisibility = com.fasterxml.jackson.annotation.JsonAutoDetect.Visibility.NONE)
public class ACLMessage {
    @com.fasterxml.jackson.annotation.JsonIgnore
    private Performative performative;
    @com.fasterxml.jackson.annotation.JsonIgnore
    private AgentID sender;
    @com.fasterxml.jackson.annotation.JsonIgnore
    private List<AgentID> receiver = new ArrayList<>();
    @com.fasterxml.jackson.annotation.JsonIgnore
    private Object content;
    @com.fasterxml.jackson.annotation.JsonIgnore
    private String language;
    @com.fasterxml.jackson.annotation.JsonIgnore
    private String ontology;
    @com.fasterxml.jackson.annotation.JsonIgnore
    private String protocol;
    @com.fasterxml.jackson.annotation.JsonIgnore
    private String conversationId;
    @com.fasterxml.jackson.annotation.JsonIgnore
    private List<AgentID> replyTo;
    @com.fasterxml.jackson.annotation.JsonIgnore
    private java.time.OffsetDateTime replyBy;
    @com.fasterxml.jackson.annotation.JsonIgnore
    private String replyWith;
    @com.fasterxml.jackson.annotation.JsonIgnore
    private String inReplyTo;
    @com.fasterxml.jackson.annotation.JsonIgnore
    private String encoding;

    public ACLMessage() {}

    public ACLMessage(Performative performative) {
        this.performative = performative;
    }

    public ACLMessage createReply(Performative performative) {
        ACLMessage reply = new ACLMessage(performative);
        if (this.replyTo != null && !this.replyTo.isEmpty()) {
            reply.setReceiver(new ArrayList<>(this.replyTo));
        } else if (this.sender != null) {
            List<AgentID> r = new ArrayList<>();
            r.add(this.sender);
            reply.setReceiver(r);
        }
        reply.setLanguage(this.language);
        reply.setOntology(this.ontology);
        reply.setProtocol(this.protocol);
        reply.setConversationId(this.conversationId);
        if (this.replyWith != null) {
            reply.setInReplyTo(this.replyWith);
        }
        return reply;
    }

    public ACLMessage createReply() {
        return createReply(null);
    }

    public Map<String, Object> toMap() {
        Map<String, Object> payload = new HashMap<>();
        if (performative != null) payload.put("performative", performative.getValue());
        if (sender != null) payload.put("sender", sender.getFullID());
        if (receiver != null && !receiver.isEmpty()) {
            List<String> receiverIds = new ArrayList<>();
            for (AgentID r : receiver) receiverIds.add(r.getFullID());
            payload.put("receiver", receiverIds);
        }
        if (replyTo != null && !replyTo.isEmpty()) {
            List<String> replyToIds = new ArrayList<>();
            for (AgentID rt : replyTo) replyToIds.add(rt.getFullID());
            payload.put("reply-to", replyToIds);
        }
        if (replyBy != null) payload.put("reply-by", replyBy.toString());
        if (protocol != null) payload.put("protocol", protocol);
        if (content != null) payload.put("content", content);
        if (ontology != null) payload.put("ontology", ontology);
        if (language != null) payload.put("language", language);
        if (conversationId != null) payload.put("conversation-id", conversationId);
        if (replyWith != null) payload.put("reply-with", replyWith);
        if (inReplyTo != null) payload.put("in-reply-to", inReplyTo);
        if (encoding != null) payload.put("encoding", encoding);

        return payload;
    }

    public static ACLMessage fromMap(Map<String, Object> data) {
        ACLMessage message = new ACLMessage();
        String perfStr = (String) data.get("performative");
        if (perfStr != null) message.setPerformative(Performative.fromString(perfStr));

        String senderStr = (String) data.get("sender");
        if (senderStr != null) message.setSender(AgentID.fromString(senderStr));

        List<String> receiverStrs = (List<String>) data.get("receiver");
        if (receiverStrs != null) {
            List<AgentID> receivers = new ArrayList<>();
            for (String r : receiverStrs) receivers.add(AgentID.fromString(r));
            message.setReceiver(receivers);
        }

        List<String> replyToStrs = (List<String>) data.get("reply-to");
        if (replyToStrs != null) {
            List<AgentID> replyTos = new ArrayList<>();
            for (String r : replyToStrs) replyTos.add(AgentID.fromString(r));
            message.setReplyTo(replyTos);
        }

        String replyByStr = (String) data.get("reply-by");
        if (replyByStr != null) message.setReplyBy(OffsetDateTime.parse(replyByStr));

        message.setProtocol((String) data.get("protocol"));
        message.setContent(data.get("content"));
        message.setOntology((String) data.get("ontology"));
        message.setLanguage((String) data.get("language"));
        message.setConversationId((String) data.get("conversation-id"));
        message.setReplyWith((String) data.get("reply-with"));
        message.setInReplyTo((String) data.get("in-reply-to"));
        message.setEncoding((String) data.get("encoding"));

        return message;
    }

    public Performative getPerformative() { return performative; }
    public void setPerformative(Performative performative) { this.performative = performative; }
    public AgentID getSender() { return sender; }
    public void setSender(AgentID sender) { this.sender = sender; }
    public List<AgentID> getReceiver() { return receiver; }
    public void setReceiver(List<AgentID> receiver) { this.receiver = receiver; }
    public void addReceiver(AgentID receiver) { this.receiver.add(receiver); }
    public Object getContent() { return content; }
    public void setContent(Object content) { this.content = content; }
    public String getLanguage() { return language; }
    public void setLanguage(String language) { this.language = language; }
    public String getOntology() { return ontology; }
    public void setOntology(String ontology) { this.ontology = ontology; }
    public String getProtocol() { return protocol; }
    public void setProtocol(String protocol) { this.protocol = protocol; }
    public String getConversationId() { return conversationId; }
    public void setConversationId(String conversationId) { this.conversationId = conversationId; }
    public List<AgentID> getReplyTo() { return replyTo; }
    public void setReplyTo(List<AgentID> replyTo) { this.replyTo = replyTo; }
    public OffsetDateTime getReplyBy() { return replyBy; }
    public void setReplyBy(OffsetDateTime replyBy) { this.replyBy = replyBy; }
    public String getReplyWith() { return replyWith; }
    public void setReplyWith(String replyWith) { this.replyWith = replyWith; }
    public String getInReplyTo() { return inReplyTo; }
    public void setInReplyTo(String inReplyTo) { this.inReplyTo = inReplyTo; }
    public String getEncoding() { return encoding; }
    public void setEncoding(String encoding) { this.encoding = encoding; }
}
