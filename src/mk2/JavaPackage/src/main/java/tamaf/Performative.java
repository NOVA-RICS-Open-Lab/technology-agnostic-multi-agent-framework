package tamaf;

public enum Performative {
    ACCEPT_PROPOSAL("accept-proposal"),
    AGREE("agree"),
    CANCEL("cancel"),
    CFP("cfp"),
    CONFIRM("confirm"),
    DISCONFIRM("disconfirm"),
    FAILURE("failure"),
    INFORM("inform"),
    INFORM_IF("inform-if"),
    INFORM_REF("inform-ref"),
    NOT_UNDERSTOOD("not-understood"),
    PROPAGATE("propagate"),
    PROPOSE("propose"),
    PROXY("proxy"),
    QUERY_IF("query-if"),
    QUERY_REF("query-ref"),
    REFUSE("refuse"),
    REJECT_PROPOSAL("reject-proposal"),
    REQUEST("request"),
    REQUEST_WHEN("request-when"),
    REQUEST_WHENEVER("request-whenever"),
    SUBSCRIBE("subscribe"),
    KEEPALIVE("keep-alive");

    private final String value;

    Performative(String value) {
        this.value = value;
    }

    public String getValue() {
        return value;
    }

    public static Performative fromString(String text) {
        for (Performative p : Performative.values()) {
            if (p.value.equalsIgnoreCase(text)) {
                return p;
            }
        }
        return null;
    }
}
