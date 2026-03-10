#include "OnnxClassifier.hpp"
#include <numeric>
#include <algorithm>
#include <cmath>
#include <iostream>

OnnxClassifier::OnnxClassifier(const std::string& model_path) 
    : env_(ORT_LOGGING_LEVEL_WARNING, "Aletheia"),
      session_(env_, model_path.c_str(), Ort::SessionOptions{nullptr}),
      memory_info_(Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU)) {
    
    // Configurar nomes de input/output
    Ort::AllocatorWithDefaultOptions allocator;
    
    // Nota: Em versões recentes do ONNX C++ API, GetInputName/GetOutputName retornam ponteiros que precisam ser geridos
    // Para simplificar e evitar leaks em C++17, usaremos uma abordagem robusta
    auto input_name_ptr = session_.GetInputNameAllocated(0, allocator);
    auto output_name_ptr = session_.GetOutputNameAllocated(0, allocator);
    
    // Copiamos os nomes pois o ponteiro do allocator será liberado
    static std::string input_name_str;
    static std::string output_name_str;
    input_name_str = input_name_ptr.get();
    output_name_str = output_name_ptr.get();

    input_names_.push_back(input_name_str.c_str());
    output_names_.push_back(output_name_str.c_str());
}

float OnnxClassifier::predict(const cv::Mat& processed_img) {
    if (processed_img.empty()) return 0.0f;

    // Converter cv::Mat HWC (Height, Width, Channel) para CHW (Channel, Height, Width)
    // O ONNX espera [1, 3, 224, 224]
    std::vector<float> input_tensor_values(3 * 224 * 224);
    
    // Desacopla canais (HWC -> CHW)
    for (int c = 0; c < 3; ++c) {
        for (int h = 0; h < 224; ++h) {
            for (int w = 0; w < 224; ++w) {
                input_tensor_values[c * 224 * 224 + h * 224 + w] = 
                    processed_img.at<cv::Vec3f>(h, w)[c];
            }
        }
    }

    // Criar tensor de entrada
    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
        memory_info_, input_tensor_values.data(), input_tensor_values.size(), 
        input_shape_.data(), input_shape_.size()
    );

    // Rodar Inferência
    auto output_tensors = session_.Run(
        Ort::RunOptions{nullptr}, 
        input_names_.data(), &input_tensor, 1, 
        output_names_.data(), 1
    );

    // Pegar Resultados (Logits)
    float* floatarr = output_tensors[0].GetTensorMutableData<float>();
    
    // O modelo MobileNetV3 costuma retornar 2 classes (0: Outro, 1: Documento)
    // Aplicamos Softmax simples para pegar a confiança do índice 1
    float logit0 = floatarr[0];
    float logit1 = floatarr[1];
    
    float max_logit = std::max(logit0, logit1);
    float exp0 = std::exp(logit0 - max_logit);
    float exp1 = std::exp(logit1 - max_logit);
    
    float prob1 = exp1 / (exp0 + exp1);

    return prob1;
}
