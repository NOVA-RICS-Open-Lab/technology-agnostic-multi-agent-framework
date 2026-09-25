#include "MTS.h"
#include "tamaf/core/Agent.h"

namespace tamaf {
namespace messaging {

MTS::MTS(tamaf::core::Agent* agent) 
    : myAgent(agent), tcpServer(4000), mtsTaskHandle(nullptr), isRunning(false), listeningPort(4000) {
    queueMutex = xSemaphoreCreateMutex();
}

MTS::~MTS() {
    StopServer();
    if (queueMutex != nullptr) {
        vSemaphoreDelete(queueMutex);
    }
}

void MTS::StartServer(const tamaf::types::Address& address) {
    StopServer();
    
    listeningPort = address.getPort();
    tcpServer = WiFiServer(listeningPort);
    if (myAgent) {
        myAgent->SetPort(listeningPort);
        myAgent->logDebug(1, "Agent active at: " + myAgent->GetAgentID().getAddress().GetString());
    }
    
    tcpServer.begin();
    isRunning = true;
    
    // Start FreeRTOS Task pinned to Core 0 (leaving Core 1 for Arduino loop/AMS)
    BaseType_t res = xTaskCreatePinnedToCore(
        MTSLoopTask,
        "MTSLoop",
        8192,           // Stack size
        this,           // Parameter
        1,              // Priority
        &mtsTaskHandle,
        0               // Core 0
    );
    if (res != pdPASS) {
        if (myAgent) myAgent->logDebug("Failed to create MTSLoopTask");
    } else {
        if (myAgent) myAgent->logDebug("Successfully created MTSLoopTask");
    }
}

void MTS::StopServer() {
    isRunning = false;
    if (mtsTaskHandle != nullptr) {
        vTaskDelete(mtsTaskHandle);
        mtsTaskHandle = nullptr;
    }
    tcpServer.end();
}

void MTS::MTSLoopTask(void* parameter) {
    MTS* mts = static_cast<MTS*>(parameter);
    
    while (mts->isRunning) {
        mts->ProcessIncomingClients();
        vTaskDelay(pdMS_TO_TICKS(10)); // Yield to watchdog
    }
    
    vTaskDelete(NULL);
}

void MTS::ProcessIncomingClients() {
    WiFiClient client = tcpServer.available();
    if (client) {
        if (client.connected()) {
            if (myAgent) myAgent->logDebug("Incoming connection received on MTS");
            unsigned long timeout = millis();
            while (client.available() < 4) {
                if (millis() - timeout > 2000) {
                    client.stop();
                    return;
                }
                delay(10);
            }
            
            uint8_t header[4];
            client.read(header, 4);
            uint32_t msgSize = (header[0] << 24) | (header[1] << 16) | (header[2] << 8) | header[3];
            
            // Limit payload size to avoid memory issues (e.g., 64KB)
            if (msgSize > 65535) {
                Serial.println("Payload too large");
                client.stop();
                return;
            }
            
            timeout = millis();
            String body = "";
            body.reserve(msgSize);
            uint32_t bytesRead = 0;
            
            while (bytesRead < msgSize) {
                if (client.available()) {
                    body += (char)client.read();
                    bytesRead++;
                    timeout = millis(); // Reset timeout on successful read
                } else {
                    if (millis() - timeout > 2000) {
                        Serial.println("Timeout waiting for body");
                        client.stop();
                        return;
                    }
                    delay(5);
                }
            }
            
            // Parse TransportMessage
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, body);
            if (!err) {
                TransportMessage tm = TransportMessage::FromJson(doc.as<JsonVariant>());
                
                // Parse inner ACLMessage
                JsonDocument aclDoc;
                DeserializationError aclErr = deserializeJson(aclDoc, tm.getPayload());
                if (!aclErr) {
                    ACLMessage acl = ACLMessage::FromJson(aclDoc.as<JsonVariant>());
                    if (myAgent) myAgent->logDebug(1, "Received message from " + acl.getSender().getName() + " (Performative: " + tamaf::enums::PerformativeToLogString(acl.getPerformative()) + ")");
                    
                    // Enqueue safely
                    if (xSemaphoreTake(queueMutex, portMAX_DELAY) == pdTRUE) {
                        messageQueue.push_back(acl);
                        xSemaphoreGive(queueMutex);
                        
                        // Notify Agent to unblock waiting behaviors
                        if (myAgent) {
                            myAgent->NotifyNewMessage();
                        }
                    }
                } else {
                    Serial.print("Failed to parse inner ACLMessage: ");
                    Serial.println(aclErr.c_str());
                }
            } else {
                Serial.print("Failed to parse TransportMessage: ");
                Serial.println(err.c_str());
            }
            
            // Send Response "200"
            String res = "200";
            uint32_t resSize = res.length();
            uint8_t resHeader[4];
            resHeader[0] = (resSize >> 24) & 0xFF;
            resHeader[1] = (resSize >> 16) & 0xFF;
            resHeader[2] = (resSize >> 8) & 0xFF;
            resHeader[3] = resSize & 0xFF;
            
            client.write(resHeader, 4);
            client.print(res);
        }
        client.stop();
    }
}

bool MTS::Send(const ACLMessage& msg) {
    if (msg.getReceivers().empty()) return false;
    
    // Copy the message to safely populate sender if missing
    ACLMessage messageToSend = msg;
    if (messageToSend.getSender().getName().isEmpty() && myAgent) {
        messageToSend.setSender(myAgent->GetAgentID());
    }
    
    bool allSent = true;
    for (const auto& receiver : messageToSend.getReceivers()) {
        if (!TCPSend(messageToSend, receiver)) {
            allSent = false;
        }
    }
    return allSent;
}

bool MTS::TCPSend(const ACLMessage& msg, const tamaf::types::AgentID& receiver) {
    return TCPSend(msg, receiver.getAddress().getIp(), receiver.getAddress().getPort(), receiver.GetFullID());
}

bool MTS::TCPSend(const ACLMessage& msg, const String& targetIp, int targetPort, const String& receiverID) {
    WiFiClient client;
    if (!client.connect(targetIp.c_str(), targetPort)) {
        Serial.print("TCPSend connection failed to ");
        Serial.print(targetIp);
        Serial.print(":");
        Serial.println(targetPort);
        return false;
    }

    // Serialize ACLMessage
    JsonDocument aclDoc;
    msg.ToJson(aclDoc.to<JsonObject>());
    String aclStr;
    serializeJson(aclDoc, aclStr);

    String senderID = msg.getSender().GetFullID();
    String actualReceiverID = receiverID;
    if (actualReceiverID.isEmpty() && !msg.getReceivers().empty()) {
        actualReceiverID = msg.getReceivers()[0].GetFullID();
    }
    
    Envelope env(senderID, actualReceiverID);
    TransportMessage tm(env, aclStr);

    JsonDocument tmDoc;
    tm.ToJson(tmDoc.to<JsonObject>());
    String tmStr;
    serializeJson(tmDoc, tmStr);
    if (myAgent) myAgent->logDebug(1, "Sent message to " + actualReceiverID + " (Performative: " + tamaf::enums::PerformativeToLogString(msg.getPerformative()) + ")");
    if (myAgent) myAgent->logDebug(2, "TCPSend payload: " + tmStr);

    // Send size header (4 bytes, big endian)
    uint32_t msgSize = tmStr.length();
    uint8_t header[4];
    header[0] = (msgSize >> 24) & 0xFF;
    header[1] = (msgSize >> 16) & 0xFF;
    header[2] = (msgSize >> 8) & 0xFF;
    header[3] = msgSize & 0xFF;
    
    client.write(header, 4);
    
    // Ensure the entire string is written
    const uint8_t* payloadBytes = (const uint8_t*)tmStr.c_str();
    uint32_t bytesWritten = 0;
    while (bytesWritten < msgSize) {
        size_t written = client.write(payloadBytes + bytesWritten, msgSize - bytesWritten);
        if (written > 0) {
            bytesWritten += written;
        } else {
            delay(10);
        }
    }
    client.flush();

    // Wait for response size (4 bytes)
    unsigned long timeout = millis();
    while (client.available() < 4) {
        if (millis() - timeout > 5000) {
            client.stop();
            return false;
        }
        delay(10);
    }
    
    uint8_t resHeader[4];
    client.read(resHeader, 4);
    uint32_t resSize = (resHeader[0] << 24) | (resHeader[1] << 16) | (resHeader[2] << 8) | resHeader[3];
    
    // Wait for response body
    timeout = millis();
    while (client.available() < resSize) {
        if (millis() - timeout > 5000) {
            client.stop();
            return false;
        }
        delay(10);
    }
    
    String responseBody = "";
    for (uint32_t i = 0; i < resSize; i++) {
        responseBody += (char)client.read();
    }
    
    if (myAgent) myAgent->logDebug("TCPSend response: " + responseBody);
    
    client.stop();
    return (responseBody == "200");
}

std::optional<ACLMessage> MTS::Receive(const ACLMessageTemplate& tmpl) {
    if (xSemaphoreTake(queueMutex, portMAX_DELAY) == pdTRUE) {
        for (auto it = messageQueue.begin(); it != messageQueue.end(); ++it) {
            if (tmpl.Match(*it)) {
                ACLMessage msg = std::move(*it);
                messageQueue.erase(it);
                xSemaphoreGive(queueMutex);
                return msg;
            }
        }
        xSemaphoreGive(queueMutex);
    }
    return std::nullopt;
}

void MTS::PutBack(const ACLMessage& msg) {
    if (xSemaphoreTake(queueMutex, portMAX_DELAY) == pdTRUE) {
        messageQueue.push_front(msg);
        xSemaphoreGive(queueMutex);
    }
}

bool MTS::HasMessages() const {
    bool hasMsg = false;
    // We cast away const to take the mutex
    if (xSemaphoreTake(const_cast<MTS*>(this)->queueMutex, portMAX_DELAY) == pdTRUE) {
        hasMsg = !messageQueue.empty();
        xSemaphoreGive(const_cast<MTS*>(this)->queueMutex);
    }
    return hasMsg;
}

} // namespace messaging
} // namespace tamaf
