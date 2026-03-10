#ifndef IMAGE_PROCESSOR_HPP
#define IMAGE_PROCESSOR_HPP

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

class ImageProcessor {
public:
    // Converte os bytes brutos recebidos via HTTP para um cv::Mat (BGR)
    static cv::Mat decode(const std::string& bytes);

    // Redimensiona e normaliza para o formato MobileNetV3 (224x224, RGB, Mean/Std)
    static cv::Mat prepare_for_classifier(const cv::Mat& img);

    // Rotaciona a imagem (0, 90, 180, 270)
    static cv::Mat rotate(const cv::Mat& img, int angle_deg);
};

#endif // IMAGE_PROCESSOR_HPP
