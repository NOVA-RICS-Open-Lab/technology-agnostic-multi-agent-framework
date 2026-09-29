import socket
import json

HOST = '0.0.0.0'
PORT = 8000

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    print(f"Python Server listening on {HOST}:{PORT}")
    
    while True:
        conn, addr = s.accept()
        with conn:
            print(f"Connected by {addr}")
            
            data_buffer = b""
            while True:
                chunk = conn.recv(1024)
                if not chunk:
                    break
                data_buffer += chunk
                # Optional: break if we detect end of HTTP body, but closing socket handles it
                
            if data_buffer:
                message_str = data_buffer.decode('utf-8').strip()
                print("\n--- Received Data ---")
                print(message_str)
