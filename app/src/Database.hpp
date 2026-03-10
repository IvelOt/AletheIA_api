#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <string>
#include <vector>
#include <sqlite3.h>

struct AuditRecord {
    std::string request_id;
    std::string identifier;
    std::string status;
    float similarity_score;
    std::string selfie_path;
};

class Database {
public:
    Database(const std::string& db_path);
    ~Database();

    bool init();
    bool log_audit(const AuditRecord& record);
    std::vector<AuditRecord> get_audit_by_identifier(const std::string& identifier);

private:
    std::string db_path_;
    sqlite3* db_ = nullptr;
};

#endif // DATABASE_HPP
