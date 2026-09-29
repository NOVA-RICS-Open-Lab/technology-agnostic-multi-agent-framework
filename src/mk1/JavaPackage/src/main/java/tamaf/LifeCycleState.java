package tamaf;

public enum LifeCycleState {
    INITIATED(1),
    ACTIVE(2),
    SUSPENDED(3),
    CLOSING(4);

    private final int value;

    LifeCycleState(int value) {
        this.value = value;
    }

    public int getValue() {
        return value;
    }
}
