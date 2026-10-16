#include "watchos/core/id_generator.hpp"
#include <iostream>
#include <cassert>
#include <set>

void testSessionIdFormatAndUniqueness() {
    std::cout << "[TEST] Running testSessionIdFormatAndUniqueness...\n";
    std::set<std::string> sessionIds;

    for (int i = 0; i < 1000; ++i) {
        std::string id = watchos::core::IdGenerator::generateSessionId();
        // Check prefix
        assert(id.substr(0, 4) == "WOS-");
        // Check length: "WOS-" (4) + YYYYMMDD (8) + "-" (1) + XXXX (4) = 17 characters
        assert(id.length() == 17);
        // Check uniqueness
        assert(sessionIds.find(id) == sessionIds.end());
        sessionIds.insert(id);
    }
    std::cout << "[PASS] Generated 1,000 unique Session IDs without collisions.\n";
}

void testRunIdGeneration() {
    std::cout << "[TEST] Running testRunIdGeneration...\n";
    std::string run1 = watchos::core::IdGenerator::generateRunId();
    std::string run2 = watchos::core::IdGenerator::generateRunId();
    assert(run1.substr(0, 4) == "RUN-");
    assert(run2.substr(0, 4) == "RUN-");
    assert(run1 != run2);

    std::string runSeq = watchos::core::IdGenerator::generateRunId(12);
    assert(runSeq == "RUN-000012");
    std::cout << "[PASS] Run ID generation formats match PRD specification.\n";
}

int main() {
    try {
        testSessionIdFormatAndUniqueness();
        testRunIdGeneration();
        std::cout << "All ID Generator unit tests passed successfully!\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "[FAIL] Test failed with exception: " << ex.what() << "\n";
        return 1;
    }
}
