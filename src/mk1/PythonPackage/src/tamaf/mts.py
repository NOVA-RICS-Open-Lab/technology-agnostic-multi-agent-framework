import socket
import json
import struct
import threading
import time
import sys
from typing import TYPE_CHECKING
from .address import Address
from .transportmessage import TransportMessage
from .utils import GetLocalIP
from .defines import (
    DEFAULT_EMA_PORT,
    DEFAULT_EMA_REGISTER_PORT,
    DEFAULT_MESSAGE_TIMEOUT_TIME,
    DEFAULT_TCP_BACKLOG,
    DEFAULT_REGISTRATION_PORT_RETRY_TIME,
    DEFAULT_LOOP_STABILIZATION_TIME,
    SERVER_STARTING_TIME,
    DEFAULT_THREAD_JOIN_TIMEOUT_TIME,
    LOCALHOST
)

if TYPE_CHECKING:
    from .agent import Agent
    from .aclmessage import ACLMessage
    from .aclmessagetemplate import ACLMessageTemplate

class MTS:

    def __init__(self, agent: 'Agent', 
                 emaAddress: Address = None, 
                 registerAddress: Address = None):
        
        self.agent = agent
        self.emaAddress = emaAddress if emaAddress else Address(ip=GetLocalIP(), port=DEFAULT_EMA_PORT)
        self.registeraddress = registerAddress if registerAddress else Address(ip=GetLocalIP(), port=DEFAULT_EMA_REGISTER_PORT)
        
        self.newMessage = False
        self.messageQueue: list['ACLMessage'] = []
        self.messageQueueLock = threading.Lock()
        self.messageServer = None
        self.serverOnline = False
        self.messageServerThread = None
        self.serverSocket = None
        self._stop_event = threading.Event()

    def TCPSend(self, transportMessage: 'TransportMessage', receiverAddress: Address) -> bool:
        messageText = json.dumps(transportMessage.ToDict()).encode('utf-8')

        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as clientSocket:
                clientSocket.connect(receiverAddress.GetTuple())

                header = struct.pack('!I', len(messageText))
                clientSocket.sendall(header + messageText)

                responseSizeRaw = self.recvAll(clientSocket, 4)
                if responseSizeRaw is None:
                    return False
                
                responseSize = struct.unpack('!I', responseSizeRaw)[0]
                
                response = self.recvAll(clientSocket, responseSize)
                if response is None:
                    return False

                response_code = response.decode("utf-8")
                if int(response_code) > 299:
                    return False

                return True
        except Exception:
            return False

    def Send(self, aclMessage: 'ACLMessage') -> bool:
        for receiver in aclMessage.receiver:
            try:
                if receiver.address.ip == self.agent.agentDescription.agentid.address.ip:
                    receiverAddress = Address(LOCALHOST, receiver.address.port)
                else:
                    receiverAddress = receiver.address

                print("\nNOVA MENSAGEM SAIU DO SERVIDOR TCP")
                print(aclMessage.ToDict())
                print("\n")

                transportmessage = TransportMessage(aclMessage, aclMessage.sender.address, receiverAddress)
                
                if not self.TCPSend(transportmessage, receiverAddress):
                    return False
            except Exception as e:
                print(f"{self.agent.agentDescription.agentid.GetFullID()} Error sending message to {receiver.GetFullID()}: {e}")
                return False
        return True

    def Receive(self, aclMessageTemplate: 'ACLMessageTemplate'):
        with self.messageQueueLock:
            for aclMessage in self.messageQueue:
                if aclMessageTemplate.Matches(aclMessage):
                    self.messageQueue.remove(aclMessage)
                    return aclMessage
        return None

    def PutBack(self, aclMessage: 'ACLMessage'):
        with self.messageQueueLock:
            self.messageQueue.insert(0, aclMessage)

    def recvAll(self, sock: socket.socket, messageLength: int):
        data = bytearray()
        while len(data) < messageLength:
            packet = sock.recv(messageLength - len(data))
            if not packet:
                return None
            data.extend(packet)
        return data
        
    def TCPServerLoop(self, address: Address):
        while not self._stop_event.is_set():
            try:
                with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as serverSocket:
                    self.serverSocket = serverSocket
                    if sys.platform == 'win32':
                        socketOption = socket.SO_EXCLUSIVEADDRUSE
                    else:
                        socketOption = socket.SO_REUSEADDR
                    
                    serverSocket.setsockopt(socket.SOL_SOCKET, socketOption, 1)
                    serverSocket.settimeout(DEFAULT_MESSAGE_TIMEOUT_TIME)
                    serverSocket.bind(address.GetTuple())
                    serverSocket.listen(DEFAULT_TCP_BACKLOG)

                    print(f"{self.agent.agentDescription.agentid.GetFullID()}: Agent Port Setup")
                    self.serverOnline = True
                    
                    while not self._stop_event.is_set():
                        try:
                            clientSocket, _ = serverSocket.accept()
                        except socket.timeout:
                            continue
                        except Exception:
                            break

                        with clientSocket:
                            print(f"{self.agent.agentDescription.agentid.GetFullID()}: Accepted connection from {clientSocket.getpeername()}")
                            try:
                                messageSizeRaw = self.recvAll(clientSocket, 4)
                                if messageSizeRaw is None:
                                    print("Failed to receive 4 byte size header")
                                    continue
                                
                                messageSize = struct.unpack('!I', messageSizeRaw)[0]
                                print(f"Header received: msgSize={messageSize}")
                                messageRaw = self.recvAll(clientSocket, messageSize)
                                if messageRaw is None:
                                    continue

                                messageDict = json.loads(messageRaw.decode("utf-8"))
                                transportMessage = TransportMessage.FromDict(messageDict)
                                aclMessage = transportMessage.aclmessage
                                
                                print(F"NOVA MENSAGEM CHEGOU NO SERVIDOR TCP: {messageDict}")

                                with self.messageQueueLock:
                                    self.messageQueue.append(aclMessage)
                                    self.newMessage = True

                                responseCode = b"200"
                                header = struct.pack('!I', len(responseCode))
                                clientSocket.sendall(header + responseCode)
                            except Exception as e:
                                print(f"{self.agent.agentDescription.agentid.GetFullID()}: Error processing incoming message: {e}")
                                responseCode = b"400"
                                header = struct.pack('!I', len(responseCode))
                                clientSocket.sendall(header + responseCode)
                    
                    self.serverOnline = False
                    self.serverSocket = None

            except Exception as e:
                self.serverOnline = False
                self.serverSocket = None
                if not self._stop_event.is_set():
                                            
                    if address.port == self.registeraddress.port:
                        print(f"{self.agent.agentDescription.agentid.GetFullID()}: Port {address.port} busy. An Agent already exists there \n ErrorCode: {e}")
                        time.sleep(DEFAULT_REGISTRATION_PORT_RETRY_TIME)
                        continue

                    if address.port == self.emaAddress.port:
                        print(f"{self.agent.agentDescription.agentid.GetFullID()}: Port {address.port} busy. EMA already exists in this environment \n ErrorCode: {e}")                    
                        self.agent.ams.Shutdown()
                        time.sleep(DEFAULT_REGISTRATION_PORT_RETRY_TIME)
                        continue
                    
                    print(f"{self.agent.agentDescription.agentid.GetFullID()}: Port {address.port} busy. Retrying... \n ErrorCode: {e}")
                    self.agent.ams.Shutdown()

                    time.sleep(DEFAULT_REGISTRATION_PORT_RETRY_TIME)
            time.sleep(DEFAULT_LOOP_STABILIZATION_TIME)

    def StartTCPServer(self, address: Address):
        self.StopMessageServer()
        
        self._stop_event.clear()
        self.messageServerThread = threading.Thread(
            target=self.TCPServerLoop, 
            args=(address,), 
            daemon=True
        )
        self.agent.agentDescription.agentid.SetPort(address.port)
        self.messageServerThread.start()
        time.sleep(SERVER_STARTING_TIME)

    def StopMessageServer(self):
        self._stop_event.set()
        
        if self.serverSocket:
            try:
                self.serverSocket.close()
            except Exception:
                pass

        if self.messageServerThread:
            self.messageServerThread.join(timeout=DEFAULT_THREAD_JOIN_TIMEOUT_TIME)
            self.messageServerThread = None
        
        self.serverOnline = False
        self.serverSocket = None
