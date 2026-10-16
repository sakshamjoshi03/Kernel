#ifndef WATCHOS_SESSION_CONFIGURATION_MANAGER_HPP
#define WATCHOS_SESSION_CONFIGURATION_MANAGER_HPP

#include "watchos/session/session.hpp"
#include <iostream>
#include <istream>
#include <ostream>
#include <vector>

namespace watchos {
namespace session {

class ConfigurationManager {
public:
    explicit ConfigurationManager(std::istream& in = std::cin, std::ostream& out = std::cout);

    // Run interactive configuration wizard for a session
    bool runInteractiveConfiguration(Session& session);

    // Interactive review and edit menu for workload
    bool runReviewAndEditMenu(Session& session);

    // Prompt and configure a single process interactively
    process::Process promptSingleProcess(int processIndex, int totalProcesses, const std::vector<process::Process>& existing);

    // Edit an existing process
    void editProcess(process::Process& proc, const std::vector<process::Process>& existing);

private:
    std::istream& in_;
    std::ostream& out_;

    // Helper input methods with validation and reprompting
    int promptInt(const std::string& promptText, int minVal, int maxVal, int defaultVal = -1);
    int64_t promptInt64(const std::string& promptText, int64_t minVal, int64_t maxVal, int64_t defaultVal = -1);
    bool promptYesNo(const std::string& promptText, bool defaultVal = false);
    std::string promptString(const std::string& promptText, const std::string& defaultVal = "");

    void configureMemory(process::Process& proc);
    void configureIo(process::Process& proc);
    void configureResources(process::Process& proc);
    void configureDisk(process::Process& proc);
    void configureSync(process::Process& proc);
    void configureAccess(process::Process& proc);
};

} // namespace session
} // namespace watchos

#endif // WATCHOS_SESSION_CONFIGURATION_MANAGER_HPP
