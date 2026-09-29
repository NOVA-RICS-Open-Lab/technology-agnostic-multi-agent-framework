from enum import Enum

class LifeCycleState(Enum):
    INITIATED = "initiated"
    ACTIVE = "active"
    SUSPENDED = "suspended"
    WAITING = "waiting"
    DELETED = "deleted"
    TRANSIT = "transit"
    CLOSING = "closing"
