# WATCH OS — Kernel Simulation & OS Analysis Engine

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.15%2B-orange.svg)](https://cmake.org/)
[![Database](https://img.shields.io/badge/Database-SQLite%203-lightgrey.svg)](https://www.sqlite.org/)
[![Phase 1](https://img.shields.io/badge/Phase%201-Complete-brightgreen.svg)]()
[![Phase 2](https://img.shields.io/badge/Phase%202-Complete-brightgreen.svg)]()

> A complete, modular, command-line-based operating system simulator and analysis engine built in C++17, CMake, and SQLite. Configure workloads once, persist them across restarts, run kernel analyses (CPU scheduling, virtual memory, deadlocks, disk scheduling, IPC synchronization, security isolation), and generate comprehensive reports.

---

## Capabilities Overview

### 1. CPU Scheduling Engine & Analytics
- **Algorithms**: FCFS, Shortest Job First (SJF), Shortest Remaining Time First (SRTF), Priority (both Preemptive and Non-Preemptive modes), and Round Robin (configurable quantum).
- **Comprehensive Metrics**: Turnaround Time ($TAT = CT - AT$), Waiting Time ($WT = TAT - BT$), Response Time ($RT = FST - AT$), Throughput, CPU Utilization, and Context Switches.
- **ASCII Gantt Charts**: Rendered from recorded simulation segments showing execution intervals, idle intervals, and preemptions.
- **Recommendation Engine**: Multi-criteria weighted normalization ranking the optimal scheduling algorithm for the active workload.
- **Starvation & Aging**: Starvation detection threshold analysis and dynamic priority aging before/after comparison.

### 2. Virtual Memory Management
- **Page Replacement Algorithms**: FIFO, Least Recently Used (LRU), and Optimal Page Replacement.
- **Metrics**: Page references, page hits, page faults, fault ratio, frame allocations, and thrashing indicators.
- **Memory Modeling**: Locality of reference simulation and page table mappings.

### 3. Deadlock Analysis & Banker's Algorithm
- **Resource Allocation Graph (RAG)**: Formatted terminal graph representation of process-to-resource allocation and request edges.
- **Banker's Algorithm**: Safe state validation, remaining need calculation, and safe sequence determination.
- **Deadlock Detection**: Multiple-instance resource reduction identifying actual deadlocks.
- **Monte Carlo Risk Estimation**: 1,000 stochastic trials computing empirical deadlock frequency and per-process involvement.

### 4. I/O & Disk Scheduling
- **Disk Arm Algorithms**: FCFS, Shortest Seek Time First (SSTF), SCAN (Elevator with configurable direction), and C-SCAN.
- **Metrics**: Total head movement, average seek time, and service sequence.
- **I/O Queue Management**: Process blocking on I/O operations and latency measurement.

### 5. Process Synchronization & Access Protection
- **Primitives**: Mutex locks, counting semaphores, bounded IPC message queues, and producer-consumer simulation.
- **Race Condition Detection**: Flags unsynchronized concurrent writes to shared variables.
- **Simulated Access Control**: Role-based permissions (root, user, guest), memory partition boundary protection, and segmentation fault tracking.

### 6. Persistence & Reports
- **Embedded SQLite 3**: Complete ACID transactional persistence with foreign key enforcement (`PRAGMA foreign_keys = ON;`).
- **Reports Export**: Generates full Markdown (`reports/report_<session>.md`) and CSV (`reports/report_<session>.csv`) reports.
- **JARVIS Explain Mode**: Generates evidence-backed WHY, COMPARE, and SUMMARY explanations grounded in actual recorded events.

---

## Project Structure

```
WATCH-OS/
├── CMakeLists.txt              # CMake configuration
├── CMakePresets.json           # Presets for building and testing
├── README.md                   # Project documentation
├── .gitignore                  # Git ignore rules
│
├── third_party/
│   └── sqlite3/                # Embedded SQLite 3 amalgamation
│
├── include/watchos/
│   ├── core/                   # Event engine, event queue, ID generator, types
│   ├── database/               # DbConnection, Schema, SessionRepository, RunRepository
│   ├── process/                # Process entities, validator, lifecycle simulator
│   ├── session/                # Session manager, configuration manager
│   ├── scheduler/              # FCFS, SJF, SRTF, Priority, RR, Gantt, Recommendation, Aging
│   ├── memory/                 # Page replacement (FIFO, LRU, Optimal), memory manager
│   ├── deadlock/               # Resource manager, Banker's, Deadlock detector, Monte Carlo
│   ├── io/                     # Disk scheduler (FCFS, SSTF, SCAN, CSCAN), I/O manager
│   ├── synchronization/        # Mutex, Semaphore, IPC, Producer-Consumer
│   ├── security/               # Access controller, partition memory protection
│   ├── experiments/            # What-if experiments, fault injector
│   ├── reporting/              # Jarvis explainer, Markdown & CSV report generator
│   └── cli/                    # Console UI, AnsiTerminal, AnalysisMenu, CliApp
│
├── src/                        # Implementation files corresponding to include headers
│   ├── main.cpp                # Application entry point
│   ├── core/
│   ├── database/
│   ├── process/
│   ├── session/
│   ├── scheduler/
│   ├── memory/
│   ├── deadlock/
│   ├── io/
│   ├── synchronization/
│   ├── security/
│   ├── experiments/
│   ├── reporting/
│   └── cli/
│
├── tests/
│   ├── unit/                   # Unit test executables (ID, Validator, DB, Schedulers, Deadlock, Memory, Disk)
│   ├── integration/            # Integration tests (Lifecycle restart, complete kernel analysis)
│   └── fixtures/               # Golden workload test fixtures
│
├── docs/                       # Architecture, database schema, and CLI guides
│   ├── architecture.md
│   ├── database-schema.md
│   └── cli-guide.md
│
├── data/                       # Default storage for watchos_sessions.db
└── reports/                    # Export directory for generated simulation reports
```

---

## Build and Run Instructions

### Prerequisites
- C++17 compiler (`g++`, `clang++`, or `MSVC`)
- CMake 3.15 or newer

### Building the Project
```powershell
cmake -B build -S .
cmake --build build
```

### Running All Test Suites
```powershell
ctest --test-dir build --output-on-failure
```
All 9 test suites will execute:
1. `TestIdGenerator`
2. `TestWorkloadValidator`
3. `TestDbPersistence`
4. `TestCpuSchedulers`
5. `TestDeadlockBankers`
6. `TestMemoryPaging`
7. `TestDiskScheduling`
8. `TestSessionLifecycle`
9. `TestCompleteKernelAnalysis`

### Running WATCH OS
```powershell
.\build\watchos.exe
```
Or specify a custom database:
```powershell
.\build\watchos.exe my_custom_sessions.db
```
