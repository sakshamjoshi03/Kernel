# WATCH OS — Architecture & System Design (Phase 1 & Phase 2)

## 1. System Architecture Overview
WATCH OS is a modular, high-precision operating system simulator and analysis engine written in **C++17** with persistent storage backed by **SQLite 3**.

```
+---------------------------------------------------------------------------------------+
|                                WATCH OS CLI Console                                   |
|   (ConsoleUi, AnsiTerminal, ConfigurationManager, AnalysisMenu, CliApp)              |
+------------------------------------------+--------------------------------------------+
                                           |
                                           v
+---------------------------------------------------------------------------------------+
|                             Shared Simulation Engine                                  |
|   (SimulationEngine, EventQueue, Event, RunContext)                                   |
+----+--------------------+--------------------+--------------------+-------------------+
     |                    |                    |                    |                   
     v                    v                    v                    v                   
+------------+    +---------------+    +---------------+    +---------------+           
| Scheduler  |    | Virtual Memory|    | Deadlock &    |    | I/O & Disk    |           
| Subsystem  |    | Subsystem     |    | Resource Mgr  |    | Subsystem     |           
| (FCFS,     |    | (FIFO, LRU,   |    | (Banker's,    |    | (FCFS, SSTF,  |           
|  SJF, SRTF,|    |  Optimal,     |    |  Detection,   |    |  SCAN, CSCAN, |           
|  Priority, |    |  Thrashing)   |    |  Monte Carlo) |    |  IoManager)   |           
|  RR)       |    |               |    |               |    |               |           
+------------+    +---------------+    +---------------+    +---------------+           
     |                    |                    |                    |                   
     +--------------------+--------------------+--------------------+                   
                          |
                          v
+---------------------------------------------------------------------------------------+
|                    Process Lifecycle, Security & IPC Subsystems                       |
|   (LifecycleSimulator, AccessController, SyncManager)                                |
+------------------------------------------+--------------------------------------------+
                                           |
                                           v
+---------------------------------------------------------------------------------------+
|                       Reporting & Explanation Engine                                  |
|   (JarvisExplainer, ReportGenerator - Markdown & CSV Exporters)                       |
+------------------------------------------+--------------------------------------------+
                                           |
                                           v
+---------------------------------------------------------------------------------------+
|                         SQLite 3 Persistence Layer                                    |
|   (SessionRepository, RunRepository, Schema, DbConnection, TransactionGuard)           |
+---------------------------------------------------------------------------------------+
```

## 2. Subsystems Detail

### Shared Event-Driven Engine (`include/watchos/core/`)
- `SimulationEngine`: Owns the integer simulated clock and simulation state.
- `EventQueue`: Deterministic priority queue ordered by timestamp $\rightarrow$ event priority $\rightarrow$ entity ID $\rightarrow$ unique event sequence ID.
- `Event`: Event types for operation completion, I/O completion, process arrival, resource requests, quantum expiration, and faults.
- `RunContext`: Workload snapshot, parameters, random seed, and execution telemetry.

### CPU Scheduling Subsystem (`include/watchos/scheduler/`)
- Algorithms:
  - **FCFS**: Non-preemptive, arrival-ordered, deterministic tie-breaking.
  - **SJF**: Shortest Job First (non-preemptive).
  - **SRTF**: Shortest Remaining Time First (preemptive).
  - **Priority**: Preemptive and non-preemptive modes (lower value = higher priority).
  - **Round Robin**: Preemptive with configurable time quantum $Q$.
- `MetricsCalculator`: Turnaround time ($CT - AT$), Waiting time ($TAT - BT$), Response time ($FST - AT$), Throughput, CPU Utilization, and Context Switches.
- `GanttChart`: Renders clear ASCII Gantt charts showing intervals, idle time, and preemptions.
- `RecommendationEngine`: Multi-criteria normalized scoring across waiting time, turnaround time, response time, context switches, and starvation risk.
- `AgingAnalyzer`: Evaluates priority scheduling with and without dynamic aging.

### Virtual Memory Subsystem (`include/watchos/memory/`)
- `PageReplacementSimulator`: Models physical frames and virtual page references using **FIFO**, **LRU**, and **Optimal** page replacement.
- Detects page hits, page faults, fault ratio, total replacements, and thrashing warnings.
- `MemoryManager`: Generates realistic reference strings using working-set locality of reference and page table mappings.

### Deadlock & Resource Management (`include/watchos/deadlock/`)
- `ResourceManager`: Tracks resource types, allocations, and maximum claims; renders Resource Allocation Graphs (RAG).
- `BankersAlgorithm`: Computes safe execution sequence or detects unsafe states.
- `DeadlockDetector`: Multi-instance resource graph reduction detecting deadlocked processes.
- `DeadlockFrequencyEstimator`: Monte Carlo stochastic simulation running 1,000 randomized trials to compute empirical deadlock frequency.

### I/O and Disk Head Motion (`include/watchos/io/`)
- `DiskScheduler`: Implements **FCFS**, **SSTF**, **SCAN (Elevator)**, and **C-SCAN (Circular SCAN)**.
- Reports total head movement, service sequence, and average seek time.
- `IoManager`: Maintains per-device queues and models process blocking on I/O.

### Process Synchronization & Access Protection (`include/watchos/synchronization/`, `include/watchos/security/`)
- `SyncManager`: Mutex locks, counting semaphores, bounded IPC message queues, race condition detection, and producer-consumer simulation.
- `AccessController`: Role-based access control (RBAC), memory partition boundary protection, segmentation fault detection, and audit logging.

### Reporting & Explanations (`include/watchos/reporting/`)
- `JarvisExplainer`: Generates WHY (delays), COMPARE, and SUMMARY explanations grounded in actual recorded events.
- `ReportGenerator`: Exports comprehensive Markdown and CSV reports.
