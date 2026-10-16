# WATCH OS — Complete Database Data Dictionary & Schema Reference

**Document Version:** 2.0  
**Target Engine:** SQLite 3 (Amalgamation)  
**Database File:** `data/watchos_sessions.db`  
**Location:** [`docs/DATABASE_DATA_DICTIONARY.md`](file:///c:/Users/saksh/OneDrive/Desktop/EDITH/docs/DATABASE_DATA_DICTIONARY.md)

---

## 1. Overview & Architectural Groupings

The WATCH OS persistence layer is organized into **6 functional subsystems** containing **24 relational entities** and 1 schema migration registry. Every table enforces 3NF relational integrity, foreign key cascading, and domain `CHECK` constraints.

```
┌────────────────────────────────────────────────────────────────────────────────────────┐
│                                   SUBSYSTEM MAP                                        │
├───────────────────────────────┬────────────────────────────────────────────────────────┤
│ 1. Session & Workload Core    │ sessions, configuration_versions, processes            │
│ 2. Subsystem Input Specs      │ io_requests, resource_claims, disk_requests,           │
│                               │ memory_workloads, sync_configs, access_policies        │
│ 3. Simulation Runs & History  │ simulation_runs, simulation_events, gantt_segments,   │
│                               │ process_state_history                                  │
│ 4. Analysis Results           │ scheduling_results, memory_results, deadlock_results,  │
│                               │ io_results, permission_events, resource_allocations    │
│ 5. What-If & Checkpoints      │ experiment_results, checkpoints                        │
│ 6. Auditing & Reporting       │ schema_migrations, event_logs, reports                 │
└───────────────────────────────┴────────────────────────────────────────────────────────┘
```

---

## 2. Comprehensive Table & Column Dictionary

---

### Subsystem 1: Session & Workload Core

#### 1. `sessions`
* **What it shows:** High-level user sessions. Represents an isolated workspace containing configured workloads and their associated simulation runs.
* **Primary Key:** `session_id`

| Column | Data Type | Constraints | Description & Usage |
|---|---|---|---|
| `session_id` | `TEXT` | `PRIMARY KEY` | Unique human-readable session identifier (e.g. `WOS-20261016-ALLFEAT`). Prefix `WOS-` + Date + Monotonic Sequence. |
| `title` | `TEXT` | Nullable | Optional user-defined display name or title for the session. |
| `created_at` | `TEXT` | `NOT NULL` | ISO-8601 UTC timestamp recording when the session was created. |
| `updated_at` | `TEXT` | Nullable | ISO-8601 UTC timestamp updated whenever workload configuration changes. |
| `last_opened_at`| `TEXT` | Nullable | ISO-8601 UTC timestamp tracking when the session was last resumed in the CLI. |
| `ended_at` | `TEXT` | Nullable | Optional timestamp recording session archival or completion. |
| `status` | `TEXT` | `NOT NULL DEFAULT 'DRAFT'` | Session lifecycle state (`DRAFT`, `CONFIGURING`, `READY`, `ACTIVE`, `FINALIZED`, `ARCHIVED`). |
| `active_config_version` | `INTEGER` | `NOT NULL DEFAULT 1` | Points to the active workload configuration version to load upon resume. |
| `description` | `TEXT` | Nullable | Detailed notes regarding the workload, research objective, or test scenario. |
| `notes` | `TEXT` | Nullable | Additional user scratchpad notes. |
| `metadata` | `TEXT` | Nullable | JSON string storing client UI settings, environment tags, or custom metadata. |

---

#### 2. `configuration_versions`
* **What it shows:** Immutable version snapshots of a session's workload. Editing a workload creates a new version, preserving historical inputs for previous simulation runs.
* **Primary Key:** `id` (Auto-increment) | **Composite Unique:** `(session_id, version)`
* **Foreign Key:** `session_id` $\to$ `sessions(session_id) ON DELETE CASCADE`

| Column | Data Type | Constraints | Description & Usage |
|---|---|---|---|
| `id` | `INTEGER` | `PRIMARY KEY AUTOINCREMENT` | Surrogate numeric identifier for the configuration record. |
| `session_id` | `TEXT` | `NOT NULL`, `FOREIGN KEY` | The parent session this configuration belongs to. |
| `version` | `INTEGER` | `NOT NULL CHECK (version >= 1)` | Monotonic version integer (1, 2, 3...) unique within the session. |
| `created_at` | `TEXT` | `NOT NULL` | ISO-8601 timestamp recording when this configuration version was created. |
| `is_finalized` | `INTEGER` | `NOT NULL DEFAULT 0 CHECK (is_finalized IN (0, 1))` | Boolean flag (`1` = locked for simulation; `0` = editable draft). |
| `settings_json`| `TEXT` | Nullable | JSON-serialized system settings (quantum, page frames, aging thresholds) applied to this version. |

---

#### 3. `processes`
* **What it shows:** Core simulated OS processes belonging to a specific session configuration version.
* **Primary Key:** `id` (Auto-increment) | **Composite Unique:** `(session_id, config_version, pid)`
* **Foreign Key:** `session_id` $\to$ `sessions(session_id) ON DELETE CASCADE`

| Column | Data Type | Constraints | Description & Usage |
|---|---|---|---|
| `id` | `INTEGER` | `PRIMARY KEY AUTOINCREMENT` | Surrogate record identifier. |
| `session_id` | `TEXT` | `NOT NULL`, `FOREIGN KEY` | Links process to its parent session. |
| `config_version` | `INTEGER` | `NOT NULL DEFAULT 1` | Isolates the process definition to a specific configuration revision. |
| `pid` | `INTEGER` | `NOT NULL` | Simulated Process ID (e.g. `101`, `102`). Unique per session version. |
| `arrival_time` | `INTEGER` | `NOT NULL CHECK (arrival_time >= 0)` | Clock tick at which the process enters the READY queue. Cannot be negative. |
| `cpu_burst` | `INTEGER` | `NOT NULL CHECK (cpu_burst > 0)` | Total execution cycles/time required on the CPU. Must be strictly positive. |
| `priority` | `INTEGER` | `NOT NULL` | Scheduling priority integer (lower numerical value = higher priority). |
| `memory_demand`| `INTEGER` | `CHECK (memory_demand IS NULL OR memory_demand > 0)` | Optional resident memory allocation request in memory units (e.g. 64 units). |

---

### Subsystem 2: Subsystem Input Specifications

#### 4. `process_io_requests`
* **What it shows:** Discrete I/O burst requests scheduled during a process's CPU execution.
* **Foreign Key:** `session_id` $\to$ `sessions(session_id) ON DELETE CASCADE`

| Column | Data Type | Constraints | Description & Usage |
|---|---|---|---|
| `id` | `INTEGER` | `PRIMARY KEY AUTOINCREMENT` | Surrogate request identifier. |
| `session_id` | `TEXT` | `NOT NULL`, `FOREIGN KEY` | Parent session. |
| `config_version` | `INTEGER` | `NOT NULL DEFAULT 1` | Configuration version. |
| `pid` | `INTEGER` | `NOT NULL` | Process initiating the I/O operation. |
| `request_time` | `INTEGER` | `NOT NULL CHECK (request_time >= 0)` | Relative execution offset within the CPU burst when I/O is requested. |
| `duration` | `INTEGER` | `NOT NULL CHECK (duration > 0)` | Time units the process remains in WAITING state before returning to READY. |
| `device` | `TEXT` | `NOT NULL DEFAULT 'DISK'` | Target peripheral device name (`DISK`, `NETWORK_CARD`, `KEYBOARD`). |

---

#### 5. `resource_types`
* **What it shows:** Inventory of finite, reusable system resources available in the OS simulation.
* **Composite Unique:** `(session_id, resource_name)`
* **Foreign Key:** `session_id` $\to$ `sessions(session_id) ON DELETE CASCADE`

| Column | Data Type | Constraints | Description & Usage |
|---|---|---|---|
| `id` | `INTEGER` | `PRIMARY KEY AUTOINCREMENT` | Surrogate resource identifier. |
| `session_id` | `TEXT` | `NOT NULL`, `FOREIGN KEY` | Parent session. |
| `resource_name`| `TEXT` | `NOT NULL` | Descriptive name (e.g. `CPU_CORES`, `DB_MUTEX`, `NVME_CHANNEL`). |
| `total_instances` | `INTEGER` | `NOT NULL CHECK (total_instances >= 0)` | Total physical/logical instances available in the system pool. |

---

#### 6. `process_resource_claims`
* **What it shows:** Static maximum resource demand declarations and initial allocations for Banker's Algorithm deadlock avoidance.
* **Foreign Key:** `session_id` $\to$ `sessions(session_id) ON DELETE CASCADE`

| Column | Data Type | Constraints | Description & Usage |
|---|---|---|---|
| `id` | `INTEGER` | `PRIMARY KEY AUTOINCREMENT` | Surrogate claim identifier. |
| `session_id` | `TEXT` | `NOT NULL`, `FOREIGN KEY` | Parent session. |
| `config_version` | `INTEGER` | `NOT NULL DEFAULT 1` | Workload configuration version. |
| `pid` | `INTEGER` | `NOT NULL` | The claimant process. |
| `resource_type`| `TEXT` | `NOT NULL` | Target resource matching `resource_types.resource_name`. |
| `max_claim` | `INTEGER` | `NOT NULL CHECK (max_claim >= 0)` | Maximum instances the process will ever request simultaneously. |
| `initial_allocation` | `INTEGER` | `CHECK (0 <= initial_allocation <= max_claim)` | Instances pre-allocated to the process at start. Cannot exceed `max_claim`. |

---

#### 7. `resource_allocations`
* **What it shows:** Dynamic, per-run state snapshots of allocated and pending resource counts.
* **Foreign Key:** `run_id` $\to$ `simulation_runs(run_id) ON DELETE CASCADE`

| Column | Data Type | Constraints | Description & Usage |
|---|---|---|---|
| `id` | `INTEGER` | `PRIMARY KEY AUTOINCREMENT` | Surrogate identifier. |
| `run_id` | `TEXT` | `NOT NULL`, `FOREIGN KEY` | Associated simulation execution run. |
| `pid` | `INTEGER` | `NOT NULL` | Target process ID. |
| `resource_name`| `TEXT` | `NOT NULL` | Resource identifier. |
| `allocated_count` | `INTEGER` | `CHECK (allocated_count >= 0)` | Currently held instances during the run. |
| `requested_count` | `INTEGER` | `CHECK (requested_count >= 0)` | Currently pending request instances blocking the process. |

---

#### 8. `process_disk_requests`
* **What it shows:** Cylinder/block address access sequence for disk arm motion simulation.
* **Foreign Key:** `session_id` $\to$ `sessions(session_id) ON DELETE CASCADE`

| Column | Data Type | Constraints | Description & Usage |
|---|---|---|---|
| `id` | `INTEGER` | `PRIMARY KEY AUTOINCREMENT` | Surrogate record identifier. |
| `session_id` | `TEXT` | `NOT NULL`, `FOREIGN KEY` | Parent session. |
| `config_version` | `INTEGER` | `NOT NULL DEFAULT 1` | Configuration version. |
| `pid` | `INTEGER` | `NOT NULL` | Initiating process ID. |
| `block_or_cylinder` | `INTEGER` | `CHECK (block_or_cylinder >= 0)` | Target track/cylinder number on the storage platter (e.g. 0–199). |
| `operation_type` | `TEXT` | `NOT NULL DEFAULT 'READ'` | Disk operation type (`READ`, `WRITE`). |
| `volume_bytes` | `INTEGER` | `CHECK (volume_bytes > 0)` | Data transfer payload size in bytes (e.g. 4096 bytes). |

---

#### 9. `memory_workloads`
* **What it shows:** Virtual memory paging reference strings and frame configuration per process.
* **Foreign Key:** `session_id` $\to$ `sessions(session_id) ON DELETE CASCADE`

| Column | Data Type | Constraints | Description & Usage |
|---|---|---|---|
| `id` | `INTEGER` | `PRIMARY KEY AUTOINCREMENT` | Surrogate workload identifier. |
| `session_id` | `TEXT` | `NOT NULL`, `FOREIGN KEY` | Parent session. |
| `config_version` | `INTEGER` | `NOT NULL DEFAULT 1` | Configuration version. |
| `pid` | `INTEGER` | `NOT NULL` | Target process ID. |
| `page_size` | `INTEGER` | `CHECK (page_size > 0)` | Virtual page size in KB (default: 4 KB). |
| `frame_count` | `INTEGER` | `CHECK (frame_count > 0)` | Number of physical memory frames allocated to this process. |
| `reference_string_json` | `TEXT` | Nullable | JSON array of virtual page numbers accessed (e.g. `[7, 0, 1, 2, 0, 3]`). |

---

#### 10. `synchronization_configs`
* **What it shows:** Critical section specifications and concurrency synchronization primitive definitions.
* **Foreign Key:** `session_id` $\to$ `sessions(session_id) ON DELETE CASCADE`

| Column | Data Type | Constraints | Description & Usage |
|---|---|---|---|
| `id` | `INTEGER` | `PRIMARY KEY AUTOINCREMENT` | Surrogate record identifier. |
| `session_id` | `TEXT` | `NOT NULL`, `FOREIGN KEY` | Parent session. |
| `config_version` | `INTEGER` | `NOT NULL DEFAULT 1` | Configuration version. |
| `pid` | `INTEGER` | `NOT NULL` | Participating process ID. |
| `primitive_name` | `TEXT` | `NOT NULL` | Primitive type (`MUTEX`, `COUNTING_SEMAPHORE`, `BARRIER`). |
| `cs_start_time` | `INTEGER` | `CHECK (cs_start_time >= 0)` | Clock time process attempts to acquire lock/enter critical section. |
| `cs_duration` | `INTEGER` | `CHECK (cs_duration > 0)` | Execution duration spent inside protected critical section. |

---

#### 11. `access_policies`
* **What it shows:** Role-Based Access Control (RBAC) security rules determining resource permissions.
* **Foreign Key:** `session_id` $\to$ `sessions(session_id) ON DELETE CASCADE`

| Column | Data Type | Constraints | Description & Usage |
|---|---|---|---|
| `id` | `INTEGER` | `PRIMARY KEY AUTOINCREMENT` | Surrogate policy identifier. |
| `session_id` | `TEXT` | `NOT NULL`, `FOREIGN KEY` | Parent session. |
| `user_or_role` | `TEXT` | `NOT NULL` | Security principal (e.g. `root`, `guest`, `web_service`). |
| `resource_name`| `TEXT` | `NOT NULL` | Protected target (`system_config`, `storage_pool`, `kernel_memory`). |
| `operation` | `TEXT` | `NOT NULL` | Operation requested (`READ`, `WRITE`, `EXECUTE`). |
| `policy` | `TEXT` | `NOT NULL DEFAULT 'ALLOW'` | Enforcement decision rule (`ALLOW` or `DENY`). |

---

### Subsystem 3: Simulation Runs & Timeline History

#### 12. `simulation_runs`
* **What it shows:** One execution run of an analysis module against a specific session configuration.
* **Primary Key:** `run_id` | **Foreign Key:** `session_id` $\to$ `sessions(session_id) ON DELETE CASCADE`

| Column | Data Type | Constraints | Description & Usage |
|---|---|---|---|
| `run_id` | `TEXT` | `PRIMARY KEY` | Unique execution identifier (e.g. `RUN-20261016-ALLFEAT`). |
| `session_id` | `TEXT` | `NOT NULL`, `FOREIGN KEY` | The session against which this run was executed. |
| `module` | `TEXT` | `NOT NULL` | Simulated engine (`CPU_SCHEDULING`, `MEMORY_ANALYSIS`, `DEADLOCK_ANALYSIS`). |
| `status` | `TEXT` | `CHECK (status IN ('RUNNING','COMPLETED','FAILED','ABORTED'))` | Lifecycle status of the execution run. |
| `parameters` | `TEXT` | Nullable | Parameter string (e.g. algorithms evaluated, quantum, random seed). |
| `created_at` | `TEXT` | `NOT NULL` | ISO-8601 UTC timestamp of execution launch. |

---

#### 13. `simulation_events`
* **What it shows:** Micro-level discrete event timeline tracking every state transition and kernel dispatch in chronological order.
* **Foreign Key:** `run_id` $\to$ `simulation_runs(run_id) ON DELETE CASCADE`

| Column | Data Type | Constraints | Description & Usage |
|---|---|---|---|
| `id` | `INTEGER` | `PRIMARY KEY AUTOINCREMENT` | Surrogate event order identifier. |
| `run_id` | `TEXT` | `NOT NULL`, `FOREIGN KEY` | Parent simulation run. |
| `timestamp` | `INTEGER` | `CHECK (timestamp >= 0)` | Simulated clock tick when event occurred. |
| `event_type` | `TEXT` | `NOT NULL` | Category (`PROCESS_ARRIVE`, `CPU_DISPATCH`, `PREEMPTION`, `IO_REQUEST`). |
| `entity_id` | `INTEGER` | Nullable | Process ID or resource ID associated with the event. |
| `details` | `TEXT` | Nullable | Human-readable explanation of event cause and effect. |

---

#### 14. `process_state_history`
* **What it shows:** Formal 5-state process lifecycle transition ledger (NEW, READY, RUNNING, WAITING, TERMINATED).
* **Foreign Key:** `run_id` $\to$ `simulation_runs(run_id) ON DELETE CASCADE`

| Column | Data Type | Constraints | Description & Usage |
|---|---|---|---|
| `id` | `INTEGER` | `PRIMARY KEY AUTOINCREMENT` | Surrogate transition identifier. |
| `run_id` | `TEXT` | `NOT NULL`, `FOREIGN KEY` | Parent simulation run. |
| `pid` | `INTEGER` | `NOT NULL` | The transitioning process. |
| `timestamp` | `INTEGER` | `CHECK (timestamp >= 0)` | Clock time of state change. |
| `from_state` | `TEXT` | `NOT NULL` | Preceding state (`NEW`, `READY`, `RUNNING`, `WAITING`). |
| `to_state` | `TEXT` | `NOT NULL` | Succeeding state (`READY`, `RUNNING`, `WAITING`, `TERMINATED`). |
| `reason` | `TEXT` | Nullable | Trigger condition (e.g. `Arrival`, `CPU completed`, `I/O blocked`). |

---

#### 15. `gantt_segments`
* **What it shows:** Contiguous, non-overlapping intervals representing CPU occupancy and idle periods.
* **Foreign Key:** `run_id` $\to$ `simulation_runs(run_id) ON DELETE CASCADE`

| Column | Data Type | Constraints | Description & Usage |
|---|---|---|---|
| `id` | `INTEGER` | `PRIMARY KEY AUTOINCREMENT` | Surrogate segment identifier. |
| `run_id` | `TEXT` | `NOT NULL`, `FOREIGN KEY` | Parent simulation run. |
| `start_time` | `INTEGER` | `CHECK (start_time >= 0)` | Clock start of the segment. |
| `end_time` | `INTEGER` | `CHECK (end_time >= start_time)` | Clock end of the segment. Must be $\ge$ `start_time`. |
| `pid` | `INTEGER` | Nullable | Process occupying CPU (or `0` / `NULL` if idle). |
| `is_idle` | `INTEGER` | `NOT NULL DEFAULT 0` | Flag (`1` = CPU had no ready processes; `0` = process running). |
| `is_context_switch`| `INTEGER`| `NOT NULL DEFAULT 0` | Flag (`1` = segment represents context switch overhead). |

---

### Subsystem 4: Simulation Results

#### 16. `scheduling_results`
* **What it shows:** Per-process CPU scheduling performance metrics computed at simulation conclusion.
* **Foreign Key:** `run_id` $\to$ `simulation_runs(run_id) ON DELETE CASCADE`

| Column | Data Type | Constraints | Description & Usage |
|---|---|---|---|
| `id` | `INTEGER` | `PRIMARY KEY AUTOINCREMENT` | Surrogate result identifier. |
| `run_id` | `TEXT` | `NOT NULL`, `FOREIGN KEY` | Parent simulation run. |
| `pid` | `INTEGER` | `NOT NULL` | Simulated process ID. |
| `arrival_time` | `INTEGER` | `CHECK (arrival_time >= 0)` | Copied arrival time. |
| `cpu_burst` | `INTEGER` | `CHECK (cpu_burst > 0)` | Copied execution burst. |
| `priority` | `INTEGER` | `NOT NULL` | Priority value. |
| `completion_time`| `INTEGER` | `CHECK (completion_time >= arrival_time)` | Clock time process terminated ($C_i$). |
| `turnaround_time`| `INTEGER` | `CHECK (turnaround_time >= cpu_burst)` | Total elapsed residency time: $TAT = C_i - A_i$. |
| `waiting_time` | `INTEGER` | `CHECK (waiting_time >= 0)` | Total time spent waiting in READY queue: $WT = TAT - Burst$. |
| `response_time`| `INTEGER` | `CHECK (response_time >= 0)` | Latency to first CPU dispatch: $RT = Start - A_i$. |

---

#### 17. `memory_results`
* **What it shows:** Page replacement algorithm efficiency statistics (FIFO, LRU, Optimal).
* **Foreign Key:** `run_id` $\to$ `simulation_runs(run_id) ON DELETE CASCADE`

| Column | Data Type | Constraints | Description & Usage |
|---|---|---|---|
| `id` | `INTEGER` | `PRIMARY KEY AUTOINCREMENT` | Surrogate record identifier. |
| `run_id` | `TEXT` | `NOT NULL`, `FOREIGN KEY` | Parent simulation run. |
| `algorithm` | `TEXT` | `NOT NULL` | Paging algorithm (`FIFO`, `LRU`, `OPTIMAL`). |
| `total_references` | `INTEGER` | `CHECK (total_references >= 0)` | Count of memory page requests processed. |
| `page_faults` | `INTEGER` | `CHECK (page_faults >= 0)` | Number of references causing page faults. |
| `page_hits` | `INTEGER` | `CHECK (page_hits >= 0)` | Number of references satisfied by resident frames. |
| `fault_ratio` | `REAL` | `CHECK (fault_ratio >= 0.0 AND fault_ratio <= 1.0)` | Ratio of faults to total accesses ($\frac{\text{faults}}{\text{total}}$). |
| `replacements` | `INTEGER` | `CHECK (replacements >= 0)` | Number of victim pages evicted to secondary storage. |
| `frames_allocated` | `INTEGER` | `CHECK (frames_allocated > 0)` | Physical frame capacity allocated to the test. |
| `thrashing_detected` | `INTEGER` | `NOT NULL DEFAULT 0` | Boolean indicator if fault frequency exceeded thrashing threshold. |

---

#### 18. `deadlock_results`
* **What it shows:** Banker's Algorithm safety evaluation and Monte Carlo stochastic deadlock frequency results.
* **Foreign Key:** `run_id` $\to$ `simulation_runs(run_id) ON DELETE CASCADE`

| Column | Data Type | Constraints | Description & Usage |
|---|---|---|---|
| `id` | `INTEGER` | `PRIMARY KEY AUTOINCREMENT` | Surrogate identifier. |
| `run_id` | `TEXT` | `NOT NULL`, `FOREIGN KEY` | Parent simulation run. |
| `is_deadlocked`| `INTEGER` | `NOT NULL DEFAULT 0` | Flag (`1` = circular wait detected; `0` = deadlock-free). |
| `is_safe` | `INTEGER` | `NOT NULL DEFAULT 1` | Flag (`1` = system is in safe state; `0` = unsafe state). |
| `safe_sequence`| `TEXT` | Nullable | Discovered safe execution path (e.g. `P101 -> P103 -> P104 -> P102`). |
| `deadlocked_processes` | `TEXT` | Nullable | List of PIDs entangled in circular wait graph. |
| `frequency_trials` | `INTEGER` | `CHECK (frequency_trials >= 0)` | Number of stochastic simulation trials executed (e.g. 1000). |
| `frequency_deadlocks` | `INTEGER` | `CHECK (frequency_deadlocks >= 0)` | Number of trials that terminated in deadlock. |
| `empirical_frequency` | `REAL` | `CHECK (0.0 <= empirical_frequency <= 1.0)` | Empirical probability: $\frac{\text{deadlocks}}{\text{trials}}$. |

---

#### 19. `io_results`
* **What it shows:** Disk head travel metrics across disk arm scheduling algorithms.
* **Foreign Key:** `run_id` $\to$ `simulation_runs(run_id) ON DELETE CASCADE`

| Column | Data Type | Constraints | Description & Usage |
|---|---|---|---|
| `id` | `INTEGER` | `PRIMARY KEY AUTOINCREMENT` | Surrogate record identifier. |
| `run_id` | `TEXT` | `NOT NULL`, `FOREIGN KEY` | Parent simulation run. |
| `algorithm` | `TEXT` | `NOT NULL` | Disk scheduler (`FCFS`, `SSTF`, `SCAN`, `C-SCAN`). |
| `total_head_movement` | `INTEGER` | `CHECK (total_head_movement >= 0)` | Cumulative cylinder tracks traversed by the disk head. |
| `average_head_movement`| `REAL` | `CHECK (average_head_movement >= 0.0)` | Average seek distance per serviced I/O request. |
| `service_order`| `TEXT` | `NOT NULL` | Comma-separated cylinder sequence in order of service. |

---

#### 20. `permission_events`
* **What it shows:** Access control and memory protection boundary enforcement log.
* **Foreign Key:** `run_id` $\to$ `simulation_runs(run_id) ON DELETE CASCADE`

| Column | Data Type | Constraints | Description & Usage |
|---|---|---|---|
| `id` | `INTEGER` | `PRIMARY KEY AUTOINCREMENT` | Surrogate audit identifier. |
| `run_id` | `TEXT` | `NOT NULL`, `FOREIGN KEY` | Parent simulation run. |
| `actor` | `TEXT` | `NOT NULL` | Initiating process or security principal (e.g. `PID 101`, `root`). |
| `resource` | `TEXT` | `NOT NULL` | Target object accessed (e.g. `system_config`, `0x00401000`). |
| `operation` | `TEXT` | `NOT NULL` | Operation attempted (`READ`, `WRITE`, `EXECUTE`). |
| `decision` | `TEXT` | `NOT NULL CHECK (decision IN ('ALLOW','DENY'))` | Kernel security decision (`ALLOW` or `DENY`). |
| `reason` | `TEXT` | `NOT NULL` | Explanation (e.g. `Root privileges`, `Out-of-bounds segmentation fault`). |

---

### Subsystem 5: What-If Experiments & Checkpointing

#### 21. `experiment_results`
* **What it shows:** Comparative sensitivity analysis logging metric deltas when system parameters are tuned.

| Column | Data Type | Constraints | Description & Usage |
|---|---|---|---|
| `id` | `INTEGER` | `PRIMARY KEY AUTOINCREMENT` | Surrogate experiment identifier. |
| `experiment_id`| `TEXT` | `NOT NULL` | Experiment cohort identifier (e.g. `EXP-001`). |
| `base_run_id` | `TEXT` | `NOT NULL` | Baseline run identifier prior to parameter change. |
| `variant_run_id`| `TEXT` | `NOT NULL` | Variant run identifier with modified parameter. |
| `parameter_changed` | `TEXT` | `NOT NULL` | Target parameter modified (e.g. `time_quantum`, `frame_count`). |
| `base_value` | `TEXT` | `NOT NULL` | Original baseline parameter value. |
| `variant_value`| `TEXT` | `NOT NULL` | Modified variant parameter value. |
| `metric_delta_json` | `TEXT` | `NOT NULL` | JSON delta showing percentage changes in average waiting time, turnaround, etc. |

---

#### 22. `checkpoints`
* **What it shows:** Serialized simulation state snapshots for pausing, resuming, or debugging runs at specific simulation timestamps.
* **Primary Key:** `checkpoint_id`
* **Foreign Keys:** `session_id` $\to$ `sessions`, `run_id` $\to$ `simulation_runs`

| Column | Data Type | Constraints | Description & Usage |
|---|---|---|---|
| `checkpoint_id`| `TEXT` | `PRIMARY KEY` | Unique snapshot identifier (e.g. `CHK-20261016-01`). |
| `session_id` | `TEXT` | `NOT NULL`, `FOREIGN KEY` | Associated session. |
| `run_id` | `TEXT` | `NOT NULL`, `FOREIGN KEY` | Associated simulation run. |
| `simulated_time` | `INTEGER` | `CHECK (simulated_time >= 0)` | Simulation clock tick at snapshot capture. |
| `state_snapshot` | `TEXT` | `NOT NULL` | JSON state containing registers, ready queue, running PID, remaining bursts. |
| `created_at` | `TEXT` | `NOT NULL` | Wall-clock timestamp when snapshot was captured. |

---

### Subsystem 6: Auditing & Reporting

#### 23. `schema_migrations`
* **What it shows:** Applied database migration ledger ensuring schema evolution without data loss.
* **Primary Key:** `version`

| Column | Data Type | Constraints | Description & Usage |
|---|---|---|---|
| `version` | `INTEGER` | `PRIMARY KEY` | Monotonic schema migration version integer (e.g. `1`). |
| `applied_at` | `TEXT` | `NOT NULL` | ISO-8601 timestamp when migration was executed. |
| `description` | `TEXT` | Nullable | Human-readable migration summary. |

---

#### 24. `event_logs`
* **What it shows:** Operational system audit log capturing application events, warnings, errors, and informational notices.
* **Foreign Key:** `session_id` $\to$ `sessions(session_id) ON DELETE CASCADE`

| Column | Data Type | Constraints | Description & Usage |
|---|---|---|---|
| `id` | `INTEGER` | `PRIMARY KEY AUTOINCREMENT` | Surrogate log entry identifier. |
| `session_id` | `TEXT` | Nullable, `FOREIGN KEY` | Session context if triggered inside a session; NULL if global. |
| `log_level` | `TEXT` | `NOT NULL` | Log severity (`INFO`, `WARNING`, `ERROR`, `SUCCESS`). |
| `source` | `TEXT` | `NOT NULL` | Kernel module originating log (`KERNEL_BOOT`, `SCHEDULER`, `STORAGE`). |
| `message` | `TEXT` | `NOT NULL` | Detailed log text. |
| `timestamp` | `TEXT` | `NOT NULL` | ISO-8601 UTC timestamp. |

---

#### 25. `reports`
* **What it shows:** Registry of generated and exported analytical reports (Markdown and CSV formats).
* **Primary Key:** `report_id`
* **Foreign Keys:** `session_id` $\to$ `sessions`, `run_id` $\to$ `simulation_runs`

| Column | Data Type | Constraints | Description & Usage |
|---|---|---|---|
| `report_id` | `TEXT` | `PRIMARY KEY` | Unique report identifier (e.g. `RPT-WOS-20261016-ALLFEAT`). |
| `session_id` | `TEXT` | `NOT NULL`, `FOREIGN KEY` | Source session. |
| `run_id` | `TEXT` | `NOT NULL`, `FOREIGN KEY` | Source simulation run. |
| `report_format`| `TEXT` | `NOT NULL` | Export format (`MARKDOWN` or `CSV`). |
| `export_path` | `TEXT` | `NOT NULL` | File path on disk (e.g. `reports/report_WOS-20261016-ALLFEAT.md`). |
| `generated_at` | `TEXT` | `NOT NULL` | ISO-8601 generation timestamp. |

---

## 3. Compatibility Views

To maintain 100% adherence to earlier specifications, two relational views are automatically defined:
- **`session_configurations`**: Maps `(id, session_id, version AS config_version, settings_json AS config_json, is_finalized AS finalized, created_at)` from `configuration_versions`.
- **`session_checkpoints`**: Maps `(checkpoint_id, session_id, run_id, simulated_time, state_snapshot, created_at)` from `checkpoints`.
