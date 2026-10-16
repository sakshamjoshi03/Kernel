#include "watchos/session/session_manager.hpp"
#include "watchos/database/schema.hpp"
#include "tests/fixtures/test_workloads.hpp"
#include <iostream>
#include <cassert>
#include <cstdio>

int main() {
    std::cout << "============================================================\n";
    std::cout << "[INTEGRATION TEST] Session Lifecycle (Create -> Save -> Restart -> Resume)\n";
    std::cout << "============================================================\n";

    const std::string dbFile = "test_lifecycle.db";
    std::remove(dbFile.c_str());

    std::string savedSessionId;

    // --- PHASE 1: Create session, configure processes, finalize, and exit ---
    {
        std::cout << "[PHASE 1] Initializing application session 1...\n";
        watchos::database::DbConnection db(dbFile);
        db.open();
        watchos::database::Schema::initializeSchema(db);

        watchos::session::SessionManager mgr(db);
        auto session = mgr.createNewSession("Integration test session");
        assert(session != nullptr);
        savedSessionId = session->getSessionId();
        std::cout << "  Created Session ID: " << savedSessionId << "\n";

        // Configure with golden workload
        session->setProcesses(watchos::test::getGoldenWorkload());

        std::string err;
        bool ok = mgr.saveActiveSession(true, &err);
        assert(ok);
        assert(err.empty());
        std::cout << "  Successfully saved and finalized workload.\n";

        mgr.closeActiveSession();
        db.close();
        std::cout << "  Application shutdown simulated (database connection closed).\n";
    }

    // --- PHASE 2: Application Restart - Open DB and Resume Session ---
    {
        std::cout << "\n[PHASE 2] Application relaunching (new process / connection)...\n";
        watchos::database::DbConnection db(dbFile);
        db.open();
        watchos::database::Schema::initializeSchema(db);

        watchos::session::SessionManager mgr(db);
        assert(mgr.sessionExists(savedSessionId));

        auto resumed = mgr.resumeSession(savedSessionId);
        assert(resumed != nullptr);
        assert(resumed->getSessionId() == savedSessionId);
        assert(resumed->getStatus() == watchos::core::SessionStatus::FINALIZED);
        assert(resumed->getDescription() == "Integration test session");

        const auto& procs = resumed->getProcesses();
        assert(procs.size() == 3);

        // Verify PID 1
        assert(procs[0].getPid() == 1);
        assert(procs[0].getArrivalTime() == 0);
        assert(procs[0].getCpuBurst() == 5);
        assert(procs[0].getPriority() == 2);
        assert(procs[0].getMemoryConfig().isConfigured);
        assert(procs[0].getMemoryConfig().demandUnits == 128);
        assert(procs[0].getIoRequests().size() == 1);
        assert(procs[0].getIoRequests()[0].requestTime == 2);
        assert(procs[0].getIoRequests()[0].duration == 3);
        assert(procs[0].getIoRequests()[0].device == "DISK");

        // Verify PID 2
        assert(procs[1].getPid() == 2);
        assert(procs[1].getArrivalTime() == 1);
        assert(procs[1].getCpuBurst() == 3);
        assert(procs[1].getPriority() == 1);
        assert(procs[1].getMemoryConfig().isConfigured);
        assert(procs[1].getMemoryConfig().demandUnits == 64);

        // Verify PID 3
        assert(procs[2].getPid() == 3);
        assert(procs[2].getArrivalTime() == 2);
        assert(procs[2].getCpuBurst() == 1);
        assert(procs[2].getPriority() == 3);
        assert(!procs[2].getMemoryConfig().isConfigured);

        std::cout << "  Session " << savedSessionId << " resumed with 100% data fidelity!\n";
        mgr.closeActiveSession();
        db.close();
    }

    std::remove(dbFile.c_str());
    std::cout << "\n[PASS] Integration test completed successfully.\n";
    return 0;
}
