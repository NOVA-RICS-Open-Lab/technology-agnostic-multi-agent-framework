import time
from skilled_module import SkilledAgent
from transport_module import TransportAgent
from product_module import ProductAgent
from constants import SKILLS, ResourceType, Locations

def main():
    while True:
        print("\nAgent Launcher")
        print("1. Launch Resource 1 (Drill)")
        print("2. Launch Resource 2 (Screw)")
        print("3. Launch Transport Agent")
        print("4. Launch Product Agent")
        print("5. Launch FMS Tower Agent")
        print("0. Exit")
        
        choice = input("Select option: ")
        
        if choice == '1':
            # SkilledAgent(name, skills, resource_type, location)
            # SKILLS[0] is ["Drill", "RESOURCE_TYPE", "..."] -> wrap in list to be String[][]
            agent = SkilledAgent("Resource_1", [SKILLS[0]], ResourceType.Station.name, Locations.A.name)
            agent.Start()
            print("Resource 1 (Drill) started.")

        elif choice == '2':
            agent = SkilledAgent("Resource_2", [SKILLS[1]], ResourceType.Station.name, Locations.B.name)
            agent.Start()
            print("Resource 2 (Screw) started.")

        elif choice == '3':
            # TransportAgent(name, skills, resource_type, location)
            agent = TransportAgent("Transport_1", [SKILLS[2], SKILLS[3]], ResourceType.Robot.name, Locations.A.name)
            agent.Start()
            print("Transport Agent started.")

        elif choice == '4':
            # ProductAgent(name, process, location)
            agent = ProductAgent("Product_1", ["Drill", "Screw"], Locations.A.name)
            agent.Start()
            print("Product Agent started.")

            
        elif choice == '5':
            agent = SkilledAgent("FMS Tower", [SKILLS[0],SKILLS[1]], ResourceType.Station.name, Locations.D.name)
            agent.Start()
            print("FMS Tower Agent started.")
            
        elif choice == '0':
            print("Exiting...")
            break
        else:
            print("Invalid option.")

if __name__ == "__main__":
    main()
