import threading
import sys
import os

# Ensure tamaf is in the path if needed, but typically it should be installed or in the same root
# sys.path.append(os.path.join(os.path.dirname(__file__), '..', '..', '..', 'mk1', 'PythonPackage', 'src'))

from tamaf import (
    Agent, 
    FIPAContractNetResponder, 
    FIPARequestResponder, 
    Performative, 
    ACLMessageTemplate, 
    ServiceDescription
)
from libraries import execute_skill
from constants import (
    DF_SERVICE_TYPE_SKILL,
    DF_SERVICE_TYPE_RESOURCE,
    ONTOLOGY_NEGOTIATE_NEXT_RESOURCE,
    ONTOLOGY_REQUEST_EXECUTE_SKILL,
    get_host_tag
)

class SkilledAgent(Agent):
    def __init__(self, name, skills, resource_type, location):
        host_tag = get_host_tag()
        if not name.endswith(f"_{host_tag}"):
            name = f"{name}_{host_tag}"
        super().__init__(name)
        self.my_skills = skills # String[][] in Java
        self.my_resource_type = resource_type
        self.my_location = location
        for skill in self.my_skills:
            sd = ServiceDescription(name=skill[0], type=skill[1])
            self.agentDescription.AddService(sd)

    def Setup(self, emaInteraction):
        
        # Add ContractNet Responder for negotiation
        template_cnet = ACLMessageTemplate()
        
        template_cnet.SetPerformative(Performative.CFP)
        template_cnet.SetOntology(ONTOLOGY_NEGOTIATE_NEXT_RESOURCE)

        self.AddBehavior(CNETSkilledResponder(self, template_cnet))
        
        # Add Request Responder for skill execution
        template_req = ACLMessageTemplate()
        
        template_req.SetPerformative(Performative.REQUEST)
        template_req.SetOntology(ONTOLOGY_REQUEST_EXECUTE_SKILL)
        
        self.AddBehavior(REQExecuteSkillResponder(self, template_req))

class CNETSkilledResponder(FIPAContractNetResponder):
    def handleCfp(self, cfp_msg):
        reply = cfp_msg.CreateReply(Performative.PROPOSE)
        reply.content = "1"
        return reply

    def handleAcceptProposal(self, cfp_msg, propose_msg, accept_msg):
        reply = accept_msg.CreateReply(Performative.INFORM)
        reply.content = self.agent.my_location
        return reply

class REQExecuteSkillResponder(FIPARequestResponder):
    def __init__(self, agent, request_template):
        super().__init__(agent, request_template)
        self.thread = None
        self.result = None

    def handleRequest(self, request_msg):
        reply = request_msg.CreateReply(Performative.AGREE)
        return reply

    def prepareResultNotification(self, request_msg, agree_msg):
        if self.thread is None:
            # Start skill execution in a separate thread
            content = request_msg.content
            
            for skill in self.agent.my_skills:
                if skill[0] == content:
                    urlSkill = skill[2]
                    break
            
            self.thread = threading.Thread(target=self._run_skill, args=(urlSkill,))
            self.thread.start()
            return None # Java returns null while thread is alive
        
        if self.thread.is_alive():
            return None
        
        # Thread finished
        reply = request_msg.CreateReply(Performative.INFORM)
        reply.content = str(self.result)
        
        # Reset for next request
        self.thread = None
        self.result = None
        
        return reply

    def _run_skill(self, skill_url):
        self.result = execute_skill(skill_url)
