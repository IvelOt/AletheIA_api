#include "ImageProcessor.hpp"
#include <iostream>

cv::Mat ImageProcessor::decode(const std::string& bytes) {
    if (bytes.empty()) {
        return cv::Mat();
    }
    std::vector<char> buffer(bytes.begin(), bytes.end());
    return cv::imdecode(buffer, cv::IMREAD_COLOR);
}

cv::Mat ImageProcessor::rotate(const cv::Mat& img, int angle_deg) {
    if (img.empty()) return cv::Mat();

    cv::Mat rotated;
    if (angle_deg == 0) return img.clone();
    else if (angle_deg == 90) cv::rotate(img, rotated, cv::ROTATE_90_CLOCKWISE);
    else if (angle_deg == 180) cv::rotate(img, rotated, cv::ROTATE_180);
    else if (angle_deg == 270) cv::rotate(img, rotated, cv::ROTATE_90_COUNTERCLOCKWISE);
    else return img.clone();

    return rotated;
}

cv::Mat ImageProcessor::prepare_for_classifier(const cv::Mat& img) {
    if (img.empty()) return cv::Mat();

    cv::Mat resized;
    cv::resize(img, resized, cv::Size(224, 224));
    
    // Converte BGR para RGB (O ONNX espera RGB)
    cv::Mat rgb;
    cv::cvtColor(resized, rgb, cv::COLOR_BGR2RGB);

    // Normalização (Float32 / 255.0)
    cv::Mat normalized;
    rgb.convertTo(normalized, CV_32FC3, 1.0 / 255.0);

    // Subtração de Média e Divisão pelo Desvio Padrão (Padrão ImageNet)
    // Mean: [0.485, 0.456, 0.406], Std: [0.229, 0.224, 0.225]
    cv::Scalar mean(0.485, 0.456, 0.406);
    cv::Scalar std(0.229, 0.224, 0.225);
    
    cv::subtract(normalized, mean, normalized);
    cv::divide(normalized, std, normalized);

    return normalized;
}
