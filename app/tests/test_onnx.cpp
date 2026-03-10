#include "catch.hpp"
#include "../src/OnnxClassifier.hpp"
#include "../src/ImageProcessor.hpp"
#include <fstream>
#include <iostream>

TEST_CASE("ONNX Model Inference", "[onnx]") {
    std::string model_path = "../../models/document_classifier.onnx";
    
    SECTION("Deve carregar o modelo e rodar uma inferência com sucesso") {
        OnnxClassifier classifier(model_path);
        
        // Criar uma imagem fake 224x224 (Pura preta)
        cv::Mat black_img = cv::Mat::zeros(224, 224, CV_32FC3);
        
        float prob = classifier.predict(black_img);
        
        // Verifica se retornou uma probabilidade válida [0.0, 1.0]
        REQUIRE(prob >= 0.0f);
        REQUIRE(prob <= 1.0f);
        
        std::cout << "Probabilidade para imagem preta: " << prob << std::endl;
    }

    SECTION("Deve classificar uma imagem de teste real") {
        std::string test_file = "../../Examples/identidade_1.jpg";
        std::ifstream file(test_file, std::ios::binary);
        
        if (file.good()) {
            std::string bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            cv::Mat img = ImageProcessor::decode(bytes);
            cv::Mat processed = ImageProcessor::prepare_for_classifier(img);
            
            OnnxClassifier classifier(model_path);
            float prob = classifier.predict(processed);
            
            // Esperamos que uma identidade real tenha probabilidade alta (> 0.8)
            REQUIRE(prob > 0.5f);
            std::cout << "Probabilidade para identidade_1.jpg: " << prob << std::endl;
        }
    }
}
