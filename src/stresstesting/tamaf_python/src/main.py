import sys
import os
import time
import argparse
import subprocess

# Expand the default port range before TAMAF imports to accommodate up to 500 agents
os.environ["AVAILABLE_PORTS_END"] = "5000"
# Increase heartbeat interval to 60s to reduce keepalive spam during sequential registration
os.environ["DEFAULT_EMA_HEARTBEAT_INTERVAL"] = "60000"


# Add the path to TAMAF python package to sys.path
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', 'mk3', 'PythonPackage', 'src')))

from tamaf import LaunchEMA
from receiver import ReceiverAgent
from sender import SenderAgent

def run_test(num_agents, num_test):
    print(f"==================================================")
    print(f"Starting TAMAF test with {num_agents} sender agents... test number {num_test}")
    print(f"==================================================")
    
    ema = LaunchEMA(debug=False)
    time.sleep(1) # Give EMA time to start
    
    receiver = ReceiverAgent("receiver")
    receiver.Start()
    
    # Wait for receiver to register
    while receiver.ams.GetLifeCycleState().name != "ACTIVE":
        if receiver.ams.GetLifeCycleState().name == "CLOSING":
            print("Warning: Receiver closed unexpectedly during registration.")
            break
        time.sleep(0.05)

    
    receiver_ip = receiver.agentDescription.agentid.address.ip
    receiver_port = receiver.agentDescription.agentid.address.port

    senders = []
    
    start_registration = time.time()
    trigger_time = time.time() + 10 + (num_agents * 0.5)  # Buffer for sequential registration
    
    for i in range(num_agents):
        t1 = time.time()
        sender = SenderAgent(f"sender{i+1}", trigger_time, receiver_ip, receiver_port)
        sender.Start()
        senders.append(sender)
        
        while sender.ams.GetLifeCycleState().name != "ACTIVE":
            if sender.ams.GetLifeCycleState().name == "CLOSING":
                print(f"Warning: {sender.getName()} closed unexpectedly during registration.")
                break
            time.sleep(0.01)
        t2 = time.time()
        print(f"sender{i+1} took {t2-t1:.4f} seconds to register.")
    
    end_registration = time.time()
    registration_time = end_registration - start_registration
    
    print(f"SUCCESS: All {num_agents} agents are fully registered and ACTIVE in the EMA.")
    print(f"Registration took {registration_time:.4f} seconds. Waiting for trigger time...")
    
    # Wait until past trigger time
    wait_time = trigger_time - time.time()
    if wait_time > 0:
        time.sleep(wait_time + 2) # Buffer to allow all ACKs to arrive
    else:
        time.sleep(2)
        
    print("Collecting results...")
    # Collect results
    all_done = False
    timeout = time.time() + 20 # Max wait 20 secs for ACKs
    while not all_done and time.time() < timeout:
        all_done = True
        for sender in senders:
            if getattr(sender, 'rtt', None) is None:
                all_done = False
                break
        if not all_done:
            time.sleep(0.5)
            
    if time.time() >= timeout:
        print("WARNING: Timeout reached while waiting for ACKs.")
    else:
        print(f"Test number {num_test} with {num_agents} agents completed successfully.")
    
    results_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'results'))
    if not os.path.exists(results_dir):
        os.makedirs(results_dir)
        
    with open(os.path.join(results_dir, f"{num_test}_test_{num_agents}_agent_testresults.txt"), "w") as f:
        f.write(f"Registration Time: {registration_time}\n")
        f.write("Round Trip Times:\n")
        for sender in senders:
            rtt_val = sender.rtt if sender.rtt is not None else "TIMEOUT"
            f.write(f"{sender.agentDescription.agentid.name}: {rtt_val}\n")
            
    # Stop agents
    for sender in senders:
        sender.Stop()
    receiver.Stop()
    ema.Stop()
    
    time.sleep(2)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='TAMAF Stress Test')
    
    parser.add_argument('--scales', type=int, nargs='+', help='Scales to test (if omitted, runs all scales)', default=None)
    
    # Hidden arguments passed to the subprocess
    parser.add_argument('--execute-test', type=int, help=argparse.SUPPRESS, default=None)
    parser.add_argument('--num-test', type=int, help=argparse.SUPPRESS, default=None)
    
    args = parser.parse_args()

    # If --execute-test is present, we are inside the subprocess.
    if args.execute_test is not None:
        # Pass BOTH the scale and the loop number to your function.
        # Note: If your function is defined as def run_test(num_test, scale): swap these variables!
        run_test(args.execute_test, args.num_test)
        
    # Otherwise, we are in the main process. Run the 10x loop.
    else:
        if args.scales is not None:
            test_scales = args.scales
        else:
            test_scales = [1, 5, 10, 15, 20, 30, 40, 50, 75, 100, 200]
            
        number_of_tests = 10

        for i in range(1, number_of_tests + 1):
            print(f"\n=== Starting Test Loop {i}/{number_of_tests} ===")
            for scale in test_scales:
                print(f"Running test for scale {scale} (Run {i}) in a new process...")
                
                # Pass BOTH the scale and the current loop iteration (i) to the subprocess
                subprocess.run([
                    sys.executable, __file__, 
                    "--execute-test", str(scale), 
                    "--num-test", str(i)
                ])
                
                print(f"Waiting 1 seconds for ports to be fully released before the next test...")
                time.sleep(1)

