#include "watchos/scheduler/recommendation_engine.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

namespace watchos {
namespace scheduler {

namespace {
    double normalize(double val, double minVal, double maxVal) {
        if (std::abs(maxVal - minVal) < 1e-9) {
            return 0.0;
        }
        return (val - minVal) / (maxVal - minVal);
    }
}

std::vector<AlgorithmRank> RecommendationEngine::evaluateAndRank(
    const std::vector<AggregateMetrics>& results,
    const ObjectiveWeights& weights
) {
    std::vector<AlgorithmRank> ranks;
    if (results.empty()) return ranks;

    double minWt = results[0].avgWaitingTime, maxWt = results[0].avgWaitingTime;
    double minTat = results[0].avgTurnaroundTime, maxTat = results[0].avgTurnaroundTime;
    double minRt = results[0].avgResponseTime, maxRt = results[0].avgResponseTime;
    double minCs = results[0].totalContextSwitches, maxCs = results[0].totalContextSwitches;
    double minStarv = results[0].starvationBreaches, maxStarv = results[0].starvationBreaches;

    for (const auto& r : results) {
        minWt = std::min(minWt, r.avgWaitingTime);
        maxWt = std::max(maxWt, r.avgWaitingTime);
        minTat = std::min(minTat, r.avgTurnaroundTime);
        maxTat = std::max(maxTat, r.avgTurnaroundTime);
        minRt = std::min(minRt, r.avgResponseTime);
        maxRt = std::max(maxRt, r.avgResponseTime);
        minCs = std::min(minCs, static_cast<double>(r.totalContextSwitches));
        maxCs = std::max(maxCs, static_cast<double>(r.totalContextSwitches));
        minStarv = std::min(minStarv, static_cast<double>(r.starvationBreaches));
        maxStarv = std::max(maxStarv, static_cast<double>(r.starvationBreaches));
    }

    for (const auto& r : results) {
        double nWt = normalize(r.avgWaitingTime, minWt, maxWt);
        double nTat = normalize(r.avgTurnaroundTime, minTat, maxTat);
        double nRt = normalize(r.avgResponseTime, minRt, maxRt);
        double nCs = normalize(r.totalContextSwitches, minCs, maxCs);
        double nStarv = normalize(r.starvationBreaches, minStarv, maxStarv);

        double compositeScore = (weights.weightWaiting * nWt) +
                                (weights.weightTurnaround * nTat) +
                                (weights.weightResponse * nRt) +
                                (weights.weightContextSwitches * nCs) +
                                (weights.weightStarvation * nStarv);

        AlgorithmRank ar;
        ar.algorithmName = r.algorithmName;
        ar.score = compositeScore;
        ar.metrics = r;

        std::ostringstream expl;
        expl << "Normalized scores [WT: " << std::fixed << std::setprecision(2) << nWt
             << ", TAT: " << nTat
             << ", RT: " << nRt
             << ", CS: " << nCs
             << ", Starv: " << nStarv << "]";
        ar.explanation = expl.str();

        ranks.push_back(ar);
    }

    // Sort by composite score (lowest score is best)
    std::sort(ranks.begin(), ranks.end(), [](const AlgorithmRank& a, const AlgorithmRank& b) {
        return a.score < b.score;
    });

    for (size_t i = 0; i < ranks.size(); ++i) {
        ranks[i].rank = static_cast<int>(i + 1);
    }

    return ranks;
}

std::string RecommendationEngine::formatComparisonTable(const std::vector<AlgorithmRank>& ranking) {
    if (ranking.empty()) return "No algorithms evaluated.\n";

    std::ostringstream oss;
    oss << "+------+-----------------------------+---------+---------+---------+-------+-------+---------+\n";
    oss << "| Rank | Algorithm                   | Avg WT  | Avg TAT | Avg RT  | CS    | Starv | Score   |\n";
    oss << "+------+-----------------------------+---------+---------+---------+-------+-------+---------+\n";

    for (const auto& r : ranking) {
        oss << "| " << std::setw(4) << r.rank
            << " | " << std::left << std::setw(27) << r.algorithmName << std::right
            << " | " << std::setw(7) << std::fixed << std::setprecision(2) << r.metrics.avgWaitingTime
            << " | " << std::setw(7) << r.metrics.avgTurnaroundTime
            << " | " << std::setw(7) << r.metrics.avgResponseTime
            << " | " << std::setw(5) << r.metrics.totalContextSwitches
            << " | " << std::setw(5) << r.metrics.starvationBreaches
            << " | " << std::setw(7) << r.score << " |\n";
    }
    oss << "+------+-----------------------------+---------+---------+---------+-------+-------+---------+\n";

    oss << "\nRecommended Algorithm for this Workload: " << ranking.front().algorithmName << "\n";
    oss << "Scoring Formula: Score = 0.30*WT_norm + 0.25*TAT_norm + 0.20*RT_norm + 0.15*CS_norm + 0.10*Starv_norm (lower is better)\n";
    oss << "Note: Recommendations depend on workload objectives and trade-offs.\n";

    return oss.str();
}

} // namespace scheduler
} // namespace watchos
