package tamaf;

@com.fasterxml.jackson.annotation.JsonAutoDetect(fieldVisibility = com.fasterxml.jackson.annotation.JsonAutoDetect.Visibility.NONE, getterVisibility = com.fasterxml.jackson.annotation.JsonAutoDetect.Visibility.NONE)
public class Envelope {
    @com.fasterxml.jackson.annotation.JsonIgnore
    private TransportDescription senderTransportDescription;
    @com.fasterxml.jackson.annotation.JsonIgnore
    private TransportDescription receiverTransportDescription;

    public Envelope() {}

    public Envelope(TransportDescription sender, TransportDescription receiver) {
        this.senderTransportDescription = sender;
        this.receiverTransportDescription = receiver;
    }

    public TransportDescription getSenderTransportDescription() { return senderTransportDescription; }
    public TransportDescription getReceiverTransportDescription() { return receiverTransportDescription; }

    public java.util.Map<String, Object> toMap() {
        java.util.Map<String, Object> map = new java.util.HashMap<>();
        if (senderTransportDescription != null) map.put("sender-transport-description", senderTransportDescription.toMap());
        if (receiverTransportDescription != null) map.put("receiver-transport-description", receiverTransportDescription.toMap());
        return map;
    }

    @SuppressWarnings("unchecked")
    public static Envelope fromMap(java.util.Map<String, Object> map) {
        if (map == null) return null;
        TransportDescription sender = TransportDescription.fromMap((java.util.Map<String, Object>) map.get("sender-transport-description"));
        TransportDescription receiver = TransportDescription.fromMap((java.util.Map<String, Object>) map.get("receiver-transport-description"));
        return new Envelope(sender, receiver);
    }
}
