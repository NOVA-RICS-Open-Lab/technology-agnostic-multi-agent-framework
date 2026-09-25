from .address import Address

class AgentID:
    
    def __init__(self, name: str, address: Address = None):
        self.name = name
        self.address = address

    def SetPort(self, port: int):
        if self.address:
            self.address.SetPort(port)

    def GetFullID(self) -> str:
        if self.address:
            return f"{self.name}@{self.address.ip}:{self.address.port}"
        return f"{self.name}@None:None"
    
    def GetIDNoPort(self) -> str:
        if self.address:
            return f"{self.name}@{self.address.ip}"
        return f"{self.name}@None"

    @classmethod
    def FromString(cls, aidString: str):
        name, networkPart = aidString.split('@', 1)
        ip, portString = networkPart.rsplit(':', 1)

        if name == "None":
            name = None

        if ip == "None":
            return cls(name=name, address=None)

        if portString != "None":
            port = int(portString)
            return cls(name=name, address=Address(ip=ip, port=port))
        else:
            return cls(name=name, address=Address(ip=ip))
            
    @staticmethod
    def Matches(template: 'AgentID', target: 'AgentID'):
        if target is None:
            return False
            
        # If template has a name, it must match.
        if template.name is not None:
            if template.name == target.name:
                # Name matches. If address is also provided in template, we check it.
                if template.address is None:
                    return True
                
                # If template HAS address, we check it.
                if target.address is None:
                    return False
                
                if template.address.ip is not None and template.address.ip != target.address.ip:
                    return False
                
                if template.address.port is not None and template.address.port != target.address.port:
                    return False
                
                return True
            else:
                return False
        
        # If name is None in template, we must match by address
        if template.address is not None:
            if target.address is None:
                return False
            
            if template.address.ip is not None and template.address.ip != target.address.ip:
                return False
            
            if template.address.port is not None and template.address.port != target.address.port:
                return False
            
            return True

        # Empty template matches everything
        return True
