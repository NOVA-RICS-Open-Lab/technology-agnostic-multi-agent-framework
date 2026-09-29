import json
import time
from tamaf import (
    Agent, 
    OneShotBehavior, 
    FIPAContractNetInitiator, 
    FIPARequestInitiator, 
    Performative, 
    AgentDescription, 
    ServiceDescription,
    ACLMessage
)
from constants import (
    DF_SERVICE_TYPE_SKILL,
    DF_SERVICE_TYPE_TRANSPORT,
    DF_SERVICE_NAME_TRANSPORT,
    ONTOLOGY_NEGOTIATE_NEXT_RESOURCE,
    ONTOLOGY_REQUEST_TRANSPORT,
    ONTOLOGY_REQUEST_EXECUTE_SKILL,
    get_host_tag
)

class ProductAgent(Agent):
    def __init__(self, name, process, location):
        """
        :param name: Agent name
        :param process: List of skill names (strings)
        :param location: Initial location (string key from Locations)
        """
        host_tag = get_host_tag()
        if not name.endswith(f"_{host_tag}"):
            name = f"{name}_{host_tag}"
        super().__init__(name)
        self.my_process = process 
        self.my_location = location
        self.resource_executer = None
        self.next_location = None
        self.current_skill_index = 0

    def Setup(self, emaInteraction):
        self.AddBehavior(NextSkillBehaviour(self))

    def currentSkill(self):
        if self.current_skill_index < len(self.my_process):
            return self.my_process[self.current_skill_index]
        return None

    def hasNextSkill(self):
        return self.current_skill_index + 1 < len(self.my_process)

    def setNextSkill(self):
        self.current_skill_index += 1

class NextSkillBehaviour(OneShotBehavior):
    def Action(self):
        skill = self.agent.currentSkill()
        if skill is None:
            print(f"[{self.agent.getName()}] All skills completed.")
            return

        print(f"[{self.agent.getName()}] Searching for skill: {skill}")
        
        searchAD = AgentDescription()
        sd = ServiceDescription(name=skill)
        searchAD.AddService(sd)
        
        result = self.emaInteraction.Search(searchAD)
        
        if not result:
            print(f"[{self.agent.getName()}] No agents found for skill {skill}. Retrying...")
            time.sleep(1)
            self.agent.AddBehavior(NextSkillBehaviour(self.agent))
            return

        cfp = ACLMessage(Performative.CFP)
        for ad in result:
            cfp.receiver.append(ad.agentid)
        
        cfp.ontology = ONTOLOGY_NEGOTIATE_NEXT_RESOURCE
        cfp.content = skill
        
        self.agent.AddBehavior(CNETProposalInitiator(self.agent, cfp))

class CNETProposalInitiator(FIPAContractNetInitiator):
    def handleProposals(self, proposals, responses):
        if not proposals:
            print(f"[{self.agent.getName()}] No proposals received. Retrying NextSkillBehaviour...")
            self.agent.AddBehavior(NextSkillBehaviour(self.agent))
            return False

        # Pick the first proposal
        best_proposal = proposals[0]
        accept = best_proposal.CreateReply(Performative.ACCEPT_PROPOSAL)
        responses.append(accept)
        
        # Reject others
        for i in range(1, len(proposals)):
            reject = proposals[i].CreateReply(Performative.REJECT_PROPOSAL)
            responses.append(reject)
        
        return True

    def handleInform(self, msg):
        self.agent.resource_executer = msg.sender
        self.agent.next_location = msg.content
        
        print(f"[{self.agent.getName()}] Proposal accepted by {msg.sender.GetFullID()}. Next location: {self.agent.next_location}")
        
        # Search for transport
        searchAD = AgentDescription()
        sd = ServiceDescription(name=DF_SERVICE_NAME_TRANSPORT, type=DF_SERVICE_TYPE_TRANSPORT)
        searchAD.AddService(sd)
        
        result = self.emaInteraction.Search(searchAD)
        
        if not result:
            print(f"[{self.agent.getName()}] Transport agent not found!")
            return True

        req = ACLMessage(Performative.REQUEST)
        req.receiver.append(result[0].agentid)
        req.ontology = ONTOLOGY_REQUEST_TRANSPORT
        
        # Based on transport_module.py, use "b" for target location
        content = {
            "a": self.agent.my_location,
            "b": self.agent.next_location
        }
        req.content = json.dumps(content)
        
        self.agent.AddBehavior(REQUESTExecuteTransportInitiator(self.agent, req))
        return True

class REQUESTExecuteTransportInitiator(FIPARequestInitiator):
    def handleInform(self, msg):
        self.agent.my_location = self.agent.next_location
        print(f"[{self.agent.getName()}] Transport completed. Arrived at {self.agent.my_location}")
        
        req = ACLMessage(Performative.REQUEST)
        req.receiver.append(self.agent.resource_executer)
        req.ontology = ONTOLOGY_REQUEST_EXECUTE_SKILL
        req.content = self.agent.currentSkill()
        
        self.agent.AddBehavior(REQUESTExecuteSkillInitiator(self.agent, req))
        return True

    def handleRefuse(self, msg):
        print(f"[{self.agent.getName()}] Transport refused. Retrying NextSkillBehaviour...")
        self.agent.AddBehavior(NextSkillBehaviour(self.agent))
        return True

class REQUESTExecuteSkillInitiator(FIPARequestInitiator):
    def handleInform(self, msg):
        print(f"[{self.agent.getName()}] Skill {self.agent.currentSkill()} completed.")
        
        if self.agent.hasNextSkill():
            self.agent.setNextSkill()
            self.agent.AddBehavior(NextSkillBehaviour(self.agent))
        else:
            print(f"[{self.agent.getName()}] All skills completed. Returning to A.")
            
            # Transport back to A
            searchAD = AgentDescription()
            sd = ServiceDescription(name=DF_SERVICE_NAME_TRANSPORT, type=DF_SERVICE_TYPE_TRANSPORT)
            searchAD.AddService(sd)
            result = self.emaInteraction.Search(searchAD)
            
            if result:
                req = ACLMessage(Performative.REQUEST)
                req.receiver.append(result[0].agentid)
                req.ontology = ONTOLOGY_REQUEST_TRANSPORT
                content = {
                    "a": self.agent.my_location,
                    "b": "A" 
                }
                req.content = json.dumps(content)
                self.agent.AddBehavior(TransportBackBehaviour(self.agent, req))
            else:
                self.agent.Stop()
        return True

    def handleRefuse(self, msg):
        print(f"[{self.agent.getName()}] Skill execution refused. Retrying NextSkillBehaviour...")
        self.agent.AddBehavior(NextSkillBehaviour(self.agent))
        return True

class TransportBackBehaviour(FIPARequestInitiator):
    def handleInform(self, msg):
        print(f"[{self.agent.getName()}] Back at A. Stopping.")
        self.agent.Stop()
        return True
    
    def handleFailure(self, msg):
        print(f"[{self.agent.getName()}] Failed to return to A. Stopping.")
        self.agent.Stop()
        return True
