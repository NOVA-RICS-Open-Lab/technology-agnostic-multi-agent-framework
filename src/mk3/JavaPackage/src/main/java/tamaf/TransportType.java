package tamaf;

public enum TransportType {
    HTTP("http");

    private final String value;

    TransportType(String value) {
        this.value = value;
    }

    public String getValue() {
        return value;
    }

    public static TransportType fromValue(String text) {
        for (TransportType t : TransportType.values()) {
            if (t.value.equalsIgnoreCase(text)) {
                return t;
            }
        }
        return HTTP;
    }
}
