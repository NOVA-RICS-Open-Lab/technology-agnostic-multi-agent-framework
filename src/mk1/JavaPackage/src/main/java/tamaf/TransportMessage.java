package tamaf;

import com.fasterxml.jackson.annotation.JsonAutoDetect;
import com.fasterxml.jackson.annotation.JsonIgnore;

@JsonAutoDetect(fieldVisibility = JsonAutoDetect.Visibility.NONE, getterVisibility = JsonAutoDetect.Visibility.NONE)
public class TransportMessage {
    @JsonIgnore
    private ACLMessage aclMessage;
    @JsonIgnore
    private Envelope envelope;

    public TransportMessage() {}

    public TransportMessage(ACLMessage aclMessage, Address senderAddress, Address receiverAddress) {
        this.aclMessage = aclMessage;
        if (senderAddress != null && receiverAddress != null) {
            this.envelope = new Envelope(
                new TransportDescription(senderAddress.getHttpLink()),
                new TransportDescription(receiverAddress.getHttpLink())
            );
        }
    }

    public ACLMessage getAclMessage() { return aclMessage; }
    public Envelope getEnvelope() { return envelope; }
    public void setEnvelope(Envelope envelope) { this.envelope = envelope; }

    public java.util.Map<String, Object> toMap() {
        java.util.Map<String, Object> map = new java.util.HashMap<>();
        if (aclMessage != null) map.put("aclmessage", aclMessage.toMap());
        if (envelope != null) map.put("envelope", envelope.toMap());
        return map;
    }

    @SuppressWarnings("unchecked")
    public static TransportMessage fromMap(java.util.Map<String, Object> map) {
        if (map == null) return null;
        ACLMessage acl = ACLMessage.fromMap((java.util.Map<String, Object>) map.get("aclmessage"));
        Envelope env = Envelope.fromMap((java.util.Map<String, Object>) map.get("envelope"));
        TransportMessage tm = new TransportMessage();
        tm.aclMessage = acl;
        tm.envelope = env;
        return tm;
    }
}
