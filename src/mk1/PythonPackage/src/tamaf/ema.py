import time
import threading
import socket
import json
import struct
from itertools import chain
from typing import TYPE_CHECKING
from .agent import Agent
from .agentid import AgentID
from .agentdescription import AgentDescription
from .aclmessage import ACLMessage
from .aclmessagetemplate import ACLMessageTemplate
from .performative import Performative
from .tickerbehavior import TickerBehavior
from .cyclicbehavior import CyclicBehavior
from .agentmanagementontology import AgentManagementOntology
from .environmentmanagementontology import EnvironmentManagementOntology
from .transportmessage import TransportMessage
from .defines import (
    DEFAULT_EMA_NAME,
    DEFAULT_EMA_HEARTBEAT_INTERVAL,
    DEFAULT_EMA_EXTERNAL_HEARTBEAT_TIMEOUT,
    DEFAULT_AGENT_ALREADY_EXISTS,
    DEFAULT_EMA_HAS_NO_MORE_PORTS,
    DEFAULT_OUTSIDE_AGENT,
    MULTICAST_ADDRESS,
    ALL_INTERFACES,
    DEFAULT_MULTICAST_PORT,
    DEFAULT_MULTICAST_IP,
    DEFAULT_UDP_PACKETSIZE,
    AVAILABLEPORTS,
    DEFAULT_AGENT_HEARTBEAT_FAILURE_THRESHOLD,
    DEFAULT_EMA_HEARTBEAT_FAILURE_THRESHOLD,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_LOCAL_SEARCH_ACTION,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_KEEPALIVE_ACTION,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_REGISTER_ACTION,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_REGISTER_RESULT_ACTION,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_DEREGISTER_ACTION,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_DEREGISTER_RESULT_ACTION,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_MODIFY_ACTION,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_MODIFY_RESULT_ACTION,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_GLOBAL_SEARCH_ACTION,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_SEARCH_RESULT_ACTION,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_EXTERNAL_SEARCH_ACTION,
    DEFAULT_AGENTMANAGEMENT_ONTOLOGY_INFORM_CONCEPT,
    DEFAULT_ENVIRONMENTMANAGEMENT_ONTOLOGY_NAME,
    DEFAULT_ENVIRONMENTMANAGEMENT_ONTOLOGY_KEEPALIVE_ACTION,
    DEFAULT_ENVIRONMENTMANAGEMENT_ONTOLOGY_ENVIRONMENTUDPATE_CONCEPT,
    DEFAULT_AGENT_STILL_ALIVE_THRESHOLD
)

class EMA(Agent):
    
    class ASDS:
        def __init__(self, agent: 'Agent', debug: bool = False):
            self.agent = agent
            self.debug = debug
            self.localAgentRepository: list[AgentDescription] = []
            self.repositoryLastUpdate = time.time()
            self.localHeartbeatRepository: dict[str, int] = {}
            self.externalEMAHeartbeatRepository: dict[str, dict] = {}
            self.externalEMAAgentRepository: dict[str, list[AgentDescription]] = {}
            self.availablePortsInternal = set(AVAILABLEPORTS)
        
        def PortsLeft(self) -> bool:
            return len(self.availablePortsInternal) > 0            
        
        def ReturnPort(self, port: int):
            self.availablePortsInternal.add(port)    
             
        def GetPort(self) -> int:
            return self.availablePortsInternal.pop()

        def BeatAgent(self, agentDescription: AgentDescription):

            if not self.LocalSearch(AgentDescription(AgentID(agentDescription.agentid.name))):
                self.RegisterAgent(agentDescription)
     
            self.localHeartbeatRepository[agentDescription.agentid.GetFullID()] = 0
            self.repositoryLastUpdate = time.time()
            
        def BeatEMA(self, beatingEMAAid: AgentID, lastUpdate: float):            
            beatingEMAIP = beatingEMAAid.address.ip

            if beatingEMAIP not in self.externalEMAHeartbeatRepository:
                self.agent.Send(ACLMessage(
                    sender=self.agent.agentDescription.agentid, 
                    receiver=[beatingEMAAid], 
                    performative=Performative.REQUEST, 
                    content=AgentManagementOntology.CreateAgentDescriptionMessageContent(DEFAULT_AGENTMANAGEMENT_ONTOLOGY_LOCAL_SEARCH_ACTION, [AgentDescription()]), 
                    ontology=DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME
                ))
                self.externalEMAHeartbeatRepository[beatingEMAIP] = {
                    "heartbeat": 0,
                    "lastupdate": lastUpdate
                }
                return
            
            if self.externalEMAHeartbeatRepository[beatingEMAIP]["lastupdate"] < lastUpdate:
                self.agent.Send(ACLMessage(
                    sender=self.agent.agentDescription.agentid, 
                    receiver=[beatingEMAAid], 
                    performative=Performative.REQUEST, 
                    content=AgentManagementOntology.CreateAgentDescriptionMessageContent(DEFAULT_AGENTMANAGEMENT_ONTOLOGY_LOCAL_SEARCH_ACTION, [AgentDescription()]), 
                    ontology=DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME
                ))

            self.externalEMAHeartbeatRepository[beatingEMAIP]["heartbeat"] = 0
            self.externalEMAHeartbeatRepository[beatingEMAIP]["lastupdate"] = lastUpdate
        
        def HandlePersistance(self):

            if self.debug:
                print("\n////////////////////////////\nECOSYSTEM OVERVIEW\n////////////////////////////\n")
                
                print("INTERIOR AGENTS - HEARTBEAT REPOSITORY:")
                print(self.localHeartbeatRepository)
                
                print("INTERIOR AGENTS - AGENT REPOSITORY:")
                print(self.localAgentRepository)
                
                print("EXTERIOR AGENTS - HEARTBEAT REPOSITORY::")
                print(self.externalEMAHeartbeatRepository)
                
                print("EXTERIOR AGENTS - AGENT REPOSITORY:")
                print(self.externalEMAAgentRepository)
                
                print("\n////////////////////////////")
            
            for agentid in list(self.localHeartbeatRepository.keys()):
                self.localHeartbeatRepository[agentid] += 1
                if self.localHeartbeatRepository[agentid] >= DEFAULT_AGENT_HEARTBEAT_FAILURE_THRESHOLD:
                    self.DeRegister(AgentDescription(AgentID.FromString(agentid)))
            

            for emaIP in list(self.externalEMAHeartbeatRepository.keys()):
                self.externalEMAHeartbeatRepository[emaIP]["heartbeat"] += 1
                if self.externalEMAHeartbeatRepository[emaIP]["heartbeat"] >= DEFAULT_EMA_HEARTBEAT_FAILURE_THRESHOLD:
                    self.externalEMAHeartbeatRepository.pop(emaIP, None)
                    self.externalEMAAgentRepository.pop(emaIP, None)

        def RegisterAgent(self, agentDescription: AgentDescription):
            preventiveSearchresults = self.LocalSearch(AgentDescription(AgentID(agentDescription.agentid.name)))

            if preventiveSearchresults != []:
                    
                if self.localHeartbeatRepository[preventiveSearchresults[0].agentid.GetFullID()] > DEFAULT_AGENT_STILL_ALIVE_THRESHOLD:
                    return preventiveSearchresults[0].agentid.address.port
                else:
                    return DEFAULT_AGENT_ALREADY_EXISTS

            if agentDescription.agentid.address.ip != self.agent.agentDescription.agentid.address.ip:
                toReturn = DEFAULT_OUTSIDE_AGENT

            else:

                if  not self.PortsLeft():
                    return DEFAULT_EMA_HAS_NO_MORE_PORTS
                    
                toReturn = self.GetPort()

                agentDescription.agentid.SetPort(toReturn)

            self.localAgentRepository.append(agentDescription)
            self.localHeartbeatRepository[agentDescription.agentid.GetFullID()] = 0
            self.repositoryLastUpdate = time.time()

            return toReturn
            
        def DeRegister(self, agentDescription: AgentDescription):
            self.localAgentRepository = [ad for ad in self.localAgentRepository if ad.agentid.GetFullID() != agentDescription.agentid.GetFullID()]
            agentPresence = self.localHeartbeatRepository.pop(agentDescription.agentid.GetFullID(), None)
            if agentDescription.agentid.address and agentDescription.agentid.address.port:
                self.ReturnPort(agentDescription.agentid.address.port)
            if agentPresence is not None:
                self.repositoryLastUpdate = time.time()

        def LocalSearch(self, template: AgentDescription = None) -> list[AgentDescription]:
            if template is None:
                template = AgentDescription()
            results = [agent for agent in self.localAgentRepository if agent.Matches(template)]
            return results
        
        def ExternalSearch(self, template: AgentDescription = None) -> list[AgentDescription]:
            if template is None:
                template = AgentDescription()
            searchPool = chain(*self.externalEMAAgentRepository.values())
            results = [agent for agent in searchPool if agent.Matches(template)]
            return results
        
        def GlobalSearch(self, template: AgentDescription = None) -> list[AgentDescription]:
            if template is None:
                template = AgentDescription()
            results = self.LocalSearch(template) + self.ExternalSearch(template)
            return results

        def Modify(self, newAgentDescription: AgentDescription) -> bool:
            for i, registered in enumerate(self.localAgentRepository):
                if registered.agentid.GetFullID() == newAgentDescription.agentid.GetFullID():
                    self.localAgentRepository[i] = newAgentDescription
                    self.repositoryLastUpdate = time.time()
                    return True
            return False
        
        def GetRepositoryLastUpdate(self):
            return self.repositoryLastUpdate
        
        def RegisterEMA(self, newEMAIP: str, agentDescriptions: list[AgentDescription]):
            self.externalEMAAgentRepository[newEMAIP] = agentDescriptions

    def __init__(self, name=DEFAULT_EMA_NAME, debug=False):
        super().__init__(name)
        self.asds = self.ASDS(self, debug=debug)

    class ExternalHeartbeatSenderBehavior(TickerBehavior):
        def __init__(self, agent: 'EMA'):
            super().__init__(agent, DEFAULT_EMA_HEARTBEAT_INTERVAL)

        def UDPSend(self, transportMessage: TransportMessage) -> bool:
            messageText = json.dumps(transportMessage.ToDict()).encode('utf-8')
            try:
                with socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP) as clientSocket:
                    clientSocket.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 1)
                    my_physical_ip = self.agent.agentDescription.agentid.address.ip
                    clientSocket.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_IF, socket.inet_aton(my_physical_ip))
                    clientSocket.settimeout(DEFAULT_EMA_EXTERNAL_HEARTBEAT_TIMEOUT)

                    header = struct.pack('!I', len(messageText))
                    payload = header + messageText
                    clientSocket.sendto(payload, MULTICAST_ADDRESS)
                    return True
            except Exception:
                return False

        def OnTick(self):
            tm = TransportMessage(
                ACLMessage(
                    sender=self.agent.agentDescription.agentid,
                    performative=Performative.KEEPALIVE,
                    content=EnvironmentManagementOntology.CreateUpdateMessageContent(DEFAULT_ENVIRONMENTMANAGEMENT_ONTOLOGY_KEEPALIVE_ACTION, DEFAULT_ENVIRONMENTMANAGEMENT_ONTOLOGY_ENVIRONMENTUDPATE_CONCEPT, self.agent.asds.GetRepositoryLastUpdate()),
                    ontology=DEFAULT_ENVIRONMENTMANAGEMENT_ONTOLOGY_NAME,
                ),
                senderAddress=self.agent.agentDescription.agentid.address
            )
            self.UDPSend(tm)
    
    class ExternalHeartbeatServerBehavior(CyclicBehavior):
        def OnStart(self):
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
            self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.sock.setblocking(False)
            self.sock.bind((ALL_INTERFACES, DEFAULT_MULTICAST_PORT))
            mreq = struct.pack("4s4s", socket.inet_aton(DEFAULT_MULTICAST_IP), socket.inet_aton(self.agent.agentDescription.agentid.address.ip))
            self.sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

        def Action(self):
            try:
                data, _ = self.sock.recvfrom(DEFAULT_UDP_PACKETSIZE) 
                if not data:
                    return

                messageSizeRaw = data[:4]
                messageSize = struct.unpack('!I', messageSizeRaw)[0]
                messageRaw = data[4 : 4 + messageSize]
                messageDict = json.loads(messageRaw.decode("utf-8"))
                transportMessage = TransportMessage.FromDict(messageDict)

                senderIP = transportMessage.aclmessage.sender.address.ip
                if senderIP == self.agent.agentDescription.agentid.address.ip:
                    return

                if not EnvironmentManagementOntology.Validate(transportMessage.aclmessage.content):
                    return
                
                lastUpdate = EnvironmentManagementOntology.GetUpdateFromAttributes(transportMessage.aclmessage.content)
                self.agent.asds.BeatEMA(transportMessage.aclmessage.sender, lastUpdate)
            except BlockingIOError:
                pass
            except Exception as e:
                print(f"Server Error: {e}")

        def OnEnd(self):
            self.sock.close()
    
    class EnvironmentPersistanceBehavior(TickerBehavior):
        def __init__(self, agent: 'EMA'):
            super().__init__(agent, DEFAULT_EMA_HEARTBEAT_INTERVAL)
            
        def OnTick(self):
            self.agent.asds.HandlePersistance()
    
    class RequestHandlerBehavior(CyclicBehavior):
        def Action(self):
            aclMessage = self.Receive(ACLMessageTemplate(ontology=DEFAULT_AGENTMANAGEMENT_ONTOLOGY_NAME))
            if not aclMessage:
                return

            if AgentManagementOntology.Validate(aclMessage.content):
                action = AgentManagementOntology.GetAction(aclMessage.content)
                agentDescriptions = AgentManagementOntology.GetAgentDescriptionsFromAttributes(aclMessage.content)
                
                try:
                    if action == DEFAULT_AGENTMANAGEMENT_ONTOLOGY_KEEPALIVE_ACTION:
                        agentDescription = agentDescriptions[0]
                        self.agent.asds.BeatAgent(agentDescription)
                    elif action == DEFAULT_AGENTMANAGEMENT_ONTOLOGY_REGISTER_ACTION:
                        agentDescription = agentDescriptions[0]
                        response = aclMessage.CreateReply(Performative.INFORM)
                        newPort = self.agent.asds.RegisterAgent(agentDescription)
                        if newPort is not None:
                            if newPort == DEFAULT_OUTSIDE_AGENT or newPort == DEFAULT_AGENT_ALREADY_EXISTS or newPort == DEFAULT_EMA_HAS_NO_MORE_PORTS:
                                response.content = AgentManagementOntology.CreateResultMessageContent(DEFAULT_AGENTMANAGEMENT_ONTOLOGY_REGISTER_RESULT_ACTION, DEFAULT_AGENTMANAGEMENT_ONTOLOGY_INFORM_CONCEPT, newPort)
                                self.Send(response)
                            else:
                                newAgentAddress = agentDescription.agentid.address
                                newAgentAddress.port = newPort
                                response.content = AgentManagementOntology.CreateAgentAddressMessageContent(DEFAULT_AGENTMANAGEMENT_ONTOLOGY_REGISTER_RESULT_ACTION, newAgentAddress)
                                self.Send(response)
                    elif action == DEFAULT_AGENTMANAGEMENT_ONTOLOGY_DEREGISTER_ACTION:
                        agentDescription = agentDescriptions[0]
                        self.agent.asds.DeRegister(agentDescription)
                        response = aclMessage.CreateReply(Performative.INFORM)
                        response.content = AgentManagementOntology.CreateResultMessageContent(DEFAULT_AGENTMANAGEMENT_ONTOLOGY_DEREGISTER_RESULT_ACTION, DEFAULT_AGENTMANAGEMENT_ONTOLOGY_INFORM_CONCEPT, 1)
                        self.Send(response)
                    elif action == DEFAULT_AGENTMANAGEMENT_ONTOLOGY_MODIFY_ACTION:
                        agentDescription = agentDescriptions[0]
                        self.agent.asds.Modify(agentDescription)
                        response = aclMessage.CreateReply(Performative.INFORM)
                        response.content = AgentManagementOntology.CreateResultMessageContent(DEFAULT_AGENTMANAGEMENT_ONTOLOGY_MODIFY_RESULT_ACTION, DEFAULT_AGENTMANAGEMENT_ONTOLOGY_INFORM_CONCEPT, 1)
                        self.Send(response)
                    elif action == DEFAULT_AGENTMANAGEMENT_ONTOLOGY_GLOBAL_SEARCH_ACTION:
                        agentDescription = agentDescriptions[0]
                        results = self.agent.asds.GlobalSearch(agentDescription)
                        response = aclMessage.CreateReply(Performative.INFORM)
                        response.content = AgentManagementOntology.CreateAgentDescriptionMessageContent(action=DEFAULT_AGENTMANAGEMENT_ONTOLOGY_SEARCH_RESULT_ACTION, agentDescriptions=results)
                        self.Send(response)
                    elif action == DEFAULT_AGENTMANAGEMENT_ONTOLOGY_LOCAL_SEARCH_ACTION:
                        agentDescription = agentDescriptions[0]
                        results = self.agent.asds.LocalSearch(agentDescription)
                        response = aclMessage.CreateReply(Performative.INFORM)
                        response.content = AgentManagementOntology.CreateAgentDescriptionMessageContent(action=DEFAULT_AGENTMANAGEMENT_ONTOLOGY_SEARCH_RESULT_ACTION, agentDescriptions=results)
                        self.Send(response)
                    elif action == DEFAULT_AGENTMANAGEMENT_ONTOLOGY_EXTERNAL_SEARCH_ACTION:
                        agentDescription = agentDescriptions[0]
                        results = self.agent.asds.ExternalSearch(agentDescription)
                        response = aclMessage.CreateReply(Performative.INFORM)
                        response.content = AgentManagementOntology.CreateAgentDescriptionMessageContent(action=DEFAULT_AGENTMANAGEMENT_ONTOLOGY_SEARCH_RESULT_ACTION, agentDescriptions=results)
                        self.Send(response)
                    elif action == DEFAULT_AGENTMANAGEMENT_ONTOLOGY_SEARCH_RESULT_ACTION:
                        self.agent.asds.RegisterEMA(aclMessage.sender.address.ip, agentDescriptions)
                except Exception as e:
                    import traceback
                    traceback.print_exc()
                    pass

    def Setup(self, emaInteraction):
        self.AddBehavior(self.EnvironmentPersistanceBehavior(self))
        self.AddBehavior(self.RequestHandlerBehavior(self))
        self.AddBehavior(self.ExternalHeartbeatServerBehavior(self))
        self.AddBehavior(self.ExternalHeartbeatSenderBehavior(self))

def LaunchEMA(debug = False) -> Agent:
    newEma = EMA(debug=debug)
    newEma.Start()
    return newEma
