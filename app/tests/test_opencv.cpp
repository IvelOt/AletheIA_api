#include "catch.hpp"
#include "../src/ImageProcessor.hpp"
#include <fstream>
#include <iostream>

TEST_CASE("OpenCV Image Processing", "[opencv]") {
    
    SECTION("Deve falhar ao decodificar bytes vazios") {
        cv::Mat img = ImageProcessor::decode("");
        REQUIRE(img.empty());
    }

    SECTION("Deve carregar e processar uma imagem real de teste") {
        // Tenta ler um dos exemplos de identidade
        std::string test_file = "../../Examples/identidade_1.jpg";
        std::ifstream file(test_file, std::ios::binary);
        
        if (file.good()) {
            std::string bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            cv::Mat img = ImageProcessor::decode(bytes);
            
            REQUIRE(!img.empty());
            REQUIRE(img.channels() == 3);

            // Testar redimensionamento para ONNX (MobileNetV3: 224x224)
            cv::Mat processed = ImageProcessor::prepare_for_classifier(img);
            REQUIRE(processed.rows == 224);
            REQUIRE(processed.cols == 224);
            REQUIRE(processed.type() == CV_32FC3); // Float32 com 3 canais
        } else {
            std::cout << "Aviso: Imagem de teste não encontrada em " << test_file << ". Pulando parte do teste." << std::endl;
        }
    }

    SECTION("Deve rotacionar a imagem corretamente") {
        cv::Mat black = cv::Mat::zeros(100, 200, CV_8UC3); // Larga (100x200)
        cv::Mat rotated = ImageProcessor::rotate(black, 90);
        
        REQUIRE(rotated.rows == 200); // Agora deve ser alta (200x100)
        REQUIRE(rotated.cols == 100);
    }
}
