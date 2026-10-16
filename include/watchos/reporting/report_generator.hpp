#ifndef WATCHOS_REPORTING_REPORT_GENERATOR_HPP
#define WATCHOS_REPORTING_REPORT_GENERATOR_HPP

#include "watchos/session/session.hpp"
#include "watchos/scheduler/scheduling_metrics.hpp"
#include "watchos/memory/page_replacement.hpp"
#include "watchos/deadlock/bankers_algorithm.hpp"
#include "watchos/deadlock/deadlock_detector.hpp"
#include "watchos/io/disk_scheduler.hpp"
#include <string>
#include <vector>

namespace watchos {
namespace reporting {

struct CompleteReportData {
    std::string sessionId;
    std::string runId;
    std::string generatedAt;
    std::vector<process::Process> processes;
    std::vector<scheduler::AggregateMetrics> schedulingMetrics;
    std::vector<memory::PageSimulationResult> memoryResults;
    deadlock::BankersResult bankersResult;
    deadlock::DeadlockDetectionResult deadlockResult;
    io::DiskScheduleResult diskResult;
    std::vector<std::string> auditLogs;
};

class ReportGenerator {
public:
    static std::string generateMarkdownReport(const CompleteReportData& data);
    static std::string generateCsvReport(const CompleteReportData& data);

    static bool exportToFile(const std::string& filePath, const std::string& content);
};

} // namespace reporting
} // namespace watchos

#endif // WATCHOS_REPORTING_REPORT_GENERATOR_HPP
