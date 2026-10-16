#include "watchos/cli/cli_app.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    std::string dbPath = "data/watchos_sessions.db";
    if (argc > 1) {
        dbPath = argv[1];
    }

    try {
        watchos::cli::CliApp app(dbPath);
        return app.run();
    } catch (const std::exception& ex) {
        std::cerr << "[CRITICAL ERROR] Unhandled exception: " << ex.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "[CRITICAL ERROR] Unknown failure occurred." << std::endl;
        return 2;
    }
}
