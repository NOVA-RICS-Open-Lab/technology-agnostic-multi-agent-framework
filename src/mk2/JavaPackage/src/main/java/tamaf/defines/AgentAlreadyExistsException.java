package tamaf.defines;

public class AgentAlreadyExistsException extends RuntimeException {
    public AgentAlreadyExistsException() {
        super("Agent with this name is already registered and active in EMA.");
    }
}
