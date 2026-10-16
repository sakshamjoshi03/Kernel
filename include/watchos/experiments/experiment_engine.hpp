#ifndef WATCHOS_EXPERIMENTS_EXPERIMENT_ENGINE_HPP
#define WATCHOS_EXPERIMENTS_EXPERIMENT_ENGINE_HPP

#include "watchos/process/process.hpp"
#include "watchos/core/types.hpp"
#include "watchos/scheduler/scheduling_metrics.hpp"
#include <string>
#include <vector>

namespace watchos {
namespace experiments {

struct WhatIfParameter {
    std::string parameterName;
    std::string baseValue;
    std::string variantValue;
};

struct WhatIfExperimentResult {
    std::string experimentId;
    std::string baseRunId;
    std::string variantRunId;
    WhatIfParameter parameter;
    scheduler::AggregateMetrics baseMetrics;
    scheduler::AggregateMetrics variantMetrics;
    std::string comparativeAnalysis;
};

class ExperimentEngine {
public:
    static WhatIfExperimentResult runQuantumExperiment(
        const std::vector<process::Process>& workload,
        core::TimeUnits baseQuantum,
        core::TimeUnits variantQuantum,
        const core::SystemSettings& settings
    );

    static WhatIfExperimentResult runOverheadExperiment(
        const std::vector<process::Process>& workload,
        core::TimeUnits baseOverhead,
        core::TimeUnits variantOverhead,
        const core::SystemSettings& settings
    );

    static std::string formatComparison(const WhatIfExperimentResult& result);
};

} // namespace experiments
} // namespace watchos

#endif // WATCHOS_EXPERIMENTS_EXPERIMENT_ENGINE_HPP
