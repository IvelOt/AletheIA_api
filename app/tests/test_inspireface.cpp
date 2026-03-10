#include "catch.hpp"
#include <inspireface.h>
#include <iostream>

TEST_CASE("InspireFace SDK Initialization", "[inspireface]") {
    // O caminho para o modelo Pikachu
    // Tenta apontar diretamente para o arquivo Pikachu ou o diretório pai
    std::string model_path = "../../models/.inspireface/ms/tunmxy/InspireFace/Pikachu";

    SECTION("Deve inicializar o SDK com o modelo Pikachu") {
        HResult ret = HFLaunchInspireFace(model_path.c_str());
        
        if (ret != HSUCCEED) {
            std::cout << "Falha ao carregar InspireFace. Erro: " << ret << std::endl;
            std::cout << "Caminho tentado: " << model_path << std::endl;
        }
        
        REQUIRE(ret == HSUCCEED);
    }
}
