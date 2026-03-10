#include "catch.hpp"
#include "../src/BiometricIndex.hpp"
#include <cstdio>
#include <vector>

TEST_CASE("HNSWLib Biometric Search", "[biometrics]") {
    std::string test_index = "test_faces.index";
    std::remove(test_index.c_str());

    BiometricIndex index(test_index, 3, 100); // 3 dimensões para teste rápido
    REQUIRE(index.init() == true);

    SECTION("Deve adicionar e buscar vetores corretamente") {
        // Vetores normalizados (Simulando Embeddings Faciais)
        std::vector<float> rosto_a = {1.0f, 0.0f, 0.0f}; // ID 1
        std::vector<float> rosto_b = {0.0f, 1.0f, 0.0f}; // ID 2
        
        index.add_face(1, rosto_a);
        index.add_face(2, rosto_b);

        // Busca com um vetor muito parecido com o rosto A
        std::vector<float> query = {0.99f, 0.1f, 0.0f};
        
        auto [id_encontrado, similaridade] = index.search_face(query);
        
        REQUIRE(id_encontrado == 1);
        REQUIRE(similaridade > 0.9f); // Deve ter alta similaridade com o rosto A
        
        // Testa o salvamento em disco
        index.save();
    }
}

TEST_CASE("HNSWLib Load From Disk", "[biometrics]") {
    std::string test_index = "test_faces.index";
    
    // Tenta carregar o índice salvo pelo teste anterior
    BiometricIndex index_loaded(test_index, 3, 100);
    REQUIRE(index_loaded.init() == true);

    SECTION("Deve recuperar vetores salvos anteriormente") {
        // Ao buscar pelo rosto B perfeitamente
        std::vector<float> query_exata = {0.0f, 1.0f, 0.0f};
        auto [id, sim] = index_loaded.search_face(query_exata);

        REQUIRE(id == 2);
        // Considerando problemas de precisão de ponto flutuante na serialização
        REQUIRE(sim > 0.99f); 
    }
}
