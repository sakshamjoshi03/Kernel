#ifndef WATCHOS_CORE_RUN_CONTEXT_HPP
#define WATCHOS_CORE_RUN_CONTEXT_HPP

#include "watchos/core/types.hpp"
#include "watchos/process/process.hpp"
#include <string>
#include <vector>
#include <map>

namespace watchos {
namespace core {

class RunContext {
public:
    RunContext() = default;
    RunContext(const std::string& runId, const std::string& sessionId, const std::string& moduleName)
        : runId_(runId), sessionId_(sessionId), moduleName_(moduleName) {}

    const std::string& getRunId() const { return runId_; }
    void setRunId(const std::string& id) { runId_ = id; }

    const std::string& getSessionId() const { return sessionId_; }
    void setSessionId(const std::string& id) { sessionId_ = id; }

    const std::string& getModuleName() const { return moduleName_; }
    void setModuleName(const std::string& mod) { moduleName_ = mod; }

    uint64_t getRandomSeed() const { return randomSeed_; }
    void setRandomSeed(uint64_t seed) { randomSeed_ = seed; }

    const std::vector<process::Process>& getWorkloadSnapshot() const { return workloadSnapshot_; }
    std::vector<process::Process>& getWorkloadSnapshot() { return workloadSnapshot_; }
    void setWorkloadSnapshot(const std::vector<process::Process>& procs) { workloadSnapshot_ = procs; }

    const SystemSettings& getSettings() const { return settings_; }
    SystemSettings& getSettings() { return settings_; }
    void setSettings(const SystemSettings& s) { settings_ = s; }

    void setParam(const std::string& key, const std::string& val) { params_[key] = val; }
    std::string getParam(const std::string& key, const std::string& defVal = "") const {
        auto it = params_.find(key);
        return (it != params_.end()) ? it->second : defVal;
    }

    const std::map<std::string, std::string>& getAllParams() const { return params_; }

    TimeUnits getSimulatedClock() const { return simulatedClock_; }
    void setSimulatedClock(TimeUnits t) { simulatedClock_ = t; }

private:
    std::string runId_;
    std::string sessionId_;
    std::string moduleName_;
    uint64_t randomSeed_ = 42;
    std::vector<process::Process> workloadSnapshot_;
    SystemSettings settings_;
    std::map<std::string, std::string> params_;
    TimeUnits simulatedClock_ = 0;
};

} // namespace core
} // namespace watchos

#endif // WATCHOS_CORE_RUN_CONTEXT_HPP
