import sqlite3
import json
from datetime import datetime

DB_PATH = 'data/watchos_sessions.db'

def get_columns(cursor, table_name):
    cursor.execute(f"PRAGMA table_info({table_name});")
    return [row[1] for row in cursor.fetchall()]

def populate():
    conn = sqlite3.connect(DB_PATH)
    cur = conn.cursor()
    cur.execute("PRAGMA foreign_keys = ON;")

    timestamp = datetime.utcnow().strftime("%Y-%m-%dT%H:%M:%SZ")
    session_id = "WOS-20261016-ALLFEAT"
    run_id = "RUN-20261016-ALLFEAT"

    print(f"Connecting to {DB_PATH}...")
    print(f"Target Session ID: {session_id}")

    # Check if session already exists
    cur.execute("SELECT session_id FROM sessions WHERE session_id = ?", (session_id,))
    if cur.fetchone():
        print(f"Session {session_id} already exists. Appending unique suffix...")
        session_id = f"WOS-{datetime.utcnow().strftime('%Y%m%d%H%M%S')}"
        run_id = f"RUN-{datetime.utcnow().strftime('%Y%m%d%H%M%S')}"

    # 1. sessions
    sess_cols = get_columns(cur, "sessions")
    sess_data = {
        "session_id": session_id,
        "title": "Comprehensive All-Features Enterprise Kernel Session",
        "created_at": timestamp,
        "updated_at": timestamp,
        "last_opened_at": timestamp,
        "ended_at": None,
        "status": "FINALIZED",
        "active_config_version": 1,
        "description": "Full-featured multi-subsystem simulation with CPU, Memory, Disk, IPC, Deadlock, and RBAC",
        "notes": "Verified against all 24 relational tables in WATCH OS architecture.",
        "metadata": json.dumps({"creator": "Antigravity Senior QA", "features": "all_subsystems"})
    }
    cols_to_use = [c for c in sess_cols if c in sess_data]
    placeholders = ", ".join(["?"] * len(cols_to_use))
    sql = f"INSERT INTO sessions ({', '.join(cols_to_use)}) VALUES ({placeholders})"
    cur.execute(sql, [sess_data[c] for c in cols_to_use])
    print(f"[OK] 1. sessions record created.")

    # 2. configuration_versions
    cv_cols = get_columns(cur, "configuration_versions")
    cv_data = {
        "session_id": session_id,
        "version": 1,
        "created_at": timestamp,
        "is_finalized": 1,
        "settings_json": json.dumps({"time_quantum": 3, "memory_frames": 3, "aging_interval": 5})
    }
    cols_to_use = [c for c in cv_cols if c in cv_data]
    placeholders = ", ".join(["?"] * len(cols_to_use))
    cur.execute(f"INSERT INTO configuration_versions ({', '.join(cols_to_use)}) VALUES ({placeholders})", [cv_data[c] for c in cols_to_use])
    print(f"[OK] 2. configuration_versions record created.")

    # 3. processes
    procs = [
        (session_id, 1, 101, 0, 6, 1, 64),  # Web Server
        (session_id, 1, 102, 1, 8, 2, 128), # Database Engine
        (session_id, 1, 103, 2, 4, 3, 32),  # Log Aggregator
        (session_id, 1, 104, 4, 3, 4, 16),  # Cache Cleaner
    ]
    cur.executemany(
        "INSERT INTO processes (session_id, config_version, pid, arrival_time, cpu_burst, priority, memory_demand) "
        "VALUES (?, ?, ?, ?, ?, ?, ?)", procs
    )
    print(f"[OK] 3. processes records created ({len(procs)} processes).")

    # 4. process_io_requests
    io_requests = [
        (session_id, 1, 101, 2, 2, "DISK"),
        (session_id, 1, 102, 3, 3, "NETWORK_CARD"),
        (session_id, 1, 103, 1, 2, "DISK"),
    ]
    cur.executemany(
        "INSERT INTO process_io_requests (session_id, config_version, pid, request_time, duration, device) "
        "VALUES (?, ?, ?, ?, ?, ?)", io_requests
    )
    print(f"[OK] 4. process_io_requests records created ({len(io_requests)} requests).")

    # 5. resource_types
    resources = [
        (session_id, "CPU_CORES", 4),
        (session_id, "DB_MUTEX", 1),
        (session_id, "NVME_CHANNEL", 2),
    ]
    cur.executemany(
        "INSERT INTO resource_types (session_id, resource_name, total_instances) VALUES (?, ?, ?)",
        resources
    )
    print(f"[OK] 5. resource_types records created ({len(resources)} types).")

    # 6. process_resource_claims
    claims = [
        (session_id, 1, 101, "CPU_CORES", 2, 1),
        (session_id, 1, 101, "NVME_CHANNEL", 1, 1),
        (session_id, 1, 102, "CPU_CORES", 3, 2),
        (session_id, 1, 102, "DB_MUTEX", 1, 1),
        (session_id, 1, 103, "NVME_CHANNEL", 1, 0),
        (session_id, 1, 104, "CPU_CORES", 1, 0),
    ]
    cur.executemany(
        "INSERT INTO process_resource_claims (session_id, config_version, pid, resource_type, max_claim, initial_allocation) "
        "VALUES (?, ?, ?, ?, ?, ?)", claims
    )
    print(f"[OK] 6. process_resource_claims records created ({len(claims)} claims).")

    # 8. process_disk_requests
    disks = [
        (session_id, 1, 101, 45, "READ", 4096),
        (session_id, 1, 102, 120, "WRITE", 16384),
        (session_id, 1, 103, 85, "READ", 8192),
        (session_id, 1, 104, 14, "READ", 2048),
    ]
    cur.executemany(
        "INSERT INTO process_disk_requests (session_id, config_version, pid, block_or_cylinder, operation_type, volume_bytes) "
        "VALUES (?, ?, ?, ?, ?, ?)", disks
    )
    print(f"[OK] 8. process_disk_requests records created ({len(disks)} requests).")

    # 9. memory_workloads
    mem_refs = json.dumps([1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5])
    mem_workloads = [
        (session_id, 1, 101, 4, 3, mem_refs),
        (session_id, 1, 102, 4, 4, mem_refs),
    ]
    cur.executemany(
        "INSERT INTO memory_workloads (session_id, config_version, pid, page_size, frame_count, reference_string_json) "
        "VALUES (?, ?, ?, ?, ?, ?)", mem_workloads
    )
    print(f"[OK] 9. memory_workloads records created.")

    # 10. synchronization_configs
    sync_configs = [
        (session_id, 1, 101, "MUTEX_DB", 2, 3),
        (session_id, 1, 102, "SEM_BUFFER_FULL", 3, 2),
    ]
    cur.executemany(
        "INSERT INTO synchronization_configs (session_id, config_version, pid, primitive_name, cs_start_time, cs_duration) "
        "VALUES (?, ?, ?, ?, ?, ?)", sync_configs
    )
    print(f"[OK] 10. synchronization_configs records created.")

    # 11. access_policies
    policies = [
        (session_id, "root", "system_config", "WRITE", "ALLOW"),
        (session_id, "guest", "system_config", "WRITE", "DENY"),
        (session_id, "web_service", "storage_pool", "READ", "ALLOW"),
    ]
    cur.executemany(
        "INSERT INTO access_policies (session_id, user_or_role, resource_name, operation, policy) "
        "VALUES (?, ?, ?, ?, ?)", policies
    )
    print(f"[OK] 11. access_policies records created.")

    # 12. simulation_runs
    cur.execute(
        "INSERT INTO simulation_runs (run_id, session_id, module, status, parameters, created_at) "
        "VALUES (?, ?, ?, 'COMPLETED', ?, ?)",
        (run_id, session_id, "INTEGRATED_SUITE", "All Schedulers, Bankers, Paging, Disk, Security", timestamp)
    )
    print(f"[OK] 12. simulation_runs record created ({run_id}).")

    # 7. resource_allocations (linked to run_id)
    allocs = [
        (run_id, 101, "CPU_CORES", 1, 0),
        (run_id, 101, "NVME_CHANNEL", 1, 0),
        (run_id, 102, "CPU_CORES", 2, 1),
        (run_id, 102, "DB_MUTEX", 1, 0),
    ]
    cur.executemany(
        "INSERT INTO resource_allocations (run_id, pid, resource_name, allocated_count, requested_count) "
        "VALUES (?, ?, ?, ?, ?)", allocs
    )
    print(f"[OK] 7. resource_allocations records created.")

    # 13. simulation_events
    events = [
        (run_id, 0, "PROCESS_ARRIVE", 101, "P101 arrived at ready queue"),
        (run_id, 0, "CPU_DISPATCH", 101, "P101 dispatched to CPU"),
        (run_id, 1, "PROCESS_ARRIVE", 102, "P102 arrived at ready queue"),
        (run_id, 2, "PROCESS_ARRIVE", 103, "P103 arrived at ready queue"),
        (run_id, 4, "PROCESS_ARRIVE", 104, "P104 arrived at ready queue"),
        (run_id, 6, "PROCESS_TERMINATE", 101, "P101 burst completed"),
        (run_id, 6, "CPU_DISPATCH", 102, "P102 dispatched to CPU"),
        (run_id, 14, "PROCESS_TERMINATE", 102, "P102 burst completed"),
        (run_id, 14, "CPU_DISPATCH", 103, "P103 dispatched to CPU"),
        (run_id, 18, "PROCESS_TERMINATE", 103, "P103 burst completed"),
        (run_id, 18, "CPU_DISPATCH", 104, "P104 dispatched to CPU"),
        (run_id, 21, "PROCESS_TERMINATE", 104, "P104 burst completed"),
    ]
    cur.executemany(
        "INSERT INTO simulation_events (run_id, timestamp, event_type, entity_id, details) VALUES (?, ?, ?, ?, ?)",
        events
    )
    print(f"[OK] 13. simulation_events records created ({len(events)} events).")

    # 14. process_state_history
    history = [
        (run_id, 101, 0, "NEW", "READY", "Process creation"),
        (run_id, 101, 0, "READY", "RUNNING", "Scheduler dispatch"),
        (run_id, 101, 6, "RUNNING", "TERMINATED", "Completed"),
        (run_id, 102, 1, "NEW", "READY", "Process creation"),
        (run_id, 102, 6, "READY", "RUNNING", "Scheduler dispatch"),
        (run_id, 102, 14, "RUNNING", "TERMINATED", "Completed"),
        (run_id, 103, 2, "NEW", "READY", "Process creation"),
        (run_id, 103, 14, "READY", "RUNNING", "Scheduler dispatch"),
        (run_id, 103, 18, "RUNNING", "TERMINATED", "Completed"),
        (run_id, 104, 4, "NEW", "READY", "Process creation"),
        (run_id, 104, 18, "READY", "RUNNING", "Scheduler dispatch"),
        (run_id, 104, 21, "RUNNING", "TERMINATED", "Completed"),
    ]
    cur.executemany(
        "INSERT INTO process_state_history (run_id, pid, timestamp, from_state, to_state, reason) VALUES (?, ?, ?, ?, ?, ?)",
        history
    )
    print(f"[OK] 14. process_state_history records created ({len(history)} transitions).")

    # 15. gantt_segments
    gantt = [
        (run_id, 0, 6, 101, 0, 0),
        (run_id, 6, 14, 102, 0, 1),
        (run_id, 14, 18, 103, 0, 1),
        (run_id, 18, 21, 104, 0, 1),
    ]
    cur.executemany(
        "INSERT INTO gantt_segments (run_id, start_time, end_time, pid, is_idle, is_context_switch) VALUES (?, ?, ?, ?, ?, ?)",
        gantt
    )
    print(f"[OK] 15. gantt_segments records created.")

    # 16. scheduling_results
    sched_res = [
        # (run_id, pid, arrival, burst, pri, completion, tat, wt, rt)
        (run_id, 101, 0, 6, 1, 6, 6, 0, 0),
        (run_id, 102, 1, 8, 2, 14, 13, 5, 5),
        (run_id, 103, 2, 4, 3, 18, 16, 12, 12),
        (run_id, 104, 4, 3, 4, 21, 17, 14, 14),
    ]
    cur.executemany(
        "INSERT INTO scheduling_results (run_id, pid, arrival_time, cpu_burst, priority, completion_time, turnaround_time, waiting_time, response_time) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)", sched_res
    )
    print(f"[OK] 16. scheduling_results records created ({len(sched_res)} records).")

    # 17. memory_results
    mem_res = [
        (run_id, "FIFO", 12, 9, 3, 0.75, 6, 3, 0),
        (run_id, "LRU", 12, 8, 4, 0.6667, 5, 3, 0),
        (run_id, "OPTIMAL", 12, 6, 6, 0.50, 3, 3, 0),
    ]
    cur.executemany(
        "INSERT INTO memory_results (run_id, algorithm, total_references, page_faults, page_hits, fault_ratio, replacements, frames_allocated, thrashing_detected) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)", mem_res
    )
    print(f"[OK] 17. memory_results records created.")

    # 18. deadlock_results
    cur.execute(
        "INSERT INTO deadlock_results (run_id, is_deadlocked, is_safe, safe_sequence, deadlocked_processes, frequency_trials, frequency_deadlocks, empirical_frequency) "
        "VALUES (?, 0, 1, 'P101 -> P103 -> P104 -> P102', '', 1000, 0, 0.0)",
        (run_id,)
    )
    print(f"[OK] 18. deadlock_results record created.")

    # 19. io_results
    io_res = [
        (run_id, "FCFS", 640, 80.0, "98,183,37,122,14,124,65,67"),
        (run_id, "SSTF", 236, 29.5, "65,67,45,37,14,98,122,124,183"),
        (run_id, "SCAN", 208, 26.0, "65,67,85,98,120,122,124,183,199,45,37,14"),
        (run_id, "C-SCAN", 382, 47.75, "65,67,85,98,120,122,124,183,199,0,14,37,45"),
    ]
    cur.executemany(
        "INSERT INTO io_results (run_id, algorithm, total_head_movement, average_head_movement, service_order) "
        "VALUES (?, ?, ?, ?, ?)", io_res
    )
    print(f"[OK] 19. io_results records created.")

    # 20. permission_events
    sec_events = [
        (run_id, "PID 101", "system_config", "WRITE", "ALLOW", "Root execution privileges"),
        (run_id, "PID 102", "system_config", "WRITE", "DENY", "Unauthorized write by user process"),
        (run_id, "PID 102", "storage_pool", "READ", "ALLOW", "Read access authorized"),
    ]
    cur.executemany(
        "INSERT INTO permission_events (run_id, actor, resource, operation, decision, reason) "
        "VALUES (?, ?, ?, ?, ?, ?)", sec_events
    )
    print(f"[OK] 20. permission_events records created.")

    # 21. experiment_results
    cur.execute(
        "INSERT INTO experiment_results (experiment_id, base_run_id, variant_run_id, parameter_changed, base_value, variant_value, metric_delta_json) "
        "VALUES (?, ?, ?, 'time_quantum', '2', '4', ?)",
        ("EXP-001", run_id, run_id, json.dumps({"context_switch_reduction": 4, "avg_turnaround_delta": -1.2}))
    )
    print(f"[OK] 21. experiment_results record created.")

    # 22. checkpoints
    cur.execute(
        "INSERT INTO checkpoints (checkpoint_id, session_id, run_id, simulated_time, state_snapshot, created_at) "
        "VALUES (?, ?, ?, 10, ?, ?)",
        ("CHK-20261016-01", session_id, run_id, json.dumps({"active_pid": 102, "completed_pids": [101], "ready_queue": [103, 104]}), timestamp)
    )
    print(f"[OK] 22. checkpoints record created.")

    # 23. event_logs
    logs = [
        (session_id, "INFO", "KERNEL_BOOT", "Kernel simulation session initialized with 4 processes", timestamp),
        (session_id, "INFO", "SCHEDULER", "FCFS, SJF, SRTF, Priority and Round Robin executed", timestamp),
        (session_id, "INFO", "DEADLOCK_ENGINE", "Banker safe state verified: P101 -> P103 -> P104 -> P102", timestamp),
        (session_id, "SUCCESS", "REPORT_GENERATOR", "Analytical reports exported to reports/ directory", timestamp),
    ]
    cur.executemany(
        "INSERT INTO event_logs (session_id, log_level, source, message, timestamp) VALUES (?, ?, ?, ?, ?)",
        logs
    )
    print(f"[OK] 23. event_logs records created.")

    # 24. reports
    cur.execute(
        "INSERT INTO reports (report_id, session_id, run_id, report_format, export_path, generated_at) "
        "VALUES (?, ?, ?, 'MARKDOWN', ?, ?)",
        (f"RPT-{session_id}", session_id, run_id, f"reports/report_{session_id}.md", timestamp)
    )
    cur.execute(
        "INSERT INTO reports (report_id, session_id, run_id, report_format, export_path, generated_at) "
        "VALUES (?, ?, ?, 'CSV', ?, ?)",
        (f"RPT-CSV-{session_id}", session_id, run_id, f"reports/report_{session_id}.csv", timestamp)
    )
    print(f"[OK] 24. reports records created.")

    conn.commit()
    conn.close()
    print("=" * 60)
    print(f"SUCCESS: Session '{session_id}' populated across all 24 entities!")
    print("All prior dummy/historical data was preserved intact.")

if __name__ == "__main__":
    populate()
