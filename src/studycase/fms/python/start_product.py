import time
import agent_launcher as al

agent = al.ProductAgent("Product_1", ["Drill", "Screw"], al.Locations.A.name)
agent.Start()
print("Product Agent started.", flush=True)

while True:
    time.sleep(1)
