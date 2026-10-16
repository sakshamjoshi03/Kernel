#include "watchos/synchronization/sync_manager.hpp"
#include <sstream>
#include <iomanip>

namespace watchos {
namespace synchronization {

bool SyncManager::acquireMutex(const std::string& mutexName, core::ProcessId pid, core::TimeUnits time) {
    auto it = mutexOwner_.find(mutexName);
    if (it == mutexOwner_.end() || it->second == -1) {
        mutexOwner_[mutexName] = pid;
        history_.push_back({time, pid, mutexName, "ACQUIRE", "Lock granted directly"});
        return true;
    }

    if (it->second == pid) {
        // Re-entrant or already owned
        return true;
    }

    // Mutex is locked by another process; block requesting PID
    mutexWaitQueue_[mutexName].push_back(pid);
    history_.push_back({time, pid, mutexName, "BLOCK", "Contention: lock owned by PID " + std::to_string(it->second)});
    return false;
}

bool SyncManager::releaseMutex(const std::string& mutexName, core::ProcessId pid, core::TimeUnits time) {
    auto it = mutexOwner_.find(mutexName);
    if (it == mutexOwner_.end() || it->second != pid) {
        history_.push_back({time, pid, mutexName, "ERROR", "Process does not own the mutex"});
        return false;
    }

    history_.push_back({time, pid, mutexName, "RELEASE", "Lock released"});
    auto& waitQ = mutexWaitQueue_[mutexName];
    if (!waitQ.empty()) {
        core::ProcessId nextPid = waitQ.front();
        waitQ.pop_front();
        mutexOwner_[mutexName] = nextPid;
        history_.push_back({time, nextPid, mutexName, "WAKEUP", "Lock transferred to waiting process"});
    } else {
        mutexOwner_[mutexName] = -1;
    }
    return true;
}

void SyncManager::createCountingSemaphore(const std::string& semName, int32_t initialCount) {
    semaphoreCount_[semName] = initialCount;
}

bool SyncManager::semWait(const std::string& semName, core::ProcessId pid, core::TimeUnits time) {
    int32_t& count = semaphoreCount_[semName];
    count--;
    if (count < 0) {
        semaphoreWaitQueue_[semName].push_back(pid);
        history_.push_back({time, pid, semName, "SEM_WAIT_BLOCK", "Count=" + std::to_string(count) + "; process queued"});
        return false;
    }
    history_.push_back({time, pid, semName, "SEM_WAIT_OK", "Count=" + std::to_string(count)});
    return true;
}

bool SyncManager::semSignal(const std::string& semName, core::ProcessId pid, core::TimeUnits time) {
    int32_t& count = semaphoreCount_[semName];
    count++;
    history_.push_back({time, pid, semName, "SEM_SIGNAL", "Count=" + std::to_string(count)});
    auto& waitQ = semaphoreWaitQueue_[semName];
    if (!waitQ.empty()) {
        core::ProcessId nextPid = waitQ.front();
        waitQ.pop_front();
        history_.push_back({time, nextPid, semName, "SEM_WAKEUP", "Process awakened from semaphore queue"});
    }
    return true;
}

void SyncManager::createMessageQueue(const std::string& queueName, size_t capacity) {
    MessageQueue mq;
    mq.capacity = capacity;
    messageQueues_[queueName] = mq;
}

bool SyncManager::sendMessage(const std::string& queueName, core::ProcessId senderPid, const std::string& payload, core::TimeUnits time) {
    auto& mq = messageQueues_[queueName];
    if (mq.messages.size() >= mq.capacity) {
        mq.waitingSenders.push_back(senderPid);
        history_.push_back({time, senderPid, queueName, "BLOCK", "Queue full (" + std::to_string(mq.capacity) + " msgs)"});
        return false;
    }

    mq.messages.push_back(payload);
    history_.push_back({time, senderPid, queueName, "MESSAGE_SENT", "Payload: " + payload});

    if (!mq.waitingReceivers.empty()) {
        core::ProcessId recvPid = mq.waitingReceivers.front();
        mq.waitingReceivers.pop_front();
        history_.push_back({time, recvPid, queueName, "WAKEUP", "Awakened receiver on message arrival"});
    }
    return true;
}

bool SyncManager::receiveMessage(const std::string& queueName, core::ProcessId receiverPid, std::string& outPayload, core::TimeUnits time) {
    auto& mq = messageQueues_[queueName];
    if (mq.messages.empty()) {
        mq.waitingReceivers.push_back(receiverPid);
        history_.push_back({time, receiverPid, queueName, "BLOCK", "Queue empty; receiver blocked"});
        return false;
    }

    outPayload = mq.messages.front();
    mq.messages.pop_front();
    history_.push_back({time, receiverPid, queueName, "MESSAGE_RECEIVED", "Payload: " + outPayload});

    if (!mq.waitingSenders.empty()) {
        core::ProcessId sendPid = mq.waitingSenders.front();
        mq.waitingSenders.pop_front();
        history_.push_back({time, sendPid, queueName, "WAKEUP", "Buffer space available; sender awakened"});
    }
    return true;
}

void SyncManager::recordSharedVariableAccess(const std::string& varName, core::ProcessId pid, const std::string& op, core::TimeUnits time, bool inCriticalSection) {
    if (!inCriticalSection && op == "WRITE") {
        history_.push_back({time, pid, varName, "RACE_DETECTED", "Unsynchronized WRITE to shared variable '" + varName + "' detected!"});
    } else {
        history_.push_back({time, pid, varName, "SYNC_ACCESS", "Synchronized " + op + " in critical section"});
    }
}

void SyncManager::runProducerConsumerSimulation(size_t bufferCapacity, int32_t itemsToProduce) {
    history_.clear();
    createCountingSemaphore("EMPTY_SLOTS", static_cast<int32_t>(bufferCapacity));
    createCountingSemaphore("FULL_SLOTS", 0);
    acquireMutex("BUFFER_MUTEX", -1, 0); // initialize unlocked

    core::TimeUnits t = 0;
    core::ProcessId producerPid = 101;
    core::ProcessId consumerPid = 102;

    for (int i = 1; i <= itemsToProduce; ++i) {
        // Producer produces item
        t += 1;
        semWait("EMPTY_SLOTS", producerPid, t);
        acquireMutex("BUFFER_MUTEX", producerPid, t);
        history_.push_back({t, producerPid, "BUFFER", "PRODUCED", "Item #" + std::to_string(i) + " stored into buffer"});
        releaseMutex("BUFFER_MUTEX", producerPid, t);
        semSignal("FULL_SLOTS", producerPid, t);

        // Consumer consumes item
        t += 1;
        semWait("FULL_SLOTS", consumerPid, t);
        acquireMutex("BUFFER_MUTEX", consumerPid, t);
        history_.push_back({t, consumerPid, "BUFFER", "CONSUMED", "Item #" + std::to_string(i) + " retrieved from buffer"});
        releaseMutex("BUFFER_MUTEX", consumerPid, t);
        semSignal("EMPTY_SLOTS", consumerPid, t);
    }
}

std::string SyncManager::formatEventLog() const {
    std::ostringstream oss;
    oss << "Process Synchronization & IPC Event Log:\n";
    oss << "+------+-----+--------------------+--------------------+------------------------------------------+\n";
    oss << "| Time | PID | Primitive          | Action             | Details                                  |\n";
    oss << "+------+-----+--------------------+--------------------+------------------------------------------+\n";

    for (const auto& ev : history_) {
        oss << "| " << std::setw(4) << ev.timestamp
            << " | " << std::setw(3) << ev.pid
            << " | " << std::left << std::setw(18) << ev.primitiveName << std::right
            << " | " << std::left << std::setw(18) << ev.action << std::right
            << " | " << std::left << std::setw(40) << ev.details.substr(0, 40) << std::right << " |\n";
    }
    oss << "+------+-----+--------------------+--------------------+------------------------------------------+\n";
    return oss.str();
}

} // namespace synchronization
} // namespace watchos
