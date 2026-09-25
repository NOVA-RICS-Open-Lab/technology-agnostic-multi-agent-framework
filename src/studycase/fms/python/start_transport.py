import time
import agent_launcher as al

agent = al.TransportAgent("Transport_1", [al.SKILLS[2], al.SKILLS[3]], al.ResourceType.Robot.name, al.Locations.A.name)
agent.Start()
print("Transport Agent started.", flush=True)

while True:
    time.sleep(1)
