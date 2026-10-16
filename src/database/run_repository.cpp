#include "watchos/database/run_repository.hpp"
#include "watchos/core/id_generator.hpp"

namespace watchos {
namespace database {

RunRepository::RunRepository(DbConnection& db) : db_(db) {}

void RunRepository::createRun(const std::string& runId, const std::string& sessionId, const std::string& module, const std::string& parameters) {
    auto stmt = db_.prepare(
        "INSERT INTO simulation_runs (run_id, session_id, module, status, parameters, created_at) "
        "VALUES (?, ?, ?, 'RUNNING', ?, ?);"
    );
    stmt.bindText(1, runId);
    stmt.bindText(2, sessionId);
    stmt.bindText(3, module);
    stmt.bindText(4, parameters);
    stmt.bindText(5, core::IdGenerator::getCurrentTimestampIso());
    stmt.step();
}

void RunRepository::updateRunStatus(const std::string& runId, const std::string& status) {
    auto stmt = db_.prepare("UPDATE simulation_runs SET status = ? WHERE run_id = ?;");
    stmt.bindText(1, status);
    stmt.bindText(2, runId);
    stmt.step();
}

void RunRepository::recordSimulationEvent(const std::string& runId, core::TimeUnits timestamp, const std::string& eventType, core::ProcessId entityId, const std::string& details) {
    auto stmt = db_.prepare(
        "INSERT INTO simulation_events (run_id, timestamp, event_type, entity_id, details) "
        "VALUES (?, ?, ?, ?, ?);"
    );
    stmt.bindText(1, runId);
    stmt.bindInt64(2, timestamp);
    stmt.bindText(3, eventType);
    stmt.bindInt(4, entityId);
    stmt.bindText(5, details);
    stmt.step();
}

void RunRepository::recordGanttSegment(const std::string& runId, core::TimeUnits startTime, core::TimeUnits endTime, core::ProcessId pid, bool isIdle, bool isContextSwitch) {
    auto stmt = db_.prepare(
        "INSERT INTO gantt_segments (run_id, start_time, end_time, pid, is_idle, is_context_switch) "
        "VALUES (?, ?, ?, ?, ?, ?);"
    );
    stmt.bindText(1, runId);
    stmt.bindInt64(2, startTime);
    stmt.bindInt64(3, endTime);
    stmt.bindInt(4, pid);
    stmt.bindInt(5, isIdle ? 1 : 0);
    stmt.bindInt(6, isContextSwitch ? 1 : 0);
    stmt.step();
}

void RunRepository::recordSchedulingResult(const SchedulingResultRecord& rec) {
    auto stmt = db_.prepare(
        "INSERT INTO scheduling_results (run_id, pid, arrival_time, cpu_burst, priority, completion_time, turnaround_time, waiting_time, response_time) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);"
    );
    stmt.bindText(1, rec.runId);
    stmt.bindInt(2, rec.pid);
    stmt.bindInt64(3, rec.arrivalTime);
    stmt.bindInt64(4, rec.cpuBurst);
    stmt.bindInt(5, rec.priority);
    stmt.bindInt64(6, rec.completionTime);
    stmt.bindInt64(7, rec.turnaroundTime);
    stmt.bindInt64(8, rec.waitingTime);
    stmt.bindInt64(9, rec.responseTime);
    stmt.step();
}

void RunRepository::recordMemoryResult(const std::string& runId, const std::string& algo, int64_t totalRefs, int64_t faults, int64_t hits, double ratio, int64_t replacements, int frames, bool thrashing) {
    auto stmt = db_.prepare(
        "INSERT INTO memory_results (run_id, algorithm, total_references, page_faults, page_hits, fault_ratio, replacements, frames_allocated, thrashing_detected) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);"
    );
    stmt.bindText(1, runId);
    stmt.bindText(2, algo);
    stmt.bindInt64(3, totalRefs);
    stmt.bindInt64(4, faults);
    stmt.bindInt64(5, hits);
    stmt.bindDouble(6, ratio);
    stmt.bindInt64(7, replacements);
    stmt.bindInt(8, frames);
    stmt.bindInt(9, thrashing ? 1 : 0);
    stmt.step();
}

void RunRepository::recordDeadlockResult(const std::string& runId, bool isDeadlocked, bool isSafe, const std::string& safeSeq, const std::string& deadlockedProcs, int trials, int dlTrials, double freq) {
    auto stmt = db_.prepare(
        "INSERT INTO deadlock_results (run_id, is_deadlocked, is_safe, safe_sequence, deadlocked_processes, frequency_trials, frequency_deadlocks, empirical_frequency) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?);"
    );
    stmt.bindText(1, runId);
    stmt.bindInt(2, isDeadlocked ? 1 : 0);
    stmt.bindInt(3, isSafe ? 1 : 0);
    stmt.bindText(4, safeSeq);
    stmt.bindText(5, deadlockedProcs);
    stmt.bindInt(6, trials);
    stmt.bindInt(7, dlTrials);
    stmt.bindDouble(8, freq);
    stmt.step();
}

void RunRepository::recordIoResult(const std::string& runId, const std::string& algo, int64_t totalMovement, double avgMovement, const std::string& serviceOrder) {
    auto stmt = db_.prepare(
        "INSERT INTO io_results (run_id, algorithm, total_head_movement, average_head_movement, service_order) "
        "VALUES (?, ?, ?, ?, ?);"
    );
    stmt.bindText(1, runId);
    stmt.bindText(2, algo);
    stmt.bindInt64(3, totalMovement);
    stmt.bindDouble(4, avgMovement);
    stmt.bindText(5, serviceOrder);
    stmt.step();
}

void RunRepository::recordPermissionEvent(const std::string& runId, const std::string& actor, const std::string& resource, const std::string& operation, const std::string& decision, const std::string& reason) {
    auto stmt = db_.prepare(
        "INSERT INTO permission_events (run_id, actor, resource, operation, decision, reason) "
        "VALUES (?, ?, ?, ?, ?, ?);"
    );
    stmt.bindText(1, runId);
    stmt.bindText(2, actor);
    stmt.bindText(3, resource);
    stmt.bindText(4, operation);
    stmt.bindText(5, decision);
    stmt.bindText(6, reason);
    stmt.step();
}

std::vector<RunRecord> RunRepository::getRunsForSession(const std::string& sessionId) {
    std::vector<RunRecord> list;
    auto stmt = db_.prepare(
        "SELECT run_id, session_id, module, status, parameters, created_at "
        "FROM simulation_runs WHERE session_id = ? ORDER BY created_at DESC;"
    );
    stmt.bindText(1, sessionId);
    while (stmt.step()) {
        RunRecord rec;
        rec.runId = stmt.getText(0);
        rec.sessionId = stmt.getText(1);
        rec.module = stmt.getText(2);
        rec.status = stmt.getText(3);
        rec.parameters = stmt.getText(4);
        rec.createdAt = stmt.getText(5);
        list.push_back(rec);
    }
    return list;
}

std::vector<EventRecord> RunRepository::getEventsForRun(const std::string& runId) {
    std::vector<EventRecord> list;
    auto stmt = db_.prepare(
        "SELECT id, run_id, timestamp, event_type, entity_id, details "
        "FROM simulation_events WHERE run_id = ? ORDER BY timestamp ASC, id ASC;"
    );
    stmt.bindText(1, runId);
    while (stmt.step()) {
        EventRecord rec;
        rec.id = stmt.getInt64(0);
        rec.runId = stmt.getText(1);
        rec.timestamp = stmt.getInt64(2);
        rec.eventType = stmt.getText(3);
        rec.entityId = stmt.getInt(4);
        rec.details = stmt.getText(5);
        list.push_back(rec);
    }
    return list;
}

std::vector<GanttRecord> RunRepository::getGanttSegmentsForRun(const std::string& runId) {
    std::vector<GanttRecord> list;
    auto stmt = db_.prepare(
        "SELECT run_id, start_time, end_time, pid, is_idle, is_context_switch "
        "FROM gantt_segments WHERE run_id = ? ORDER BY start_time ASC;"
    );
    stmt.bindText(1, runId);
    while (stmt.step()) {
        GanttRecord rec;
        rec.runId = stmt.getText(0);
        rec.startTime = stmt.getInt64(1);
        rec.endTime = stmt.getInt64(2);
        rec.pid = stmt.getInt(3);
        rec.isIdle = stmt.getInt(4) == 1;
        rec.isContextSwitch = stmt.getInt(5) == 1;
        list.push_back(rec);
    }
    return list;
}

std::vector<SchedulingResultRecord> RunRepository::getSchedulingResultsForRun(const std::string& runId) {
    std::vector<SchedulingResultRecord> list;
    auto stmt = db_.prepare(
        "SELECT run_id, pid, arrival_time, cpu_burst, priority, completion_time, turnaround_time, waiting_time, response_time "
        "FROM scheduling_results WHERE run_id = ? ORDER BY pid ASC;"
    );
    stmt.bindText(1, runId);
    while (stmt.step()) {
        SchedulingResultRecord rec;
        rec.runId = stmt.getText(0);
        rec.pid = stmt.getInt(1);
        rec.arrivalTime = stmt.getInt64(2);
        rec.cpuBurst = stmt.getInt64(3);
        rec.priority = stmt.getInt(4);
        rec.completionTime = stmt.getInt64(5);
        rec.turnaroundTime = stmt.getInt64(6);
        rec.waitingTime = stmt.getInt64(7);
        rec.responseTime = stmt.getInt64(8);
        list.push_back(rec);
    }
    return list;
}

void RunRepository::saveCheckpoint(const std::string& checkpointId, const std::string& sessionId, const std::string& runId, core::TimeUnits simTime, const std::string& stateJson) {
    auto stmt = db_.prepare(
        "INSERT INTO checkpoints (checkpoint_id, session_id, run_id, simulated_time, state_snapshot, created_at) "
        "VALUES (?, ?, ?, ?, ?, ?);"
    );
    stmt.bindText(1, checkpointId);
    stmt.bindText(2, sessionId);
    stmt.bindText(3, runId);
    stmt.bindInt64(4, simTime);
    stmt.bindText(5, stateJson);
    stmt.bindText(6, core::IdGenerator::getCurrentTimestampIso());
    stmt.step();
}

std::string RunRepository::loadCheckpoint(const std::string& checkpointId) {
    auto stmt = db_.prepare("SELECT state_snapshot FROM checkpoints WHERE checkpoint_id = ?;");
    stmt.bindText(1, checkpointId);
    if (stmt.step()) {
        return stmt.getText(0);
    }
    return "";
}

void RunRepository::saveReportRecord(const std::string& reportId, const std::string& sessionId, const std::string& runId, const std::string& format, const std::string& path) {
    auto stmt = db_.prepare(
        "INSERT INTO reports (report_id, session_id, run_id, report_format, export_path, generated_at) "
        "VALUES (?, ?, ?, ?, ?, ?);"
    );
    stmt.bindText(1, reportId);
    stmt.bindText(2, sessionId);
    stmt.bindText(3, runId);
    stmt.bindText(4, format);
    stmt.bindText(5, path);
    stmt.bindText(6, core::IdGenerator::getCurrentTimestampIso());
    stmt.step();
}

void RunRepository::recordProcessStateTransition(const StateHistoryRecord& rec) {
    auto stmt = db_.prepare(
        "INSERT INTO process_state_history (run_id, pid, timestamp, from_state, to_state, reason) "
        "VALUES (?, ?, ?, ?, ?, ?);"
    );
    stmt.bindText(1, rec.runId);
    stmt.bindInt(2, rec.pid);
    stmt.bindInt64(3, rec.timestamp);
    stmt.bindText(4, rec.fromState);
    stmt.bindText(5, rec.toState);
    stmt.bindText(6, rec.reason);
    stmt.step();
}

std::vector<StateHistoryRecord> RunRepository::getProcessStateHistory(const std::string& runId) {
    std::vector<StateHistoryRecord> list;
    auto stmt = db_.prepare(
        "SELECT run_id, pid, timestamp, from_state, to_state, reason "
        "FROM process_state_history WHERE run_id = ? ORDER BY timestamp ASC, id ASC;"
    );
    stmt.bindText(1, runId);
    while (stmt.step()) {
        StateHistoryRecord rec;
        rec.runId = stmt.getText(0);
        rec.pid = stmt.getInt(1);
        rec.timestamp = stmt.getInt64(2);
        rec.fromState = stmt.getText(3);
        rec.toState = stmt.getText(4);
        rec.reason = stmt.getText(5);
        list.push_back(rec);
    }
    return list;
}

void RunRepository::recordResourceAllocation(const ResourceAllocationRecord& rec) {
    auto stmt = db_.prepare(
        "INSERT INTO resource_allocations (run_id, pid, resource_name, allocated_count, requested_count) "
        "VALUES (?, ?, ?, ?, ?);"
    );
    stmt.bindText(1, rec.runId);
    stmt.bindInt(2, rec.pid);
    stmt.bindText(3, rec.resourceName);
    stmt.bindInt(4, rec.allocatedCount);
    stmt.bindInt(5, rec.requestedCount);
    stmt.step();
}

std::vector<ResourceAllocationRecord> RunRepository::getResourceAllocations(const std::string& runId) {
    std::vector<ResourceAllocationRecord> list;
    auto stmt = db_.prepare(
        "SELECT run_id, pid, resource_name, allocated_count, requested_count "
        "FROM resource_allocations WHERE run_id = ? ORDER BY pid ASC, id ASC;"
    );
    stmt.bindText(1, runId);
    while (stmt.step()) {
        ResourceAllocationRecord rec;
        rec.runId = stmt.getText(0);
        rec.pid = stmt.getInt(1);
        rec.resourceName = stmt.getText(2);
        rec.allocatedCount = stmt.getInt(3);
        rec.requestedCount = stmt.getInt(4);
        list.push_back(rec);
    }
    return list;
}

void RunRepository::recordMemoryWorkload(const MemoryWorkloadRecord& rec) {
    auto stmt = db_.prepare(
        "INSERT INTO memory_workloads (session_id, config_version, pid, page_size, frame_count, reference_string_json) "
        "VALUES (?, ?, ?, ?, ?, ?);"
    );
    stmt.bindText(1, rec.sessionId);
    stmt.bindInt(2, rec.configVersion);
    stmt.bindInt(3, rec.pid);
    stmt.bindInt(4, rec.pageSize);
    stmt.bindInt(5, rec.frameCount);
    stmt.bindText(6, rec.refStringJson);
    stmt.step();
}

std::vector<MemoryWorkloadRecord> RunRepository::getMemoryWorkloads(const std::string& sessionId, int32_t configVersion) {
    std::vector<MemoryWorkloadRecord> list;
    auto stmt = db_.prepare(
        "SELECT session_id, config_version, pid, page_size, frame_count, reference_string_json "
        "FROM memory_workloads WHERE session_id = ? AND config_version = ? ORDER BY pid ASC;"
    );
    stmt.bindText(1, sessionId);
    stmt.bindInt(2, configVersion);
    while (stmt.step()) {
        MemoryWorkloadRecord rec;
        rec.sessionId = stmt.getText(0);
        rec.configVersion = stmt.getInt(1);
        rec.pid = stmt.getInt(2);
        rec.pageSize = stmt.getInt(3);
        rec.frameCount = stmt.getInt(4);
        rec.refStringJson = stmt.getText(5);
        list.push_back(rec);
    }
    return list;
}

std::vector<LogRecord> RunRepository::getEventLogsForSession(const std::string& sessionId) {
    std::vector<LogRecord> list;
    auto stmt = db_.prepare(
        "SELECT id, session_id, log_level, source, message, timestamp "
        "FROM event_logs WHERE session_id = ? ORDER BY id ASC;"
    );
    stmt.bindText(1, sessionId);
    while (stmt.step()) {
        LogRecord rec;
        rec.id = stmt.getInt64(0);
        rec.sessionId = stmt.getText(1);
        rec.logLevel = stmt.getText(2);
        rec.source = stmt.getText(3);
        rec.message = stmt.getText(4);
        rec.timestamp = stmt.getText(5);
        list.push_back(rec);
    }
    return list;
}

} // namespace database
} // namespace watchos

