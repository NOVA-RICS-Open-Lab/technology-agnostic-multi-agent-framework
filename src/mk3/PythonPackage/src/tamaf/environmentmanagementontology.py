from .defines import (
    DEFAULT_ENVIRONMENTMANAGEMENT_ONTOLOGY_NAME,
    DEFAULT_ENVIRONMENTMANAGEMENT_ONTOLOGY_ENVIRONMENTUDPATE_CONCEPT,
    DEFAULT_ENVIRONMENTMANAGEMENT_ONTOLOGY_KEEPALIVE_ACTION
)

class EnvironmentManagementOntology():
    
    name = DEFAULT_ENVIRONMENTMANAGEMENT_ONTOLOGY_NAME
    actions = [DEFAULT_ENVIRONMENTMANAGEMENT_ONTOLOGY_KEEPALIVE_ACTION]
    concepts = [DEFAULT_ENVIRONMENTMANAGEMENT_ONTOLOGY_ENVIRONMENTUDPATE_CONCEPT]

    @staticmethod
    def Validate(content: dict):
        if content['action'] not in EnvironmentManagementOntology.actions:
            return False
        return True
    
    @staticmethod
    def GetAction(content):
        if not EnvironmentManagementOntology.Validate(content):
            return
        return content['action']
    
    @staticmethod
    def GetUpdateFromAttributes(content):
        if not EnvironmentManagementOntology.Validate(content):
            return
        
        concept = content.get('concept')
        
        if concept == DEFAULT_ENVIRONMENTMANAGEMENT_ONTOLOGY_ENVIRONMENTUDPATE_CONCEPT:
            return float(content['attributes'])
    
    @staticmethod
    def CreateUpdateMessageContent(action: str, concept: str,  content: float) -> dict:
        return {
            "action": action,
            "concept": concept,
            "attributes": content
        }
