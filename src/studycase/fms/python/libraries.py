import requests
import threading
import time
from constants import CONTROLLER_IP

# Semaphores to limit concurrency, replicating Java's Semaphore(1)
kitt_being_used = threading.Semaphore(1)
hardware_being_used = threading.Semaphore(1)

def execute_transport(skill_id):
    """
    Executes a transport skill.
    Replicates kittens_library.executeTransporte
    """
    url = f"http://{CONTROLLER_IP}/passadeiras?skill={skill_id}"
    return _execute_request(url, kitt_being_used)

def execute_station(skill_id):
    """
    Executes a station skill.
    Replicates executionLibrary.executeStation
    """
    url = f"http://{CONTROLLER_IP}/estacao?skill={skill_id}"
    return _execute_request(url, hardware_being_used)

def execute_skill(skill_url):
    """
    Executes a skill using a full URL.
    Replicates executionLibrary.executeSkill
    """
    # executeSkill in Java doesn't seem to use a semaphore in the provided snippet
    # but let's be safe and use hardware_being_used if it's a station/hardware skill
    return _execute_request(skill_url, hardware_being_used)

def _execute_request(url, semaphore):
    """
    Helper function to perform the HTTP request with semaphore and timeout.
    """
    with semaphore:
        # Replicating Java's Thread.sleep(100)
        time.sleep(0.1)
        
        print(f"Executing request to: {url}")
        try:
            # Replicating Java's long timeout (3600 seconds = 1 hour)
            response = requests.get(url, timeout=3600)
            response.raise_for_status()
            print(f"Request successful: {response.status_code}")
            return response.text
        except requests.exceptions.RequestException as e:
            print(f"Error during request: {e}")
            return None
