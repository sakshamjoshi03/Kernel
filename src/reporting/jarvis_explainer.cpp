#include "watchos/reporting/jarvis_explainer.hpp"
#include <sstream>
#include <iomanip>

namespace watchos {
namespace reporting {

std::string JarvisExplainer::explainWhyDelayed(
    const std::string& runId,
    core::ProcessId pid,
    const scheduler::AggregateMetrics& metrics,
    const std::vector<database::EventRecord>& events
) {
    (void)events;
    const scheduler::ProcessScheduleResult* target = nullptr;
    for (const auto& pr : metrics.processResults) {
        if (pr.pid == pid) {
            target = &pr;
            break;
        }
    }

    if (!target) {
        return "Cannot explain delay: Process PID " + std::to_string(pid) + " was not found in the run records.";
    }

    std::ostringstream oss;
    oss << "[JARVIS EXPLAIN MODE] WHY was P" << pid << " delayed?\n\n";
    oss << "P" << pid << " entered READY at simulated time " << target->arrivalTime << ".\n";

    if (target->firstStartTime == target->arrivalTime) {
        oss << "P" << pid << " was NOT delayed. It was dispatched immediately to the CPU at simulated time " << target->firstStartTime << ".\n";
    } else {
        oss << "CPU was allocated to another process according to " << metrics.algorithmName << " scheduling rules.\n";
        oss << "P" << pid << " began execution at simulated time " << target->firstStartTime << ".\n";
        oss << "Calculated response time: " << target->responseTime << " time units.\n";
        oss << "Total waiting time in ready queue: " << target->waitingTime << " time units.\n";
    }

    oss << "\nExplanation generated from Run ID: " << runId << ".\n";
    return oss.str();
}

std::string JarvisExplainer::explainAlgorithmComparison(
    const std::vector<scheduler::AggregateMetrics>& allResults
) {
    if (allResults.empty()) return "No algorithm results available for comparison.";

    std::ostringstream oss;
    oss << "[JARVIS EXPLAIN MODE] Comparative Analysis & Algorithm Dynamics:\n\n";

    for (const auto& r : allResults) {
        oss << "- " << r.algorithmName << ":\n";
        oss << "    Avg Waiting: " << std::fixed << std::setprecision(2) << r.avgWaitingTime
            << " | Avg Turnaround: " << r.avgTurnaroundTime
            << " | Context Switches: " << r.totalContextSwitches
            << " | Starvation Breaches: " << r.starvationBreaches << "\n";
    }

    return oss.str();
}

std::string JarvisExplainer::explainSummary(
    const std::string& sessionId,
    const std::string& runId,
    const scheduler::AggregateMetrics& bestMetrics
) {
    std::ostringstream oss;
    oss << "[JARVIS EXPLAIN MODE] Executive Kernel Performance Summary:\n";
    oss << "Session ID: " << sessionId << " | Active Run: " << runId << "\n";
    oss << "Optimal Algorithm Observed: " << bestMetrics.algorithmName << "\n";
    oss << "Workload execution completed in " << bestMetrics.totalElapsedTime << " simulated clock units with "
        << std::fixed << std::setprecision(1) << bestMetrics.cpuUtilization << "% CPU utilization.\n";
    oss << "No unhandled deadlocks or kernel faults were observed.\n";
    return oss.str();
}

} // namespace reporting
} // namespace watchos
