#include "watchos/database/db_connection.hpp"
#include "watchos/database/schema.hpp"
#include "watchos/database/session_repository.hpp"
#include "tests/fixtures/test_workloads.hpp"
#include <iostream>
#include <cassert>

void testSchemaAndSessionCrud() {
    std::cout << "[TEST] Running testSchemaAndSessionCrud...\n";
    watchos::database::DbConnection db(":memory:");
    db.open();
    watchos::database::Schema::initializeSchema(db);

    watchos::database::SessionRepository repo(db);

    // Create session
    watchos::session::Session sess("WOS-20261016-TEST", "Unit test session");
    sess.setProcesses(watchos::test::getGoldenWorkload());
    repo.createSession(sess);

    assert(repo.sessionExists("WOS-20261016-TEST"));

    // Save workload
    repo.saveWorkload(sess, true);

    // Load session back
    watchos::session::Session loaded;
    bool ok = repo.loadSession("WOS-20261016-TEST", loaded);
    assert(ok);
    assert(loaded.getSessionId() == "WOS-20261016-TEST");
    assert(loaded.getStatus() == watchos::core::SessionStatus::FINALIZED);
    assert(loaded.getProcesses().size() == 3);

    // Verify process 1 details
    const auto& p1 = loaded.getProcesses()[0];
    assert(p1.getPid() == 1);
    assert(p1.getArrivalTime() == 0);
    assert(p1.getCpuBurst() == 5);
    assert(p1.getPriority() == 2);
    assert(p1.getMemoryConfig().isConfigured == true);
    assert(p1.getMemoryConfig().demandUnits == 128);
    assert(p1.getIoRequests().size() == 1);
    assert(p1.getIoRequests()[0].duration == 3);
    assert(p1.getIoRequests()[0].device == "DISK");

    // Verify session listing
    auto list = repo.listSessions();
    assert(list.size() == 1);
    assert(list[0].processCount == 3);

    std::cout << "[PASS] Session CRUD and relational mapping verified.\n";
}

void testForeignKeyIntegrity() {
    std::cout << "[TEST] Running testForeignKeyIntegrity...\n";
    watchos::database::DbConnection db(":memory:");
    db.open();
    watchos::database::Schema::initializeSchema(db);

    // Attempt to insert a configuration_version pointing to a non-existent session
    bool caughtFkError = false;
    try {
        auto stmt = db.prepare(
            "INSERT INTO configuration_versions (session_id, version, created_at, is_finalized, settings_json) "
            "VALUES ('NON_EXISTENT_SESSION', 1, '2026-10-16', 0, '{}');"
        );
        stmt.step();
    } catch (const watchos::database::DbException& ex) {
        caughtFkError = true;
    }
    assert(caughtFkError);
    std::cout << "[PASS] SQLite Foreign Key constraint enforced successfully.\n";
}

void testTransactionRollback() {
    std::cout << "[TEST] Running testTransactionRollback...\n";
    watchos::database::DbConnection db(":memory:");
    db.open();
    watchos::database::Schema::initializeSchema(db);

    watchos::database::SessionRepository repo(db);
    watchos::session::Session sess("WOS-20261016-TX01", "Transaction test");
    repo.createSession(sess);

    try {
        watchos::database::TransactionGuard tx(db);
        auto stmt = db.prepare("UPDATE sessions SET description = 'Updated in TX' WHERE session_id = 'WOS-20261016-TX01';");
        stmt.step();
        // Intentional exception without commit
        throw std::runtime_error("Simulated crash during transaction");
    } catch (...) {}

    watchos::session::Session checkSess;
    repo.loadSession("WOS-20261016-TX01", checkSess);
    assert(checkSess.getDescription() == "Transaction test");
    std::cout << "[PASS] Transaction rollback restored original state cleanly.\n";
}

int main() {
    try {
        testSchemaAndSessionCrud();
        testForeignKeyIntegrity();
        testTransactionRollback();
        std::cout << "All Database Persistence unit tests passed successfully!\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "[FAIL] Test failed with exception: " << ex.what() << "\n";
        return 1;
    }
}
