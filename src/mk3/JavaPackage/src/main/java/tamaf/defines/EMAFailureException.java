package tamaf.defines;

public class EMAFailureException extends RuntimeException {
    public EMAFailureException() {
        super("EMA is not responding.");
    }

    public EMAFailureException(String message) {
        super(message);
    }
}
