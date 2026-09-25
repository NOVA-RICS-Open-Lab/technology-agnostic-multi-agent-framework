from .defines import (
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_AGENTADDRESS_CONCEPT,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_AGENTDESCRIPTION_CONCEPT,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_INFORM_CONCEPT,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_REGISTER_ACTION,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_REGISTER_RESULT_ACTION,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_DEREGISTER_ACTION,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_DEREGISTER_RESULT_ACTION,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_MODIFY_ACTION,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_MODIFY_RESULT_ACTION,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_EXTERNAL_SEARCH_ACTION,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_LOCAL_SEARCH_ACTION,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_GLOBAL_SEARCH_ACTION,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_SEARCH_RESULT_ACTION,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_KEEPALIVE_ACTION
)
from typing import TYPE_CHECKING
from .address import Address

if TYPE_CHECKING:
    from .agentdescription import AgentDescription

class AgentManagementOntology():
    
    name = DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME
    actions = [
        DEFAULT_AGENTMANAGEMENT_ONTOLOGY_REGISTER_ACTION,
        DEFAULT_AGENTMANAGEMENT_ONTOLOGY_REGISTER_RESULT_ACTION,
        DEFAULT_AGENTMANAGEMENT_ONTOLOGY_DEREGISTER_ACTION,
        DEFAULT_AGENTMANAGEMENT_ONTOLOGY_DEREGISTER_RESULT_ACTION,
        DEFAULT_AGENTMANAGEMENT_ONTOLOGY_MODIFY_ACTION,
        DEFAULT_AGENTMANAGEMENT_ONTOLOGY_MODIFY_RESULT_ACTION,
        DEFAULT_AGENTMANAGEMENT_ONTOLOGY_EXTERNAL_SEARCH_ACTION,
        DEFAULT_AGENTMANAGEMENT_ONTOLOGY_LOCAL_SEARCH_ACTION,
        DEFAULT_AGENTMANAGEMENT_ONTOLOGY_GLOBAL_SEARCH_ACTION,
        DEFAULT_AGENTMANAGEMENT_ONTOLOGY_SEARCH_RESULT_ACTION,
        DEFAULT_AGENTMANAGEMENT_ONTOLOGY_KEEPALIVE_ACTION
    ]
    concepts = [DEFAULT_AGENTMANAGEMENT_ONTOLOGY_AGENTDESCRIPTION_CONCEPT, DEFAULT_AGENTMANAGEMENT_ONTOLOGY_AGENTADDRESS_CONCEPT, DEFAULT_AGENTMANAGEMENT_ONTOLOGY_INFORM_CONCEPT]

    @staticmethod
    def Validate(content: dict):
        if content['action'] not in AgentManagementOntology.actions:
            return False
        
        if content['concept'] not in AgentManagementOntology.concepts:
            return False
        
        return True
    
    @staticmethod
    def GetAction(content):
        if not AgentManagementOntology.Validate(content):
            return
        return content['action']
    
    @staticmethod
    def GetConcept(content):
        if not AgentManagementOntology.Validate(content):
            return
        return content['concept']
    
    @staticmethod
    def GetAttributes(content):
        if not AgentManagementOntology.Validate(content):
            return
        return content['attributes']

    @staticmethod
    def GetAgentDescriptionsFromAttributes(content: dict) -> list['AgentDescription']:
        from .agentdescription import AgentDescription
        
        if not AgentManagementOntology.Validate(content):
            return
        
        concept = content.get('concept')
        
        if concept == DEFAULT_AGENTMANAGEMENT_ONTOLOGY_AGENTDESCRIPTION_CONCEPT:
            return [AgentDescription.FromDict(attr) for attr in content['attributes']]
    
    @staticmethod
    def GetAgentAddressFromAttributes(content: dict) -> Address:
        if not AgentManagementOntology.Validate(content):
            return
        
        concept = content.get('concept')
        
        if concept == DEFAULT_AGENTMANAGEMENT_ONTOLOGY_AGENTADDRESS_CONCEPT:
            return Address.FromString(content['attributes'])
    
    @staticmethod
    def CreateAgentDescriptionMessageContent(action: str, agentDescriptions: list['AgentDescription']) -> dict:
        return {
            "action": action,
            "concept": DEFAULT_AGENTMANAGEMENT_ONTOLOGY_AGENTDESCRIPTION_CONCEPT,
            "attributes": [agentDescription.ToDict() for agentDescription in agentDescriptions]
        }

    @staticmethod
    def CreateAgentAddressMessageContent(action: str, agentAddress: Address) -> dict:
        return {
            "action": action,
            "concept": DEFAULT_AGENTMANAGEMENT_ONTOLOGY_AGENTADDRESS_CONCEPT,
            "attributes": agentAddress.GetString()
        }
    
    @staticmethod
    def CreateResultMessageContent(action: str, concept: str, inform: int) -> dict:
        return {
            "action": action,
            "concept": concept,
            "attributes": inform
        }
