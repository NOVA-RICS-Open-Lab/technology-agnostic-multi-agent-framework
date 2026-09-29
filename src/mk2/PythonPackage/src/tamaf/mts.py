import socket
import json
import struct
import threading
import time
import sys
import asyncio
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
        
        self.newMessageEvent = None 
        self.messageQueue: list['ACLMessage'] = []
        self.messageQueueLock = threading.Lock()
        self.messageServer = None
        self.serverOnline = False
        self._stop_event = None

    async def TCPSendAsync(self, transportMessage: 'TransportMessage', receiverAddress: Address):
        messageText = json.dumps(transportMessage.ToDict()).encode('utf-8')
        try:
            reader, writer = await asyncio.open_connection(receiverAddress.ip, receiverAddress.port)
            sock = writer.get_extra_info('socket')
            if sock is not None:
                sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
            header = struct.pack('!I', len(messageText))
            writer.write(header + messageText)
            await writer.drain()

            responseSizeRaw = await reader.readexactly(4)
            responseSize = struct.unpack('!I', responseSizeRaw)[0]
            
            responseRaw = await reader.readexactly(responseSize)
            response_code = responseRaw.decode("utf-8")
            
            writer.close()
            await writer.wait_closed()
            
            if int(response_code) > 299:
                return False
            return True
        except Exception as e:
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
                
                # Fire and forget async task
                loop = self.agent.ams.get_shared_loop()
                asyncio.run_coroutine_threadsafe(self.TCPSendAsync(transportmessage, receiverAddress), loop)
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

    async def handle_client(self, reader, writer):
        peer = writer.get_extra_info('peername')
        sock = writer.get_extra_info('socket')
        if sock is not None:
            sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        print(f"{self.agent.agentDescription.agentid.GetFullID()}: Accepted connection from {peer}")
        try:
            messageSizeRaw = await reader.readexactly(4)
            messageSize = struct.unpack('!I', messageSizeRaw)[0]
            print(f"Header received: msgSize={messageSize}")
            
            messageRaw = await reader.readexactly(messageSize)
            messageDict = json.loads(messageRaw.decode("utf-8"))
            transportMessage = TransportMessage.FromDict(messageDict)
            aclMessage = transportMessage.aclmessage
            
            print(f"NOVA MENSAGEM CHEGOU NO SERVIDOR TCP: {messageDict}")

            with self.messageQueueLock:
                self.messageQueue.append(aclMessage)
                if self.newMessageEvent:
                    self.newMessageEvent.set()

            responseCode = b"200"
            header = struct.pack('!I', len(responseCode))
            writer.write(header + responseCode)
            await writer.drain()
        except Exception as e:
            print(f"{self.agent.agentDescription.agentid.GetFullID()}: Error processing incoming message: {e}")
            try:
                responseCode = b"400"
                header = struct.pack('!I', len(responseCode))
                writer.write(header + responseCode)
                await writer.drain()
            except Exception:
                pass
        finally:
            writer.close()
            await writer.wait_closed()
        
    async def TCPServerLoop(self, address: Address):
        if self._stop_event is None:
            self._stop_event = asyncio.Event()
        
        while not self._stop_event.is_set():
            try:
                self.messageServer = await asyncio.start_server(
                    self.handle_client, 
                    address.ip, 
                    address.port,
                    reuse_address=True
                )
                print(f"{self.agent.agentDescription.agentid.GetFullID()}: Agent Port Setup")
                self.serverOnline = True
                
                async with self.messageServer:
                    # Wait until stop event is set or server fails
                    wait_task = asyncio.create_task(self._stop_event.wait())
                    serve_task = asyncio.create_task(self.messageServer.serve_forever())
                    
                    done, pending = await asyncio.wait(
                        [wait_task, serve_task],
                        return_when=asyncio.FIRST_COMPLETED
                    )
                    
                    for task in pending:
                        task.cancel()
                        
                self.serverOnline = False
                self.messageServer = None
                break

            except Exception as e:
                self.serverOnline = False
                self.messageServer = None
                if not self._stop_event.is_set():
                    if address.port == self.registeraddress.port:
                        print(f"{self.agent.agentDescription.agentid.GetFullID()}: Port {address.port} busy. An Agent already exists there \n ErrorCode: {e}")
                        await asyncio.sleep(DEFAULT_REGISTRATION_PORT_RETRY_TIME)
                        continue

                    if address.port == self.emaAddress.port:
                        print(f"{self.agent.agentDescription.agentid.GetFullID()}: Port {address.port} busy. EMA already exists in this environment \n ErrorCode: {e}")                    
                        self.agent.ams.Shutdown()
                        await asyncio.sleep(DEFAULT_REGISTRATION_PORT_RETRY_TIME)
                        continue
                    
                    print(f"{self.agent.agentDescription.agentid.GetFullID()}: Port {address.port} busy. Retrying... \n ErrorCode: {e}")
                    self.agent.ams.Shutdown()
                    await asyncio.sleep(DEFAULT_REGISTRATION_PORT_RETRY_TIME)
            
    def StartTCPServer(self, address: Address):
        self.StopMessageServer()
        loop = self.agent.ams.get_shared_loop()
        
        if self._stop_event is not None:
            import threading
            if threading.current_thread() is self.agent.ams._shared_loop_thread:
                self._stop_event.clear()
            else:
                asyncio.run_coroutine_threadsafe(self._clear_stop_event(), loop).result()
        
        self.serverTask = asyncio.run_coroutine_threadsafe(self.TCPServerLoop(address), loop)
        self.agent.agentDescription.agentid.SetPort(address.port)

    async def _clear_stop_event(self):
        if self._stop_event:
            self._stop_event.clear()

    async def _set_stop_event(self):
        if self._stop_event:
            self._stop_event.set()

    def StopMessageServer(self):
        if hasattr(self, 'serverTask') and self.serverTask is not None:
            old_task = self.serverTask
            self.serverTask = None
            old_event = self._stop_event
            self._stop_event = asyncio.Event()
            
            # Send dummy connection to unblock
            try:
                temp_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                temp_sock.settimeout(0.5)
                temp_sock.connect((self.emaAddress.ip, self.emaAddress.port))
                temp_sock.close()
            except:
                pass
            
            async def wait_for_it():
                if old_event:
                    old_event.set()
                try:
                    await asyncio.wrap_future(old_task)
                except (asyncio.CancelledError, Exception):
                    pass

            loop = self.agent.ams.get_shared_loop()
            
            import threading
            if threading.current_thread() is self.agent.ams._shared_loop_thread:
                asyncio.ensure_future(wait_for_it(), loop=loop)
            else:
                future = asyncio.run_coroutine_threadsafe(wait_for_it(), loop)
                future.result() # Wait for shutdown
            
        self.serverOnline = False
