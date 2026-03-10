#include "Database.hpp"
#include <iostream>

Database::Database(const std::string& db_path) : db_path_(db_path) {}

Database::~Database() {
    if (db_) {
        sqlite3_close(db_);
    }
}

bool Database::init() {
    int rc = sqlite3_open(db_path_.c_str(), &db_);
    if (rc) {
        std::cerr << "Erro ao abrir banco: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    const char* sql = "CREATE TABLE IF NOT EXISTS audit ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "request_id TEXT,"
                      "identifier TEXT,"
                      "status TEXT,"
                      "similarity_score REAL,"
                      "selfie_path TEXT,"
                      "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP);";

    char* errMsg = nullptr;
    rc = sqlite3_exec(db_, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Erro ao criar tabela: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool Database::log_audit(const AuditRecord& record) {
    if (!db_) return false;

    const char* sql = "INSERT INTO audit (request_id, identifier, status, similarity_score, selfie_path) "
                      "VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, record.request_id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, record.identifier.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, record.status.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 4, record.similarity_score);
    sqlite3_bind_text(stmt, 5, record.selfie_path.c_str(), -1, SQLITE_STATIC);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

std::vector<AuditRecord> Database::get_audit_by_identifier(const std::string& identifier) {
    std::vector<AuditRecord> results;
    if (!db_) return results;

    const char* sql = "SELECT request_id, identifier, status, similarity_score, selfie_path FROM audit WHERE identifier = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return results;
    }

    sqlite3_bind_text(stmt, 1, identifier.c_str(), -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        AuditRecord rec;
        rec.request_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        rec.identifier = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        rec.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        rec.similarity_score = sqlite3_column_double(stmt, 3);
        rec.selfie_path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        results.push_back(rec);
    }

    sqlite3_finalize(stmt);
    return results;
}
