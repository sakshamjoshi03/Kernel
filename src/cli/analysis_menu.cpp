#include "watchos/cli/analysis_menu.hpp"
#include "watchos/cli/console_ui.hpp"
#include "watchos/cli/ansi_terminal.hpp"
#include "watchos/core/id_generator.hpp"
#include "watchos/scheduler/fcfs.hpp"
#include "watchos/scheduler/sjf.hpp"
#include "watchos/scheduler/srtf.hpp"
#include "watchos/scheduler/priority_scheduler.hpp"
#include "watchos/scheduler/round_robin.hpp"
#include "watchos/scheduler/gantt_chart.hpp"
#include "watchos/scheduler/recommendation_engine.hpp"
#include "watchos/scheduler/aging_analyzer.hpp"
#include "watchos/process/lifecycle_simulator.hpp"
#include "watchos/memory/memory_manager.hpp"
#include "watchos/deadlock/resource_manager.hpp"
#include "watchos/deadlock/bankers_algorithm.hpp"
#include "watchos/deadlock/deadlock_detector.hpp"
#include "watchos/deadlock/deadlock_frequency.hpp"
#include "watchos/io/disk_scheduler.hpp"
#include "watchos/io/io_manager.hpp"
#include "watchos/synchronization/sync_manager.hpp"
#include "watchos/security/access_controller.hpp"
#include "watchos/reporting/jarvis_explainer.hpp"
#include "watchos/reporting/report_generator.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>

namespace watchos {
namespace cli {

AnalysisMenu::AnalysisMenu(session::Session& session, database::DbConnection& db)
    : session_(session), db_(db), runRepo_(db) {}

void AnalysisMenu::runMenuLoop() {
    bool inLoop = true;
    while (inLoop) {
        printAnalysisMenu();
        std::cout << "\nWATCH OS > ";
        std::string line;
        if (!std::getline(std::cin, line)) {
            break;
        }

        size_t first = line.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) continue;
        size_t last = line.find_last_not_of(" \t\r\n");
        line = line.substr(first, (last - first + 1));

        // Check if numeric menu choice
        try {
            size_t idx = 0;
            int opt = std::stoi(line, &idx);
            if (idx == line.length()) {
                if (opt == 18) {
                    inLoop = false;
                    break;
                }
                executeOption(opt);
                continue;
            }
        } catch (...) {}

        // Command parser
        if (line == "exit" || line == "quit" || line == "back") {
            inLoop = false;
        } else {
            executeCommand(line);
        }
    }
}

void AnalysisMenu::printAnalysisMenu() {
    std::cout << "\n" << AnsiTerminal::cyan() << AnsiTerminal::bold()
              << "============================================================\n"
              << "            WATCH OS — KERNEL ANALYSIS CONSOLE\n"
              << "============================================================\n"
              << AnsiTerminal::reset()
              << "Active Session: " << session_.getSessionId() << " | Workload: " << session_.getProcesses().size() << " Processes\n\n"
              << " 01  Find Best CPU Scheduling Algorithm\n"
              << " 02  Run All Scheduling Algorithms\n"
              << " 03  Display Gantt Charts\n"
              << " 04  Show Process Lifecycle\n"
              << " 05  Analyze Context Switching\n"
              << " 06  Detect Starvation\n"
              << " 07  Apply Aging and Compare\n"
              << " 08  Run Deadlock Analysis\n"
              << " 09  Estimate Deadlock Frequency\n"
              << " 10  Virtual Memory Analysis\n"
              << " 11  Disk Usage & I/O Analysis\n"
              << " 12  Process Synchronization & IPC\n"
              << " 13  Permissions & Protection\n"
              << " 14  Run Complete Kernel Analysis\n"
              << " 15  Compare Previous Runs\n"
              << " 16  View Session Logs\n"
              << " 17  Generate Final Report\n"
              << " 18  Return to Main Menu\n";
}

void AnalysisMenu::executeCommand(const std::string& cmd) {
    std::string lowerCmd = cmd;
    std::transform(lowerCmd.begin(), lowerCmd.end(), lowerCmd.begin(), ::tolower);

    if (lowerCmd == "help" || lowerCmd == "?") {
        printAnalysisMenu();
    } else if (lowerCmd == "schedule all" || lowerCmd == "run all") {
        opt02_runAllSchedulers();
    } else if (lowerCmd == "best" || lowerCmd == "find best" || lowerCmd == "best scheduler") {
        opt01_findBestScheduler();
    } else if (lowerCmd == "gantt" || lowerCmd == "display gantt") {
        opt03_displayGanttCharts();
    } else if (lowerCmd == "show lifecycle" || lowerCmd == "lifecycle") {
        opt04_showProcessLifecycle();
    } else if (lowerCmd == "context switch" || lowerCmd == "context") {
        opt05_analyzeContextSwitching();
    } else if (lowerCmd == "starvation" || lowerCmd == "detect starvation") {
        opt06_detectStarvation();
    } else if (lowerCmd == "aging" || lowerCmd == "apply aging") {
        opt07_applyAgingAndCompare();
    } else if (lowerCmd == "deadlock analyze" || lowerCmd == "deadlock") {
        opt08_runDeadlockAnalysis();
    } else if (lowerCmd == "deadlock frequency" || lowerCmd == "monte carlo") {
        opt09_estimateDeadlockFrequency();
    } else if (lowerCmd == "memory analyze" || lowerCmd == "memory" || lowerCmd == "paging") {
        opt10_virtualMemoryAnalysis();
    } else if (lowerCmd == "disk analyze" || lowerCmd == "disk" || lowerCmd == "io") {
        opt11_diskUsageAndIoAnalysis();
    } else if (lowerCmd == "sync" || lowerCmd == "ipc") {
        opt12_processSynchronizationAndIpc();
    } else if (lowerCmd == "permissions" || lowerCmd == "security" || lowerCmd == "protection") {
        opt13_permissionsAndProtection();
    } else if (lowerCmd == "complete" || lowerCmd == "all analysis") {
        opt14_runCompleteKernelAnalysis();
    } else if (lowerCmd == "compare runs" || lowerCmd == "compare") {
        opt15_comparePreviousRuns();
    } else if (lowerCmd == "view logs" || lowerCmd == "logs") {
        opt16_viewSessionLogs();
    } else if (lowerCmd == "report generate" || lowerCmd == "report") {
        opt17_generateFinalReport();
    } else {
        ConsoleUi::printWarning("Unknown command '" + cmd + "'. Supported commands: 'schedule all', 'show lifecycle', 'deadlock analyze', 'memory analyze', 'disk analyze', 'compare runs', 'report generate' or enter options 01-18.");
    }
}

void AnalysisMenu::executeOption(int option) {
    switch (option) {
        case 1: opt01_findBestScheduler(); break;
        case 2: opt02_runAllSchedulers(); break;
        case 3: opt03_displayGanttCharts(); break;
        case 4: opt04_showProcessLifecycle(); break;
        case 5: opt05_analyzeContextSwitching(); break;
        case 6: opt06_detectStarvation(); break;
        case 7: opt07_applyAgingAndCompare(); break;
        case 8: opt08_runDeadlockAnalysis(); break;
        case 9: opt09_estimateDeadlockFrequency(); break;
        case 10: opt10_virtualMemoryAnalysis(); break;
        case 11: opt11_diskUsageAndIoAnalysis(); break;
        case 12: opt12_processSynchronizationAndIpc(); break;
        case 13: opt13_permissionsAndProtection(); break;
        case 14: opt14_runCompleteKernelAnalysis(); break;
        case 15: opt15_comparePreviousRuns(); break;
        case 16: opt16_viewSessionLogs(); break;
        case 17: opt17_generateFinalReport(); break;
        default:
            ConsoleUi::printWarning("Invalid option number: " + std::to_string(option) + ". Choose between 01 and 18.");
            break;
    }
}

void AnalysisMenu::ensureSchedulersRun() {
    if (cachedMetrics_.empty()) {
        opt02_runAllSchedulers();
    }
}

void AnalysisMenu::opt01_findBestScheduler() {
    if (session_.getProcesses().empty()) {
        ConsoleUi::printWarning("No processes configured in this session. Add processes in Configuration Menu before running simulation.");
        return;
    }
    ensureSchedulersRun();
    if (cachedMetrics_.empty()) {
        ConsoleUi::printWarning("No scheduling metrics available to evaluate.");
        return;
    }
    std::cout << "\nEvaluating best scheduling algorithm using multi-objective normalization...\n";
    auto ranking = scheduler::RecommendationEngine::evaluateAndRank(cachedMetrics_);
    std::cout << scheduler::RecommendationEngine::formatComparisonTable(ranking) << "\n";
}

void AnalysisMenu::opt02_runAllSchedulers() {
    if (session_.getProcesses().empty()) {
        ConsoleUi::printWarning("No processes configured in this session. Add processes in Configuration Menu before running simulation.");
        return;
    }

    std::string runId = core::IdGenerator::generateRunId();
    latestRunId_ = runId;
    std::cout << "\n[RUN] Creating new analysis run...\n";
    std::cout << "[RUN] Session: " << session_.getSessionId() << "\n";
    std::cout << "[RUN] Run ID: " << runId << "\n";
    std::cout << "[RUN] Algorithm set: FCFS, SJF, SRTF, Priority (Non-Preemptive), Priority (Preemptive), Round Robin\n\n";

    runRepo_.createRun(runId, session_.getSessionId(), "CPU_SCHEDULING", "Algorithms: FCFS, SJF, SRTF, Priority, RR");

    cachedMetrics_.clear();

    scheduler::FcfsScheduler fcfs;
    scheduler::SjfScheduler sjf;
    scheduler::SrtfScheduler srtf;
    scheduler::PriorityScheduler priNonPreempt(false);
    scheduler::PriorityScheduler priPreempt(true);
    scheduler::RoundRobinScheduler rr(session_.getSettings().defaultTimeQuantum);

    cachedMetrics_.push_back(fcfs.schedule(session_.getProcesses(), session_.getSettings()));
    cachedMetrics_.push_back(sjf.schedule(session_.getProcesses(), session_.getSettings()));
    cachedMetrics_.push_back(srtf.schedule(session_.getProcesses(), session_.getSettings()));
    cachedMetrics_.push_back(priNonPreempt.schedule(session_.getProcesses(), session_.getSettings()));
    cachedMetrics_.push_back(priPreempt.schedule(session_.getProcesses(), session_.getSettings()));
    cachedMetrics_.push_back(rr.schedule(session_.getProcesses(), session_.getSettings()));

    // Persist scheduling results & Gantt segments
    for (const auto& agg : cachedMetrics_) {
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
            runRepo_.recordSchedulingResult(rec);
        }
        for (const auto& seg : agg.ganttSegments) {
            runRepo_.recordGanttSegment(runId, seg.startTime, seg.endTime, seg.pid, seg.isIdle, seg.isContextSwitch);
        }
    }

    runRepo_.updateRunStatus(runId, "COMPLETED");

    std::cout << "+--------------------------------+---------+---------+---------+-------+---------+\n";
    std::cout << "| Algorithm                      | Avg WT  | Avg TAT | Avg RT  | CS    | CPU Util|\n";
    std::cout << "+--------------------------------+---------+---------+---------+-------+---------+\n";
    for (const auto& m : cachedMetrics_) {
        std::cout << "| " << std::left << std::setw(30) << m.algorithmName << std::right
                  << " | " << std::setw(7) << std::fixed << std::setprecision(2) << m.avgWaitingTime
                  << " | " << std::setw(7) << m.avgTurnaroundTime
                  << " | " << std::setw(7) << m.avgResponseTime
                  << " | " << std::setw(5) << m.totalContextSwitches
                  << " | " << std::setw(6) << m.cpuUtilization << "% |\n";
    }
    std::cout << "+--------------------------------+---------+---------+---------+-------+---------+\n";
    ConsoleUi::printSuccess("Results saved successfully to SQLite under Run ID: " + runId);
}

void AnalysisMenu::opt03_displayGanttCharts() {
    if (session_.getProcesses().empty()) {
        ConsoleUi::printWarning("No processes configured in this session.");
        return;
    }
    ensureSchedulersRun();
    std::cout << "\n============================================================\n";
    std::cout << "                   GANTT CHART VISUALIZER\n";
    std::cout << "============================================================\n";
    for (const auto& agg : cachedMetrics_) {
        std::cout << scheduler::GanttChart::render(agg.algorithmName, agg.ganttSegments);
    }
}

void AnalysisMenu::opt04_showProcessLifecycle() {
    if (session_.getProcesses().empty()) {
        ConsoleUi::printWarning("No processes configured in this session.");
        return;
    }
    std::cout << "\n============================================================\n";
    std::cout << "                 PROCESS LIFECYCLE TRACKER\n";
    std::cout << "============================================================\n";
    process::LifecycleSimulator sim(session_.getProcesses());

    std::string runId = latestRunId_.empty() ? core::IdGenerator::generateRunId() : latestRunId_;
    if (latestRunId_.empty()) {
        latestRunId_ = runId;
        runRepo_.createRun(runId, session_.getSessionId(), "PROCESS_LIFECYCLE", "State transitions tracking");
    }

    core::TimeUnits t = 0;
    // Step through process arrivals and simulated execution
    for (const auto& p : session_.getProcesses()) {
        sim.transition(p.getPid(), core::ProcessState::READY, p.getArrivalTime(), "Process arrived");
        runRepo_.recordProcessStateTransition({runId, p.getPid(), p.getArrivalTime(), "NEW", "READY", "Process arrived"});

        t = std::max(t, p.getArrivalTime());
        sim.transition(p.getPid(), core::ProcessState::RUNNING, t, "Dispatched to CPU");
        runRepo_.recordProcessStateTransition({runId, p.getPid(), t, "READY", "RUNNING", "Dispatched to CPU"});

        t += p.getCpuBurst();
        sim.transition(p.getPid(), core::ProcessState::TERMINATED, t, "CPU burst completed");
        runRepo_.recordProcessStateTransition({runId, p.getPid(), t, "RUNNING", "TERMINATED", "CPU burst completed"});
    }

    std::cout << "+-----+---------+------------+-------------+----------+--------------------------------------+\n";
    std::cout << "| PID | Arrival | First Start| Completion  | Total WT | Final State / Reason                 |\n";
    std::cout << "+-----+---------+------------+-------------+----------+--------------------------------------+\n";
    for (const auto& pair : sim.getAllMetrics()) {
        const auto& m = pair.second;
        std::cout << "| " << std::setw(3) << m.pid
                  << " | " << std::setw(7) << m.arrivalTime
                  << " | " << std::setw(10) << m.firstCpuStartTime
                  << " | " << std::setw(11) << m.completionTime
                  << " | " << std::setw(8) << m.readyQueueWait
                  << " | " << std::left << std::setw(36) << m.terminationReason << std::right << " |\n";
    }
    std::cout << "+-----+---------+------------+-------------+----------+--------------------------------------+\n";
    ConsoleUi::printSuccess("Process lifecycle transitions recorded under Run ID: " + runId);
}

void AnalysisMenu::opt05_analyzeContextSwitching() {
    if (session_.getProcesses().empty()) {
        ConsoleUi::printWarning("No processes configured in this session.");
        return;
    }
    ensureSchedulersRun();
    std::cout << "\nContext Switching Dynamics Analysis:\n";
    std::cout << "+--------------------------------+-----------------+-----------------------+\n";
    std::cout << "| Algorithm                      | Initial Starts  | Voluntary / Preempt CS|\n";
    std::cout << "+--------------------------------+-----------------+-----------------------+\n";
    for (const auto& m : cachedMetrics_) {
        std::cout << "| " << std::left << std::setw(30) << m.algorithmName << std::right
                  << " | " << std::setw(15) << session_.getProcesses().size()
                  << " | " << std::setw(21) << m.totalContextSwitches << " |\n";
    }
    std::cout << "+--------------------------------+-----------------+-----------------------+\n";
}

void AnalysisMenu::opt06_detectStarvation() {
    if (session_.getProcesses().empty()) {
        ConsoleUi::printWarning("No processes configured in this session.");
        return;
    }
    ensureSchedulersRun();
    std::cout << "\nStarvation Risk Detector (Threshold = 10 Time Units):\n";
    bool anyStarvation = false;
    for (const auto& m : cachedMetrics_) {
        if (m.starvationBreaches > 0) {
            std::cout << "  - [" << m.algorithmName << "]: " << m.starvationBreaches
                      << " processes exceeded threshold (Longest Wait: " << m.maxWaitingTime << " units).\n";
            anyStarvation = true;
        }
    }
    if (!anyStarvation) {
        std::cout << "  [OK] No starvation threshold breaches detected for this workload across algorithms.\n";
    }
}

void AnalysisMenu::opt07_applyAgingAndCompare() {
    if (session_.getProcesses().empty()) {
        ConsoleUi::printWarning("No processes configured in this session.");
        return;
    }
    std::cout << "\nExecuting Priority Scheduling Aging Experiment (Interval=5, Increment=1)...\n";
    auto comp = scheduler::AgingAnalyzer::compare(session_.getProcesses(), session_.getSettings(), false, 5, 1, 10);
    std::cout << scheduler::AgingAnalyzer::formatComparisonReport(comp);
}

void AnalysisMenu::opt08_runDeadlockAnalysis() {
    if (session_.getProcesses().empty()) {
        ConsoleUi::printWarning("No processes configured in this session.");
        return;
    }
    std::cout << "\nExecuting Resource Allocation & Deadlock Detection Analysis...\n";
    deadlock::ResourceManager rm;
    rm.loadFromWorkload(session_.getProcesses());

    std::cout << rm.renderResourceAllocationGraph() << "\n";

    std::vector<core::ProcessId> pids;
    for (const auto& p : session_.getProcesses()) pids.push_back(p.getPid());

    auto bankers = deadlock::BankersAlgorithm::evaluateSafeState(rm, pids);
    std::cout << deadlock::BankersAlgorithm::formatSafeStateReport(bankers) << "\n";

    auto det = deadlock::DeadlockDetector::detect(rm, pids);
    std::cout << det.report << "\n";

    // Persist deadlock results to database
    std::string runId = latestRunId_.empty() ? core::IdGenerator::generateRunId() : latestRunId_;
    if (latestRunId_.empty()) {
        latestRunId_ = runId;
        runRepo_.createRun(runId, session_.getSessionId(), "DEADLOCK_ANALYSIS", "Bankers & RAG evaluation");
    }

    std::string safeSeqStr;
    for (size_t i = 0; i < bankers.safeSequence.size(); ++i) {
        if (i > 0) safeSeqStr += " -> ";
        safeSeqStr += "P" + std::to_string(bankers.safeSequence[i]);
    }
    std::string dlProcsStr;
    for (size_t i = 0; i < det.deadlockedProcesses.size(); ++i) {
        if (i > 0) dlProcsStr += ", ";
        dlProcsStr += "P" + std::to_string(det.deadlockedProcesses[i]);
    }

    runRepo_.recordDeadlockResult(runId, det.isDeadlocked, bankers.isSafe, safeSeqStr, dlProcsStr, 0, 0, 0.0);
    ConsoleUi::printSuccess("Deadlock evaluation results persisted under Run ID: " + runId);
}

void AnalysisMenu::opt09_estimateDeadlockFrequency() {
    if (session_.getProcesses().empty()) {
        ConsoleUi::printWarning("No processes configured in this session.");
        return;
    }
    std::cout << "\nRunning Monte Carlo Deadlock Risk Simulation (1,000 Stochastic Trials)...\n";
    deadlock::ResourceManager rm;
    rm.loadFromWorkload(session_.getProcesses());

    std::vector<core::ProcessId> pids;
    for (const auto& p : session_.getProcesses()) pids.push_back(p.getPid());

    auto freq = deadlock::DeadlockFrequencyEstimator::estimate(rm, pids, 1000, 42);
    std::cout << freq.report << "\n";

    std::string runId = latestRunId_.empty() ? core::IdGenerator::generateRunId() : latestRunId_;
    if (latestRunId_.empty()) {
        latestRunId_ = runId;
        runRepo_.createRun(runId, session_.getSessionId(), "DEADLOCK_MONTE_CARLO", "1000 Stochastic Trials");
    }
    runRepo_.recordDeadlockResult(runId, freq.deadlockedTrials > 0, freq.deadlockedTrials == 0, "", "", freq.totalTrials, freq.deadlockedTrials, freq.empiricalFrequency);
}

void AnalysisMenu::opt10_virtualMemoryAnalysis() {
    if (session_.getProcesses().empty()) {
        ConsoleUi::printWarning("No processes configured in this session.");
        return;
    }
    std::cout << "\nExecuting Virtual Memory & Page Replacement Analysis (Frames = 3)...\n";
    memory::MemoryManager memMgr(16, 4);
    auto results = memMgr.runComparativeAnalysis(session_.getProcesses(), 3);
    for (const auto& res : results) {
        std::cout << memory::PageReplacementSimulator::formatResultTable(res) << "\n";
    }

    std::string runId = latestRunId_.empty() ? core::IdGenerator::generateRunId() : latestRunId_;
    if (latestRunId_.empty()) {
        latestRunId_ = runId;
        runRepo_.createRun(runId, session_.getSessionId(), "MEMORY_ANALYSIS", "FIFO, LRU, Optimal");
    }
    for (const auto& r : results) {
        runRepo_.recordMemoryResult(runId, r.algorithmName, r.totalReferences, r.pageFaults, r.pageHits, r.faultRatio, r.replacements, r.frameCount, r.thrashingDetected);
    }
    ConsoleUi::printSuccess("Memory paging analysis results persisted under Run ID: " + runId);
}

void AnalysisMenu::opt11_diskUsageAndIoAnalysis() {
    std::cout << "\nExecuting Disk Head Motion & I/O Scheduling Analysis...\n";
    std::vector<int32_t> requests = {98, 183, 37, 122, 14, 124, 65, 67};
    int32_t head = 53;

    auto fcfs = io::DiskScheduler::scheduleFcfs(head, requests);
    auto sstf = io::DiskScheduler::scheduleSstf(head, requests);
    auto scan = io::DiskScheduler::scheduleScan(head, requests, 199, "UP");
    auto cscan = io::DiskScheduler::scheduleCscan(head, requests, 199);

    std::cout << io::DiskScheduler::formatResultReport(fcfs);
    std::cout << io::DiskScheduler::formatResultReport(sstf);
    std::cout << io::DiskScheduler::formatResultReport(scan);
    std::cout << io::DiskScheduler::formatResultReport(cscan);

    std::string runId = latestRunId_.empty() ? core::IdGenerator::generateRunId() : latestRunId_;
    if (latestRunId_.empty()) {
        latestRunId_ = runId;
        runRepo_.createRun(runId, session_.getSessionId(), "IO_DISK_SCHEDULING", "Head 53, 8 cylinder requests");
    }

    auto saveIo = [&](const io::DiskScheduleResult& r) {
        std::string orderStr;
        for (size_t i = 0; i < r.serviceOrder.size(); ++i) {
            if (i > 0) orderStr += ",";
            orderStr += std::to_string(r.serviceOrder[i]);
        }
        runRepo_.recordIoResult(runId, r.algorithmName, r.totalHeadMovement, r.averageHeadMovement, orderStr);
    };
    saveIo(fcfs);
    saveIo(sstf);
    saveIo(scan);
    saveIo(cscan);
    ConsoleUi::printSuccess("Disk I/O scheduling results persisted under Run ID: " + runId);
}

void AnalysisMenu::opt12_processSynchronizationAndIpc() {
    std::cout << "\nRunning Simulated Process Synchronization & IPC Demonstration...\n";
    synchronization::SyncManager sync;
    sync.runProducerConsumerSimulation(5, 6);
    std::cout << sync.formatEventLog() << "\n";
}

void AnalysisMenu::opt13_permissionsAndProtection() {
    if (session_.getProcesses().empty()) {
        ConsoleUi::printWarning("No processes configured in this session.");
        return;
    }
    std::cout << "\nRunning Access Protection & Memory Isolation Simulation...\n";
    security::AccessController sec;

    sec.assignRole(session_.getProcesses()[0].getPid(), "root");

    core::TimeUnits t = 1;
    for (const auto& p : session_.getProcesses()) {
        sec.checkAccess(p.getPid(), "user_file", "READ", t++);
        sec.checkAccess(p.getPid(), "system_config", "WRITE", t++);
        // Memory boundary partition test: base=1000, limit=500
        sec.checkMemoryAccess(p.getPid(), 1200, 1000, 500, t++); // Valid
        sec.checkMemoryAccess(p.getPid(), 1600, 1000, 500, t++); // Segfault
    }

    std::cout << sec.formatAuditTable() << "\n";

    std::string runId = latestRunId_.empty() ? core::IdGenerator::generateRunId() : latestRunId_;
    if (latestRunId_.empty()) {
        latestRunId_ = runId;
        runRepo_.createRun(runId, session_.getSessionId(), "SECURITY_PROTECTION", "RBAC & Memory isolation audit");
    }
    for (const auto& ev : sec.getAuditLog()) {
        runRepo_.recordPermissionEvent(runId, "PID " + std::to_string(ev.pid), ev.resource, ev.operation, ev.granted ? "ALLOW" : "DENY", ev.explanation);
    }
    ConsoleUi::printSuccess("Security access events recorded under Run ID: " + runId);
}

void AnalysisMenu::opt14_runCompleteKernelAnalysis() {
    if (session_.getProcesses().empty()) {
        ConsoleUi::printWarning("No processes configured in this session. Please add processes first.");
        return;
    }
    std::cout << "\n============================================================\n";
    std::cout << "           RUNNING COMPLETE KERNEL ANALYSIS SUITE\n";
    std::cout << "============================================================\n";
    opt02_runAllSchedulers();
    opt08_runDeadlockAnalysis();
    opt10_virtualMemoryAnalysis();
    opt11_diskUsageAndIoAnalysis();
    opt13_permissionsAndProtection();

    if (!cachedMetrics_.empty()) {
        std::cout << reporting::JarvisExplainer::explainSummary(session_.getSessionId(), latestRunId_, cachedMetrics_[0]);
    }
}

void AnalysisMenu::opt15_comparePreviousRuns() {
    auto runs = runRepo_.getRunsForSession(session_.getSessionId());
    if (runs.empty()) {
        std::cout << "\n[RUNS] No previous simulation runs recorded for this session.\n";
        return;
    }

    std::cout << "\nSimulation Run History for Session " << session_.getSessionId() << ":\n";
    std::cout << "+--------------------+-----------------------+-----------+------------------------------------------+\n";
    std::cout << "| Run ID             | Timestamp             | Status    | Module Parameters                        |\n";
    std::cout << "+--------------------+-----------------------+-----------+------------------------------------------+\n";
    for (const auto& r : runs) {
        std::cout << "| " << std::setw(18) << r.runId
                  << " | " << std::setw(21) << r.createdAt
                  << " | " << std::setw(9) << r.status
                  << " | " << std::left << std::setw(40) << r.parameters.substr(0, 40) << std::right << " |\n";
    }
    std::cout << "+--------------------+-----------------------+-----------+------------------------------------------+\n";
}

void AnalysisMenu::opt16_viewSessionLogs() {
    std::cout << "\nFetching session audit event logs from database...\n";
    auto events = runRepo_.getEventsForRun(latestRunId_);
    if (!events.empty()) {
        std::cout << "+------+-----+--------------------+------------------------------------------+\n";
        std::cout << "| Time | PID | Event Type         | Details                                  |\n";
        std::cout << "+------+-----+--------------------+------------------------------------------+\n";
        for (const auto& e : events) {
            std::cout << "| " << std::setw(4) << e.timestamp
                      << " | " << std::setw(3) << e.entityId
                      << " | " << std::left << std::setw(18) << e.eventType << std::right
                      << " | " << std::left << std::setw(40) << e.details.substr(0, 40) << std::right << " |\n";
        }
        std::cout << "+------+-----+--------------------+------------------------------------------+\n";
    }

    auto sessionLogs = runRepo_.getEventLogsForSession(session_.getSessionId());
    if (!sessionLogs.empty()) {
        std::cout << "\nOperational Event Logs for Session " << session_.getSessionId() << ":\n";
        std::cout << "+----+-------+--------------------+------------------------------------------+\n";
        std::cout << "| ID | Level | Source             | Message                                  |\n";
        std::cout << "+----+-------+--------------------+------------------------------------------+\n";
        for (const auto& l : sessionLogs) {
            std::cout << "| " << std::setw(2) << l.id
                      << " | " << std::left << std::setw(5) << l.logLevel << std::right
                      << " | " << std::left << std::setw(18) << l.source << std::right
                      << " | " << std::left << std::setw(40) << l.message.substr(0, 40) << std::right << " |\n";
        }
        std::cout << "+----+-------+--------------------+------------------------------------------+\n";
    } else if (events.empty()) {
        std::cout << "No event records found for this session or latest run.\n";
    }
}

void AnalysisMenu::opt17_generateFinalReport() {
    if (session_.getProcesses().empty()) {
        ConsoleUi::printWarning("No processes configured in this session to generate report.");
        return;
    }
    ensureSchedulersRun();
    std::cout << "\nGenerating integrated final report...\n";

    memory::MemoryManager memMgr(16, 4);
    deadlock::ResourceManager rm;
    rm.loadFromWorkload(session_.getProcesses());
    std::vector<core::ProcessId> pids;
    for (const auto& p : session_.getProcesses()) pids.push_back(p.getPid());

    reporting::CompleteReportData reportData;
    reportData.sessionId = session_.getSessionId();
    reportData.runId = latestRunId_.empty() ? core::IdGenerator::generateRunId() : latestRunId_;
    reportData.generatedAt = core::IdGenerator::getCurrentTimestampIso();
    reportData.processes = session_.getProcesses();
    reportData.schedulingMetrics = cachedMetrics_;
    reportData.memoryResults = memMgr.runComparativeAnalysis(session_.getProcesses(), 3);
    reportData.bankersResult = deadlock::BankersAlgorithm::evaluateSafeState(rm, pids);
    reportData.deadlockResult = deadlock::DeadlockDetector::detect(rm, pids);
    reportData.diskResult = io::DiskScheduler::scheduleSstf(53, {98, 183, 37, 122, 14, 124, 65, 67});

    std::string mdContent = reporting::ReportGenerator::generateMarkdownReport(reportData);
    std::string csvContent = reporting::ReportGenerator::generateCsvReport(reportData);

    std::string mdPath = "reports/report_" + reportData.sessionId + ".md";
    std::string csvPath = "reports/report_" + reportData.sessionId + ".csv";

    reporting::ReportGenerator::exportToFile(mdPath, mdContent);
    reporting::ReportGenerator::exportToFile(csvPath, csvContent);

    runRepo_.saveReportRecord("RPT-" + reportData.sessionId, reportData.sessionId, reportData.runId, "MARKDOWN", mdPath);
    runRepo_.saveReportRecord("RPT-CSV-" + reportData.sessionId, reportData.sessionId, reportData.runId, "CSV", csvPath);

    ConsoleUi::printSuccess("Markdown report exported to: " + mdPath);
    ConsoleUi::printSuccess("CSV report exported to: " + csvPath);
}

} // namespace cli
} // namespace watchos
