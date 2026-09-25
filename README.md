<div  align="center"> 

# Technology Agnostic Multi-Agent Framework

[![License: MIT](https://img.shields.io/badge/License-MIT-red.svg)](https://opensource.org/licenses/MIT)
<!-- [![DOI](https://img.shields.io/badge/DOI-ICPS_2024-darkblue)]() -->
[![NOVA SST: Thesis](https://img.shields.io/badge/NOVA_SST-Thesis-blue)]()

[Raúl Mestre Dinis](https://scholar.google.pt/citations?user=k1GIyqcAAAAJ&hl=pt-PT)<sup>1</sup>,
[António Pegado](https://scholar.google.com/citations?user=k7npfUsAAAAJ&hl=pt-PT)<sup>1</sup>,
[Nelson Freitas](https://scholar.google.com/citations?user=BDOCGnoAAAAJ&hl=pt-PT)<sup>1</sup>,
[André Rocha](https://scholar.google.pt/citations?user=k1GIyqcAAAAJ&hl=pt-PT)<sup>1</sup>,

<sup>1</sup> **NOVA School of Science and Technology, Center of Technology and Systems (UNINOVA-CTS)**,
and Associated Lab of Intelligent Systems (LASI), NOVA University
Lisbon, 2829-516 Lisbon, Portugal

The **Technology-Agnostic Multi-Agent Framework (TAMAF)** provides an open, distributed control architecture designed to address the interoperability and integration challenges of Multi-Agent Systems (MAS) in Cyber-Physical Production Systems and Industry 4.0. Built around modularity and platform agnosticism, TAMAF establishes a unified communication and coordination layer that seamlessly spans all hierarchy levels: from low-level **Edge computing** on embedded hardware to mid-level **Fog computing** and high-level **Cloud computing**. The framework is validated across heterogeneous environments with native implementations in **Python** (Fog/Cloud), **Java** (Cloud/Enterprise), and **C++** (ESP32/Edge), enabling autonomous, resilient, and collaborative distributed automation.

## <div align="center">Get Started</div>
</div>

The framework provides three interoperable packages under [`src/mk3`](./src/mk3) implemented in Python, Java, and C++ (ESP32):

### Installing TAMAF Python Package from Git

- **Requirements:** Python >= 3.9, `pip`

**Direct Git Installation**
```bash
pip install "git+https://github.com/NOVA-RICS-Open-Lab/technology-agnostic-multi-agent-framework.git#subdirectory=src/mk3/PythonPackage"
```

**Verify installation:**
```bash
python -c "import tamaf; print('TAMAF successfully installed!')"
```

### Installing TAMAF Python Package

- **Requirements:** Python >= 3.9, `pip`
- **Location:** [`src/mk3/PythonPackage`](./src/mk3/PythonPackage)

**Clone this repository to your local machine:**
```bash
git clone https://github.com/NOVA-RICS-Open-Lab/technology-agnostic-multi-agent-framework.git
cd technology-agnostic-multi-agent-framework
```

**Install Python Package:**
```bash
pip install ./src/mk3/PythonPackage
```

**Verify installation:**
```bash
python -c "import tamaf; print('TAMAF successfully installed!')"
```

### Installing TAMAF Java Package
- **Requirements:** JDK 17+, Apache Maven (`mvn`)
- **Location:** [`src/mk3/JavaPackage`](./src/mk3/JavaPackage)

**Clone this repository to your local machine:**
```bash
git clone https://github.com/NOVA-RICS-Open-Lab/technology-agnostic-multi-agent-framework.git
cd technology-agnostic-multi-agent-framework
```

**Compile and install to local Maven cache:**
```bash
cd src/mk3/JavaPackage
mvn clean install
```
*(Use `mvn clean install -DskipTests` to skip unit tests).*

**Add to your project (`pom.xml`):**
```xml
<dependency>
    <groupId>tamaf</groupId>
    <artifactId>JavaPackage</artifactId>
    <version>1.0-SNAPSHOT</version>
</dependency>
```

### Installing TAMAF ESP32 C++ Package for Arduino IDE
- **Requirements:** ESP32 Board support, `ArduinoJson` (^7.0.0), C++17 with exceptions enabled
- **Location:** [`src/mk3/esp32package`](./src/mk3/esp32package)

**Clone this repository to your local machine:**
```bash
git clone https://github.com/NOVA-RICS-Open-Lab/technology-agnostic-multi-agent-framework.git
```

1. In Arduino IDE, navigate to **Sketch** > **Include Library** > **Add .ZIP Library...**.
2. Select [`src/mk3/esp32package.zip`](./src/mk3/esp32package/zip/esp32package.zip).  
   *(Alternatively, copy [`src/mk3/esp32package/TAMAF-ESP32`](./src/mk3/esp32package/TAMAF-ESP32) into your Arduino libraries folder: `~/Documents/Arduino/libraries`).*
3. Open **Tools** > **Manage Libraries...**, search for **`ArduinoJson`**, and install version `7.x`.

**Include in your code:**
```cpp
#include <tamaf.h>
```

### Installing TAMAF ESP32 C++ Package for Platformio

**Clone this repository to your local machine:**
```bash
git clone https://github.com/NOVA-RICS-Open-Lab/technology-agnostic-multi-agent-framework.git
```

**In your PlatformIO project's `platformio.ini`:**
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
build_unflags = -std=gnu++11 -fno-exceptions
build_flags = -std=gnu++17 -fexceptions
lib_deps =
    bblanchon/ArduinoJson @ ^7.1.0
    symlink://path/to/technology-agnostic-multi-agent-framework/src/mk3/esp32package
```

**Include in your code:**
```cpp
#include <tamaf.h>
```

---

### Usage instructions
Once your desired package(s) are installed, you now have the tools and abstractions available to create your own Multi-Agent System (MAS).

At the core of TAMAF are three fundamental building blocks:
- **EMA (Environment Management Agent)**: The environment acting as the directory facilitator for agent registration, discovery, and lifecycle management.
- **Agents**: Autonomous entities with distinct network identities (`AgentID`), an Agent Management System (AMS) to schedule tasks, and a Message Transport Service (MTS) to communicate.
- **Behaviors**: Atomic or composite execution units representing an agent's capabilities, logic, and protocols.

---

#### 1. Starting the EMA

Before starting any agents, an **EMA** instance must be running on your network (default port: `4000`). In Python, you can launch an EMA directly in your script or run it as a standalone background:

```python
import time
from tamaf import LaunchEMA

ema = LaunchEMA(debug=False)
time.sleep(1)
```

---

#### 2. Agents

Agents are created by subclassing `Agent` and overriding the lifecycle methods:
- `Setup(self, emainteraction)`: Invoked automatically when the agent starts and finishes registration with the EMA. Use this method to attach initial behaviors and register offered services.
- `TakeDown(self, emainteraction)`: Invoked when the agent is shutting down to perform cleanups or deregister services.

##### Agent Lifecycle Controls:
- `agent.Start()`: Spawns the agent's internal event loop and registers it with the EMA.
- `agent.Stop()`: Deregisters the agent from the EMA and cleanly shuts down its thread.
- `agent.IsAlive()`: Returns `True` if the agent is actively executing.
- `agent.Suspend()` / `agent.Resume()`: Pauses and resumes the agent's behavior execution cycle.
- `agent.AddBehavior(behavior)` / `agent.RemoveBehavior(behavior)`: Dynamically attaches or detaches behaviors.

##### Example: Defining and Running an Agent

```python
import time
from tamaf import Agent, LaunchEMA

# 1. Start EMA
ema = LaunchEMA(debug=False)
time.sleep(1)

# 2. Define custom agent
class WorkerAgent(Agent):
    def __init__(self, name: str):
        super().__init__(name)

    def Setup(self, emainteraction):
        print(f"[{self.getName()}] Agent initialized and registered with EMA.")

    def TakeDown(self, emainteraction):
        print(f"[{self.getName()}] Agent shutting down.")

# 3. Instantiate and start
worker = WorkerAgent("Worker-1")
worker.Start()

time.sleep(2)

# 4. Clean shutdown
worker.Stop()
ema.Stop()
```

---

#### 3. Behaviors

Behaviors represent the concurrent execution tasks scheduled cooperatively by the agent's Agent Management System (AMS). TAMAF provides multiple behavior primitives:

##### `OneShotBehavior`
Executes its `Action()` method exactly once and automatically finishes. Ideal for initialization tasks, triggering single actions, or sending one-off alerts.

```python
from tamaf import OneShotBehavior

class WelcomeBehavior(OneShotBehavior):
    def Action(self):
        print(f"[{self.agent.getName()}] Executing one-time task.")
```

##### `CyclicBehavior`
Executes its `Action()` method repeatedly in an infinite loop until explicitly removed or the agent terminates. Ideal for polling sensor inputs or continuously monitoring message streams.

```python
import time
from tamaf import CyclicBehavior

class MonitorBehavior(CyclicBehavior):
    def Action(self):
        print(f"[{self.agent.getName()}] Monitoring environment status...")
```

##### `SimpleBehavior`
A general-purpose behavior where execution repeats until the `Done()` method returns `True`.

```python
from tamaf import SimpleBehavior

class CountToThreeBehavior(SimpleBehavior):
    def __init__(self):
        super().__init__()
        self.count = 0

    def Action(self):
        self.count += 1
        print(f"[{self.agent.getName()}] Count: {self.count}")

    def Done(self) -> bool:
        return self.count >= 3  # Terminates after 3 iterations
```

##### `TickerBehavior`
Executes the `OnTick()` method periodically at a fixed interval specified in milliseconds.

```python
from tamaf import TickerBehavior

class PeriodicPingBehavior(TickerBehavior):
    def __init__(self, agent, interval_ms: int = 2000):
        super().__init__(agent, interval_ms)

    def OnTick(self):
        print(f"[{self.agent.getName()}] Periodic tick triggered every {int(self.period * 1000)}ms.")
```

##### `WakerBehavior`
Executes the `OnWake()` method once after an elapsed delay specified in milliseconds, then terminates.

```python
from tamaf import WakerBehavior

class DelayedAlarmBehavior(WakerBehavior):
    def __init__(self, agent, delay_ms: int = 5000):
        super().__init__(agent, delay_ms)

    def OnWake(self):
        print(f"[{self.agent.getName()}] Timer expired! Executing delayed task.")
```

##### `SequentialBehavior`
Combines multiple sub-behaviors and executes them sequentially in the order they were added.

```python
from tamaf import SequentialBehavior

class PipelineBehavior(SequentialBehavior):
    def __init__(self, agent):
        super().__init__(agent)
        self.AddSubBehavior(WelcomeBehavior())
        self.AddSubBehavior(CountToThreeBehavior())
```

##### `ParallelBehavior`
Executes multiple sub-behaviors concurrently in cooperative turns. The `finishWhenAnyDone` parameter controls termination:
- `finishWhenAnyDone=True` (default): Terminates when *any* child behavior completes.
- `finishWhenAnyDone=False`: Terminates only when *all* child behaviors have completed.

```python
from tamaf import ParallelBehavior

# Inside agent Setup:
# Execute multiple behaviors concurrently until ALL sub-behaviors complete
parallel = ParallelBehavior(self, finishWhenAnyDone=False)
parallel.AddSubBehavior(WelcomeBehavior())
parallel.AddSubBehavior(CountToThreeBehavior())
self.AddBehavior(parallel)
```

##### `FSMBehavior`
Implements a Finite State Machine (FSM) where each state is a child behavior. State transitions are triggered by the integer return code produced by a state's `OnEnd()` method upon completion.

```python
from tamaf import FSMBehavior, OneShotBehavior

class CheckSensorsState(OneShotBehavior):
    def Action(self):
        print(f"[{self.agent.getName()}] State: Checking environment sensors...")

    def OnEnd(self) -> int:
        # Return transition code: 0 = Normal, 1 = Alert/Error
        sensor_ok = True
        return 0 if sensor_ok else 1

class NormalState(OneShotBehavior):
    def Action(self):
        print(f"[{self.agent.getName()}] State: Operating within normal parameters.")

class AlertState(OneShotBehavior):
    def Action(self):
        print(f"[{self.agent.getName()}] State: Handling alert condition!")

# Inside agent Setup:
fsm = FSMBehavior(self)

# Register initial and final states
fsm.AddInitialState(CheckSensorsState(), "CHECK_SENSORS")
fsm.AddFinalState(NormalState(), "STATE_NORMAL")
fsm.AddFinalState(AlertState(), "STATE_ALERT")

# Define transitions based on OnEnd return code
fsm.AddTransition("CHECK_SENSORS", "STATE_NORMAL", transitionCode=0)
fsm.AddTransition("CHECK_SENSORS", "STATE_ALERT", transitionCode=1)

self.AddBehavior(fsm)
```

---

#### 4. Inter-Agent Communication (FIPA-ACL)

Agents communicate asynchronously using FIPA-compliant **ACL Messages** (`ACLMessage`), standardized speech acts (`Performative`), and pattern matching templates (`ACLMessageTemplate`).

TAMAF provides built-in interaction protocols including **`FIPARequest`** and **`FIPAContractNet`**.

##### FIPA Request Server / Responder:
```python
from tamaf import Agent, ACLMessageTemplate, Performative, FIPARequestResponder

class CalculatorAgent(Agent):
    def Setup(self, emainteraction):
        # Match incoming REQUEST messages
        template = ACLMessageTemplate(performatives={Performative.REQUEST})

        class RequestHandler(FIPARequestResponder):
            def handleRequest(self, requestMsg):
                print(f"[{self.agent.getName()}] Received request from {requestMsg.sender.name}: {requestMsg.content}")
                # Acknowledge agreement to handle request
                return requestMsg.CreateReply(Performative.AGREE)

            def prepareResultNotification(self, requestMsg, responseMsg):
                # Process the work and reply with final INFORM containing results
                reply = requestMsg.CreateReply(Performative.INFORM)
                reply.content = f"Processed '{requestMsg.content}' successfully."
                return reply

        self.AddBehavior(RequestHandler(self, template))
```

##### FIPA Request Client / Initiator:
```python
from tamaf import Agent, AgentID, Address, ACLMessage, Performative, FIPARequestInitiator, OneShotBehavior, GetLocalIP

class RequesterAgent(Agent):
    def __init__(self, name: str, target_port: int):
        super().__init__(name)
        self.target_port = target_port

    def Setup(self, emainteraction):
        class SendJobBehavior(OneShotBehavior):
            def Action(self):
                target = AgentID(name="CalculatorAgent", address=Address(GetLocalIP(), self.agent.target_port))
                
                # Compose ACL message
                msg = ACLMessage(
                    sender=self.agent.agentDescription.agentid,
                    performative=Performative.REQUEST,
                    receiver=[target],
                    content="Compute task payload #42"
                )

                # Use FIPA Request protocol
                class JobInitiator(FIPARequestInitiator):
                    def handleAgree(self, reply):
                        print(f"[{self.agent.getName()}] Target agreed to perform task.")
                        return True

                    def handleInform(self, reply):
                        print(f"[{self.agent.getName()}] Success! Received result: {reply.content}")
                        return True

                    def handleRefuse(self, reply):
                        print(f"[{self.agent.getName()}] Request was refused.")
                        return True

                self.agent.AddBehavior(JobInitiator(self.agent, msg))

        self.AddBehavior(SendJobBehavior())
```

---

#### 5. Dynamic Agent Discovery via EMA

Agents can dynamically search the EMA directory facilitator to discover peers without prior knowledge of their network ports:

```python
from tamaf import AgentID, Address, AgentDescription, ServiceDescription, GetLocalIP

# Inside a behavior with access to self.emaInteraction:
def discover_agents(self):
    # Search by Agent name pattern
    search_template = AgentDescription(AgentID(name="CalculatorAgent", address=Address(GetLocalIP())))
    discovered_agents = self.emaInteraction.Search(search_template)

    for desc in discovered_agents:
        print(f"Discovered peer: {desc.agentid.name} at {desc.agentid.address.ip}:{desc.agentid.address.port}")
```

---

#### 6. Complete End-to-End Example

Here is a full runnable script demonstrating the EMA, a responder agent, and an initiator agent communicating over FIPA-ACL:

```python
import time
from tamaf import (
    Agent, AgentID, Address, ACLMessage, Performative, ACLMessageTemplate,
    OneShotBehavior, FIPARequestInitiator, FIPARequestResponder, LaunchEMA, GetLocalIP
)

# 1. Start EMA
ema = LaunchEMA(debug=False)
time.sleep(1)

# 2. Define Responder Agent
class ServiceProvider(Agent):
    def Setup(self, emainteraction):
        template = ACLMessageTemplate(performatives={Performative.REQUEST})
        
        class ServiceHandler(FIPARequestResponder):
            def handleRequest(self, req):
                print(f"[Provider] Accepted task '{req.content}' from {req.sender.name}")
                return req.CreateReply(Performative.AGREE)

            def prepareResultNotification(self, req, resp):
                reply = req.CreateReply(Performative.INFORM)
                reply.content = f"Result of [{req.content}] -> COMPLETED"
                return reply

        self.AddBehavior(ServiceHandler(self, template))

# 3. Define Initiator Agent
class ClientAgent(Agent):
    def __init__(self, name: str, target_port: int):
        super().__init__(name)
        self.target_port = target_port

    def Setup(self, emainteraction):
        class DispatchRequest(OneShotBehavior):
            def Action(self):
                target = AgentID("Provider", Address(GetLocalIP(), self.agent.target_port))
                req = ACLMessage(
                    sender=self.agent.agentDescription.agentid,
                    performative=Performative.REQUEST,
                    receiver=[target],
                    content="Process Dataset A"
                )

                class Protocol(FIPARequestInitiator):
                    def handleAgree(self, msg):
                        print("[Client] Provider agreed to process request.")
                        return True
                        
                    def handleInform(self, msg):
                        print(f"[Client] Success notification: {msg.content}")
                        return True

                self.agent.AddBehavior(Protocol(self.agent, req))

        self.AddBehavior(DispatchRequest())

# 4. Instantiate and launch agents
provider = ServiceProvider("Provider")
provider.Start()
time.sleep(1)

client = ClientAgent("Client", provider.agentDescription.agentid.address.port)
client.Start()

# 5. Wait for message exchange to complete, then clean up
time.sleep(3)
client.Stop()
provider.Stop()
ema.Stop()
print("MAS execution finished successfully.")
```

---

### Stress Testing

The stress testing suite benchmarks the scalability, registration efficiency, and messaging throughput of **TAMAF (Python)**, **TAMAF (Java)**, and **JADE** across varying agent scales (1, 5, 10, 15, 20, 30, 40, 50, 75, 100, and 200 agents, tested over 10 independent iterations each).

In each test run:
1. An **EMA** (or JADE Main Container) is initialized.
2. A **Receiver Agent** registers and waits for incoming ACL messages.
3. $N$ **Sender Agents** register sequentially, recording the total registration time.
4. All sender agents synchronize on an upcoming `trigger_time`, simultaneously dispatching request messages to the receiver.
5. The receiver replies with an acknowledgment (ACK), allowing each sender to measure its **Round Trip Time (RTT)** and record any timeouts.

All stress testing code is located under [`src/stresstesting`](./src/stresstesting).

---

#### 1. Running TAMAF Python Stress Tests
- **Requirements:** Python >= 3.9, TAMAF Python package installed
- **Location:** [`src/stresstesting/tamaf_python`](./src/stresstesting/tamaf_python)

**Run all default scales (1 to 200 agents, 10 runs each):**
```bash
cd src/stresstesting/tamaf_python/src
python main.py
```

**Run specific agent scale(s):**
```bash
python main.py --scales 1 5 10 20
```

Raw test logs will be saved to `src/stresstesting/tamaf_python/results/` (e.g., `1_test_20_agent_testresults.txt`).

---

#### 2. Running TAMAF Java Stress Tests
- **Requirements:** JDK 17+, Apache Maven (`mvn`), Python >= 3.9, TAMAF Java package ad TAMAF Python package installed
- **Location:** [`src/stresstesting/tamaf_java`](./src/stresstesting/tamaf_java)

**Compile test classes (if source modified):**
```bash
cd src/stresstesting/tamaf_java
javac -cp "bin;../../mk3/JavaPackage/target/classes;%USERPROFILE%/.m2/repository/com/fasterxml/jackson/core/jackson-databind/2.15.2/jackson-databind-2.15.2.jar;%USERPROFILE%/.m2/repository/com/fasterxml/jackson/core/jackson-core/2.15.2/jackson-core-2.15.2.jar;%USERPROFILE%/.m2/repository/com/fasterxml/jackson/core/jackson-annotations/2.15.2/jackson-annotations-2.15.2.jar;%USERPROFILE%/.m2/repository/com/fasterxml/jackson/datatype/jackson-datatype-jsr310/2.15.2/jackson-datatype-jsr310-2.15.2.jar" -d bin src/*.java
```

**Run all default scales (1 to 200 agents, 10 runs each):**
```bash
cd src/stresstesting/tamaf_java
python run_tests.py
```

**Run specific agent scale(s):**
```bash
python run_tests.py 1 5 10 20
```

Raw test logs will be saved to `src/stresstesting/tamaf_java/results/` (e.g., `1_test_20_agent_testresults.txt`).

---

#### 3. Running JADE Baseline Stress Tests
- **Requirements:** JDK 17+, JADE library (`jade.jar`)
- **Location:** [`src/stresstesting/jade`](./src/stresstesting/jade)

**Compile test classes:**
```bash
cd src/stresstesting/jade/src
javac -cp ".;<path-to-jade.jar>" *.java
```

**Run all default scales (1 to 200 agents, 10 runs each):**
```bash
cd src/stresstesting/jade/src
java -cp ".;<path-to-jade.jar>" Main
```

**Run specific agent scale(s):**
```bash
java -cp ".;<path-to-jade.jar>" Main 1 5 10 20
```

Raw test logs will be saved to `src/stresstesting/jade/results/` (e.g., `1_test_20_agent_testresults.txt`).

---

#### 4. Running Results and Analysis
- **Requirements:** Python >= 3.9
- **Location:** [`src/stresstesting/resultsAndAnalysis`](./src/stresstesting/resultsAndAnalysis)

**Install the analysis dependencies:**
```bash
pip install -r src/stresstesting/resultsAndAnalysis/requirements.txt
```

> You must run **`consolidate.py` first** before running the plotting or regression analysis scripts, as both tools depend directly on the consolidated dataset (`comparison_results.txt`) it produces.

**Step 1: Consolidate Raw Results**
Parses all raw test result logs across JADE, TAMAF Python, and TAMAF Java, computes average registration times, round trip times, and timeouts, and writes the summarized `comparison_results.txt`:
```bash
cd src/stresstesting/resultsAndAnalysis
python consolidate.py
```

**Step 2: Generate Visual Comparison Plots**
Reads `comparison_results.txt` and exports comparative PDF charts:
```bash
python plot_results.py
```
- Generated files: `registration_time.pdf` and `avg_rtt.pdf` in `src/stresstesting/resultsAndAnalysis/`.

**Step 3: Run Mathematical Regression Analysis**
Fits Linear ($y = ax + b$), Quadratic ($y = ax^2 + bx + c$), and Power Law ($y = a \cdot x^b$) models for each framework to evaluate algorithmic complexity and growth rates, printing $R^2$ and RMSE metrics:
```bash
python regression_analysis.py
```

---

### Study Case

The functional use case, described in detail in the [Thesis](/docs/thesis.pdf), can be found inside [here](src\studycase)

---

### Project Structure
For you to better understand this repository organization here is a quick overview of its structure and where to find what you might be looking for:
```
technology-agnostic-multi-agent-framework
├── docs                # documentation assets
│   └── thesis.pdf          # master thesis
└── src                 # developed code
    ├── mk1                # TAMAF packages version 1
    ├── mk2                # TAMAF packages version 2
    ├── mk3                # TAMAF packages version 3
    ├── stresstesting      # stresstesting code
    └── studycase          # functional use case code
```

## <div align="center">Contribution Guidelines</div>
NOVA RICS Open Lab open source, and we welcome contributions from the community! See the [Contribution](CONTRIBUTING.md) guide for more information on the development workflow and the internals of the wandb library. For project related bugs and feature requests, visit [GitHub Issues](https://github.com/NOVA-RICS-Open-Lab/technology-agnostic-multi-agent-framework/issues) or contact novaricsopenlab@gmail.com

<!-- ## <div align="center">Citation</div>
If you use or intend to build on top of  the work in this repo, please consider citing our project:
```bibtex
@inproceedings{,
  author={},
  booktitle={}, 
  title={}, 
  year={2},
  doi={}
}
``` -->

## <div align="center">License</div>
This repository is released under the MIT License. Please see the [LICENSE](LICENSE) file for more details.

## <div align="center">Contacts</div>
For any questions regarding this or any other project please contact us at novaricsopenlab@gmail.com or enroll in our [Discussion Forum](https://github.com/NOVA-RICS-Open-Lab/technology-agnostic-multi-agent-framework/discussions) for sharing your ideas and sharing projects.
