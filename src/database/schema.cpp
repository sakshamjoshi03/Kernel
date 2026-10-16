#include "watchos/database/schema.hpp"

namespace watchos {
namespace database {

void Schema::initializeSchema(DbConnection& db) {
    const std::string ddl = R"SQL(
    -- 1. Sessions table
    CREATE TABLE IF NOT EXISTS sessions (
        session_id TEXT PRIMARY KEY,
        created_at TEXT NOT NULL,
        status TEXT NOT NULL DEFAULT 'DRAFT',
        description TEXT,
        metadata TEXT
    );

    -- 2. Configuration Versions
    CREATE TABLE IF NOT EXISTS configuration_versions (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        session_id TEXT NOT NULL,
        version INTEGER NOT NULL CHECK (version >= 1),
        created_at TEXT NOT NULL,
        is_finalized INTEGER NOT NULL DEFAULT 0,
        settings_json TEXT,
        FOREIGN KEY (session_id) REFERENCES sessions(session_id) ON DELETE CASCADE,
        UNIQUE(session_id, version)
    );

    -- 3. Processes table
    CREATE TABLE IF NOT EXISTS processes (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        session_id TEXT NOT NULL,
        config_version INTEGER NOT NULL DEFAULT 1,
        pid INTEGER NOT NULL,
        arrival_time INTEGER NOT NULL CHECK (arrival_time >= 0),
        cpu_burst INTEGER NOT NULL CHECK (cpu_burst > 0),
        priority INTEGER NOT NULL,
        memory_demand INTEGER DEFAULT NULL CHECK (memory_demand IS NULL OR memory_demand > 0),
        FOREIGN KEY (session_id) REFERENCES sessions(session_id) ON DELETE CASCADE,
        UNIQUE(session_id, config_version, pid)
    );

    -- 4. Process I/O requests
    CREATE TABLE IF NOT EXISTS process_io_requests (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        session_id TEXT NOT NULL,
        config_version INTEGER NOT NULL DEFAULT 1,
        pid INTEGER NOT NULL,
        request_time INTEGER NOT NULL CHECK (request_time >= 0),
        duration INTEGER NOT NULL CHECK (duration > 0),
        device TEXT NOT NULL DEFAULT 'DISK',
        FOREIGN KEY (session_id) REFERENCES sessions(session_id) ON DELETE CASCADE
    );

    -- 5. Resource types
    CREATE TABLE IF NOT EXISTS resource_types (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        session_id TEXT NOT NULL,
        resource_name TEXT NOT NULL,
        total_instances INTEGER NOT NULL CHECK (total_instances >= 0),
        FOREIGN KEY (session_id) REFERENCES sessions(session_id) ON DELETE CASCADE,
        UNIQUE(session_id, resource_name)
    );

    -- 6. Process resource claims
    CREATE TABLE IF NOT EXISTS process_resource_claims (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        session_id TEXT NOT NULL,
        config_version INTEGER NOT NULL DEFAULT 1,
        pid INTEGER NOT NULL,
        resource_type TEXT NOT NULL,
        max_claim INTEGER NOT NULL CHECK (max_claim >= 0),
        initial_allocation INTEGER NOT NULL DEFAULT 0 CHECK (initial_allocation >= 0 AND initial_allocation <= max_claim),
        FOREIGN KEY (session_id) REFERENCES sessions(session_id) ON DELETE CASCADE
    );

    -- 7. Resource allocations per run
    CREATE TABLE IF NOT EXISTS resource_allocations (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        run_id TEXT NOT NULL,
        pid INTEGER NOT NULL,
        resource_name TEXT NOT NULL,
        allocated_count INTEGER NOT NULL DEFAULT 0 CHECK (allocated_count >= 0),
        requested_count INTEGER NOT NULL DEFAULT 0 CHECK (requested_count >= 0),
        FOREIGN KEY (run_id) REFERENCES simulation_runs(run_id) ON DELETE CASCADE
    );

    -- 8. Process disk requests
    CREATE TABLE IF NOT EXISTS process_disk_requests (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        session_id TEXT NOT NULL,
        config_version INTEGER NOT NULL DEFAULT 1,
        pid INTEGER NOT NULL,
        block_or_cylinder INTEGER NOT NULL CHECK (block_or_cylinder >= 0),
        operation_type TEXT NOT NULL DEFAULT 'READ',
        volume_bytes INTEGER NOT NULL DEFAULT 4096 CHECK (volume_bytes > 0),
        FOREIGN KEY (session_id) REFERENCES sessions(session_id) ON DELETE CASCADE
    );

    -- 9. Memory workloads
    CREATE TABLE IF NOT EXISTS memory_workloads (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        session_id TEXT NOT NULL,
        config_version INTEGER NOT NULL DEFAULT 1,
        pid INTEGER NOT NULL,
        page_size INTEGER NOT NULL DEFAULT 4 CHECK (page_size > 0),
        frame_count INTEGER NOT NULL DEFAULT 3 CHECK (frame_count > 0),
        reference_string_json TEXT,
        FOREIGN KEY (session_id) REFERENCES sessions(session_id) ON DELETE CASCADE
    );

    -- 10. Synchronization configurations
    CREATE TABLE IF NOT EXISTS synchronization_configs (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        session_id TEXT NOT NULL,
        config_version INTEGER NOT NULL DEFAULT 1,
        pid INTEGER NOT NULL,
        primitive_name TEXT NOT NULL,
        cs_start_time INTEGER NOT NULL CHECK (cs_start_time >= 0),
        cs_duration INTEGER NOT NULL CHECK (cs_duration > 0),
        FOREIGN KEY (session_id) REFERENCES sessions(session_id) ON DELETE CASCADE
    );

    -- 11. Access policies
    CREATE TABLE IF NOT EXISTS access_policies (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        session_id TEXT NOT NULL,
        user_or_role TEXT NOT NULL,
        resource_name TEXT NOT NULL,
        operation TEXT NOT NULL,
        policy TEXT NOT NULL DEFAULT 'ALLOW',
        FOREIGN KEY (session_id) REFERENCES sessions(session_id) ON DELETE CASCADE
    );

    -- 12. Simulation runs
    CREATE TABLE IF NOT EXISTS simulation_runs (
        run_id TEXT PRIMARY KEY,
        session_id TEXT NOT NULL,
        module TEXT NOT NULL,
        status TEXT NOT NULL DEFAULT 'RUNNING' CHECK (status IN ('RUNNING', 'COMPLETED', 'FAILED', 'ABORTED')),
        parameters TEXT,
        created_at TEXT NOT NULL,
        FOREIGN KEY (session_id) REFERENCES sessions(session_id) ON DELETE CASCADE
    );

    -- 13. Simulation events
    CREATE TABLE IF NOT EXISTS simulation_events (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        run_id TEXT NOT NULL,
        timestamp INTEGER NOT NULL CHECK (timestamp >= 0),
        event_type TEXT NOT NULL,
        entity_id INTEGER,
        details TEXT,
        FOREIGN KEY (run_id) REFERENCES simulation_runs(run_id) ON DELETE CASCADE
    );

    -- 14. Process state history
    CREATE TABLE IF NOT EXISTS process_state_history (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        run_id TEXT NOT NULL,
        pid INTEGER NOT NULL,
        timestamp INTEGER NOT NULL CHECK (timestamp >= 0),
        from_state TEXT NOT NULL,
        to_state TEXT NOT NULL,
        reason TEXT,
        FOREIGN KEY (run_id) REFERENCES simulation_runs(run_id) ON DELETE CASCADE
    );

    -- 15. Gantt segments
    CREATE TABLE IF NOT EXISTS gantt_segments (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        run_id TEXT NOT NULL,
        start_time INTEGER NOT NULL CHECK (start_time >= 0),
        end_time INTEGER NOT NULL CHECK (end_time >= start_time),
        pid INTEGER,
        is_idle INTEGER NOT NULL DEFAULT 0,
        is_context_switch INTEGER NOT NULL DEFAULT 0,
        FOREIGN KEY (run_id) REFERENCES simulation_runs(run_id) ON DELETE CASCADE
    );

    -- 16. Scheduling results
    CREATE TABLE IF NOT EXISTS scheduling_results (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        run_id TEXT NOT NULL,
        pid INTEGER NOT NULL,
        arrival_time INTEGER NOT NULL CHECK (arrival_time >= 0),
        cpu_burst INTEGER NOT NULL CHECK (cpu_burst > 0),
        priority INTEGER NOT NULL,
        completion_time INTEGER NOT NULL CHECK (completion_time >= arrival_time),
        turnaround_time INTEGER NOT NULL CHECK (turnaround_time >= cpu_burst),
        waiting_time INTEGER NOT NULL CHECK (waiting_time >= 0),
        response_time INTEGER NOT NULL CHECK (response_time >= 0),
        FOREIGN KEY (run_id) REFERENCES simulation_runs(run_id) ON DELETE CASCADE
    );

    -- 17. Memory Results
    CREATE TABLE IF NOT EXISTS memory_results (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        run_id TEXT NOT NULL,
        algorithm TEXT NOT NULL,
        total_references INTEGER NOT NULL CHECK (total_references >= 0),
        page_faults INTEGER NOT NULL CHECK (page_faults >= 0),
        page_hits INTEGER NOT NULL CHECK (page_hits >= 0),
        fault_ratio REAL NOT NULL CHECK (fault_ratio >= 0.0 AND fault_ratio <= 1.0),
        replacements INTEGER NOT NULL CHECK (replacements >= 0),
        frames_allocated INTEGER NOT NULL CHECK (frames_allocated > 0),
        thrashing_detected INTEGER NOT NULL DEFAULT 0,
        FOREIGN KEY (run_id) REFERENCES simulation_runs(run_id) ON DELETE CASCADE
    );

    -- 18. Deadlock Results
    CREATE TABLE IF NOT EXISTS deadlock_results (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        run_id TEXT NOT NULL,
        is_deadlocked INTEGER NOT NULL DEFAULT 0,
        is_safe INTEGER NOT NULL DEFAULT 1,
        safe_sequence TEXT,
        deadlocked_processes TEXT,
        frequency_trials INTEGER DEFAULT 0 CHECK (frequency_trials >= 0),
        frequency_deadlocks INTEGER DEFAULT 0 CHECK (frequency_deadlocks >= 0),
        empirical_frequency REAL DEFAULT 0.0 CHECK (empirical_frequency >= 0.0 AND empirical_frequency <= 1.0),
        FOREIGN KEY (run_id) REFERENCES simulation_runs(run_id) ON DELETE CASCADE
    );

    -- 19. I/O & Disk Results
    CREATE TABLE IF NOT EXISTS io_results (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        run_id TEXT NOT NULL,
        algorithm TEXT NOT NULL,
        total_head_movement INTEGER NOT NULL CHECK (total_head_movement >= 0),
        average_head_movement REAL NOT NULL CHECK (average_head_movement >= 0.0),
        service_order TEXT NOT NULL,
        FOREIGN KEY (run_id) REFERENCES simulation_runs(run_id) ON DELETE CASCADE
    );

    -- 20. Permission Events
    CREATE TABLE IF NOT EXISTS permission_events (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        run_id TEXT NOT NULL,
        actor TEXT NOT NULL,
        resource TEXT NOT NULL,
        operation TEXT NOT NULL,
        decision TEXT NOT NULL,
        reason TEXT NOT NULL,
        FOREIGN KEY (run_id) REFERENCES simulation_runs(run_id) ON DELETE CASCADE
    );

    -- 21. Experiment Results
    CREATE TABLE IF NOT EXISTS experiment_results (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        experiment_id TEXT NOT NULL,
        base_run_id TEXT NOT NULL,
        variant_run_id TEXT NOT NULL,
        parameter_changed TEXT NOT NULL,
        base_value TEXT NOT NULL,
        variant_value TEXT NOT NULL,
        metric_delta_json TEXT NOT NULL
    );

    -- 22. Checkpoints
    CREATE TABLE IF NOT EXISTS checkpoints (
        checkpoint_id TEXT PRIMARY KEY,
        session_id TEXT NOT NULL,
        run_id TEXT NOT NULL,
        simulated_time INTEGER NOT NULL CHECK (simulated_time >= 0),
        state_snapshot TEXT NOT NULL,
        created_at TEXT NOT NULL,
        FOREIGN KEY (session_id) REFERENCES sessions(session_id) ON DELETE CASCADE,
        FOREIGN KEY (run_id) REFERENCES simulation_runs(run_id) ON DELETE CASCADE
    );

    -- 23. Event logs
    CREATE TABLE IF NOT EXISTS event_logs (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        session_id TEXT,
        log_level TEXT NOT NULL,
        source TEXT NOT NULL,
        message TEXT NOT NULL,
        timestamp TEXT NOT NULL,
        FOREIGN KEY (session_id) REFERENCES sessions(session_id) ON DELETE CASCADE
    );

    -- 24. Reports
    CREATE TABLE IF NOT EXISTS reports (
        report_id TEXT PRIMARY KEY,
        session_id TEXT NOT NULL,
        run_id TEXT NOT NULL,
        report_format TEXT NOT NULL,
        export_path TEXT NOT NULL,
        generated_at TEXT NOT NULL,
        FOREIGN KEY (session_id) REFERENCES sessions(session_id) ON DELETE CASCADE,
        FOREIGN KEY (run_id) REFERENCES simulation_runs(run_id) ON DELETE CASCADE
    );

    -- Comprehensive Indexes for query optimization and relational joins
    CREATE INDEX IF NOT EXISTS idx_config_versions_session ON configuration_versions(session_id);
    CREATE INDEX IF NOT EXISTS idx_processes_lookup ON processes(session_id, config_version);
    CREATE INDEX IF NOT EXISTS idx_io_lookup ON process_io_requests(session_id, config_version, pid);
    CREATE INDEX IF NOT EXISTS idx_resource_claims_lookup ON process_resource_claims(session_id, config_version, pid);
    CREATE INDEX IF NOT EXISTS idx_resource_allocations_run ON resource_allocations(run_id, pid);
    CREATE INDEX IF NOT EXISTS idx_disk_requests_lookup ON process_disk_requests(session_id, config_version, pid);
    CREATE INDEX IF NOT EXISTS idx_memory_workloads_lookup ON memory_workloads(session_id, config_version, pid);
    CREATE INDEX IF NOT EXISTS idx_runs_session ON simulation_runs(session_id);
    CREATE INDEX IF NOT EXISTS idx_events_run_time ON simulation_events(run_id, timestamp);
    CREATE INDEX IF NOT EXISTS idx_state_history_run ON process_state_history(run_id, timestamp);
    CREATE INDEX IF NOT EXISTS idx_gantt_run ON gantt_segments(run_id, start_time);
    CREATE INDEX IF NOT EXISTS idx_scheduling_results_run ON scheduling_results(run_id);
    CREATE INDEX IF NOT EXISTS idx_memory_results_run ON memory_results(run_id);
    CREATE INDEX IF NOT EXISTS idx_deadlock_results_run ON deadlock_results(run_id);
    CREATE INDEX IF NOT EXISTS idx_io_results_run ON io_results(run_id);
    CREATE INDEX IF NOT EXISTS idx_permission_events_run ON permission_events(run_id);
    CREATE INDEX IF NOT EXISTS idx_event_logs_session ON event_logs(session_id);
    CREATE INDEX IF NOT EXISTS idx_reports_session ON reports(session_id);
    CREATE INDEX IF NOT EXISTS idx_reports_run ON reports(run_id);
    )SQL";

    db.execute(ddl);
}

} // namespace database
} // namespace watchos
