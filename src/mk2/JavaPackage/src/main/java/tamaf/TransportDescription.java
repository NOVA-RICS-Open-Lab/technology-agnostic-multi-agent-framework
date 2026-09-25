package tamaf;

@com.fasterxml.jackson.annotation.JsonAutoDetect(fieldVisibility = com.fasterxml.jackson.annotation.JsonAutoDetect.Visibility.NONE, getterVisibility = com.fasterxml.jackson.annotation.JsonAutoDetect.Visibility.NONE)
public class TransportDescription {
    @com.fasterxml.jackson.annotation.JsonIgnore
    private TransportType transportType;
    @com.fasterxml.jackson.annotation.JsonIgnore
    private String transportSpecificAddress;

    public TransportDescription() {}

    public TransportDescription(String transportSpecificAddress) {
        this(transportSpecificAddress, TransportType.HTTP);
    }

    public TransportDescription(String transportSpecificAddress, TransportType transportType) {
        this.transportSpecificAddress = transportSpecificAddress;
        this.transportType = transportType;
    }

    public TransportType getTransportType() { return transportType; }
    public String getTransportSpecificAddress() { return transportSpecificAddress; }

    public java.util.Map<String, Object> toMap() {
        java.util.Map<String, Object> map = new java.util.HashMap<>();
        map.put("transport-type", transportType.getValue());
        map.put("transport-specific-address", transportSpecificAddress);
        return map;
    }

    public static TransportDescription fromMap(java.util.Map<String, Object> map) {
        if (map == null) return null;
        String typeStr = (String) map.get("transport-type");
        TransportType type = typeStr != null ? TransportType.fromValue(typeStr) : TransportType.HTTP;
        return new TransportDescription((String) map.get("transport-specific-address"), type);
    }
}
