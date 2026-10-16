#ifndef WATCHOS_DATABASE_DB_CONNECTION_HPP
#define WATCHOS_DATABASE_DB_CONNECTION_HPP

#include "sqlite3.h"
#include <string>
#include <memory>
#include <stdexcept>
#include <functional>

namespace watchos {
namespace database {

class DbException : public std::runtime_error {
public:
    explicit DbException(const std::string& msg) : std::runtime_error(msg) {}
};

class PreparedStatement {
public:
    explicit PreparedStatement(sqlite3_stmt* stmt);
    ~PreparedStatement();

    PreparedStatement(const PreparedStatement&) = delete;
    PreparedStatement& operator=(const PreparedStatement&) = delete;
    PreparedStatement(PreparedStatement&& other) noexcept;
    PreparedStatement& operator=(PreparedStatement&& other) noexcept;

    void bindInt(int index, int value);
    void bindInt64(int index, int64_t value);
    void bindDouble(int index, double value);
    void bindText(int index, const std::string& value);
    void bindNull(int index);

    bool step(); // returns true if SQLITE_ROW, false if SQLITE_DONE
    void reset();

    int getInt(int col) const;
    int64_t getInt64(int col) const;
    double getDouble(int col) const;
    std::string getText(int col) const;
    bool isNull(int col) const;

    sqlite3_stmt* getRaw() const { return stmt_; }

private:
    sqlite3_stmt* stmt_ = nullptr;
};

class DbConnection {
public:
    explicit DbConnection(const std::string& dbPath);
    ~DbConnection();

    DbConnection(const DbConnection&) = delete;
    DbConnection& operator=(const DbConnection&) = delete;

    void open();
    void close();
    bool isOpen() const { return db_ != nullptr; }

    void execute(const std::string& sql);
    PreparedStatement prepare(const std::string& sql);

    void beginTransaction();
    void commitTransaction();
    void rollbackTransaction();

    int64_t lastInsertRowId() const;
    std::string getLastError() const;

private:
    std::string dbPath_;
    sqlite3* db_ = nullptr;
};

// RAII Transaction Guard
class TransactionGuard {
public:
    explicit TransactionGuard(DbConnection& db) : db_(db), active_(true) {
        db_.beginTransaction();
    }
    ~TransactionGuard() {
        if (active_) {
            try {
                db_.rollbackTransaction();
            } catch (...) {}
        }
    }
    void commit() {
        if (active_) {
            db_.commitTransaction();
            active_ = false;
        }
    }

private:
    DbConnection& db_;
    bool active_ = false;
};

} // namespace database
} // namespace watchos

#endif // WATCHOS_DATABASE_DB_CONNECTION_HPP
