#ifndef WATCHOS_DATABASE_SCHEMA_HPP
#define WATCHOS_DATABASE_SCHEMA_HPP

#include "watchos/database/db_connection.hpp"

namespace watchos {
namespace database {

class Schema {
public:
    // Initialize or migrate the complete WATCH OS SQLite schema
    static void initializeSchema(DbConnection& db);
};

} // namespace database
} // namespace watchos

#endif // WATCHOS_DATABASE_SCHEMA_HPP
