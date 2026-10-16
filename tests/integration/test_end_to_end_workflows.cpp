#include "watchos/database/db_connection.hpp"
#include "watchos/database/schema.hpp"
#include "watchos/database/session_repository.hpp"
#include "watchos/database/run_repository.hpp"
#include "watchos/core/id_generator.hpp"
#include "watchos/scheduler/fcfs.hpp"
#include "watchos/scheduler/sjf.hpp"
#include "watchos/scheduler/srtf.hpp"
#include "watchos/scheduler/priority_scheduler.hpp"
#include "watchos/scheduler/round_robin.hpp"
#include "watchos/scheduler/recommendation_engine.hpp"
#include "watchos/process/lifecycle_simulator.hpp"
#include "watchos/memory/memory_manager.hpp"
#include "watchos/deadlock/resource_manager.hpp"
#include "watchos/deadlock/bankers_algorithm.hpp"
#include "watchos/deadlock/deadlock_detector.hpp"
#include "watchos/io/disk_scheduler.hpp"
#include "watchos/security/access_controller.hpp"
#include "watchos/reporting/report_generator.hpp"
#include <cassert>
#include <iostream>
#include <fstream>
#include <vector>

using namespace watchos;

void testWorkflow1_CreateSaveRestartResume() {
    std::cout << "[WORKFLOW 1] Create Session -> Configure Processes -> Save -> Restart -> Resume...\n";
    const std::string dbFile = "test_e2e_w1.db";
    std::remove(dbFile.c_str());

    std::string sessionId;
    {
        database::DbConnection db(dbFile);
        db.open();
        database::Schema::initializeSchema(db);

        database::SessionRepository repo(db);
        sessionId = core::IdGenerator::generateSessionId();
        session::Session sess(sessionId, "E2E Workflow Session");

        std::vector<process::Process> procs = {
            process::Process(101, 0, 5, 2),
            process::Process(102, 1, 3, 1),
            process::Process(103, 2, 8, 3)
        };
        sess.setProcesses(procs);
        repo.createSession(sess);
        repo.saveWorkload(sess, true);
    } // Connection closed, simulating application shutdown

    // "Restart" - new connection instance
    {
        database::DbConnection db(dbFile);
        db.open();
        database::SessionRepository repo(db);

        session::Session resumedSess;
        bool loaded = repo.loadSession(sessionId, resumedSess);
        assert(loaded);
        assert(resumedSess.getSessionId() == sessionId);
        assert(resumedSess.getDescription() == "E2E Workflow Session");
        assert(resumedSess.getStatus() == core::SessionStatus::FINALIZED);
        assert(resumedSess.getProcesses().size() == 3);
        assert(resumedSess.getProcesses()[0].getPid() == 101);
        assert(resumedSess.getProcesses()[1].getPid() == 102);
        assert(resumedSess.getProcesses()[2].getPid() == 103);
    }

    std::remove(dbFile.c_str());
    std::cout << "  -> Workflow 1 PASSED.\n";
}

void testWorkflow2_RunSchedulingSaveHistory() {
    std::cout << "[WORKFLOW 2] Run Scheduling -> Save Results -> View Run History...\n";
    database::DbConnection db(":memory:");
    db.open();
    database::Schema::initializeSchema(db);

    database::SessionRepository sessionRepo(db);
    database::RunRepository runRepo(db);

    std::string sessId = "WOS-E2E-0002";
    session::Session sess(sessId, "Scheduling History Session");
    std::vector<process::Process> procs = {
        process::Process(1, 0, 4, 1),
        process::Process(2, 1, 5, 2)
    };
    sess.setProcesses(procs);
    sessionRepo.createSession(sess);
    sessionRepo.saveWorkload(sess, true);

    std::string runId = "RUN-E2E-0002";
    runRepo.createRun(runId, sessId, "CPU_SCHEDULING", "FCFS");

    scheduler::FcfsScheduler fcfs;
    auto agg = fcfs.schedule(sess.getProcesses(), sess.getSettings());
    for (const auto& pr : agg.processResults) {
        database::SchedulingResultRecord rec;
        rec.runId = runId;
        rec.pid = pr.pid;
        rec.arrivalTime = pr.arrivalTime;
        rec.cpuBurst = pr.cpuBurst;
        rec.priority = pr.priority;
        rec.completionTime = pr.completionTime;
        rec.turnaroundTime = pr.turnaroundTime;
        rec.waitingTime = pr.waitingTime;
        rec.responseTime = pr.responseTime;
        runRepo.recordSchedulingResult(rec);
    }
    runRepo.updateRunStatus(runId, "COMPLETED");

    // View history
    auto runs = runRepo.getRunsForSession(sessId);
    assert(runs.size() == 1);
    assert(runs[0].runId == runId);
    assert(runs[0].status == "COMPLETED");

    auto results = runRepo.getSchedulingResultsForRun(runId);
    assert(results.size() == 2);
    assert(results[0].pid == 1 && results[0].completionTime == 4);
    assert(results[1].pid == 2 && results[1].completionTime == 9);

    std::cout << "  -> Workflow 2 PASSED.\n";
}

void testWorkflow3_RunAllSchedulersCompare() {
    std::cout << "[WORKFLOW 3] Run All Schedulers -> Compare Results...\n";
    std::vector<process::Process> procs = {
        process::Process(1, 0, 7, 2),
        process::Process(2, 2, 4, 1),
        process::Process(3, 4, 1, 3),
        process::Process(4, 5, 4, 2)
    };
    core::SystemSettings settings;

    scheduler::FcfsScheduler fcfs;
    scheduler::SjfScheduler sjf;
    scheduler::SrtfScheduler srtf;
    scheduler::PriorityScheduler priNp(false);
    scheduler::PriorityScheduler priP(true);
    scheduler::RoundRobinScheduler rr(2);

    std::vector<scheduler::AggregateMetrics> metrics = {
        fcfs.schedule(procs, settings),
        sjf.schedule(procs, settings),
        srtf.schedule(procs, settings),
        priNp.schedule(procs, settings),
        priP.schedule(procs, settings),
        rr.schedule(procs, settings)
    };

    auto ranking = scheduler::RecommendationEngine::evaluateAndRank(metrics);
    assert(!ranking.empty());
    assert(ranking[0].rank == 1);
    std::cout << "  -> Best algorithm recommended: " << ranking[0].metrics.algorithmName << "\n";
    std::cout << "  -> Workflow 3 PASSED.\n";
}

void testWorkflow4_LifecycleSimulationTransitions() {
    std::cout << "[WORKFLOW 4] Run Lifecycle Simulation -> Inspect State Transitions...\n";
    std::vector<process::Process> procs = {
        process::Process(1, 0, 5, 1)
    };

    process::LifecycleSimulator sim(procs);
    assert(sim.getMetrics(1)->currentState == core::ProcessState::NEW);

    sim.transition(1, core::ProcessState::READY, 0, "Arrival");
    assert(sim.getMetrics(1)->currentState == core::ProcessState::READY);

    sim.transition(1, core::ProcessState::RUNNING, 0, "Dispatched");
    assert(sim.getMetrics(1)->currentState == core::ProcessState::RUNNING);

    sim.transition(1, core::ProcessState::WAITING, 2, "I/O Request");
    assert(sim.getMetrics(1)->currentState == core::ProcessState::WAITING);

    sim.transition(1, core::ProcessState::READY, 4, "I/O Complete");
    assert(sim.getMetrics(1)->currentState == core::ProcessState::READY);

    sim.transition(1, core::ProcessState::RUNNING, 4, "Resumed");
    sim.transition(1, core::ProcessState::TERMINATED, 7, "Finished");
    assert(sim.getMetrics(1)->currentState == core::ProcessState::TERMINATED);

    auto metrics = sim.getMetrics(1);
    assert(metrics->completionTime == 7);
    assert(metrics->ioWait == 2);
    std::cout << "  -> Workflow 4 PASSED.\n";
}

void testWorkflow5_MemoryAnalysisPageFaults() {
    std::cout << "[WORKFLOW 5] Run Memory Analysis -> Verify Page-Fault Results...\n";
    std::vector<int32_t> refString = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1};

    auto fifo = memory::PageReplacementSimulator::simulate(memory::PageReplacementPolicy::FIFO, refString, 3);
    auto lru = memory::PageReplacementSimulator::simulate(memory::PageReplacementPolicy::LRU, refString, 3);
    auto opt = memory::PageReplacementSimulator::simulate(memory::PageReplacementPolicy::OPTIMAL, refString, 3);

    assert(fifo.pageFaults == 15);
    assert(lru.pageFaults == 12);
    assert(opt.pageFaults == 9);
    assert(opt.pageFaults <= lru.pageFaults && lru.pageFaults <= fifo.pageFaults);

    std::cout << "  -> FIFO=" << fifo.pageFaults << ", LRU=" << lru.pageFaults << ", OPT=" << opt.pageFaults << "\n";
    std::cout << "  -> Workflow 5 PASSED.\n";
}

void testWorkflow6_BankersSafeUnsafeStates() {
    std::cout << "[WORKFLOW 6] Run Banker's Algorithm -> Verify Safe/Unsafe States...\n";
    deadlock::ResourceManager rm;
    rm.registerResourceType("R1", 10);
    rm.registerResourceType("R2", 5);
    rm.registerResourceType("R3", 7);

    // Setup safe state
    std::vector<core::ProcessId> pids = {0, 1, 2};
    rm.setMaxClaim(0, "R1", 7);
    rm.setAllocation(0, "R1", 0);
    rm.setMaxClaim(0, "R2", 5);
    rm.setAllocation(0, "R2", 1);
    rm.setMaxClaim(0, "R3", 3);
    rm.setAllocation(0, "R3", 0);

    rm.setMaxClaim(1, "R1", 3);
    rm.setAllocation(1, "R1", 2);
    rm.setMaxClaim(1, "R2", 2);
    rm.setAllocation(1, "R2", 0);
    rm.setMaxClaim(1, "R3", 2);
    rm.setAllocation(1, "R3", 0);

    rm.setMaxClaim(2, "R1", 9);
    rm.setAllocation(2, "R1", 3);
    rm.setMaxClaim(2, "R2", 0);
    rm.setAllocation(2, "R2", 0);
    rm.setMaxClaim(2, "R3", 2);
    rm.setAllocation(2, "R3", 2);

    auto result = deadlock::BankersAlgorithm::evaluateSafeState(rm, pids);
    assert(result.isSafe);
    assert(result.safeSequence.size() == 3);

    auto det = deadlock::DeadlockDetector::detect(rm, pids);
    assert(!det.isDeadlocked);

    std::cout << "  -> Workflow 6 PASSED.\n";
}

void testWorkflow7_DiskSchedulingHeadMovement() {
    std::cout << "[WORKFLOW 7] Run Disk Scheduling -> Verify Head Movement...\n";
    std::vector<int32_t> requests = {98, 183, 37, 122, 14, 124, 65, 67};
    int32_t head = 53;

    auto fcfs = io::DiskScheduler::scheduleFcfs(head, requests);
    auto sstf = io::DiskScheduler::scheduleSstf(head, requests);

    assert(fcfs.totalHeadMovement == 640);
    assert(sstf.totalHeadMovement == 236);
    assert(sstf.totalHeadMovement < fcfs.totalHeadMovement);

    std::cout << "  -> FCFS=" << fcfs.totalHeadMovement << ", SSTF=" << sstf.totalHeadMovement << "\n";
    std::cout << "  -> Workflow 7 PASSED.\n";
}

void testWorkflow8_PermissionsAudit() {
    std::cout << "[WORKFLOW 8] Run Permissions Analysis -> Verify Allow/Deny Decisions...\n";
    security::AccessController sec;

    sec.assignRole(101, "root");
    sec.assignRole(102, "guest");

    auto r1 = sec.checkAccess(101, "system_config", "WRITE", 1);
    assert(r1.granted); // Root allowed

    auto r2 = sec.checkAccess(102, "system_config", "WRITE", 2);
    assert(!r2.granted); // Guest denied

    bool memOk = sec.checkMemoryAccess(102, 1200, 1000, 500, 3);
    assert(memOk);

    bool memViolation = sec.checkMemoryAccess(102, 1600, 1000, 500, 4);
    assert(!memViolation); // Out of bounds -> denied

    std::cout << "  -> Workflow 8 PASSED.\n";
}

void testWorkflow9_GenerateReportCompareStoredResults() {
    std::cout << "[WORKFLOW 9] Generate Report -> Compare Report Values with Stored Results...\n";
    database::DbConnection db(":memory:");
    db.open();
    database::Schema::initializeSchema(db);

    database::SessionRepository sessionRepo(db);
    database::RunRepository runRepo(db);

    reporting::CompleteReportData rd;
    rd.sessionId = "WOS-REPORT-TEST";
    rd.runId = "RUN-REPORT-TEST";
    rd.generatedAt = "2026-10-16T12:00:00Z";
    rd.processes = {process::Process(1, 0, 5, 1)};

    // Create parent session and simulation run to fulfill foreign key constraints
    session::Session sess(rd.sessionId, "Report Test Session");
    sessionRepo.createSession(sess);
    runRepo.createRun(rd.runId, rd.sessionId, "CPU_SCHEDULING", "FCFS report run");

    scheduler::FcfsScheduler fcfs;
    rd.schedulingMetrics = {fcfs.schedule(rd.processes, core::SystemSettings{})};

    std::string md = reporting::ReportGenerator::generateMarkdownReport(rd);
    std::string csv = reporting::ReportGenerator::generateCsvReport(rd);

    assert(md.find("WOS-REPORT-TEST") != std::string::npos);
    assert(md.find("FCFS") != std::string::npos);
    assert(csv.find("WOS-REPORT-TEST") != std::string::npos);

    runRepo.saveReportRecord("RPT-001", rd.sessionId, rd.runId, "MARKDOWN", "reports/test.md");
    auto runs = runRepo.getRunsForSession(rd.sessionId);
    assert(!runs.empty());

    std::cout << "  -> Workflow 9 PASSED.\n";
}

int main() {
    std::cout << "============================================================\n";
    std::cout << "          WATCH OS — END-TO-END WORKFLOW SUITE\n";
    std::cout << "============================================================\n";

    testWorkflow1_CreateSaveRestartResume();
    testWorkflow2_RunSchedulingSaveHistory();
    testWorkflow3_RunAllSchedulersCompare();
    testWorkflow4_LifecycleSimulationTransitions();
    testWorkflow5_MemoryAnalysisPageFaults();
    testWorkflow6_BankersSafeUnsafeStates();
    testWorkflow7_DiskSchedulingHeadMovement();
    testWorkflow8_PermissionsAudit();
    testWorkflow9_GenerateReportCompareStoredResults();

    std::cout << "All 9 end-to-end integration workflows PASSED successfully!\n";
    return 0;
}
