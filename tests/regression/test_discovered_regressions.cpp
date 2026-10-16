#include "watchos/core/id_generator.hpp"
#include "watchos/scheduler/fcfs.hpp"
#include "watchos/scheduler/sjf.hpp"
#include "watchos/scheduler/srtf.hpp"
#include "watchos/scheduler/priority_scheduler.hpp"
#include "watchos/scheduler/round_robin.hpp"
#include "watchos/database/db_connection.hpp"
#include "watchos/database/schema.hpp"
#include "watchos/database/session_repository.hpp"
#include "watchos/database/run_repository.hpp"
#include <cassert>
#include <iostream>
#include <set>
#include <vector>

using namespace watchos;

void testRegressionSessionIdCollision() {
    std::cout << "[REGRESSION 1] High-Frequency Session ID Uniqueness...\n";
    std::set<std::string> ids;
    for (int i = 0; i < 1000; ++i) {
        std::string id = core::IdGenerator::generateSessionId();
        assert(ids.find(id) == ids.end()); // Must never collide
        ids.insert(id);
    }
    assert(ids.size() == 1000);
    std::cout << "  -> 1,000 unique Session IDs generated without collision.\n";
}

void testRegressionProcessResultPidOrdering() {
    std::cout << "[REGRESSION 2] Schedulers Output Sorted by PID...\n";
    std::vector<process::Process> procs = {
        process::Process(50, 0, 8, 1),
        process::Process(10, 0, 2, 1),
        process::Process(30, 0, 4, 1)
    };
    core::SystemSettings settings;

    auto checkOrder = [&](const scheduler::AggregateMetrics& m) {
        assert(m.processResults.size() == 3);
        assert(m.processResults[0].pid == 10);
        assert(m.processResults[1].pid == 30);
        assert(m.processResults[2].pid == 50);
    };

    scheduler::FcfsScheduler fcfs;
    checkOrder(fcfs.schedule(procs, settings));

    scheduler::SjfScheduler sjf;
    checkOrder(sjf.schedule(procs, settings));

    scheduler::SrtfScheduler srtf;
    checkOrder(srtf.schedule(procs, settings));

    scheduler::PriorityScheduler pri(false);
    checkOrder(pri.schedule(procs, settings));

    scheduler::RoundRobinScheduler rr(2);
    checkOrder(rr.schedule(procs, settings));

    std::cout << "  -> All schedulers return results deterministically sorted by PID.\n";
}

void testRegressionMultiVersionWorkloadIsolation() {
    std::cout << "[REGRESSION 3] Configuration Versions Preserve Older Run History...\n";
    database::DbConnection db(":memory:");
    db.open();
    database::Schema::initializeSchema(db);

    database::SessionRepository sessionRepo(db);
    database::RunRepository runRepo(db);

    std::string sessId = "WOS-REG-VER";
    session::Session sess(sessId, "Version Isolation");
    sess.addProcess(process::Process(1, 0, 5, 1));
    sessionRepo.createSession(sess);
    sessionRepo.saveWorkload(sess, true);

    // Run 1 under version 1
    runRepo.createRun("RUN-V1", sessId, "CPU", "v1");
    runRepo.recordSchedulingResult({"RUN-V1", 1, 0, 5, 1, 5, 5, 0, 0});
    runRepo.updateRunStatus("RUN-V1", "COMPLETED");

    // Add process for version 2
    sess.addProcess(process::Process(2, 0, 3, 2));
    sess.setActiveVersion(2);
    sessionRepo.saveWorkload(sess, true);

    // Run 2 under version 2
    runRepo.createRun("RUN-V2", sessId, "CPU", "v2");
    runRepo.recordSchedulingResult({"RUN-V2", 1, 0, 5, 1, 8, 8, 3, 3});
    runRepo.recordSchedulingResult({"RUN-V2", 2, 0, 3, 2, 3, 3, 0, 0});
    runRepo.updateRunStatus("RUN-V2", "COMPLETED");

    // Check version 1 results are preserved
    auto r1Results = runRepo.getSchedulingResultsForRun("RUN-V1");
    assert(r1Results.size() == 1);
    assert(r1Results[0].completionTime == 5);

    // Check version 2 results
    auto r2Results = runRepo.getSchedulingResultsForRun("RUN-V2");
    assert(r2Results.size() == 2);

    std::cout << "  -> Multi-version workload isolation verified. Historical runs remain intact.\n";
}

int main() {
    std::cout << "============================================================\n";
    std::cout << "       WATCH OS — REGRESSION & DEFECT AUDIT SUITE\n";
    std::cout << "============================================================\n";

    testRegressionSessionIdCollision();
    testRegressionProcessResultPidOrdering();
    testRegressionMultiVersionWorkloadIsolation();

    std::cout << "All regression test suites PASSED successfully!\n";
    return 0;
}
