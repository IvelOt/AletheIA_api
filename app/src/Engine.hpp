#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <string>
#include <vector>
#include <tuple>
#include <memory>
#include <opencv2/opencv.hpp>
#include <json.hpp>
#include <inspireface.h>

#include "Database.hpp"
#include "BiometricIndex.hpp"
#include "ImageProcessor.hpp"
#include "OnnxClassifier.hpp"

using json = nlohmann::json;

class AletheiaEngine {
public:
    AletheiaEngine(const std::string& classifier_model_path, 
                   const std::string& inspireface_model_path,
                   const std::string& db_path = "aletheia.db",
                   const std::string& index_path = "faces.index");
    ~AletheiaEngine();

    // 1:1 - Verificação Simples (Selfie vs Documento)
    json verify_images(const std::string& selfie_bytes, const std::string& document_bytes);

    // 1:N - Cadastro e Busca (Enrollment + Auditoria + Biometria)
    json enroll_user(const std::string& identifier, // CPF/RG
                    const std::string& selfie_bytes, 
                    const std::string& document_bytes);

private:
    std::unique_ptr<Database> db_;
    std::unique_ptr<BiometricIndex> index_;
    std::unique_ptr<OnnxClassifier> classifier_;
    
    HFSession session_handle_ = nullptr;
    float similarity_threshold_ = 0.55f;
    float quality_threshold_ = 0.45f;

    // Métodos Internos
    void init_inspireface(const std::string& model_path);
    
    // Converte cv::Mat para o formato interno do InspireFace
    HFImageStream create_image_stream(const cv::Mat& img);

    // Extrai vetor de biometria de uma imagem com rotação automática
    // Retorna (vetor_feature, qualidade, bounding_box)
    std::tuple<std::vector<float>, float, std::vector<int>> extract_feature_with_rotation(const cv::Mat& img);

    // Helper para salvar imagem de auditoria
    std::string save_audit_image(const std::string& bytes, const std::string& prefix);
};

#endif // ENGINE_HPP
