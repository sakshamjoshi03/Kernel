#include "watchos/database/db_connection.hpp"
#include <iostream>

namespace watchos {
namespace database {

// PreparedStatement
PreparedStatement::PreparedStatement(sqlite3_stmt* stmt) : stmt_(stmt) {}

PreparedStatement::~PreparedStatement() {
    if (stmt_) {
        sqlite3_finalize(stmt_);
        stmt_ = nullptr;
    }
}

PreparedStatement::PreparedStatement(PreparedStatement&& other) noexcept : stmt_(other.stmt_) {
    other.stmt_ = nullptr;
}

PreparedStatement& PreparedStatement::operator=(PreparedStatement&& other) noexcept {
    if (this != &other) {
        if (stmt_) {
            sqlite3_finalize(stmt_);
        }
        stmt_ = other.stmt_;
        other.stmt_ = nullptr;
    }
    return *this;
}

void PreparedStatement::bindInt(int index, int value) {
    if (sqlite3_bind_int(stmt_, index, value) != SQLITE_OK) {
        throw DbException("Failed to bind int at index " + std::to_string(index));
    }
}

void PreparedStatement::bindInt64(int index, int64_t value) {
    if (sqlite3_bind_int64(stmt_, index, value) != SQLITE_OK) {
        throw DbException("Failed to bind int64 at index " + std::to_string(index));
    }
}

void PreparedStatement::bindDouble(int index, double value) {
    if (sqlite3_bind_double(stmt_, index, value) != SQLITE_OK) {
        throw DbException("Failed to bind double at index " + std::to_string(index));
    }
}

void PreparedStatement::bindText(int index, const std::string& value) {
    if (sqlite3_bind_text(stmt_, index, value.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
        throw DbException("Failed to bind text at index " + std::to_string(index));
    }
}

void PreparedStatement::bindNull(int index) {
    if (sqlite3_bind_null(stmt_, index) != SQLITE_OK) {
        throw DbException("Failed to bind null at index " + std::to_string(index));
    }
}

bool PreparedStatement::step() {
    int rc = sqlite3_step(stmt_);
    if (rc == SQLITE_ROW) return true;
    if (rc == SQLITE_DONE) return false;
    throw DbException(std::string("SQLite step error: ") + sqlite3_errstr(rc));
}

void PreparedStatement::reset() {
    sqlite3_reset(stmt_);
    sqlite3_clear_bindings(stmt_);
}

int PreparedStatement::getInt(int col) const {
    return sqlite3_column_int(stmt_, col);
}

int64_t PreparedStatement::getInt64(int col) const {
    return sqlite3_column_int64(stmt_, col);
}

double PreparedStatement::getDouble(int col) const {
    return sqlite3_column_double(stmt_, col);
}

std::string PreparedStatement::getText(int col) const {
    const unsigned char* text = sqlite3_column_text(stmt_, col);
    if (!text) return "";
    return reinterpret_cast<const char*>(text);
}

bool PreparedStatement::isNull(int col) const {
    return sqlite3_column_type(stmt_, col) == SQLITE_NULL;
}

// DbConnection
DbConnection::DbConnection(const std::string& dbPath) : dbPath_(dbPath) {}

DbConnection::~DbConnection() {
    close();
}

void DbConnection::open() {
    if (db_) return;
    int rc = sqlite3_open(dbPath_.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::string err = db_ ? sqlite3_errmsg(db_) : "Failed to allocate sqlite3 handle";
        close();
        throw DbException("Cannot open database at " + dbPath_ + ": " + err);
    }
    // PRD Section 8: Enable SQLite foreign-key enforcement on every connection
    execute("PRAGMA foreign_keys = ON;");
    execute("PRAGMA journal_mode = WAL;");
    execute("PRAGMA synchronous = NORMAL;");
}

void DbConnection::close() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

void DbConnection::execute(const std::string& sql) {
    if (!db_) open();
    char* errmsg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errmsg);
    if (rc != SQLITE_OK) {
        std::string errStr = errmsg ? errmsg : "Unknown sqlite execute error";
        sqlite3_free(errmsg);
        throw DbException("SQL execution error: " + errStr + " in query: " + sql);
    }
}

PreparedStatement DbConnection::prepare(const std::string& sql) {
    if (!db_) open();
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw DbException(std::string("SQL prepare error: ") + sqlite3_errmsg(db_) + " query: " + sql);
    }
    return PreparedStatement(stmt);
}

void DbConnection::beginTransaction() {
    execute("BEGIN IMMEDIATE TRANSACTION;");
}

void DbConnection::commitTransaction() {
    execute("COMMIT TRANSACTION;");
}

void DbConnection::rollbackTransaction() {
    execute("ROLLBACK TRANSACTION;");
}

int64_t DbConnection::lastInsertRowId() const {
    if (!db_) return 0;
    return sqlite3_last_insert_rowid(db_);
}

std::string DbConnection::getLastError() const {
    if (!db_) return "Database closed";
    return sqlite3_errmsg(db_);
}

} // namespace database
} // namespace watchos
