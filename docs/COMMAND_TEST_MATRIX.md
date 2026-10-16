# WATCH OS — Complete CLI Command & Menu Option Test Matrix

**Audit Date:** October 2026  
**Auditor:** Senior QA Engineer & Operating Systems Specialist  
**Software Version:** WATCH OS v2.0 (C++17 / SQLite 3)  
**Execution Environment:** Windows (MinGW-w64 GCC 6.3.0, CMake 4.4.4, CTest)

---

## 1. Overview & Verification Methodology

Every discovered user-accessible command, menu option, sub-menu, alias, and natural language command was audited against the 14 core QA criteria:
1. **Command Existence:** Discovered and registered in parser/dispatch.
2. **Documented Syntax:** Invoked using exact documented syntax.
3. **Correct Function:** Dispatches to the intended logic without stubbing.
4. **Input Validation:** Rejects out-of-bounds, negative, non-numeric, or malformed inputs.
5. **Expected Output:** Produces calculated values formatted in structured tables/reports.
6. **Persistence:** Saves simulation records, results, events, and runs to SQLite.
7. **Invalid Input Handling:** Gracefully displays user-friendly errors without crashing.
8. **Missing Configuration:** Handles missing or optional subsystem configurations.
9. **No Session Behavior:** Rejects simulation requests when no session is active.
10. **Zero Process Behavior:** Gracefully alerts when session has no processes without division-by-zero.
11. **Database Resilience:** Survives foreign key/constraint rejections via transactional rollback.
12. **Control Return:** Returns control to CLI prompt cleanly upon completion.
13. **Run ID Linking:** Generates unique `RUN-YYYYMMDD-XXXX` linked to parent `WOS-YYYYMMDD-XXXX`.
14. **Calculated Results:** Produces deterministic, non-placeholder calculations.

---

## 2. Command Test Matrix

| Command / Option | Implementation Location | Expected Behavior | Test Cases | Actual Result | Status | Evidence | Defect & Resolution |
|---|---|---|---|---|---|---|---|
| **Main Menu 1: Create New Session** | `src/cli/cli_app.cpp:82` (`handleCreateSession`) | Generates session ID, prompts for description, processes, launches configuration wizard, saves to DB | Valid description, blank description, special characters, rapid sequential creation | Generates unique ID `WOS-YYYYMMDD-XXXX`, sets status to DRAFT or FINALIZED, persists to SQLite | **PASS** | `TestSessionLifecycle`, `TestIdGenerator` | Collisions occurred under rapid bursts; resolved with monotonic atomic sequence. |
| **Main Menu 2: Resume Session** | `src/cli/cli_app.cpp:113` (`handleResumeSession`) | Displays list of persisted sessions, loads session by ID or row number, resumes workload | Valid session ID, valid row number, invalid ID, empty table, malformed input | Correctly restores session metadata, settings, and process list from SQLite | **PASS** | `TestSessionLifecycle`, `TestEndToEndWorkflows` (W1) | None. |
| **Main Menu 3: View Session History** | `src/cli/cli_app.cpp:177` (`handleViewHistory`) | Displays tabular list of all saved sessions with creation dates, status, versions, process count | Empty database, single session, 50+ sessions | Formats clean ASCII table ordered by `created_at DESC` | **PASS** | `TestSessionLifecycle`, `TestDbPersistence` | None. |
| **Main Menu 4: Compare Sessions** | `src/cli/cli_app.cpp:182` (`handleCompareSessions`) | Prompts for 2 session IDs/rows, compares process count, total CPU burst, status | <2 sessions available, identical IDs, non-existent ID, valid comparison | Rejects gracefully if <2 sessions exist; displays side-by-side comparison table | **PASS** | Manual CLI validation, `TestSessionLifecycle` | None. |
| **Main Menu 5: Settings & Configuration** | `src/cli/cli_app.cpp:232` (`handleSettings`) | Views current kernel parameters (quantum, frames, aging, overhead) and toggles ANSI color | View parameters, toggle ANSI color, return to main menu | Correctly updates runtime `SystemSettings` and terminal escape mode | **PASS** | Manual CLI validation, ANSI compatibility fix | Windows 10+ virtual terminal processing enabled via `SetConsoleMode` with fallback. |
| **Main Menu 6: Exit WATCH OS** | `src/cli/cli_app.cpp:63` (`run`) | Closes active session, terminates SQLite connection, exits application cleanly | Direct exit, exit after running simulations | Terminates with return code 0; all committed records intact | **PASS** | `watchos.exe` exit testing | None. |
| **Config 1: Edit Process** | `src/session/configuration_manager.cpp:323` | Prompts for target PID, allows updating arrival, burst, priority, subsystems | Existing PID, non-existent PID, empty process list, invalid numbers | Correctly mutates target process in memory with validation | **PASS** | `TestWorkloadValidator`, `TestSessionLifecycle` | None. |
| **Config 2: Add New Process** | `src/session/configuration_manager.cpp:341` | Prompts for PID, arrival, burst, priority, subsystems; enforces PID uniqueness | Unique PID, duplicate PID, negative arrival, zero burst | Enforces `arrival >= 0`, `burst > 0`, rejects duplicate PIDs with warning | **PASS** | `TestWorkloadValidator`, `TestDbSchemaAudit` | None. |
| **Config 3: Remove Process** | `src/session/configuration_manager.cpp:346` | Prompts for PID and removes it from active session | Valid PID, non-existent PID, empty list | Removes process and re-runs workload validation | **PASS** | `TestWorkloadValidator`, `TestSessionLifecycle` | None. |
| **Config 4: Finalize Configuration & Persist** | `src/session/configuration_manager.cpp:365` | Validates entire workload, transitions status to FINALIZED, saves in SQLite transaction | Valid workload, invalid workload (duplicate PID, empty), DB write failure | Rejects invalid workloads with detailed error messages; commits transaction on success | **PASS** | `TestWorkloadValidator`, `TestDbPersistence` | None. |
| **Config 5: Return to Main Menu (Save Draft)** | `src/session/configuration_manager.cpp:373` | Saves session as DRAFT, returns to main menu without launching analysis console | Valid draft, partial draft | Persists session with `status = 'DRAFT'`, preserves configuration | **PASS** | `TestSessionLifecycle` | None. |
| **Resume 1: Enter Analysis Console** | `src/cli/cli_app.cpp:158` | Launches Analysis Console for the resumed session | Resumed finalized session, resumed draft session | Enters Analysis Console with active session context | **PASS** | `TestEndToEndWorkflows` (W1) | None. |
| **Resume 2: Review and Edit Workload** | `src/cli/cli_app.cpp:161` | Opens Review and Edit sub-menu for resumed session | Edit process, add process, remove process | Modifies workload and creates new configuration version in SQLite | **PASS** | `TestDiscoveredRegressions` (Reg 3) | Multi-version isolation verified. |
| **Resume 3: Return to Main Menu** | `src/cli/cli_app.cpp:153` | Returns from resumed session to main menu | Direct return | Returns to main loop without altering state | **PASS** | Manual CLI validation | None. |
| **Console 01 / `best`: Find Best Scheduler** | `src/cli/analysis_menu.cpp:149` | Evaluates multi-objective normalization across WT, TAT, RT, CS, and starvation; ranks algorithms | Valid workload, empty workload, single process | Displays normalized score breakdown and rank #1 recommendation | **PASS** | `TestEndToEndWorkflows` (W3), `TestCompleteKernelAnalysis` | Empty workload previously executed division; guarded with friendly alert. |
| **Console 02 / `schedule all`: Run All Schedulers** | `src/cli/analysis_menu.cpp:156` | Executes FCFS, SJF, SRTF, Priority (P & NP), RR; records run, metrics, Gantt, and results to SQLite | Normal workload, simultaneous arrivals, CPU idle gaps, single process, empty workload | Computes exact metrics; inserts into `simulation_runs`, `scheduling_results`, `gantt_segments` | **PASS** | `TestCpuSchedulers`, `TestEdgeCasesSchedulers`, `TestDbPersistence` | PID sorting in output unified across all algorithms. |
| **Console 03 / `gantt`: Display Gantt Charts** | `src/cli/analysis_menu.cpp:217` | Visualizes timeline with process segments, idle slots, and time markers | Discontinuous execution, idle gaps, preemption slices | Renders clean ASCII timeline matching calculated start/end times | **PASS** | `TestEdgeCasesSchedulers`, `TestCpuSchedulers` | Gantt segments guaranteed non-overlapping. |
| **Console 04 / `show lifecycle`: Show Lifecycle** | `src/cli/analysis_menu.cpp:226` | Tracks state transitions (NEW -> READY -> RUNNING -> WAITING -> TERMINATED) | Standard execution, I/O blocking, preemptions | Records transitions into `process_state_history`; prints lifecycle metrics table | **PASS** | `TestEndToEndWorkflows` (W4), `TestSessionLifecycle` | Persistence to `process_state_history` implemented. |
| **Console 05 / `context switch`: Context Switching** | `src/cli/analysis_menu.cpp:256` | Details voluntary vs preemption switches across algorithms | Single process (0 CS), high preemption RR, non-preemptive | Displays initial dispatch count vs preemption context switches | **PASS** | `TestEdgeCasesSchedulers`, `TestCpuSchedulers` | Single process correctly records 0 inter-process context switches. |
| **Console 06 / `starvation`: Detect Starvation** | `src/cli/analysis_menu.cpp:267` | Flags processes waiting >= 10 time units | Low burst vs high burst, heavy priority bias | Identifies algorithm-specific starvation breaches and maximum wait times | **PASS** | `TestCpuSchedulers`, `TestEdgeCasesSchedulers` | None. |
| **Console 07 / `aging`: Apply Aging and Compare** | `src/cli/analysis_menu.cpp:282` | Simulates priority aging (interval=5, increment=1) and compares with static priority | Static priority starvation, aged priority resolution | Produces before-and-after comparison showing reduced max waiting time | **PASS** | `TestCpuSchedulers`, `TestCompleteKernelAnalysis` | None. |
| **Console 08 / `deadlock analyze`: Deadlock Analysis** | `src/cli/analysis_menu.cpp:288` | Evaluates RAG graph, Banker's safe sequence discovery, and cycle detection | Safe allocation state, unsafe state, circular wait deadlock | Displays RAG adjacency, safe sequence or deadlock cycle, records into `deadlock_results` | **PASS** | `TestDeadlockBankers`, `TestEndToEndWorkflows` (W6) | Database persistence to `deadlock_results` implemented. |
| **Console 09 / `monte carlo`: Deadlock Frequency** | `src/cli/analysis_menu.cpp:312` | Runs 1,000 stochastic resource request sequences to estimate deadlock probability | Uncontended resources, highly contended resources | Computes empirical deadlock frequency with fixed seed (42) for reproducibility | **PASS** | `TestDeadlockBankers`, `TestCompleteKernelAnalysis` | Database persistence to `deadlock_results` implemented. |
| **Console 10 / `memory analyze`: Memory Analysis** | `src/cli/analysis_menu.cpp:325` | Simulates FIFO, LRU, Optimal page replacement; calculates hits, faults, ratio, thrashing | Belady reference string, small frame count (3), large frame count | Displays frame-by-frame snapshot tables, records results in `memory_results` | **PASS** | `TestMemoryPaging`, `TestEndToEndWorkflows` (W5) | Database persistence to `memory_results` implemented. |
| **Console 11 / `disk analyze`: Disk Usage & I/O** | `src/cli/analysis_menu.cpp:334` | Evaluates FCFS, SSTF, SCAN, C-SCAN head movement on disk cylinders | Standard 8-cylinder workload (head=53), boundary cylinders (0, 199) | Calculates total & average head movement, records into `io_results` | **PASS** | `TestDiskScheduling`, `TestEndToEndWorkflows` (W7) | Database persistence to `io_results` implemented. |
| **Console 12 / `sync`: Synchronization & IPC** | `src/cli/analysis_menu.cpp:350` | Simulates bounded-buffer producer-consumer with mutex, semaphores, and shared memory | Full buffer blocking, empty buffer blocking, concurrent threads | Logs atomic events (PRODUCE, CONSUME, WAIT, SIGNAL) to console | **PASS** | `TestCompleteKernelAnalysis` | Deterministic pseudo-concurrency verified. |
| **Console 13 / `permissions`: Permissions & Protection** | `src/cli/analysis_menu.cpp:357` | Enforces RBAC permissions (root vs guest) and base/limit memory partition isolation | Valid read/write, unauthorized write, address in bounds, segmentation fault | Emits ALLOW/DENY decisions and persists audit events in `permission_events` | **PASS** | `TestEndToEndWorkflows` (W8), `TestCompleteKernelAnalysis` | Database persistence to `permission_events` implemented. |
| **Console 14 / `complete`: Complete Kernel Analysis** | `src/cli/analysis_menu.cpp:377` | Integrates scheduling, deadlock, memory, disk, permissions, and Jarvis Explainer summary | Standard multi-process session, empty session | Orchestrates complete multi-subsystem simulation; records all results | **PASS** | `TestCompleteKernelAnalysis` | Guarded against empty session. |
| **Console 15 / `compare runs`: Compare Previous Runs** | `src/cli/analysis_menu.cpp:392` | Queries SQLite for all previous simulation runs for the active session | Session with 0 runs, session with multiple runs | Displays Run ID, timestamp, status, module parameters table | **PASS** | `TestEndToEndWorkflows` (W2) | Clean tabular output. |
| **Console 16 / `view logs`: View Session Logs** | `src/cli/analysis_menu.cpp:412` | Queries granular simulation events and operational session audit logs from SQLite | Session with no logs, session with multiple runs and lifecycle logs | Displays events by timestamp and session operational audit logs | **PASS** | `TestDbPersistence`, `TestDbSchemaAudit` | Dual query implemented (events + session operational logs). |
| **Console 17 / `report generate`: Generate Report** | `src/cli/analysis_menu.cpp:432` | Synthesizes full report, exports Markdown and CSV files to `reports/`, records in DB | Valid session, empty session | Exports `reports/report_<sessionId>.md` and `.csv`, records in `reports` table | **PASS** | `TestEndToEndWorkflows` (W9) | CSV metadata headers added; foreign keys satisfied. |
| **Console 18 / `back`: Return to Main Menu** | `src/cli/analysis_menu.cpp:55` | Exits Analysis Console loop and returns control to Main Menu | Normal return | Returns cleanly to Main Menu loop | **PASS** | Manual CLI validation | Clean loop exit. |
| **CLI Command: `help` / `?`** | `src/cli/analysis_menu.cpp:104` | Prints Analysis Console menu options | Direct invocation | Re-displays 18 options with active session status | **PASS** | `TestEndToEndWorkflows` | Added to command parser. |

---

## 3. Summary of Statuses

- **Total Accessible Commands Audited:** 35 (including sub-menus and command aliases)
- **PASS:** 35 (100%)
- **FAIL:** 0 (0%)
- **PARTIAL:** 0 (0%)
- **NOT IMPLEMENTED:** 0 (0%)
- **NOT TESTED:** 0 (0%)
