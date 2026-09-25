import os
import platform
from enum import Enum

# Service Types
DF_SERVICE_TYPE_SKILL = "SKILL_TYPE"
DF_SERVICE_TYPE_TRANSPORT = "TRANSPORT_TYPE"
DF_SERVICE_TYPE_RESOURCE = "RESOURCE_TYPE"
DF_SERVICE_NAME_TRANSPORT = "TRANSPORT_NAME"

# Ontologies
ONTOLOGY_NEGOTIATE_NEXT_RESOURCE = "CNET_NEXT_RESOURCE"
ONTOLOGY_REQUEST_TRANSPORT = "REQ_TRANSPORT"
ONTOLOGY_REQUEST_EXECUTE_SKILL = "REQ_EXECUTE_SKILL"
ONTOLOGY_REQUEST_UNPLUG_RESOURCE = "REQ_UNPLUG_RESOURCE"
ONTOLOGY_REQUEST_UPDATE_CONFIG = "REQ_UPDATE_CONFIG"

# IPs
CONTROLLER_IP = ""


def get_host_tag() -> str:
    if os.path.exists('/proc/device-tree/model'):
        try:
            with open('/proc/device-tree/model', 'r') as f:
                if 'raspberry pi' in f.read().lower():
                    return "RPi"
        except Exception:
            pass
    return "PC"


class ResourceType(Enum):
    Robot = 1
    Station = 2
    Human = 3

class Locations(Enum):
    A = 1
    B = 2
    C = 3
    D = 4
    E = 5
    F = 6

SKILLS = [
    ["Drill", DF_SERVICE_TYPE_RESOURCE, f"http://{CONTROLLER_IP}/estacao?skill=0"],
    ["Screw", DF_SERVICE_TYPE_RESOURCE, f"http://{CONTROLLER_IP}/estacao?skill=1"],
    [DF_SERVICE_NAME_TRANSPORT, DF_SERVICE_TYPE_TRANSPORT, f"http://{CONTROLLER_IP}/passadeiras?skill=AB"],
    ["GoToAB", DF_SERVICE_TYPE_TRANSPORT, f"http://{CONTROLLER_IP}/passadeiras?skill=AB"],
    ["GoToBC", DF_SERVICE_TYPE_TRANSPORT, f"http://{CONTROLLER_IP}/passadeiras?skill=BC"],
    ["GoToCD", DF_SERVICE_TYPE_TRANSPORT, f"http://{CONTROLLER_IP}/passadeiras?skill=CD"],
    ["GoToDE", DF_SERVICE_TYPE_TRANSPORT, f"http://{CONTROLLER_IP}/passadeiras?skill=DE"],
    ["GoToEF", DF_SERVICE_TYPE_TRANSPORT, f"http://{CONTROLLER_IP}/passadeiras?skill=EF"],
    ["GoToFA", DF_SERVICE_TYPE_TRANSPORT, f"http://{CONTROLLER_IP}/passadeiras?skill=FA"],
]
