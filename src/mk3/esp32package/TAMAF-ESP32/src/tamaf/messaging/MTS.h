#ifndef TAMAF_MTS_H
#define TAMAF_MTS_H

#include <Arduino.h>
#include <WiFi.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <deque>
#include <optional>
#include <functional>
#include "TransportMessage.h"
#include "ACLMessage.h"
#include "ACLMessageTemplate.h"
#include "tamaf/types/Address.h"

namespace tamaf {
namespace core {
    class Agent;
}
}

namespace tamaf {
namespace messaging {

class MTS {
private:
    tamaf::core::Agent* myAgent;
    WiFiServer tcpServer;
    TaskHandle_t mtsTaskHandle;
    std::deque<ACLMessage> messageQueue; 
    SemaphoreHandle_t queueMutex;
    bool isRunning;
    int listeningPort;

    static void MTSLoopTask(void* parameter);
    void ProcessIncomingClients();
    bool TCPSend(const ACLMessage& msg, const tamaf::types::AgentID& receiver);
    bool TCPSend(const ACLMessage& msg, const String& targetIp, int targetPort, const String& receiverID = "");

public:
    MTS(tamaf::core::Agent* agent);
    ~MTS();

    void StartServer(const tamaf::types::Address& address);
    void StopServer();

    // High-level Send: resolves target IP/Port from each receiver in msg.getReceivers()
    bool Send(const ACLMessage& msg);

    // Retrieves a message matching a template (or the next message if tmpl is default)
    std::optional<ACLMessage> Receive(const ACLMessageTemplate& tmpl = ACLMessageTemplate());
    
    // Puts a message back to the front of the queue
    void PutBack(const ACLMessage& msg);

    // Checks if there are messages in the queue
    bool HasMessages() const;
};

} // namespace messaging
} // namespace tamaf

#endif // TAMAF_MTS_H
