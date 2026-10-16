#ifndef WATCHOS_CLI_CLI_APP_HPP
#define WATCHOS_CLI_CLI_APP_HPP

#include "watchos/database/db_connection.hpp"
#include "watchos/session/session_manager.hpp"
#include <string>
#include <memory>

namespace watchos {
namespace cli {

class CliApp {
public:
    explicit CliApp(const std::string& dbPath);
    ~CliApp() = default;

    // Initialize database, schema, and start CLI loop
    int run();

private:
    std::string dbPath_;
    std::unique_ptr<database::DbConnection> db_;
    std::unique_ptr<session::SessionManager> sessionManager_;
    core::SystemSettings settings_;

    void handleCreateSession();
    void handleResumeSession();
    void handleViewHistory();
    void handleCompareSessions();
    void handleSettings();
};

} // namespace cli
} // namespace watchos

#endif // WATCHOS_CLI_CLI_APP_HPP
