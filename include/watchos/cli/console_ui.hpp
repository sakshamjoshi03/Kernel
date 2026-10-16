#ifndef WATCHOS_CLI_CONSOLE_UI_HPP
#define WATCHOS_CLI_CONSOLE_UI_HPP

#include "watchos/session/session.hpp"
#include <string>
#include <vector>

namespace watchos {
namespace cli {

class ConsoleUi {
public:
    static void printStartupBanner();
    static void printBootSequence();
    static void printMainMenu();
    static void printSessionTable(const std::vector<session::SessionMetadata>& sessions);
    static void printWorkloadTable(const std::vector<process::Process>& processes);
    static void printConfirmation(const std::string& sessionId);
    static void printInfo(const std::string& msg);
    static void printWarning(const std::string& msg);
    static void printError(const std::string& msg);
    static void printSuccess(const std::string& msg);
};

} // namespace cli
} // namespace watchos

#endif // WATCHOS_CLI_CONSOLE_UI_HPP
