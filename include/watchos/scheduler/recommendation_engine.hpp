#ifndef WATCHOS_SCHEDULER_RECOMMENDATION_ENGINE_HPP
#define WATCHOS_SCHEDULER_RECOMMENDATION_ENGINE_HPP

#include "watchos/scheduler/scheduling_metrics.hpp"
#include <vector>
#include <string>

namespace watchos {
namespace scheduler {

struct ObjectiveWeights {
    double weightWaiting = 0.30;
    double weightTurnaround = 0.25;
    double weightResponse = 0.20;
    double weightContextSwitches = 0.15;
    double weightStarvation = 0.10;
};

struct AlgorithmRank {
    std::string algorithmName;
    double score = 0.0;
    int rank = 1;
    AggregateMetrics metrics;
    std::string explanation;
};

class RecommendationEngine {
public:
    static std::vector<AlgorithmRank> evaluateAndRank(
        const std::vector<AggregateMetrics>& results,
        const ObjectiveWeights& weights = ObjectiveWeights{}
    );

    static std::string formatComparisonTable(const std::vector<AlgorithmRank>& ranking);
};

} // namespace scheduler
} // namespace watchos

#endif // WATCHOS_SCHEDULER_RECOMMENDATION_ENGINE_HPP
