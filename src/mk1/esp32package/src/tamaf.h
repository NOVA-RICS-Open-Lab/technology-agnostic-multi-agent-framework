#ifndef TAMAF_H
#define TAMAF_H

// Core Definitions
#include "defines.h"

// Enums
#include "tamaf/enums/LifeCycleState.h"
#include "tamaf/enums/Performative.h"
#include "tamaf/enums/TransportType.h"

// Types
#include "tamaf/types/Address.h"
#include "tamaf/types/AgentID.h"
#include "tamaf/types/ServiceDescription.h"
#include "tamaf/types/AgentDescription.h"

// Messaging
#include "tamaf/messaging/ACLMessage.h"
#include "tamaf/messaging/Envelope.h"
#include "tamaf/messaging/TransportMessage.h"
#include "tamaf/messaging/MessageTransportSystem.h"
#include "tamaf/messaging/ACLMessageTemplate.h"

// Behaviors
#include "tamaf/behaviors/Behavior.h"
#include "tamaf/behaviors/SimpleBehavior.h"
#include "tamaf/behaviors/OneShotBehavior.h"
#include "tamaf/behaviors/CyclicBehavior.h"
#include "tamaf/behaviors/TickerBehavior.h"
#include "tamaf/behaviors/WakerBehavior.h"
#include "tamaf/behaviors/SequentialBehavior.h"
#include "tamaf/behaviors/FSMBehavior.h"

// Core
#include "tamaf/core/Agent.h"

// Protocols
#include "tamaf/protocols/FIPARequestInitiator.h"
#include "tamaf/protocols/FIPARequestResponder.h"
#include "tamaf/protocols/FIPAContractNetInitiator.h"
#include "tamaf/protocols/FIPAContractNetResponder.h"

// EMA
#include "tamaf/ema/AgentManagementOntology.h"
#include "tamaf/ema/EMAInteraction.h"

// Internal Behaviors
#include "tamaf/behaviors/InitializationBehavior.h"
#include "tamaf/behaviors/ClosingBehavior.h"
#include "tamaf/behaviors/HeartBeatBehavior.h"

// More includes will be added here in subsequent steps as modules are built...

#endif // TAMAF_H
