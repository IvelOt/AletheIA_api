#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <string>
#include <vector>
#include <tuple>
#include <memory>
#include <future>
#include <opencv2/opencv.hpp>
#include <json.hpp>
#include <inspireface.h>

#include "ImageProcessor.hpp"
#include "OnnxClassifier.hpp"

using json = nlohmann::json;

struct FaceExtractionResult {
    std::vector<float> feature;
    float quality = 0.0f;
    bool success = false;
};

class AletheiaEngine {
public:
    AletheiaEngine(const std::string& classifier_model_path, 
                   const std::string& inspireface_model_path);
    ~AletheiaEngine();

    json verify_images(const std::string& selfie_bytes, const std::string& document_bytes);

private:
    std::unique_ptr<OnnxClassifier> classifier_;
    
    // Duas sessões para permitir paralelismo real
    HFSession session_selfie_ = nullptr;
    HFSession session_doc_ = nullptr;
    
    float similarity_threshold_ = 0.55f;
    float quality_threshold_ = 0.45f;

    void init_inspireface(const std::string& model_path);
    HFSession create_session();
    
    HFImageStream create_image_stream(const cv::Mat& img);
    
    // Versão que aceita a sessão específica
    FaceExtractionResult extract_feature_parallel(const cv::Mat& img, HFSession session);
};

#endif // ENGINE_HPP
