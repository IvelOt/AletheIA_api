#include "Engine.hpp"
#include <iostream>
#include <chrono>
#include <cmath>
#include <cstring>

AletheiaEngine::AletheiaEngine(const std::string& classifier_model_path, 
                               const std::string& inspireface_model_path) {
    
    std::cout << "[Aletheia Engine] Inicializando Motor Stateless..." << std::endl;
    classifier_ = std::make_unique<OnnxClassifier>(classifier_model_path);
    init_inspireface(inspireface_model_path);
    std::cout << "[Aletheia Engine] Motor carregado." << std::endl;
}

AletheiaEngine::~AletheiaEngine() {
    if (session_handle_) HFReleaseInspireFaceSession(session_handle_);
}

void AletheiaEngine::init_inspireface(const std::string& model_path) {
    if (HFLaunchInspireFace(model_path.c_str()) != HSUCCEED) {
        throw std::runtime_error("InspireFace: Launch failed.");
    }
    
    HFSessionCustomParameter param;
    std::memset(&param, 0, sizeof(param));
    param.enable_recognition = 1;
    param.enable_face_quality = 1;
    
    if (HFCreateInspireFaceSession(param, HF_DETECT_MODE_ALWAYS_DETECT, 1, 160, -1, &session_handle_) != HSUCCEED) {
        throw std::runtime_error("InspireFace: Session failed.");
    }
}

HFImageStream AletheiaEngine::create_image_stream(const cv::Mat& img) {
    HFImageStream handle;
    HFImageData data;
    data.data = img.data;
    data.width = img.cols;
    data.height = img.rows;
    data.format = HF_STREAM_BGR;
    data.rotation = HF_CAMERA_ROTATION_0;
    HFCreateImageStream(&data, &handle);
    return handle;
}

std::tuple<std::vector<float>, float> 
AletheiaEngine::extract_feature_with_rotation(const cv::Mat& img) {
    for (int angle : {0, 90, 180, 270}) {
        cv::Mat rotated = ImageProcessor::rotate(img, angle);
        HFImageStream stream = create_image_stream(rotated);
        HFMultipleFaceData faces;
        if (HFExecuteFaceTrack(session_handle_, stream, &faces) == HSUCCEED && faces.detectedNum > 0) {
            HFFaceBasicToken token = faces.tokens[0];
            float quality = 0.0f;
            if (HFFaceQualityDetect(session_handle_, token, &quality) == HSUCCEED && quality >= quality_threshold_) {
                HFFaceFeature feature_data;
                if (HFFaceFeatureExtract(session_handle_, stream, token, &feature_data) == HSUCCEED) {
                    std::vector<float> feat(feature_data.data, feature_data.data + 512);
                    float norm = 0.0f;
                    for (float v : feat) norm += v * v;
                    norm = std::sqrt(norm);
                    if (norm > 0) for (float& v : feat) v /= norm;
                    HFReleaseImageStream(stream);
                    return std::make_tuple(feat, quality);
                }
            }
        }
        HFReleaseImageStream(stream);
    }
    return std::make_tuple(std::vector<float>{}, 0.0f);
}

json AletheiaEngine::verify_images(const std::string& selfie_bytes, const std::string& document_bytes) {
    json response;
    std::string transaction_id = "tx_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    response["transaction_id"] = transaction_id;

    cv::Mat s_img = ImageProcessor::decode(selfie_bytes);
    cv::Mat d_img = ImageProcessor::decode(document_bytes);

    if (s_img.empty() || d_img.empty()) {
        response["status"] = "error";
        response["message"] = "Invalid images.";
        return response;
    }

    // 1. Validar Documento
    if (classifier_->predict(ImageProcessor::prepare_for_classifier(d_img)) < 0.8f) {
        response["status"] = "error";
        response["message"] = "Invalid document.";
        return response;
    }

    // 2. Extrair Faces
    auto [feat_d, q_d] = extract_feature_with_rotation(d_img);
    auto [feat_s, q_s] = extract_feature_with_rotation(s_img);

    if (feat_d.empty() || feat_s.empty()) {
        response["status"] = "error";
        response["message"] = "Face not found.";
        return response;
    }

    // 3. Comparação 1:1
    float sim = 0.0f;
    for (int i = 0; i < 512; ++i) sim += feat_d[i] * feat_s[i];
    
    response["status"] = (sim >= similarity_threshold_) ? "success" : "mismatch";
    response["similarity"] = sim;
    
    return response;
}
