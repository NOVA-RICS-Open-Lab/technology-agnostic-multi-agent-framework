#include "Agent.h"
#include "tamaf/behaviors/InitializationBehavior.h"
#include "tamaf/behaviors/ClosingBehavior.h"
#include "tamaf/behaviors/HeartBeatBehavior.h"
#include "tamaf/behaviors/ModifyAgentEMABehavior.h"
#include "defines.h"
#include <WiFi.h>

namespace tamaf {
namespace core {

Agent::Agent(const String& name, const tamaf::types::Address& address, int debugLevel)
    : Agent(name, address, (address.getPort() > 0 ? address : tamaf::types::Address(LOCALHOST, DEFAULT_EMA_REGISTER_PORT)), debugLevel) {}

Agent::Agent(const String& name, const tamaf::types::Address& address, const tamaf::types::Address& regAddress, int debugLevel)
    : agentDescription(tamaf::types::AgentID(name, address)), mts(this), ams(this), debugLevel_(debugLevel) {
    
    emaAddress = tamaf::types::Address(LOCALHOST, DEFAULT_EMA_PORT);
    registerAddress = regAddress;
    logDebug(1, "Agent Born: " + GetAgentID().getName());
}

void Agent::SetPort(int port) {
    agentDescription.setPort(port);
}

void Agent::logDebug(const String& message) {
    logDebug(2, message);
}

void Agent::logDebug(int level, const String& message) {
    if (debugLevel_ >= level) {
        const char* prefix = (level == 1) ? "[DEBUG-L1]" : "[DEBUG-L2]";
        Serial.print(prefix);
        Serial.print("[");
        Serial.print(GetAgentID().getName());
        Serial.print("] ");
        Serial.println(message);
    }
}

Agent::~Agent() {
    Stop();
}

void Agent::Start() {
    if (ams.IsRunning()) return;
    
    // Auto-update Agent ID IP to active WiFi local IP if previously unassigned (0.0.0.0 or empty)
    if (WiFi.status() == WL_CONNECTED) {
        String currentIp = GetAgentID().getAddress().getIp();
        if (currentIp == "0.0.0.0" || currentIp == "127.0.0.1" || currentIp == "") {
            tamaf::types::Address newAddr(WiFi.localIP().toString(), GetAgentID().getAddress().getPort());
            agentDescription.setAddress(newAddr);
        }
        if (registerAddress.getIp() == "0.0.0.0" || registerAddress.getIp() == "127.0.0.1" || registerAddress.getIp() == "") {
            registerAddress = tamaf::types::Address(WiFi.localIP().toString(), registerAddress.getPort());
        }
    }

    ams.Start();
    logDebug(1, "Agent Initiated");
}

void Agent::Stop() {
    ams.Stop();
}

void Agent::Suspend() {
    ams.Suspend();
}

void Agent::Resume() {
    ams.Resume();
}

void Agent::Loop() {
    ams.Loop();
}

tamaf::enums::LifeCycleState Agent::GetLifeCycleState() const {
    return ams.GetLifeCycleState();
}

void Agent::SetLifeCycleState(tamaf::enums::LifeCycleState state) {
    ams.SetLifeCycleState(state);
}

bool Agent::IsSuspended() const {
    return ams.IsSuspended();
}

bool Agent::IsActive() const {
    return ams.IsActive();
}

bool Agent::IsInitiated() const {
    return ams.IsInitiated();
}

bool Agent::IsRunning() const {
    return ams.IsRunning();
}

void Agent::SetHeartBeatBehavior(tamaf::behaviors::HeartBeatBehavior* hb) {
    ams.SetHeartBeatBehavior(hb);
}

tamaf::behaviors::HeartBeatBehavior* Agent::GetHeartBeatBehavior() const {
    return ams.GetHeartBeatBehavior();
}

bool Agent::Send(const tamaf::messaging::ACLMessage& msg) {
    return mts.Send(msg);
}

bool Agent::Receive(tamaf::messaging::ACLMessage& outMsg) {
    auto opt = mts.Receive();
    if (opt.has_value()) {
        outMsg = std::move(opt.value());
        return true;
    }
    return false;
}

std::optional<tamaf::messaging::ACLMessage> Agent::Receive(const tamaf::messaging::ACLMessageTemplate& tmpl) {
    return mts.Receive(tmpl);
}

void Agent::PutBack(const tamaf::messaging::ACLMessage& msg) {
    mts.PutBack(msg);
}

void Agent::NotifyNewMessage() {
    ams.NotifyNewMessage();
}

void Agent::AddBehavior(tamaf::behaviors::Behavior* behavior) {
    ams.AddBehavior(behavior);
}

void Agent::RemoveBehavior(tamaf::behaviors::Behavior* behavior) {
    ams.RemoveBehavior(behavior);
}

void Agent::BlockBehavior(tamaf::behaviors::Behavior* behavior) {
    ams.BlockBehavior(behavior);
}

void Agent::UnblockBehaviors() {
    ams.UnblockBehaviors();
}

tamaf::messaging::MTS& Agent::getMTS() {
    return mts;
}

const tamaf::messaging::MTS& Agent::getMTS() const {
    return mts;
}

const tamaf::types::AgentID& Agent::GetAgentID() const {
    return agentDescription.getAgentId();
}

const tamaf::types::AgentDescription& Agent::GetAgentDescription() const {
    return agentDescription;
}

tamaf::types::AgentDescription Agent::GetAgentDescriptionTemplate() const {
    return tamaf::types::AgentDescription(GetAgentID());
}

void Agent::UpdateAgentDescription(const tamaf::types::AgentDescription& newDesc) {
    agentDescription.clearServices();
    for (const auto& service : newDesc.getServices()) {
        agentDescription.addService(service);
    }
    AddBehavior(new tamaf::behaviors::ModifyAgentEMABehavior(this));
}

const tamaf::types::Address& Agent::GetEMAAddress() const {
    return emaAddress;
}

void Agent::setEMAAddress(const tamaf::types::Address& address) {
    emaAddress = address;
}

const tamaf::types::Address& Agent::GetRegisterAddress() const {
    return registerAddress;
}

void Agent::SetRegisterAddress(const tamaf::types::Address& address) {
    registerAddress = address;
}

} // namespace core
} // namespace tamaf
