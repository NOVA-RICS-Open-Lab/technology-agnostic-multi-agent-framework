class ServiceDescription:
    def __init__(self, name: str, type: str = None, ownership: str = None, 
                 protocols: list[str] = None, ontologies: list[str] = None, 
                 properties: dict = None):
        self.name = name
        self.type = type
        self.ownership = ownership
        self.protocols = protocols if protocols is not None else []
        self.ontologies = ontologies if ontologies is not None else []
        self.properties = properties if properties is not None else {}

    def Matches(self, template: 'ServiceDescription') -> bool:
        if template.name is not None and self.name != template.name:
            return False
        if template.type is not None and self.type != template.type:
            return False
        if template.ownership is not None and self.ownership != template.ownership:
            return False
            
        if template.protocols:
            if not all(p in self.protocols for p in template.protocols):
                return False
                
        if template.ontologies:
            if not all(o in self.ontologies for o in template.ontologies):
                return False
        
        if template.properties:
            for key, val in template.properties.items():
                if self.properties.get(key) != val:
                    return False
        
        return True

    def ToDict(self) -> dict:
        return {
            "name": self.name,
            "type": self.type,
            "ownership": self.ownership,
            "protocols": self.protocols,
            "ontologies": self.ontologies,
            "properties": self.properties
        }
    
    @classmethod
    def FromDict(cls, data: dict):
        if not data:
            return None
        return cls(
            name=data.get("name"),
            type=data.get("type"),
            ownership=data.get("ownership"),
            protocols=data.get("protocols"),
            ontologies=data.get("ontologies"),
            properties=data.get("properties")
        )
