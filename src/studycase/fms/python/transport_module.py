import threading
import json
import time
from tamaf import (
    Agent, 
    FIPARequestResponder, 
    Performative, 
    ACLMessageTemplate, 
    ServiceDescription
)
from libraries import execute_transport as execute_transport_http
from constants import (
    DF_SERVICE_TYPE_TRANSPORT,
    DF_SERVICE_TYPE_RESOURCE,
    DF_SERVICE_NAME_TRANSPORT,
    ONTOLOGY_REQUEST_TRANSPORT,
    Locations,
    get_host_tag
)

class TransportAgent(Agent):
    def __init__(self, name, skills, resource_type, location):
        host_tag = get_host_tag()
        if not name.endswith(f"_{host_tag}"):
            name = f"{name}_{host_tag}"
        super().__init__(name)
        self.my_skills = skills
        self.my_resource_type = resource_type
        self.my_location = location
        self.current_product_location = location
        for skill in self.my_skills:
            sd = ServiceDescription(name=skill[0], type=skill[1])
            self.agentDescription.AddService(sd)

    def Setup(self, emaInteraction):

        # Add Request Responder for transport execution
        template_req = ACLMessageTemplate()
        
        template_req.SetPerformative(Performative.REQUEST)
        template_req.SetOntology(ONTOLOGY_REQUEST_TRANSPORT)

        self.AddBehavior(REQExecuteTransportResponder(self, template_req))

class REQExecuteTransportResponder(FIPARequestResponder):
    def __init__(self, agent, request_template):
        super().__init__(agent, request_template)
        self.thread = None
        self.is_moving = False
        self.target_location = None

    def handleRequest(self, request_msg):
        try:
            print("DADWDADWAWDADWADWDAWDAWDWADWADAWDADWDADWADAWDAWWDWADWADWADADAWDSADWADASDWA")
            content = json.loads(request_msg.content)
            # Java code: nextLocation = content.get("b").toString()
            next_location_str = content.get("b")
            
            # Check if valid location
            if next_location_str in Locations.__members__:
                self.target_location = next_location_str
                reply = request_msg.CreateReply(Performative.AGREE)
                return reply
            else:
                reply = request_msg.CreateReply(Performative.REFUSE)
                return reply
        except (json.JSONDecodeError, AttributeError):
            reply = request_msg.CreateReply(Performative.REFUSE)
            return reply

    def prepareResultNotification(self, request_msg, agree_msg):
        if not self.is_moving:
            print("[REQExecuteTransportResponder] Starting transport thread")
            # Start transport execution in a separate thread
            self.is_moving = True
            self.thread = threading.Thread(target=self._execute_transport_logic)
            self.thread.start()
            return None
        
        if self.thread.is_alive():
            return None
        
        print(f"[REQExecuteTransportResponder] Thread finished, preparing INFORM with location {self.agent.current_product_location}")
        # Thread finished
        reply = request_msg.CreateReply(Performative.INFORM)
        reply.content = self.agent.current_product_location
        
        # Reset
        self.is_moving = False
        self.thread = None
        self.target_location = None
        
        return reply

    def _execute_transport_logic(self):
        while self.agent.current_product_location != self.target_location:
            self._execute_transport_step()
            # Adding a small sleep to avoid tight loop if execute_transport_step is too fast or fails
            time.sleep(0.1)

    def _execute_transport_step(self):
        current_loc = self.agent.current_product_location
        
        # Circular logic: A -> B -> C -> D -> E -> F -> A
        loc_list = list(Locations.__members__.keys())
        current_index = loc_list.index(current_loc)
        next_index = (current_index + 1) % len(loc_list)
        next_loc = loc_list[next_index]
        
        skill_id = f"{current_loc}{next_loc}"
        print(f"Transporting from {current_loc} to {next_loc} (Skill: {skill_id})")
        
        result = execute_transport_http(skill_id)
        if result is not None:
            self.agent.current_product_location = next_loc
            print(f"Arrived at {next_loc}")
        else:
            print(f"Failed to transport from {current_loc} to {next_loc}")
