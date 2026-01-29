# 🚀 Advanced OS Project: Concurrency & Design Patterns in C

A high-performance, multi-threaded **Graph Algorithms Server** implemented in **C**. This project demonstrates advanced system programming capabilities by combining complex graph theory algorithms with architectural design patterns and rigorous memory/thread safety analysis.

## 🧠 Key Features

### 🏗️ Architectural Design Patterns
Unlike standard C projects, this system implements high-level architectural patterns to manage complexity and concurrency:
* **Pipeline Pattern:** Decomposes request handling into a 4-stage processing chain. Each stage is handled by autonomous threads communicating via thread-safe queues.
* **Active Object:** Each pipeline stage functions as an Active Object with its own private thread and message queue, decoupling method execution from invocation.
* **Strategy & Factory Patterns:** Used to dynamically select and instantiate graph algorithms (MST, Max Flow, etc.) at runtime, ensuring loose coupling between the server logic and the algorithmic implementations.
* **Leader-Follower:** (Implemented in earlier iterations) A thread pool model to efficiently manage incoming network connections.

### 🕸️ Graph Algorithms
The server supports complex operations on graphs, processed asynchronously:
1.  **MST (Minimum Spanning Tree):** Calculated using Prim's algorithm.
2.  **Max Flow:** Solves flow network problems (Source to Sink).
3.  **Max Clique:** Finds the largest complete subgraph.
4.  **Clique Count:** Calculates the total number of cliques in the graph.
5.  **Eulerian Circuit:** Detects and prints Euler cycles.

### 🛠️ Systems & Stability
* **Multi-threading:** Extensive use of `pthread` for parallel execution.
* **Synchronization:** Protected shared resources using Mutexes (`pthread_mutex_t`) and Condition Variables (`pthread_cond_t`) to prevent race conditions.
* **Memory Safety:** Verified **100% memory leak-free** using Valgrind Memcheck.
* **Thread Safety:** Verified using Valgrind Helgrind to ensure no race conditions or deadlocks occur.

---

## ⚙️ The Pipeline Architecture

The core of the server (`server_pipeline.c`) processes client requests through a distinct 4-stage pipeline. A `Job` structure carries the context (Graph, Client Socket, Partial Results) through the stages:

```mermaid
graph LR
    Client -->|TCP Request| Listener
    Listener -->|Push Job| Q1[Queue 1]
    Q1 -->|Pop| MST[Stage 1: MST]
    MST -->|Push| Q2[Queue 2]
    Q2 -->|Pop| Flow[Stage 2: MaxFlow]
    Flow -->|Push| Q3[Queue 3]
    Q3 -->|Pop| Clique[Stage 3: MaxClique]
    Clique -->|Push| Q4[Queue 4]
    Q4 -->|Pop| Count[Stage 4: Count & Send]
    Count -->|TCP Response| Client
