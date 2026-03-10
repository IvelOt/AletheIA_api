#ifndef ONNX_CLASSIFIER_HPP
#define ONNX_CLASSIFIER_HPP

#include <onnxruntime_cxx_api.h>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

class OnnxClassifier {
public:
    OnnxClassifier(const std::string& model_path);
    ~OnnxClassifier() = default;

    // Retorna a probabilidade de ser um documento válido (RG/CNH)
    float predict(const cv::Mat& processed_img);

private:
    Ort::Env env_;
    Ort::Session session_;
    Ort::MemoryInfo memory_info_;

    std::vector<const char*> input_names_;
    std::vector<const char*> output_names_;
    
    // Dimensões: [1, 3, 224, 224]
    std::vector<int64_t> input_shape_ = {1, 3, 224, 224};
};

#endif // ONNX_CLASSIFIER_HPP
