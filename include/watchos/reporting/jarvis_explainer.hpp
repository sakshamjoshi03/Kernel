#ifndef WATCHOS_REPORTING_JARVIS_EXPLAINER_HPP
#define WATCHOS_REPORTING_JARVIS_EXPLAINER_HPP

#include "watchos/scheduler/scheduling_metrics.hpp"
#include "watchos/database/run_repository.hpp"
#include <string>
#include <vector>

namespace watchos {
namespace reporting {

class JarvisExplainer {
public:
    static std::string explainWhyDelayed(
        const std::string& runId,
        core::ProcessId pid,
        const scheduler::AggregateMetrics& metrics,
        const std::vector<database::EventRecord>& events
    );

    static std::string explainAlgorithmComparison(
        const std::vector<scheduler::AggregateMetrics>& allResults
    );

    static std::string explainSummary(
        const std::string& sessionId,
        const std::string& runId,
        const scheduler::AggregateMetrics& bestMetrics
    );
};

} // namespace reporting
} // namespace watchos

#endif // WATCHOS_REPORTING_JARVIS_EXPLAINER_HPP
