#include "watchos/reporting/report_generator.hpp"
#include "watchos/scheduler/gantt_chart.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>

namespace watchos {
namespace reporting {

std::string ReportGenerator::generateMarkdownReport(const CompleteReportData& data) {
    std::ostringstream oss;
    oss << "# WATCH OS — SIMULATION REPORT\n\n";
    oss << "**Session ID:** `" << data.sessionId << "`  \n";
    oss << "**Run ID:** `" << data.runId << "`  \n";
    oss << "**Generated At:** " << data.generatedAt << "  \n";
    oss << "**Simulation Status:** COMPLETED  \n\n";
    oss << "---\n\n";

    oss << "## 1. Executive Summary\n";
    oss << "A full-system operating simulator analysis was conducted on workload of "
        << data.processes.size() << " configured processes. Subsystems evaluated include CPU Scheduling, Virtual Memory, Deadlock/Resource Allocation, Disk Arm Motion, and Access Protection.\n\n";

    oss << "## 2. Workload Configuration\n";
    oss << "| PID | Arrival Time | CPU Burst | Priority | Memory Demand |\n";
    oss << "| :--- | :--- | :--- | :--- | :--- |\n";
    for (const auto& p : data.processes) {
        std::string memStr = p.getMemoryConfig().isConfigured ? (std::to_string(p.getMemoryConfig().demandUnits) + " units") : "Default (32 units)";
        oss << "| " << p.getPid() << " | " << p.getArrivalTime() << " | " << p.getCpuBurst() << " | " << p.getPriority() << " | " << memStr << " |\n";
    }
    oss << "\n";

    oss << "## 3. CPU Scheduling Results\n";
    oss << "| Algorithm | Avg Waiting Time | Avg Turnaround Time | Avg Response Time | Context Switches | CPU Utilization |\n";
    oss << "| :--- | :--- | :--- | :--- | :--- | :--- |\n";
    for (const auto& m : data.schedulingMetrics) {
        oss << "| " << m.algorithmName << " | "
            << std::fixed << std::setprecision(2) << m.avgWaitingTime << " | "
            << m.avgTurnaroundTime << " | "
            << m.avgResponseTime << " | "
            << m.totalContextSwitches << " | "
            << m.cpuUtilization << "% |\n";
    }
    oss << "\n";

    oss << "## 4. Gantt Charts\n```text\n";
    for (const auto& m : data.schedulingMetrics) {
        oss << scheduler::GanttChart::render(m.algorithmName, m.ganttSegments);
    }
    oss << "```\n\n";

    oss << "## 5. Memory Management & Page Replacement\n";
    oss << "| Algorithm | Total References | Page Faults | Page Hits | Fault Ratio | Thrashing Risk |\n";
    oss << "| :--- | :--- | :--- | :--- | :--- | :--- |\n";
    for (const auto& mem : data.memoryResults) {
        oss << "| " << mem.algorithmName << " | "
            << mem.totalReferences << " | "
            << mem.pageFaults << " | "
            << mem.pageHits << " | "
            << std::fixed << std::setprecision(2) << (mem.faultRatio * 100.0) << "% | "
            << (mem.thrashingDetected ? "HIGH" : "NORMAL") << " |\n";
    }
    oss << "\n";

    oss << "## 6. Deadlock & Banker's Algorithm Safe State\n";
    oss << "- **Banker's Algorithm State:** " << (data.bankersResult.isSafe ? "SAFE" : "UNSAFE") << "\n";
    oss << "- **Details:** " << data.bankersResult.explanation << "\n";
    oss << "- **Deadlock Detection State:** " << (data.deadlockResult.isDeadlocked ? "DEADLOCK PRESENT" : "DEADLOCK FREE") << "\n\n";

    oss << "## 7. Disk Head Scheduling\n";
    oss << "- **Algorithm:** " << data.diskResult.algorithmName << "\n";
    oss << "- **Initial Head Position:** " << data.diskResult.initialHead << "\n";
    oss << "- **Total Head Movement:** " << data.diskResult.totalHeadMovement << " cylinders\n";
    oss << "- **Average Movement:** " << std::fixed << std::setprecision(2) << data.diskResult.averageHeadMovement << " cylinders/request\n\n";

    oss << "## 8. Key Findings & Trade-Offs\n";
    oss << "1. Shortest remaining time / SJF algorithms optimize average waiting times at the cost of preemption overhead.\n";
    oss << "2. Round Robin provides bounded response time ideal for interactive systems.\n";
    oss << "3. Optimal page replacement establishes the theoretical minimum fault ceiling for comparison against FIFO and LRU.\n";
    oss << "4. Banker's state checks prevent resource allocation deadlocks.\n\n";

    oss << "## 9. Limitations & Assumptions\n";
    oss << "- Simulates user-space OS algorithms under controlled workloads.\n";
    oss << "- Integer simulated clock time units represent idealized execution cycles.\n";
    oss << "- Access control rules and memory protection boundaries are educational and separate from host OS security.\n";

    return oss.str();
}

std::string ReportGenerator::generateCsvReport(const CompleteReportData& data) {
    std::ostringstream oss;
    oss << "# Session ID: " << data.sessionId << "\n";
    oss << "# Run ID: " << data.runId << "\n";
    oss << "# Generated At: " << data.generatedAt << "\n";
    oss << "Section,Algorithm,Metric,Value\n";
    for (const auto& m : data.schedulingMetrics) {
        oss << "CPU," << m.algorithmName << ",AvgWaitingTime," << m.avgWaitingTime << "\n";
        oss << "CPU," << m.algorithmName << ",AvgTurnaroundTime," << m.avgTurnaroundTime << "\n";
        oss << "CPU," << m.algorithmName << ",AvgResponseTime," << m.avgResponseTime << "\n";
        oss << "CPU," << m.algorithmName << ",ContextSwitches," << m.totalContextSwitches << "\n";
        oss << "CPU," << m.algorithmName << ",CpuUtilization," << m.cpuUtilization << "\n";
    }
    for (const auto& mem : data.memoryResults) {
        oss << "Memory," << mem.algorithmName << ",PageFaults," << mem.pageFaults << "\n";
        oss << "Memory," << mem.algorithmName << ",FaultRatio," << mem.faultRatio << "\n";
    }
    oss << "Disk," << data.diskResult.algorithmName << ",TotalHeadMovement," << data.diskResult.totalHeadMovement << "\n";
    return oss.str();
}

bool ReportGenerator::exportToFile(const std::string& filePath, const std::string& content) {
    std::ofstream out(filePath);
    if (!out.is_open()) return false;
    out << content;
    return true;
}

} // namespace reporting
} // namespace watchos
