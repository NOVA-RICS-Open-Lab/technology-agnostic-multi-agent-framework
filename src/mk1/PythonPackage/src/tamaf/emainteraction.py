import time
import uuid
from typing import TYPE_CHECKING
from .agentid import AgentID
from .performative import Performative
from .aclmessage import ACLMessage
from .aclmessagetemplate import ACLMessageTemplate
from .agentmanagementontology import AgentManagementOntology
from .defines import (
    EMAFailureException,
    DEFAULT_EMA_NAME,
    DEFAULT_EMA_MESSAGE_RETRYS,
    DEFAULT_EMA_MESSAGE_RETRY_INTERVAL,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_REGISTER_ACTION,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_DEREGISTER_ACTION,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_MODIFY_ACTION,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_GLOBAL_SEARCH_ACTION,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_LOCAL_SEARCH_ACTION,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_EXTERNAL_SEARCH_ACTION,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_KEEPALIVE_ACTION,
    DEFAULT_EMA_LAUNCH_ABILITY_ACTIVE
)

if TYPE_CHECKING:
    from .behavior import Behavior
    from .agentdescription import AgentDescription

class EMAInteraction:
    
    def __init__(self, behavior: 'Behavior'):
        self.behavior = behavior
        self.messageSent = False
        self.conversationID = f"ema-{uuid.uuid4()}"

    def SendToEMA(self, aclMessage: ACLMessage) -> bool:
        from .ema import LaunchEMA

        for _ in range(DEFAULT_EMA_MESSAGE_RETRYS):
            isEMAAlive = self.behavior.Send(aclMessage)
            if isEMAAlive:
                return True
            time.sleep(DEFAULT_EMA_MESSAGE_RETRY_INTERVAL)
        
        if not DEFAULT_EMA_LAUNCH_ABILITY_ACTIVE:
            raise EMAFailureException
        
        LaunchEMA()

        for _ in range(DEFAULT_EMA_MESSAGE_RETRYS):
            isEMAAlive = self.behavior.Send(aclMessage)
            if isEMAAlive:
                return True
            time.sleep(DEFAULT_EMA_MESSAGE_RETRY_INTERVAL)
        
    
    def KeepAliveAgent(self):
        aclMessage = ACLMessage(
            sender=self.behavior.agent.agentDescription.agentid, 
            receiver=[AgentID(DEFAULT_EMA_NAME, self.behavior.agent.mts.emaAddress)], 
            content=AgentManagementOntology.CreateAgentDescriptionMessageContent(action=DEFAULT_AGENTMANAGEMENT_ONTOLOGY_KEEPALIVE_ACTION, agentDescriptions=[self.behavior.agent.agentDescription]), 
            performative=Performative.KEEPALIVE, 
            ontology=DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME
        )
        self.SendToEMA(aclMessage)

    def RegisterAgent(self):
        
        if not self.messageSent:
            registerACLMessage = ACLMessage(
                sender= self.behavior.agent.agentDescription.agentid, 
                performative=Performative.REQUEST, 
                receiver=[AgentID(DEFAULT_EMA_NAME, self.behavior.agent.mts.emaAddress)], 
                ontology=DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME, 
                content=AgentManagementOntology.CreateAgentDescriptionMessageContent(DEFAULT_AGENTMANAGEMENT_ONTOLOGY_REGISTER_ACTION, [self.behavior.agent.agentDescription]),
                conversationid=self.conversationID
            )

            self.SendToEMA(registerACLMessage)
                
            self.messageSent = True

        if self.messageSent:
            responseMessage = self.behavior.Receive(ACLMessageTemplate(sender=AgentID(DEFAULT_EMA_NAME, self.behavior.agent.mts.emaAddress), performatives={Performative.INFORM}, ontology=DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME, conversationID=self.conversationID))
            if AgentManagementOntology.Validate(responseMessage.content):
                self.messageSent = False
                return responseMessage
    
    def DeRegisterAgent(self):

        if not self.messageSent:
            deRegisterACLMessage = ACLMessage(
                sender= self.behavior.agent.agentDescription.agentid, 
                performative=Performative.REQUEST, 
                receiver=[AgentID(DEFAULT_EMA_NAME, self.behavior.agent.mts.emaAddress)], 
                ontology=DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME, 
                content=AgentManagementOntology.CreateAgentDescriptionMessageContent(DEFAULT_AGENTMANAGEMENT_ONTOLOGY_DEREGISTER_ACTION, [self.behavior.agent.agentDescription]),
                conversationid=self.conversationID
            )

            self.SendToEMA(deRegisterACLMessage)
            
            self.messageSent = True

        if self.messageSent:
            responseMessage = self.behavior.Receive(ACLMessageTemplate(sender=AgentID(DEFAULT_EMA_NAME, self.behavior.agent.mts.emaAddress), performatives={Performative.INFORM}, ontology=DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME, conversationID=self.conversationID))
            if AgentManagementOntology.Validate(responseMessage.content):
                self.messageSent = False
                return responseMessage
    
    def ModifyAgent(self):

        if not self.messageSent:
            modifyACLMessage = ACLMessage(
                sender= self.behavior.agent.agentDescription.agentid, 
                performative=Performative.REQUEST, 
                receiver=[AgentID(DEFAULT_EMA_NAME, self.behavior.agent.mts.emaAddress)], 
                ontology=DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME, 
                content=AgentManagementOntology.CreateAgentDescriptionMessageContent(DEFAULT_AGENTMANAGEMENT_ONTOLOGY_MODIFY_ACTION, [self.behavior.agent.agentDescription]),
                conversationid=self.conversationID
            )

            self.SendToEMA(modifyACLMessage)
            
            self.messageSent = True

        if self.messageSent:
            responseMessage = self.behavior.Receive(ACLMessageTemplate(sender=AgentID(DEFAULT_EMA_NAME, self.behavior.agent.mts.emaAddress), performatives={Performative.INFORM}, ontology=DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME, conversationID=self.conversationID))
            if AgentManagementOntology.Validate(responseMessage.content):
                self.messageSent = False
                return responseMessage
    
    def Search(self, agentDescription: 'AgentDescription'):

        if not self.messageSent:
            SearchACLMessage = ACLMessage(
                sender= self.behavior.agent.agentDescription.agentid, 
                performative=Performative.REQUEST, 
                receiver=[AgentID(DEFAULT_EMA_NAME, self.behavior.agent.mts.emaAddress)], 
                ontology=DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME, 
                content=AgentManagementOntology.CreateAgentDescriptionMessageContent(DEFAULT_AGENTMANAGEMENT_ONTOLOGY_GLOBAL_SEARCH_ACTION, [agentDescription]),
                conversationid=self.conversationID
            )

            self.SendToEMA(SearchACLMessage)

            self.messageSent = True

        if self.messageSent:
            responseMessage = self.behavior.Receive(ACLMessageTemplate(sender=AgentID(DEFAULT_EMA_NAME, self.behavior.agent.mts.emaAddress), performatives={Performative.INFORM}, ontology=DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME, conversationID=self.conversationID))
            if AgentManagementOntology.Validate(responseMessage.content):
                self.messageSent = False
                return AgentManagementOntology.GetAgentDescriptionsFromAttributes(responseMessage.content)
    
    def LocalSearch(self, agentDescription: 'AgentDescription'):

        if not self.messageSent:
            SearchACLMessage = ACLMessage(
                sender= self.behavior.agent.agentDescription.agentid, 
                performative=Performative.REQUEST, 
                receiver=[AgentID(DEFAULT_EMA_NAME, self.behavior.agent.mts.emaAddress)], 
                ontology=DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME, 
                content=AgentManagementOntology.CreateAgentDescriptionMessageContent(DEFAULT_AGENTMANAGEMENT_ONTOLOGY_LOCAL_SEARCH_ACTION, [agentDescription]),
                conversationid=self.conversationID
            )

            self.SendToEMA(SearchACLMessage)
            
            self.messageSent = True

        if self.messageSent:
            responseMessage = self.behavior.Receive(ACLMessageTemplate(sender=AgentID(DEFAULT_EMA_NAME, self.behavior.agent.mts.emaAddress), performatives={Performative.INFORM}, ontology=DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME, conversationID=self.conversationID))
            if AgentManagementOntology.Validate(responseMessage.content):
                self.messageSent = False
                return AgentManagementOntology.GetAgentDescriptionsFromAttributes(responseMessage.content)
    
    def ExternalSearch(self, agentDescription: 'AgentDescription'):

        if not self.messageSent:
            SearchACLMessage = ACLMessage(
                sender= self.behavior.agent.agentDescription.agentid, 
                performative=Performative.REQUEST, 
                receiver=[AgentID(DEFAULT_EMA_NAME, self.behavior.agent.mts.emaAddress)], 
                ontology=DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME, 
                content=AgentManagementOntology.CreateAgentDescriptionMessageContent(DEFAULT_AGENTMANAGEMENT_ONTOLOGY_EXTERNAL_SEARCH_ACTION, [agentDescription]),
                conversationid=self.conversationID
            )

            self.SendToEMA(SearchACLMessage)
            
            self.messageSent = True

        if self.messageSent:
            responseMessage = self.behavior.Receive(ACLMessageTemplate(sender=AgentID(DEFAULT_EMA_NAME, self.behavior.agent.mts.emaAddress), performatives={Performative.INFORM}, ontology=DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME, conversationID=self.conversationID))
            if AgentManagementOntology.Validate(responseMessage.content):
                self.messageSent = False
                return AgentManagementOntology.GetAgentDescriptionsFromAttributes(responseMessage.content)
