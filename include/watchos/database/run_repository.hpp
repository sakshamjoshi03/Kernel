#ifndef WATCHOS_DATABASE_RUN_REPOSITORY_HPP
#define WATCHOS_DATABASE_RUN_REPOSITORY_HPP

#include "watchos/database/db_connection.hpp"
#include "watchos/core/types.hpp"
#include <string>
#include <vector>

namespace watchos {
namespace database {

struct RunRecord {
    std::string runId;
    std::string sessionId;
    std::string module;
    std::string status;
    std::string parameters;
    std::string createdAt;
};

struct EventRecord {
    int64_t id = 0;
    std::string runId;
    core::TimeUnits timestamp = 0;
    std::string eventType;
    core::ProcessId entityId = 0;
    std::string details;
};

struct GanttRecord {
    std::string runId;
    core::TimeUnits startTime = 0;
    core::TimeUnits endTime = 0;
    core::ProcessId pid = 0;
    bool isIdle = false;
    bool isContextSwitch = false;
};

struct SchedulingResultRecord {
    std::string runId;
    core::ProcessId pid = 0;
    core::TimeUnits arrivalTime = 0;
    core::TimeUnits cpuBurst = 0;
    core::Priority priority = 0;
    core::TimeUnits completionTime = 0;
    core::TimeUnits turnaroundTime = 0;
    core::TimeUnits waitingTime = 0;
    core::TimeUnits responseTime = 0;
};

struct StateHistoryRecord {
    std::string runId;
    core::ProcessId pid = 0;
    core::TimeUnits timestamp = 0;
    std::string fromState;
    std::string toState;
    std::string reason;
};

struct ResourceAllocationRecord {
    std::string runId;
    core::ProcessId pid = 0;
    std::string resourceName;
    int32_t allocatedCount = 0;
    int32_t requestedCount = 0;
};

struct MemoryWorkloadRecord {
    std::string sessionId;
    int32_t configVersion = 1;
    core::ProcessId pid = 0;
    int32_t pageSize = 4;
    int32_t frameCount = 3;
    std::string refStringJson;
};

struct LogRecord {
    int64_t id = 0;
    std::string sessionId;
    std::string logLevel;
    std::string source;
    std::string message;
    std::string timestamp;
};

class RunRepository {
public:
    explicit RunRepository(DbConnection& db);

    void createRun(const std::string& runId, const std::string& sessionId, const std::string& module, const std::string& parameters);
    void updateRunStatus(const std::string& runId, const std::string& status);

    void recordSimulationEvent(const std::string& runId, core::TimeUnits timestamp, const std::string& eventType, core::ProcessId entityId, const std::string& details);
    void recordGanttSegment(const std::string& runId, core::TimeUnits startTime, core::TimeUnits endTime, core::ProcessId pid, bool isIdle, bool isContextSwitch);
    void recordSchedulingResult(const SchedulingResultRecord& rec);

    void recordProcessStateTransition(const StateHistoryRecord& rec);
    std::vector<StateHistoryRecord> getProcessStateHistory(const std::string& runId);

    void recordResourceAllocation(const ResourceAllocationRecord& rec);
    std::vector<ResourceAllocationRecord> getResourceAllocations(const std::string& runId);

    void recordMemoryWorkload(const MemoryWorkloadRecord& rec);
    std::vector<MemoryWorkloadRecord> getMemoryWorkloads(const std::string& sessionId, int32_t configVersion);

    void recordMemoryResult(const std::string& runId, const std::string& algo, int64_t totalRefs, int64_t faults, int64_t hits, double ratio, int64_t replacements, int frames, bool thrashing);
    void recordDeadlockResult(const std::string& runId, bool isDeadlocked, bool isSafe, const std::string& safeSeq, const std::string& deadlockedProcs, int trials, int dlTrials, double freq);
    void recordIoResult(const std::string& runId, const std::string& algo, int64_t totalMovement, double avgMovement, const std::string& serviceOrder);
    void recordPermissionEvent(const std::string& runId, const std::string& actor, const std::string& resource, const std::string& operation, const std::string& decision, const std::string& reason);

    std::vector<RunRecord> getRunsForSession(const std::string& sessionId);
    std::vector<EventRecord> getEventsForRun(const std::string& runId);
    std::vector<GanttRecord> getGanttSegmentsForRun(const std::string& runId);
    std::vector<SchedulingResultRecord> getSchedulingResultsForRun(const std::string& runId);
    std::vector<LogRecord> getEventLogsForSession(const std::string& sessionId);

    void saveCheckpoint(const std::string& checkpointId, const std::string& sessionId, const std::string& runId, core::TimeUnits simTime, const std::string& stateJson);
    std::string loadCheckpoint(const std::string& checkpointId);

    void saveReportRecord(const std::string& reportId, const std::string& sessionId, const std::string& runId, const std::string& format, const std::string& path);

private:
    DbConnection& db_;
};

} // namespace database
} // namespace watchos

#endif // WATCHOS_DATABASE_RUN_REPOSITORY_HPP
