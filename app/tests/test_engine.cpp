#include "catch.hpp"
#include "../src/Engine.hpp"
#include <fstream>
#include <iostream>

TEST_CASE("Stateless Engine Integration", "[engine]") {
    std::string classifier_path = "../../models/document_classifier.onnx";
    std::string isf_model_path = "../../models/.inspireface/ms/tunmxy/InspireFace/Pikachu";

    AletheiaEngine engine(classifier_path, isf_model_path);

    SECTION("Deve validar um par de imagens real (Selfie vs Identidade)") {
        auto read_file = [](const std::string& path) {
            std::ifstream file(path, std::ios::binary);
            return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        };

        std::string selfie = read_file("../../Examples/foto_1.jpeg");
        std::string document = read_file("../../Examples/identidade_1.jpg");

        REQUIRE(!selfie.empty());
        REQUIRE(!document.empty());

        json result = engine.verify_images(selfie, document);

        // Verifica se a estrutura da resposta está correta
        REQUIRE(result.contains("status"));
        REQUIRE(result.contains("similarity"));
        REQUIRE(result.contains("transaction_id"));
        
        std::cout << "Engine Integration Test Result: " << result.dump(4) << std::endl;
    }

    SECTION("Deve retornar erro para imagens inválidas") {
        json result = engine.verify_images("not_an_image", "also_not_an_image");
        REQUIRE(result["status"] == "error");
    }
}
