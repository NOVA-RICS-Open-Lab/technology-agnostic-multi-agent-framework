#ifndef TAMAF_MESSAGETRANSPORTSYSTEM_H
#define TAMAF_MESSAGETRANSPORTSYSTEM_H

#include <Arduino.h>
#include <WiFi.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <queue>
#include "TransportMessage.h"
#include "ACLMessage.h"
#include "ACLMessageTemplate.h"
#include <functional>

namespace tamaf {
namespace core {
    class Agent;
}
}

namespace tamaf {
namespace messaging {

class MessageTransportSystem {
private:
    tamaf::core::Agent* myAgent;
    WiFiServer tcpServer;
    TaskHandle_t mtsTaskHandle;
    std::queue<ACLMessage> messageQueue; 
    SemaphoreHandle_t queueMutex;
    bool isRunning;
    int listeningPort;

    static void MTSLoopTask(void* parameter);
    void ProcessIncomingClients();

public:
    MessageTransportSystem(tamaf::core::Agent* agent);
    ~MessageTransportSystem();

    void SetPort(int port);
    void Start();
    void Stop();

    // Sends an ACLMessage wrapped in a TransportMessage to the specified IP/Port over TCP
    bool TCPSend(const ACLMessage& msg, const String& targetIp, int targetPort);

    // Called by the AMS to retrieve the next message
    bool ReceiveMessage(ACLMessage& outMsg);
    
    // Retrieve a message matching a template
    bool ReceiveMessage(const ACLMessageTemplate& tmpl, ACLMessage& outMsg);
    
    // Retrieve a message matching a custom function
    bool ReceiveMessage(std::function<bool(const ACLMessage&)> matchFunc, ACLMessage& outMsg);
    
    // Checks if there are messages in the queue
    bool HasMessages() const;
};

} // namespace messaging
} // namespace tamaf

#endif // TAMAF_MESSAGETRANSPORTSYSTEM_H
