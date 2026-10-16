#include "watchos/experiments/experiment_engine.hpp"
#include "watchos/scheduler/round_robin.hpp"
#include "watchos/core/id_generator.hpp"
#include <sstream>
#include <iomanip>

namespace watchos {
namespace experiments {

WhatIfExperimentResult ExperimentEngine::runQuantumExperiment(
    const std::vector<process::Process>& workload,
    core::TimeUnits baseQuantum,
    core::TimeUnits variantQuantum,
    const core::SystemSettings& settings
) {
    WhatIfExperimentResult res;
    res.experimentId = "EXP-RR-" + std::to_string(baseQuantum) + "-VS-" + std::to_string(variantQuantum);
    res.baseRunId = core::IdGenerator::generateRunId();
    res.variantRunId = core::IdGenerator::generateRunId();
    res.parameter = {"Round Robin Quantum", std::to_string(baseQuantum), std::to_string(variantQuantum)};

    scheduler::RoundRobinScheduler baseSched(baseQuantum);
    res.baseMetrics = baseSched.schedule(workload, settings);

    scheduler::RoundRobinScheduler varSched(variantQuantum);
    res.variantMetrics = varSched.schedule(workload, settings);

    std::ostringstream oss;
    oss << "What-If Analysis: Varying Time Quantum from " << baseQuantum << " to " << variantQuantum << ":\n";
    double wtDelta = res.variantMetrics.avgWaitingTime - res.baseMetrics.avgWaitingTime;
    int csDelta = res.variantMetrics.totalContextSwitches - res.baseMetrics.totalContextSwitches;

    oss << "- Avg Waiting Time changed by " << (wtDelta >= 0 ? "+" : "") << std::fixed << std::setprecision(2) << wtDelta << " time units.\n";
    oss << "- Total Context Switches changed by " << (csDelta >= 0 ? "+" : "") << csDelta << " switches.\n";
    if (csDelta < 0) {
        oss << "- Larger quantum reduced preemptive overhead and context switches.\n";
    } else {
        oss << "- Smaller quantum enhanced responsiveness but increased switching overhead.\n";
    }

    res.comparativeAnalysis = oss.str();
    return res;
}

WhatIfExperimentResult ExperimentEngine::runOverheadExperiment(
    const std::vector<process::Process>& workload,
    core::TimeUnits baseOverhead,
    core::TimeUnits variantOverhead,
    const core::SystemSettings& settings
) {
    WhatIfExperimentResult res;
    res.experimentId = "EXP-CS-OVERHEAD";
    res.baseRunId = core::IdGenerator::generateRunId();
    res.variantRunId = core::IdGenerator::generateRunId();
    res.parameter = {"Context Switch Overhead", std::to_string(baseOverhead), std::to_string(variantOverhead)};

    core::SystemSettings setBase = settings;
    setBase.contextSwitchOverhead = baseOverhead;
    scheduler::RoundRobinScheduler sched(settings.defaultTimeQuantum);
    res.baseMetrics = sched.schedule(workload, setBase);

    core::SystemSettings setVar = settings;
    setVar.contextSwitchOverhead = variantOverhead;
    res.variantMetrics = sched.schedule(workload, setVar);

    std::ostringstream oss;
    oss << "What-If Analysis: Impact of Context Switch Overhead (" << baseOverhead << " vs " << variantOverhead << "):\n";
    oss << "- Overhead adds directly to total turnaround time and CPU penalty per switch.\n";
    res.comparativeAnalysis = oss.str();

    return res;
}

std::string ExperimentEngine::formatComparison(const WhatIfExperimentResult& result) {
    std::ostringstream oss;
    oss << "What-If Experiment [" << result.parameter.parameterName << "]:\n";
    oss << "+--------------------------------+--------------------+--------------------+\n";
    oss << "| Parameter / Metric             | Base (" << std::setw(10) << result.parameter.baseValue << ") | Variant (" << std::setw(7) << result.parameter.variantValue << ") |\n";
    oss << "+--------------------------------+--------------------+--------------------+\n";
    oss << "| Avg Waiting Time               | " << std::setw(18) << std::fixed << std::setprecision(2) << result.baseMetrics.avgWaitingTime
        << " | " << std::setw(18) << result.variantMetrics.avgWaitingTime << " |\n";
    oss << "| Avg Turnaround Time            | " << std::setw(18) << result.baseMetrics.avgTurnaroundTime
        << " | " << std::setw(18) << result.variantMetrics.avgTurnaroundTime << " |\n";
    oss << "| Avg Response Time              | " << std::setw(18) << result.baseMetrics.avgResponseTime
        << " | " << std::setw(18) << result.variantMetrics.avgResponseTime << " |\n";
    oss << "| Total Context Switches         | " << std::setw(18) << result.baseMetrics.totalContextSwitches
        << " | " << std::setw(18) << result.variantMetrics.totalContextSwitches << " |\n";
    oss << "+--------------------------------+--------------------+--------------------+\n";
    oss << "\n" << result.comparativeAnalysis << "\n";
    return oss.str();
}

} // namespace experiments
} // namespace watchos
