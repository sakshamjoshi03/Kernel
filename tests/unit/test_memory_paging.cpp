#include "watchos/memory/page_replacement.hpp"
#include <iostream>
#include <cassert>

void testPageReplacementAlgorithms() {
    std::cout << "[TEST] Running testPageReplacementAlgorithms...\n";
    // Textbook reference string:
    std::vector<int32_t> refs = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1};
    int32_t frames = 3;

    auto fifoRes = watchos::memory::PageReplacementSimulator::simulate(
        watchos::memory::PageReplacementPolicy::FIFO, refs, frames
    );
    auto lruRes = watchos::memory::PageReplacementSimulator::simulate(
        watchos::memory::PageReplacementPolicy::LRU, refs, frames
    );
    auto optRes = watchos::memory::PageReplacementSimulator::simulate(
        watchos::memory::PageReplacementPolicy::OPTIMAL, refs, frames
    );

    std::cout << "  FIFO Faults: " << fifoRes.pageFaults << "\n";
    std::cout << "  LRU Faults: " << lruRes.pageFaults << "\n";
    std::cout << "  Optimal Faults: " << optRes.pageFaults << "\n";

    // Known theoretical expectations for this string with 3 frames:
    // FIFO: 15 faults
    // LRU: 12 faults
    // Optimal: 9 faults
    assert(fifoRes.pageFaults == 15);
    assert(lruRes.pageFaults == 12);
    assert(optRes.pageFaults == 9);

    assert(optRes.pageFaults <= lruRes.pageFaults);
    assert(lruRes.pageFaults <= fifoRes.pageFaults);

    std::cout << "[PASS] FIFO (15), LRU (12), and Optimal (9) fault counts match exact textbook expectations.\n";
}

int main() {
    try {
        testPageReplacementAlgorithms();
        std::cout << "All Memory Paging unit tests passed successfully!\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "[FAIL] Test failed with exception: " << ex.what() << "\n";
        return 1;
    }
}
