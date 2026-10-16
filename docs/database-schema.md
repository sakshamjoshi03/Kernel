# WATCH OS — Complete SQLite Database Schema (Phase 1 & Phase 2)

## 1. Schema Design Principles
1. **Source of Truth**: All sessions, configurations, processes, simulation runs, events, metrics, and reports are stored persistently in SQLite 3.
2. **Foreign Key Integrity**: `PRAGMA foreign_keys = ON;` is enforced on every connection with `ON DELETE CASCADE` on parent relationships.
3. **Transaction Safety**: All composite mutations (such as saving a full workload or recording run results) are wrapped in atomic transactions (`TransactionGuard`).
4. **Reproducibility & Immutability**: Historical runs and finalized configuration versions cannot be mutated by subsequent executions.

## 2. Table Specifications

### Sessions & Workload Configuration
- **`sessions`**: `(session_id TEXT PRIMARY KEY, created_at TEXT, status TEXT, description TEXT, metadata TEXT)`
- **`configuration_versions`**: `(id INTEGER PRIMARY KEY, session_id TEXT, version INTEGER, created_at TEXT, is_finalized INTEGER, settings_json TEXT)`
- **`processes`**: `(id INTEGER PRIMARY KEY, session_id TEXT, config_version INTEGER, pid INTEGER, arrival_time INTEGER, cpu_burst INTEGER, priority INTEGER, memory_demand INTEGER)`
- **`process_io_requests`**: `(id INTEGER PRIMARY KEY, session_id TEXT, config_version INTEGER, pid INTEGER, request_time INTEGER, duration INTEGER, device TEXT)`
- **`resource_types`**: `(id INTEGER PRIMARY KEY, session_id TEXT, resource_name TEXT, total_instances INTEGER)`
- **`process_resource_claims`**: `(id INTEGER PRIMARY KEY, session_id TEXT, config_version INTEGER, pid INTEGER, resource_type TEXT, max_claim INTEGER, initial_allocation INTEGER)`
- **`process_disk_requests`**: `(id INTEGER PRIMARY KEY, session_id TEXT, config_version INTEGER, pid INTEGER, block_or_cylinder INTEGER, operation_type TEXT, volume_bytes INTEGER)`
- **`synchronization_configs`**: `(id INTEGER PRIMARY KEY, session_id TEXT, config_version INTEGER, pid INTEGER, primitive_name TEXT, cs_start_time INTEGER, cs_duration INTEGER)`
- **`access_policies`**: `(id INTEGER PRIMARY KEY, session_id TEXT, user_or_role TEXT, resource_name TEXT, operation TEXT, policy TEXT)`

### Simulation Runs, Events, and Subsystem Metrics
- **`simulation_runs`**: `(run_id TEXT PRIMARY KEY, session_id TEXT, module TEXT, status TEXT, parameters TEXT, created_at TEXT)`
- **`simulation_events`**: `(id INTEGER PRIMARY KEY, run_id TEXT, timestamp INTEGER, event_type TEXT, entity_id INTEGER, details TEXT)`
- **`gantt_segments`**: `(id INTEGER PRIMARY KEY, run_id TEXT, start_time INTEGER, end_time INTEGER, pid INTEGER, is_idle INTEGER, is_context_switch INTEGER)`
- **`scheduling_results`**: `(id INTEGER PRIMARY KEY, run_id TEXT, pid INTEGER, arrival_time INTEGER, cpu_burst INTEGER, priority INTEGER, completion_time INTEGER, turnaround_time INTEGER, waiting_time INTEGER, response_time INTEGER)`
- **`memory_results`**: `(id INTEGER PRIMARY KEY, run_id TEXT, algorithm TEXT, total_references INTEGER, page_faults INTEGER, page_hits INTEGER, fault_ratio REAL, replacements INTEGER, frames_allocated INTEGER, thrashing_detected INTEGER)`
- **`deadlock_results`**: `(id INTEGER PRIMARY KEY, run_id TEXT, is_deadlocked INTEGER, is_safe INTEGER, safe_sequence TEXT, deadlocked_processes TEXT, frequency_trials INTEGER, frequency_deadlocks INTEGER, empirical_frequency REAL)`
- **`io_results`**: `(id INTEGER PRIMARY KEY, run_id TEXT, algorithm TEXT, total_head_movement INTEGER, average_head_movement REAL, service_order TEXT)`
- **`permission_events`**: `(id INTEGER PRIMARY KEY, run_id TEXT, actor TEXT, resource TEXT, operation TEXT, decision TEXT, reason TEXT)`
- **`experiment_results`**: `(id INTEGER PRIMARY KEY, experiment_id TEXT, base_run_id TEXT, variant_run_id TEXT, parameter_changed TEXT, base_value TEXT, variant_value TEXT, metric_delta_json TEXT)`
- **`checkpoints`**: `(checkpoint_id TEXT PRIMARY KEY, session_id TEXT, run_id TEXT, simulated_time INTEGER, state_snapshot TEXT, created_at TEXT)`
- **`reports`**: `(report_id TEXT PRIMARY KEY, session_id TEXT, run_id TEXT, report_format TEXT, export_path TEXT, generated_at TEXT)`
- **`event_logs`**: `(id INTEGER PRIMARY KEY, session_id TEXT, log_level TEXT, source TEXT, message TEXT, timestamp TEXT)`
