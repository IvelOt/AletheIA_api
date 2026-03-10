#include "catch.hpp"
#include "../src/Database.hpp"
#include <cstdio> // para std::remove

TEST_CASE("Database Initialization and Logging", "[database]") {
    std::string test_db = "test_aletheia.db";
    std::remove(test_db.c_str()); // Garante um banco limpo antes do teste

    Database db(test_db);
    REQUIRE(db.init() == true);

    SECTION("Deve inserir e recuperar um registro de auditoria com sucesso") {
        AuditRecord record = {
            "req_12345",
            "123.456.789-00",
            "success",
            0.95f,
            "storage/selfies/12345.jpg"
        };

        REQUIRE(db.log_audit(record) == true);

        auto results = db.get_audit_by_identifier("123.456.789-00");
        REQUIRE(results.size() == 1);
        REQUIRE(results[0].request_id == "req_12345");
        REQUIRE(results[0].status == "success");
        REQUIRE(results[0].similarity_score == Approx(0.95f));
    }

    SECTION("Deve retornar vazio para um identificador que não existe") {
        auto results = db.get_audit_by_identifier("000.000.000-00");
        REQUIRE(results.size() == 0);
    }
}
