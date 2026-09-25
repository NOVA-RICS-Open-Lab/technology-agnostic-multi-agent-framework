#include "MessageTransportSystem.h"
#include "tamaf/core/Agent.h"

namespace tamaf {
namespace messaging {

MessageTransportSystem::MessageTransportSystem(tamaf::core::Agent* agent) 
    : myAgent(agent), tcpServer(4000), mtsTaskHandle(nullptr), isRunning(false), listeningPort(4000) {
    queueMutex = xSemaphoreCreateMutex();
}

MessageTransportSystem::~MessageTransportSystem() {
    Stop();
    if (queueMutex != nullptr) {
        vSemaphoreDelete(queueMutex);
    }
}

void MessageTransportSystem::SetPort(int port) {
    listeningPort = port;
    tcpServer = WiFiServer(listeningPort);
}

void MessageTransportSystem::Start() {
    if (isRunning) return;
    
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
        Serial.print("Failed to create MTSLoopTask, error code: ");
        Serial.println(res);
    } else {
        Serial.println("Successfully created MTSLoopTask.");
    }
}

void MessageTransportSystem::Stop() {
    isRunning = false;
    if (mtsTaskHandle != nullptr) {
        vTaskDelete(mtsTaskHandle);
        mtsTaskHandle = nullptr;
    }
    tcpServer.end();
}

void MessageTransportSystem::MTSLoopTask(void* parameter) {
    MessageTransportSystem* mts = static_cast<MessageTransportSystem*>(parameter);
    
    Serial.println("MTSLoopTask started!");
    
    unsigned long lastPrint = 0;
    while (mts->isRunning) {
        mts->ProcessIncomingClients();
        if (millis() - lastPrint > 5000) {
            Serial.println("MTSLoopTask is running...");
            lastPrint = millis();
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // Yield to watchdog
    }
    
    Serial.println("MTSLoopTask ending!");
    vTaskDelete(NULL);
}

void MessageTransportSystem::ProcessIncomingClients() {
    WiFiClient client = tcpServer.available();
    if (client) {
        if (client.connected()) {
            Serial.println("Incoming connection received on MTS");
            unsigned long timeout = millis();
            while (client.available() < 4) {
                if (millis() - timeout > 2000) {
                    Serial.println("Timeout waiting for 4-byte header");
                    client.stop();
                    return;
                }
                delay(10);
            }
            
            uint8_t header[4];
            client.read(header, 4);
            uint32_t msgSize = (header[0] << 24) | (header[1] << 16) | (header[2] << 8) | header[3];
            
            Serial.print("MTS msgSize: ");
            Serial.println(msgSize);
            
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
            
            Serial.println("MTS Received body:");
            Serial.println(body);
            
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
                    
                    // Enqueue safely
                    if (xSemaphoreTake(queueMutex, portMAX_DELAY) == pdTRUE) {
                        messageQueue.push(acl);
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

bool MessageTransportSystem::TCPSend(const ACLMessage& msg, const String& targetIp, int targetPort) {
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

    // Create and serialize TransportMessage
    // Note: getSender() needs to be constructed depending on agent, 
    // but here we just construct an envelope using the message's sender and first receiver.
    String senderID = msg.getSender().GetFullID();
    String receiverID = "";
    if (msg.getReceivers().size() > 0) {
        receiverID = msg.getReceivers()[0].GetFullID();
    }
    
    Envelope env(senderID, receiverID);
    TransportMessage tm(env, aclStr);

    JsonDocument tmDoc;
    tm.ToJson(tmDoc.to<JsonObject>());
    String tmStr;
    serializeJson(tmDoc, tmStr);
    Serial.print("TCPSend payload: ");
    Serial.println(tmStr);

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
    
    Serial.print("TCPSend resSize: ");
    Serial.print(resSize);
    Serial.print(" body: '");
    Serial.print(responseBody);
    Serial.println("'");
    
    client.stop();
    return (responseBody == "200");
}

bool MessageTransportSystem::ReceiveMessage(ACLMessage& outMsg) {
    bool success = false;
    if (xSemaphoreTake(queueMutex, portMAX_DELAY) == pdTRUE) {
        if (!messageQueue.empty()) {
            outMsg = messageQueue.front();
            messageQueue.pop();
            success = true;
        }
        xSemaphoreGive(queueMutex);
    }
    return success;
}

bool MessageTransportSystem::ReceiveMessage(const ACLMessageTemplate& tmpl, ACLMessage& outMsg) {
    return ReceiveMessage([&tmpl](const ACLMessage& msg) {
        return tmpl.Match(msg);
    }, outMsg);
}

bool MessageTransportSystem::ReceiveMessage(std::function<bool(const ACLMessage&)> matchFunc, ACLMessage& outMsg) {
    bool success = false;
    if (xSemaphoreTake(queueMutex, portMAX_DELAY) == pdTRUE) {
        std::queue<ACLMessage> tempQueue;
        while (!messageQueue.empty()) {
            ACLMessage msg = messageQueue.front();
            messageQueue.pop();
            
            if (!success && matchFunc(msg)) {
                outMsg = msg;
                success = true; // Found match, don't put it in tempQueue
            } else {
                tempQueue.push(msg); // Keep other messages
            }
        }
        // Restore non-matching messages
        messageQueue = tempQueue;
        xSemaphoreGive(queueMutex);
    }
    return success;
}

bool MessageTransportSystem::HasMessages() const {
    bool hasMsg = false;
    // We cast away const to take the mutex
    if (xSemaphoreTake(const_cast<MessageTransportSystem*>(this)->queueMutex, portMAX_DELAY) == pdTRUE) {
        hasMsg = !messageQueue.empty();
        xSemaphoreGive(const_cast<MessageTransportSystem*>(this)->queueMutex);
    }
    return hasMsg;
}

} // namespace messaging
} // namespace tamaf
