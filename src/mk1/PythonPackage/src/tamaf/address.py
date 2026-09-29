class Address:

    def __init__(self, ip: str, port: int = None):
        self.ip = ip
        self.port = port

    def SetPort(self, port: int):
        self.port = port

    def GetTuple(self):
        return self.ip, self.port
    
    def GetString(self):
        return f"{self.ip}:{self.port}"
    
    def GetHTTPLink(self):
        return f"http://{self.ip}:{self.port}"

    @classmethod
    def FromString(cls, addressString: str):
        ip, port_string = addressString.split(":")
        port = int(port_string)
        return cls(ip, port)