#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <string>
#include <vector>
#include <tuple>
#include <memory>
#include <opencv2/opencv.hpp>
#include <json.hpp>
#include <inspireface.h>

#include "ImageProcessor.hpp"
#include "OnnxClassifier.hpp"

using json = nlohmann::json;

class AletheiaEngine {
public:
    AletheiaEngine(const std::string& classifier_model_path, 
                   const std::string& inspireface_model_path);
    ~AletheiaEngine();

    // 1:1 - Verificação Stateless (Recebe, valida e retorna)
    json verify_images(const std::string& selfie_bytes, const std::string& document_bytes);

private:
    std::unique_ptr<OnnxClassifier> classifier_;
    HFSession session_handle_ = nullptr;
    
    float similarity_threshold_ = 0.55f;
    float quality_threshold_ = 0.45f;

    void init_inspireface(const std::string& model_path);
    HFImageStream create_image_stream(const cv::Mat& img);
    std::tuple<std::vector<float>, float> extract_feature_with_rotation(const cv::Mat& img);
};

#endif // ENGINE_HPP
