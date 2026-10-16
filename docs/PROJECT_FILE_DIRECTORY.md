# WATCH OS — Project File & Directory Guide

**Document:** Codebase File Inventory & Functional Purpose  
**Location:** [`docs/PROJECT_FILE_DIRECTORY.md`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/docs/PROJECT_FILE_DIRECTORY.md)  
**Technology:** C++17, SQLite 3, CMake  

---

## 1. Project Root Directory (`/`)

Build configuration, environment setup, and automation helper scripts.

| File Path | Functional Purpose |
|---|---|
| [`CMakeLists.txt`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/CMakeLists.txt) | Main build script defining compiler flags, SQLite library, `watchos_lib`, `watchos.exe`, and 13 CTest test targets. |
| [`CMakePresets.json`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/CMakePresets.json) | Standardized CMake presets for debug, release, and testing configurations. |
| [`README.md`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/README.md) | Top-level project overview, build instructions, architectural summary, and quick-start guide. |
| [`.gitignore`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/.gitignore) | Specifies untracked files and build artifacts (`build/`, binary executables, temp files). |
| [`show_tables.py`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/show_tables.py) | Python utility to inspect and display all 24 database tables, column counts, and row counts in the terminal. |
| [`populate_full_feature_session.py`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/populate_full_feature_session.py) | Helper script to insert a complete, multi-subsystem demonstration session without deleting existing database rows. |

---

## 2. Public Header Files (`include/watchos/`)

Defines all interfaces, domain models, algorithms, and subsystem contracts.

### Core Simulation Engine (`include/watchos/core/`)

| File Path | Functional Purpose |
|---|---|
| [`types.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/core/types.hpp) | Fundamental kernel types, time units, Process ID aliases, enums (`ProcessState`, `SessionStatus`). |
| [`event.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/core/event.hpp) | Defines simulation event structures, timestamps, event categories, and comparison operators for scheduling. |
| [`event_queue.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/core/event_queue.hpp) | Priority-queue data structure managing discrete chronological event dispatch with deterministic tie-breaking. |
| [`simulation_engine.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/core/simulation_engine.hpp) | Master event-driven simulation clock and execution loop coordinator. |
| [`id_generator.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/core/id_generator.hpp) | Thread-safe generator producing unique monotonic IDs (`WOS-`, `RUN-`, `EVT-`, `RPT-`) and ISO timestamps. |
| [`run_context.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/core/run_context.hpp) | Carries execution parameters, runtime configuration, and active session state during a simulation run. |

### CLI & User Interface (`include/watchos/cli/`)

| File Path | Functional Purpose |
|---|---|
| [`cli_app.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/cli/cli_app.hpp) | Top-level application orchestrator handling database boot sequence and the 6 Main Menu options. |
| [`analysis_menu.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/cli/analysis_menu.hpp) | Interactive Kernel Analysis Console loop handling options 01–18 and natural language commands. |
| [`console_ui.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/cli/console_ui.hpp) | UI formatting helpers for banners, session tables, workload grids, alerts, and input prompts. |
| [`ansi_terminal.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/cli/ansi_terminal.hpp) | Windows virtual terminal ANSI escape code initializer for colored and formatted terminal text. |

### Session Management (`include/watchos/session/`)

| File Path | Functional Purpose |
|---|---|
| [`session.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/session/session.hpp) | Session entity class maintaining active version, system settings, metadata, and process list. |
| [`session_manager.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/session/session_manager.hpp) | Manages session lifecycle: creating sessions, saving workloads in transactions, resuming, and listing history. |
| [`configuration_manager.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/session/configuration_manager.hpp) | Interactive CLI wizard for adding, editing, removing, validating, and finalizing process workloads. |

### Process Model & Lifecycle (`include/watchos/process/`)

| File Path | Functional Purpose |
|---|---|
| [`process.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/process/process.hpp) | Process domain model storing PID, arrival time, burst, priority, memory, I/O, claims, and disk requests. |
| [`workload_validator.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/process/workload_validator.hpp) | Defensive validation rules verifying arrival $\ge 0$, burst $> 0$, unique PIDs, and subsystem parameters. |
| [`lifecycle_simulator.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/process/lifecycle_simulator.hpp) | Tracks 5-state process transitions (`NEW`, `READY`, `RUNNING`, `WAITING`, `TERMINATED`) and computes latency metrics. |
| [`subsystem_configs.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/process/subsystem_configs.hpp) | Lightweight data structs for I/O requests, memory demands, disk cylinders, and synchronization locks. |

### Database Layer (`include/watchos/database/`)

| File Path | Functional Purpose |
|---|---|
| [`db_connection.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/database/db_connection.hpp) | SQLite C++ RAII wrapper managing connection lifecycle, prepared statements, and transaction guards. |
| [`schema.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/database/schema.hpp) | DDL generator declaring all 24 relational tables, 19 performance indexes, constraints, and views. |
| [`session_repository.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/database/session_repository.hpp) | Repository performing CRUD queries for sessions, configuration versions, processes, and event logs. |
| [`run_repository.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/database/run_repository.hpp) | Repository storing simulation runs, metrics, Gantt charts, state transitions, deadlock, memory, and I/O results. |

### CPU Scheduling Subsystem (`include/watchos/scheduler/`)

| File Path | Functional Purpose |
|---|---|
| [`scheduler.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/scheduler/scheduler.hpp) | Pure virtual base class interface for all CPU scheduling algorithm implementations. |
| [`fcfs.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/scheduler/fcfs.hpp) | First-Come First-Served non-preemptive scheduler. |
| [`sjf.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/scheduler/sjf.hpp) | Shortest Job First non-preemptive scheduler. |
| [`srtf.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/scheduler/srtf.hpp) | Shortest Remaining Time First preemptive scheduler. |
| [`priority_scheduler.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/scheduler/priority_scheduler.hpp) | Priority-based scheduler supporting both preemptive and non-preemptive execution modes. |
| [`round_robin.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/scheduler/round_robin.hpp) | Round Robin preemptive scheduler with configurable time quantum slicing. |
| [`scheduling_metrics.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/scheduler/scheduling_metrics.hpp) | Computes turnaround, waiting, response times, CPU utilization, throughput, and starvation breaches. |
| [`gantt_chart.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/scheduler/gantt_chart.hpp) | Renders timeline intervals and idle periods into structured ASCII visual Gantt charts. |
| [`recommendation_engine.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/scheduler/recommendation_engine.hpp) | Multi-objective normalization engine ranking schedulers based on wait time, turnaround, and preemption overhead. |
| [`aging_analyzer.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/scheduler/aging_analyzer.hpp) | Simulates dynamic priority aging to demonstrate starvation prevention over time. |

### Virtual Memory Subsystem (`include/watchos/memory/`)

| File Path | Functional Purpose |
|---|---|
| [`page_replacement.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/memory/page_replacement.hpp) | Simulates FIFO, LRU, and Optimal page replacement algorithms and detects thrashing. |
| [`memory_manager.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/memory/memory_manager.hpp) | Orchestrates comparative memory analysis across multiple processes and frame allocations. |

### Deadlock Subsystem (`include/watchos/deadlock/`)

| File Path | Functional Purpose |
|---|---|
| [`resource_manager.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/deadlock/resource_manager.hpp) | Tracks resource pools, claims, and allocations; renders Resource Allocation Graphs (RAG). |
| [`bankers_algorithm.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/deadlock/bankers_algorithm.hpp) | Evaluates safety criteria and discovers safe execution sequences using Dijkstra's Banker's algorithm. |
| [`deadlock_detector.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/deadlock/deadlock_detector.hpp) | Detects circular wait cycles in Resource Allocation Graphs. |
| [`deadlock_frequency.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/deadlock/deadlock_frequency.hpp) | Monte Carlo stochastic simulation estimating empirical deadlock probability across 1,000 trials. |

### Storage & I/O Subsystem (`include/watchos/io/`)

| File Path | Functional Purpose |
|---|---|
| [`disk_scheduler.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/io/disk_scheduler.hpp) | Simulates disk arm cylinder seek optimization using FCFS, SSTF, SCAN, and C-SCAN. |
| [`io_manager.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/io/io_manager.hpp) | Coordinates peripheral I/O queues and blocking/completion state transitions. |

### Synchronization, Security & Reporting (`include/watchos/...`)

| File Path | Functional Purpose |
|---|---|
| [`synchronization/sync_manager.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/synchronization/sync_manager.hpp) | Simulates mutexes, semaphores, and bounded-buffer producer-consumer IPC concurrency. |
| [`security/access_controller.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/security/access_controller.hpp) | Enforces Role-Based Access Control (RBAC) and base/limit memory partition isolation. |
| [`experiments/experiment_engine.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/experiments/experiment_engine.hpp) | Manages parameter-tuning what-if experiment cohorts and computes metric deltas. |
| [`experiments/fault_injector.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/experiments/fault_injector.hpp) | Injects simulated synthetic hardware delays and process crashes to test resiliency. |
| [`reporting/report_generator.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/reporting/report_generator.hpp) | Synthesizes full session simulation data into formatted Markdown and CSV report files. |
| [`reporting/jarvis_explainer.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/include/watchos/reporting/jarvis_explainer.hpp) | Natural language explanation engine providing analytical summaries and trade-off rationales. |

---

## 3. C++ Source Files (`src/`)

Implements the logic declared in the public header files.

| File Path | Functional Purpose |
|---|---|
| [`src/main.cpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/src/main.cpp) | Program entry point; parses CLI arguments, locates SQLite database, and starts `CliApp`. |
| `src/cli/*.cpp` | Implements user interface menus, terminal escape modes, and analysis commands. |
| `src/core/*.cpp` | Implements discrete event queues, ID generation, and simulation engine dispatch. |
| `src/database/*.cpp` | Implements SQLite connection RAII, prepared statements, schema initialization, and repositories. |
| `src/scheduler/*.cpp` | Implements the 6 CPU scheduling algorithms, Gantt generation, and recommendation ranking. |
| `src/memory/*.cpp` | Implements FIFO, LRU, Optimal page replacement, and memory management. |
| `src/deadlock/*.cpp` | Implements RAG graph traversal, Banker's algorithm, deadlock detection, and Monte Carlo trials. |
| `src/io/*.cpp` | Implements FCFS, SSTF, SCAN, C-SCAN head movement, and I/O request management. |
| `src/process/*.cpp` | Implements process data models, validation checks, and lifecycle transition tracking. |
| `src/session/*.cpp` | Implements session persistence, configuration review wizards, and multi-version snapshots. |
| `src/security/*.cpp` | Implements access control permission evaluation and memory boundary checking. |
| `src/synchronization/*.cpp` | Implements producer-consumer simulation, mutex locks, and counting semaphores. |
| `src/experiments/*.cpp` | Implements parameter sensitivity experiment tracking and fault injection. |
| `src/reporting/*.cpp` | Implements Markdown and CSV export generation and natural language analytical summaries. |

---

## 4. Automated Test Suites (`tests/`)

CTest test suites verifying unit logic, subsystem integrations, end-to-end workflows, and regressions.

| File Path | Functional Purpose |
|---|---|
| [`tests/fixtures/test_workloads.hpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/tests/fixtures/test_workloads.hpp) | Shared reusable process workload fixtures (e.g. standard 5-process test workload). |
| [`tests/unit/test_id_generator.cpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/tests/unit/test_id_generator.cpp) | Tests ID format, timestamp consistency, and monotonic counter uniqueness under high load. |
| [`tests/unit/test_workload_validator.cpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/tests/unit/test_workload_validator.cpp) | Tests validation rules rejecting negative arrivals, zero burst, and duplicate PIDs. |
| [`tests/unit/test_db_persistence.cpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/tests/unit/test_db_persistence.cpp) | Tests SQLite connection, prepared statements, session creation, and workload save/load. |
| [`tests/unit/test_cpu_schedulers.cpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/tests/unit/test_cpu_schedulers.cpp) | Tests turnaround, waiting, and response times across all 6 scheduling algorithms. |
| [`tests/unit/test_deadlock_bankers.cpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/tests/unit/test_deadlock_bankers.cpp) | Tests Banker's safe sequence discovery, RAG cycle detection, and Monte Carlo estimation. |
| [`tests/unit/test_memory_paging.cpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/tests/unit/test_memory_paging.cpp) | Tests FIFO, LRU, Optimal page fault counts and frame snapshots against theoretical values. |
| [`tests/unit/test_disk_scheduling.cpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/tests/unit/test_disk_scheduling.cpp) | Tests disk head travel calculation for FCFS, SSTF, SCAN, and C-SCAN. |
| [`tests/unit/test_db_schema_audit.cpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/tests/unit/test_db_schema_audit.cpp) | Audits all 24 relational tables, `PRAGMA` integrity checks, foreign key cascading, and transactions. |
| [`tests/unit/test_edge_cases_schedulers.cpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/tests/unit/test_edge_cases_schedulers.cpp) | Tests single process, simultaneous arrivals, CPU idle gaps, and empty workload safety. |
| [`tests/integration/test_session_lifecycle.cpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/tests/integration/test_session_lifecycle.cpp) | Tests session creation, DRAFT/FINALIZED states, workload editing, and database reload. |
| [`tests/integration/test_complete_kernel_analysis.cpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/tests/integration/test_complete_kernel_analysis.cpp) | Tests integrated multi-subsystem simulation, recommendation engine, and Jarvis Explainer. |
| [`tests/integration/test_end_to_end_workflows.cpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/tests/integration/test_end_to_end_workflows.cpp) | Tests all 9 complete user workflows (creation, scheduling, memory, deadlock, disk, reporting). |
| [`tests/regression/test_discovered_regressions.cpp`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/tests/regression/test_discovered_regressions.cpp) | Prevents regressions for discovered bugs (session ID collisions, result sorting, version isolation). |

---

## 5. Storage, Reports, Documentation & Third-Party

| Directory / File Path | Functional Purpose |
|---|---|
| [`data/watchos_sessions.db`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/data/watchos_sessions.db) | Embedded SQLite database file storing all active sessions, configuration versions, runs, and results. |
| `reports/*.md` & `reports/*.csv` | Generated analytical reports exported by the reporting module in Markdown and CSV formats. |
| [`docs/QA_AUDIT_REPORT.md`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/docs/QA_AUDIT_REPORT.md) | Comprehensive QA audit report summarizing project health, test results, and recommendations. |
| [`docs/COMMAND_TEST_MATRIX.md`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/docs/COMMAND_TEST_MATRIX.md) | Test matrix evaluating all 35 CLI commands across the 14 functional criteria. |
| [`docs/DB_SCHEMA_AUDIT.md`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/docs/DB_SCHEMA_AUDIT.md) | Relational database schema audit verifying normalization, cascading, constraints, and PRAGMAs. |
| [`docs/DATABASE_DATA_DICTIONARY.md`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/docs/DATABASE_DATA_DICTIONARY.md) | Column-by-column data dictionary for all 24 relational tables and migration registries. |
| [`docs/BUGS_AND_FIXES.md`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/docs/BUGS_AND_FIXES.md) | Defect tracking log documenting root causes, code diffs, and regression test suites. |
| [`docs/architecture.md`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/docs/architecture.md) | High-level system architecture and component interaction diagram. |
| [`docs/cli-guide.md`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/docs/cli-guide.md) | User command-line manual and interactive walkthrough guide. |
| [`docs/database-schema.md`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/docs/database-schema.md) | Initial database schema documentation. |
| [`third_party/sqlite3/`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/third_party/sqlite3/) | Embedded SQLite 3.49.1 C amalgamation source (`sqlite3.c`, `sqlite3.h`). |
