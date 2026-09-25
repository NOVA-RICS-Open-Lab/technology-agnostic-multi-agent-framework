package tamaf.defines;

public class EMANoMorePortsException extends RuntimeException {
    public EMANoMorePortsException() {
        super("No more ports available in EMA.");
    }
}
