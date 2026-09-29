from typing import TYPE_CHECKING
from .agentid import AgentID
from .servicedescription import ServiceDescription

if TYPE_CHECKING:
    pass

class AgentDescription:
    
    def __init__(self, agentid: AgentID = None):
        self.agentid = agentid
        self.services: list[ServiceDescription] = []

    def AddService(self, service: ServiceDescription):
        self.services.append(service)

    def Matches(self, template: 'AgentDescription') -> bool:
        if template.agentid is not None:
            if self.agentid is None:
                return False
            
            # Match by name if provided in template
            if template.agentid.name is not None and template.agentid.name != "" and template.agentid.name != "*" and self.agentid.name != template.agentid.name:
                return False
            
            # If template has address and name is not wildcard/empty, check its non-None parts
            if template.agentid.name != "" and template.agentid.name != "*":
                if template.agentid.address is not None:
                    if self.agentid.address is None:
                        return False
                    if template.agentid.address.ip is not None and self.agentid.address.ip != template.agentid.address.ip:
                        return False
                    if template.agentid.address.port is not None and self.agentid.address.port != template.agentid.address.port:
                        return False

        # All services in template must exist in this description
        for templateService in template.services:
            serviceFound = False
            for service in self.services:
                if service.Matches(templateService):
                    serviceFound = True
                    break
            if not serviceFound:
                return False
                
        return True
    
    def ToDict(self) -> dict:
        toReturn = {
            "services": [service.ToDict() for service in self.services]
        }
        
        if self.agentid is not None:
            toReturn["agentid"] = self.agentid.GetFullID()
    
        return toReturn
    
    @classmethod
    def FromDict(cls, data: dict) -> 'AgentDescription':
        aidText = data.get("agentid", None)
        aid = AgentID.FromString(aidText) if aidText else None
        
        agentDescription = cls(agentid=aid)

        services = data.get("services", [])
        for serviceData in services:
            service = ServiceDescription.FromDict(serviceData)
            if service:
                agentDescription.AddService(service)

        return agentDescription

    def copy(self):
        new_desc = AgentDescription(self.agentid)
        for service in self.services:
            new_desc.AddService(service)
        return new_desc
