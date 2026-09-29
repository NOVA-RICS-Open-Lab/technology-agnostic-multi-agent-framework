import time
import agent_launcher as al

agent = al.SkilledAgent("FMS_Tower", [al.SKILLS[0], al.SKILLS[1]], al.ResourceType.Station.name, al.Locations.D.name)
agent.Start()
print("FMS Tower Agent started.", flush=True)

while True:
    time.sleep(1)
