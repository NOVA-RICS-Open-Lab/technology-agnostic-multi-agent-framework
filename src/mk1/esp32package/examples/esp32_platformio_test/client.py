import socket
import json
import time
import sys

if len(sys.argv) < 2:
    print("Usage: python client.py <ESP32_IP>")
    sys.exit(1)

ESP32_IP = sys.argv[1]
PORT = 4000

# TAMAF ACLMessage Envelope wrapper
acl_message = {
    "performative": "REQUEST",
    "sender": "PythonClient@192.168.0.10:8000",
    "receivers": [f"MyAgent@{ESP32_IP}:4000"],
    "content": "Hello from Python!",
    "conversationId": "test-conv-1"
}

transport_message = {
    "envelope": {
        "sender": "PythonClient@192.168.0.10:8000",
        "receiver": f"MyAgent@{ESP32_IP}:4000"
    },
    "payload": json.dumps(acl_message)
}

body = json.dumps(transport_message)

http_request = (
    "POST / HTTP/1.1\r\n"
    f"Host: {ESP32_IP}:{PORT}\r\n"
    "Content-Type: application/json\r\n"
    f"Content-Length: {len(body)}\r\n"
    "\r\n"
    f"{body}"
)

print(f"Sending message to {ESP32_IP}:{PORT}...")
try:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((ESP32_IP, PORT))
        s.sendall(http_request.encode('utf-8'))
    print("Message sent successfully!")
except Exception as e:
    print(f"Failed to send: {e}")
