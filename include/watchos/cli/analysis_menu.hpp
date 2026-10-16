#ifndef WATCHOS_CLI_ANALYSIS_MENU_HPP
#define WATCHOS_CLI_ANALYSIS_MENU_HPP

#include "watchos/session/session.hpp"
#include "watchos/database/db_connection.hpp"
#include "watchos/database/run_repository.hpp"
#include "watchos/scheduler/scheduling_metrics.hpp"
#include <memory>
#include <string>
#include <vector>

namespace watchos {
namespace cli {

class AnalysisMenu {
public:
    AnalysisMenu(session::Session& session, database::DbConnection& db);

    void runMenuLoop();

private:
    session::Session& session_;
    database::DbConnection& db_;
    database::RunRepository runRepo_;
    std::vector<scheduler::AggregateMetrics> cachedMetrics_;
    std::string latestRunId_;

    void printAnalysisMenu();
    void executeOption(int option);
    void executeCommand(const std::string& cmd);

    void opt01_findBestScheduler();
    void opt02_runAllSchedulers();
    void opt03_displayGanttCharts();
    void opt04_showProcessLifecycle();
    void opt05_analyzeContextSwitching();
    void opt06_detectStarvation();
    void opt07_applyAgingAndCompare();
    void opt08_runDeadlockAnalysis();
    void opt09_estimateDeadlockFrequency();
    void opt10_virtualMemoryAnalysis();
    void opt11_diskUsageAndIoAnalysis();
    void opt12_processSynchronizationAndIpc();
    void opt13_permissionsAndProtection();
    void opt14_runCompleteKernelAnalysis();
    void opt15_comparePreviousRuns();
    void opt16_viewSessionLogs();
    void opt17_generateFinalReport();

    void ensureSchedulersRun();
};

} // namespace cli
} // namespace watchos

#endif // WATCHOS_CLI_ANALYSIS_MENU_HPP
