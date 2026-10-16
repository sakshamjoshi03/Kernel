#include "watchos/database/session_repository.hpp"
#include "watchos/core/id_generator.hpp"
#include <iostream>

namespace watchos {
namespace database {

SessionRepository::SessionRepository(DbConnection& db) : db_(db) {}

void SessionRepository::createSession(const session::Session& session) {
    auto stmt = db_.prepare(
        "INSERT INTO sessions (session_id, created_at, status, description, metadata) "
        "VALUES (?, ?, ?, ?, ?);"
    );
    stmt.bindText(1, session.getSessionId());
    stmt.bindText(2, session.getCreatedAt());
    stmt.bindText(3, core::sessionStatusToString(session.getStatus()));
    stmt.bindText(4, session.getDescription());
    stmt.bindText(5, "{}");
    stmt.step();

    // Create initial configuration version 1
    auto stmtVer = db_.prepare(
        "INSERT INTO configuration_versions (session_id, version, created_at, is_finalized, settings_json) "
        "VALUES (?, 1, ?, 0, ?);"
    );
    stmtVer.bindText(1, session.getSessionId());
    stmtVer.bindText(2, session.getCreatedAt());
    stmtVer.bindText(3, "{}");
    stmtVer.step();
}

void SessionRepository::saveWorkload(const session::Session& session, bool finalize) {
    TransactionGuard tx(db_);

    int32_t version = session.getActiveVersion();
    const std::string& sessionId = session.getSessionId();

    // Upsert or update configuration_version
    auto stmtCheckVer = db_.prepare("SELECT id FROM configuration_versions WHERE session_id = ? AND version = ?;");
    stmtCheckVer.bindText(1, sessionId);
    stmtCheckVer.bindInt(2, version);

    if (stmtCheckVer.step()) {
        auto stmtUpd = db_.prepare(
            "UPDATE configuration_versions SET is_finalized = ? WHERE session_id = ? AND version = ?;"
        );
        stmtUpd.bindInt(1, finalize ? 1 : 0);
        stmtUpd.bindText(2, sessionId);
        stmtUpd.bindInt(3, version);
        stmtUpd.step();
    } else {
        auto stmtIns = db_.prepare(
            "INSERT INTO configuration_versions (session_id, version, created_at, is_finalized, settings_json) "
            "VALUES (?, ?, ?, ?, ?);"
        );
        stmtIns.bindText(1, sessionId);
        stmtIns.bindInt(2, version);
        stmtIns.bindText(3, core::IdGenerator::getCurrentTimestampIso());
        stmtIns.bindInt(4, finalize ? 1 : 0);
        stmtIns.bindText(5, "{}");
        stmtIns.step();
    }

    if (finalize) {
        auto stmtStatus = db_.prepare("UPDATE sessions SET status = 'FINALIZED' WHERE session_id = ?;");
        stmtStatus.bindText(1, sessionId);
        stmtStatus.step();
    }

    // Save processes
    saveProcesses(sessionId, version, session.getProcesses());

    tx.commit();
}

void SessionRepository::saveProcesses(const std::string& sessionId, int32_t version, const std::vector<process::Process>& processes) {
    // Delete existing processes & subsystem data for this session & version
    {
        auto delIo = db_.prepare("DELETE FROM process_io_requests WHERE session_id = ? AND config_version = ?;");
        delIo.bindText(1, sessionId);
        delIo.bindInt(2, version);
        delIo.step();

        auto delClaims = db_.prepare("DELETE FROM process_resource_claims WHERE session_id = ? AND config_version = ?;");
        delClaims.bindText(1, sessionId);
        delClaims.bindInt(2, version);
        delClaims.step();

        auto delDisk = db_.prepare("DELETE FROM process_disk_requests WHERE session_id = ? AND config_version = ?;");
        delDisk.bindText(1, sessionId);
        delDisk.bindInt(2, version);
        delDisk.step();

        auto delSync = db_.prepare("DELETE FROM synchronization_configs WHERE session_id = ? AND config_version = ?;");
        delSync.bindText(1, sessionId);
        delSync.bindInt(2, version);
        delSync.step();

        auto delProc = db_.prepare("DELETE FROM processes WHERE session_id = ? AND config_version = ?;");
        delProc.bindText(1, sessionId);
        delProc.bindInt(2, version);
        delProc.step();
    }

    auto procStmt = db_.prepare(
        "INSERT INTO processes (session_id, config_version, pid, arrival_time, cpu_burst, priority, memory_demand) "
        "VALUES (?, ?, ?, ?, ?, ?, ?);"
    );

    auto ioStmt = db_.prepare(
        "INSERT INTO process_io_requests (session_id, config_version, pid, request_time, duration, device) "
        "VALUES (?, ?, ?, ?, ?, ?);"
    );

    auto claimStmt = db_.prepare(
        "INSERT INTO process_resource_claims (session_id, config_version, pid, resource_type, max_claim, initial_allocation) "
        "VALUES (?, ?, ?, ?, ?, ?);"
    );

    auto diskStmt = db_.prepare(
        "INSERT INTO process_disk_requests (session_id, config_version, pid, block_or_cylinder, operation_type, volume_bytes) "
        "VALUES (?, ?, ?, ?, ?, ?);"
    );

    auto syncStmt = db_.prepare(
        "INSERT INTO synchronization_configs (session_id, config_version, pid, primitive_name, cs_start_time, cs_duration) "
        "VALUES (?, ?, ?, ?, ?, ?);"
    );

    for (const auto& proc : processes) {
        procStmt.reset();
        procStmt.bindText(1, sessionId);
        procStmt.bindInt(2, version);
        procStmt.bindInt(3, proc.getPid());
        procStmt.bindInt64(4, proc.getArrivalTime());
        procStmt.bindInt64(5, proc.getCpuBurst());
        procStmt.bindInt(6, proc.getPriority());
        if (proc.getMemoryConfig().isConfigured) {
            procStmt.bindInt64(7, proc.getMemoryConfig().demandUnits);
        } else {
            procStmt.bindNull(7);
        }
        procStmt.step();

        // I/O requests
        for (const auto& io : proc.getIoRequests()) {
            ioStmt.reset();
            ioStmt.bindText(1, sessionId);
            ioStmt.bindInt(2, version);
            ioStmt.bindInt(3, proc.getPid());
            ioStmt.bindInt64(4, io.requestTime);
            ioStmt.bindInt64(5, io.duration);
            ioStmt.bindText(6, io.device);
            ioStmt.step();
        }

        // Resource claims
        for (const auto& claim : proc.getResourceClaims()) {
            claimStmt.reset();
            claimStmt.bindText(1, sessionId);
            claimStmt.bindInt(2, version);
            claimStmt.bindInt(3, proc.getPid());
            claimStmt.bindText(4, claim.resourceType);
            claimStmt.bindInt(5, claim.maxClaim);
            claimStmt.bindInt(6, claim.initialAllocation);
            claimStmt.step();
        }

        // Disk requests
        for (const auto& disk : proc.getDiskRequests()) {
            diskStmt.reset();
            diskStmt.bindText(1, sessionId);
            diskStmt.bindInt(2, version);
            diskStmt.bindInt(3, proc.getPid());
            diskStmt.bindInt(4, disk.blockOrCylinder);
            diskStmt.bindText(5, disk.operationType);
            diskStmt.bindInt64(6, disk.dataVolumeBytes);
            diskStmt.step();
        }

        // Sync requirements
        for (const auto& sync : proc.getSyncRequirements()) {
            syncStmt.reset();
            syncStmt.bindText(1, sessionId);
            syncStmt.bindInt(2, version);
            syncStmt.bindInt(3, proc.getPid());
            syncStmt.bindText(4, sync.primitiveName);
            syncStmt.bindInt64(5, sync.csStartTime);
            syncStmt.bindInt64(6, sync.csDuration);
            syncStmt.step();
        }
    }
}

bool SessionRepository::loadSession(const std::string& sessionId, session::Session& outSession) {
    auto stmt = db_.prepare("SELECT session_id, created_at, status, description FROM sessions WHERE session_id = ?;");
    stmt.bindText(1, sessionId);
    if (!stmt.step()) {
        return false;
    }

    outSession.setSessionId(stmt.getText(0));
    outSession.setCreatedAt(stmt.getText(1));
    outSession.setStatus(core::stringToSessionStatus(stmt.getText(2)));
    outSession.setDescription(stmt.getText(3));

    // Get highest active configuration version
    auto stmtVer = db_.prepare(
        "SELECT MAX(version) FROM configuration_versions WHERE session_id = ?;"
    );
    stmtVer.bindText(1, sessionId);
    int32_t activeVer = 1;
    if (stmtVer.step() && !stmtVer.isNull(0)) {
        activeVer = stmtVer.getInt(0);
    }
    outSession.setActiveVersion(activeVer);

    // Load processes
    std::vector<process::Process> procs;
    loadProcesses(sessionId, activeVer, procs);
    outSession.setProcesses(procs);

    return true;
}

void SessionRepository::loadProcesses(const std::string& sessionId, int32_t version, std::vector<process::Process>& outProcesses) {
    outProcesses.clear();

    auto procStmt = db_.prepare(
        "SELECT pid, arrival_time, cpu_burst, priority, memory_demand "
        "FROM processes WHERE session_id = ? AND config_version = ? ORDER BY pid ASC;"
    );
    procStmt.bindText(1, sessionId);
    procStmt.bindInt(2, version);

    while (procStmt.step()) {
        core::ProcessId pid = procStmt.getInt(0);
        core::TimeUnits arrivalTime = procStmt.getInt64(1);
        core::TimeUnits cpuBurst = procStmt.getInt64(2);
        core::Priority priority = procStmt.getInt(3);

        process::Process proc(pid, arrivalTime, cpuBurst, priority);

        if (!procStmt.isNull(4)) {
            process::MemoryConfig mem;
            mem.isConfigured = true;
            mem.demandUnits = procStmt.getInt64(4);
            proc.setMemoryConfig(mem);
        }

        outProcesses.push_back(proc);
    }

    // Now populate subsystem fields for each process
    for (auto& proc : outProcesses) {
        // I/O requests
        auto ioStmt = db_.prepare(
            "SELECT request_time, duration, device FROM process_io_requests "
            "WHERE session_id = ? AND config_version = ? AND pid = ? ORDER BY request_time ASC;"
        );
        ioStmt.bindText(1, sessionId);
        ioStmt.bindInt(2, version);
        ioStmt.bindInt(3, proc.getPid());
        while (ioStmt.step()) {
            process::IoRequest io;
            io.requestTime = ioStmt.getInt64(0);
            io.duration = ioStmt.getInt64(1);
            io.device = ioStmt.getText(2);
            proc.addIoRequest(io);
        }

        // Resource claims
        auto claimStmt = db_.prepare(
            "SELECT resource_type, max_claim, initial_allocation FROM process_resource_claims "
            "WHERE session_id = ? AND config_version = ? AND pid = ?;"
        );
        claimStmt.bindText(1, sessionId);
        claimStmt.bindInt(2, version);
        claimStmt.bindInt(3, proc.getPid());
        while (claimStmt.step()) {
            process::ResourceClaim claim;
            claim.resourceType = claimStmt.getText(0);
            claim.maxClaim = claimStmt.getInt(1);
            claim.initialAllocation = claimStmt.getInt(2);
            proc.addResourceClaim(claim);
        }

        // Disk requests
        auto diskStmt = db_.prepare(
            "SELECT block_or_cylinder, operation_type, volume_bytes FROM process_disk_requests "
            "WHERE session_id = ? AND config_version = ? AND pid = ?;"
        );
        diskStmt.bindText(1, sessionId);
        diskStmt.bindInt(2, version);
        diskStmt.bindInt(3, proc.getPid());
        while (diskStmt.step()) {
            process::DiskRequest disk;
            disk.blockOrCylinder = diskStmt.getInt(0);
            disk.operationType = diskStmt.getText(1);
            disk.dataVolumeBytes = diskStmt.getInt64(2);
            proc.addDiskRequest(disk);
        }

        // Synchronization configs
        auto syncStmt = db_.prepare(
            "SELECT primitive_name, cs_start_time, cs_duration FROM synchronization_configs "
            "WHERE session_id = ? AND config_version = ? AND pid = ?;"
        );
        syncStmt.bindText(1, sessionId);
        syncStmt.bindInt(2, version);
        syncStmt.bindInt(3, proc.getPid());
        while (syncStmt.step()) {
            process::SyncRequirement sync;
            sync.primitiveName = syncStmt.getText(0);
            sync.csStartTime = syncStmt.getInt64(1);
            sync.csDuration = syncStmt.getInt64(2);
            proc.addSyncRequirement(sync);
        }
    }
}

std::vector<session::SessionMetadata> SessionRepository::listSessions() {
    std::vector<session::SessionMetadata> list;

    auto stmt = db_.prepare(
        "SELECT s.session_id, s.created_at, s.status, s.description, "
        "COALESCE((SELECT MAX(version) FROM configuration_versions cv WHERE cv.session_id = s.session_id), 1) as ver, "
        "COALESCE((SELECT COUNT(*) FROM processes p WHERE p.session_id = s.session_id), 0) as pcount, "
        "COALESCE((SELECT COUNT(*) FROM simulation_runs sr WHERE sr.session_id = s.session_id), 0) as rcount "
        "FROM sessions s ORDER BY s.created_at DESC;"
    );

    while (stmt.step()) {
        session::SessionMetadata meta;
        meta.sessionId = stmt.getText(0);
        meta.createdAt = stmt.getText(1);
        meta.status = core::stringToSessionStatus(stmt.getText(2));
        meta.description = stmt.getText(3);
        meta.activeVersion = stmt.getInt(4);
        meta.processCount = static_cast<size_t>(stmt.getInt64(5));
        meta.runCount = static_cast<size_t>(stmt.getInt64(6));
        list.push_back(meta);
    }

    return list;
}

bool SessionRepository::sessionExists(const std::string& sessionId) {
    auto stmt = db_.prepare("SELECT 1 FROM sessions WHERE session_id = ?;");
    stmt.bindText(1, sessionId);
    return stmt.step();
}

void SessionRepository::updateSessionStatus(const std::string& sessionId, core::SessionStatus status) {
    auto stmt = db_.prepare("UPDATE sessions SET status = ? WHERE session_id = ?;");
    stmt.bindText(1, core::sessionStatusToString(status));
    stmt.bindText(2, sessionId);
    stmt.step();
}

bool SessionRepository::deleteSession(const std::string& sessionId) {
    auto stmt = db_.prepare("DELETE FROM sessions WHERE session_id = ?;");
    stmt.bindText(1, sessionId);
    stmt.step();
    return true;
}

void SessionRepository::logEvent(const std::string& sessionId, const std::string& level, const std::string& source, const std::string& message) {
    try {
        auto stmt = db_.prepare(
            "INSERT INTO event_logs (session_id, log_level, source, message, timestamp) "
            "VALUES (?, ?, ?, ?, ?);"
        );
        stmt.bindText(1, sessionId);
        stmt.bindText(2, level);
        stmt.bindText(3, source);
        stmt.bindText(4, message);
        stmt.bindText(5, core::IdGenerator::getCurrentTimestampIso());
        stmt.step();
    } catch (...) {
        // Logging failure should not crash application
    }
}

} // namespace database
} // namespace watchos
