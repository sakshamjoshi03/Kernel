#ifndef WATCHOS_SYNCHRONIZATION_SYNC_MANAGER_HPP
#define WATCHOS_SYNCHRONIZATION_SYNC_MANAGER_HPP

#include "watchos/core/types.hpp"
#include <string>
#include <vector>
#include <deque>
#include <map>

namespace watchos {
namespace synchronization {

struct SyncEventRecord {
    core::TimeUnits timestamp = 0;
    core::ProcessId pid = 0;
    std::string primitiveName;
    std::string action; // ACQUIRE, RELEASE, BLOCK, WAKEUP, MESSAGE_SENT, MESSAGE_RECEIVED, RACE_DETECTED
    std::string details;
};

class SyncManager {
public:
    SyncManager() = default;

    // Mutex operations
    bool acquireMutex(const std::string& mutexName, core::ProcessId pid, core::TimeUnits time);
    bool releaseMutex(const std::string& mutexName, core::ProcessId pid, core::TimeUnits time);

    // Semaphore operations
    void createCountingSemaphore(const std::string& semName, int32_t initialCount);
    bool semWait(const std::string& semName, core::ProcessId pid, core::TimeUnits time);
    bool semSignal(const std::string& semName, core::ProcessId pid, core::TimeUnits time);

    // Bounded IPC Message Queue
    void createMessageQueue(const std::string& queueName, size_t capacity);
    bool sendMessage(const std::string& queueName, core::ProcessId senderPid, const std::string& payload, core::TimeUnits time);
    bool receiveMessage(const std::string& queueName, core::ProcessId receiverPid, std::string& outPayload, core::TimeUnits time);

    // Race condition detection
    void recordSharedVariableAccess(const std::string& varName, core::ProcessId pid, const std::string& op, core::TimeUnits time, bool inCriticalSection);

    // Run simulated producer-consumer scenario
    void runProducerConsumerSimulation(size_t bufferCapacity = 5, int32_t itemsToProduce = 10);

    const std::vector<SyncEventRecord>& getEventHistory() const { return history_; }
    std::string formatEventLog() const;

private:
    std::map<std::string, core::ProcessId> mutexOwner_;
    std::map<std::string, std::deque<core::ProcessId>> mutexWaitQueue_;

    std::map<std::string, int32_t> semaphoreCount_;
    std::map<std::string, std::deque<core::ProcessId>> semaphoreWaitQueue_;

    struct MessageQueue {
        size_t capacity = 5;
        std::deque<std::string> messages;
        std::deque<core::ProcessId> waitingSenders;
        std::deque<core::ProcessId> waitingReceivers;
    };
    std::map<std::string, MessageQueue> messageQueues_;

    std::vector<SyncEventRecord> history_;
};

} // namespace synchronization
} // namespace watchos

#endif // WATCHOS_SYNCHRONIZATION_SYNC_MANAGER_HPP
