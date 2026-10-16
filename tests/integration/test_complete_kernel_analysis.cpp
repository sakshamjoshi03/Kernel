#include "watchos/session/session_manager.hpp"
#include "watchos/database/schema.hpp"
#include "watchos/database/run_repository.hpp"
#include "watchos/scheduler/fcfs.hpp"
#include "watchos/scheduler/sjf.hpp"
#include "watchos/scheduler/srtf.hpp"
#include "watchos/scheduler/priority_scheduler.hpp"
#include "watchos/scheduler/round_robin.hpp"
#include "watchos/memory/memory_manager.hpp"
#include "watchos/deadlock/resource_manager.hpp"
#include "watchos/deadlock/bankers_algorithm.hpp"
#include "watchos/deadlock/deadlock_detector.hpp"
#include "watchos/io/disk_scheduler.hpp"
#include "watchos/security/access_controller.hpp"
#include "watchos/reporting/report_generator.hpp"
#include "watchos/core/id_generator.hpp"
#include "tests/fixtures/test_workloads.hpp"
#include <iostream>
#include <cassert>
#include <fstream>
#include <cstdio>

int main() {
    std::cout << "============================================================\n";
    std::cout << "[INTEGRATION TEST] Complete Kernel Analysis & End-to-End Persistence\n";
    std::cout << "============================================================\n";

    const std::string dbFile = "test_kernel_analysis.db";
    std::remove(dbFile.c_str());

    {
        watchos::database::DbConnection db(dbFile);
        db.open();
        watchos::database::Schema::initializeSchema(db);

        watchos::session::SessionManager sessMgr(db);
        watchos::database::RunRepository runRepo(db);

        // 1. Create and persist session
        auto session = sessMgr.createNewSession("Kernel Analysis Test Session");
        session->setProcesses(watchos::test::getGoldenWorkload());
        sessMgr.saveActiveSession(true);

        std::string runId = watchos::core::IdGenerator::generateRunId();
        runRepo.createRun(runId, session->getSessionId(), "FULL_KERNEL_ANALYSIS", "All subsystems integrated");

        // 2. CPU Scheduling
        watchos::core::SystemSettings settings;
        watchos::scheduler::FcfsScheduler fcfs;
        watchos::scheduler::SrtfScheduler srtf;
        auto mFcfs = fcfs.schedule(session->getProcesses(), settings);
        auto mSrtf = srtf.schedule(session->getProcesses(), settings);

        std::vector<watchos::scheduler::AggregateMetrics> allSched = {mFcfs, mSrtf};
        for (const auto& agg : allSched) {
            for (const auto& pr : agg.processResults) {
                watchos::database::SchedulingResultRecord rec;
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
        }

        // 3. Virtual Memory Analysis
        watchos::memory::MemoryManager memMgr(16, 4);
        auto memResults = memMgr.runComparativeAnalysis(session->getProcesses(), 3);
        assert(memResults.size() == 3);

        // 4. Deadlock Analysis
        watchos::deadlock::ResourceManager resMgr;
        resMgr.loadFromWorkload(session->getProcesses());
        std::vector<watchos::core::ProcessId> pids = {1, 2, 3};
        auto bankersRes = watchos::deadlock::BankersAlgorithm::evaluateSafeState(resMgr, pids);
        auto dlRes = watchos::deadlock::DeadlockDetector::detect(resMgr, pids);

        // 5. Disk Scheduling
        auto diskRes = watchos::io::DiskScheduler::scheduleSstf(53, {98, 183, 37, 122, 14, 124, 65, 67});

        // 6. Security Access Control
        watchos::security::AccessController sec;
        sec.checkAccess(1, "system_file", "READ", 1);
        sec.checkAccess(2, "system_file", "WRITE", 2);

        // 7. Generate Full Report
        watchos::reporting::CompleteReportData reportData;
        reportData.sessionId = session->getSessionId();
        reportData.runId = runId;
        reportData.generatedAt = watchos::core::IdGenerator::getCurrentTimestampIso();
        reportData.processes = session->getProcesses();
        reportData.schedulingMetrics = allSched;
        reportData.memoryResults = memResults;
        reportData.bankersResult = bankersRes;
        reportData.deadlockResult = dlRes;
        reportData.diskResult = diskRes;

        std::string mdContent = watchos::reporting::ReportGenerator::generateMarkdownReport(reportData);
        assert(mdContent.find("WATCH OS — SIMULATION REPORT") != std::string::npos);

        std::string testReportFile = "test_export_report.md";
        bool exported = watchos::reporting::ReportGenerator::exportToFile(testReportFile, mdContent);
        assert(exported);

        // Verify exported file exists and has content
        std::ifstream verifyStream(testReportFile);
        assert(verifyStream.good());
        verifyStream.close();
        std::remove(testReportFile.c_str());

        runRepo.updateRunStatus(runId, "COMPLETED");

        // Verify records in database
        auto runs = runRepo.getRunsForSession(session->getSessionId());
        assert(runs.size() == 1);
        assert(runs[0].status == "COMPLETED");

        auto savedSched = runRepo.getSchedulingResultsForRun(runId);
        assert(!savedSched.empty());

        sessMgr.closeActiveSession();
        db.close();
    }

    std::remove(dbFile.c_str());
    std::cout << "[PASS] Complete kernel analysis integrated workflow and persistence verified!\n";
    return 0;
}
