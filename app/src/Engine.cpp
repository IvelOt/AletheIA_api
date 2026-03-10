#include "Engine.hpp"
#include <iostream>
#include <fstream>
#include <chrono>
#include <sys/stat.h>
#include <cmath>
#include <cstring>

void make_dir(const std::string& path) {
    mkdir(path.c_str(), 0777);
}

AletheiaEngine::AletheiaEngine(const std::string& classifier_model_path, 
                               const std::string& inspireface_model_path,
                               const std::string& db_path,
                               const std::string& index_path) {
    
    std::cout << "[Aletheia Engine] Inicializando componentes..." << std::endl;
    
    db_ = std::make_unique<Database>(db_path);
    if (!db_->init()) throw std::runtime_error("Database failed.");
    
    index_ = std::make_unique<BiometricIndex>(index_path, 512, 10000);
    if (!index_->init()) throw std::runtime_error("Biometric index failed.");
    
    classifier_ = std::make_unique<OnnxClassifier>(classifier_model_path);
    
    init_inspireface(inspireface_model_path);
    
    make_dir("storage");
    make_dir("storage/selfies");
    make_dir("storage/documents");
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
    param.enable_face_pose = 1;
    
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

std::tuple<std::vector<float>, float, std::vector<int>> 
AletheiaEngine::extract_feature_with_rotation(const cv::Mat& img) {
    
    for (int angle : {0, 90, 180, 270}) {
        cv::Mat rotated = ImageProcessor::rotate(img, angle);
        HFImageStream stream = create_image_stream(rotated);
        
        HFMultipleFaceData faces;
        if (HFExecuteFaceTrack(session_handle_, stream, &faces) == HSUCCEED && faces.detectedNum > 0) {
            HFaceRect rect = faces.rects[0];
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
                    return std::make_tuple(feat, quality, std::vector<int>{(int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height});
                }
            }
        }
        HFReleaseImageStream(stream);
    }
    return std::make_tuple(std::vector<float>{}, 0.0f, std::vector<int>{});
}

std::string AletheiaEngine::save_audit_image(const std::string& bytes, const std::string& prefix) {
    auto now = std::chrono::system_clock::now().time_since_epoch().count();
    std::string path = "storage/" + prefix + "/" + std::to_string(now) + ".jpg";
    std::ofstream out(path, std::ios::binary);
    out.write(bytes.data(), bytes.size());
    return path;
}

json AletheiaEngine::verify_images(const std::string& selfie_bytes, const std::string& document_bytes) {
    json response;
    cv::Mat s_img = ImageProcessor::decode(selfie_bytes);
    cv::Mat d_img = ImageProcessor::decode(document_bytes);

    if (s_img.empty() || d_img.empty()) {
        response["status"] = "error";
        response["message"] = "Invalid images.";
        return response;
    }

    if (classifier_->predict(ImageProcessor::prepare_for_classifier(d_img)) < 0.8f) {
        response["status"] = "error";
        response["message"] = "Invalid document.";
        return response;
    }

    auto [feat_d, q_d, b_d] = extract_feature_with_rotation(d_img);
    auto [feat_s, q_s, b_s] = extract_feature_with_rotation(s_img);

    if (feat_d.empty() || feat_s.empty()) {
        response["status"] = "error";
        response["message"] = "Face not found.";
        return response;
    }

    float sim = 0.0f;
    for (int i = 0; i < 512; ++i) sim += feat_d[i] * feat_s[i];
    
    response["status"] = (sim >= similarity_threshold_) ? "success" : "mismatch";
    response["similarity"] = sim;
    return response;
}

json AletheiaEngine::enroll_user(const std::string& identifier, 
                               const std::string& selfie_bytes, 
                               const std::string& document_bytes) {
    json response;
    std::string req_id = "req_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());

    cv::Mat s_img = ImageProcessor::decode(selfie_bytes);
    cv::Mat d_img = ImageProcessor::decode(document_bytes);

    if (s_img.empty() || d_img.empty()) {
        response["status"] = "error";
        response["message"] = "Invalid images.";
        return response;
    }

    if (classifier_->predict(ImageProcessor::prepare_for_classifier(d_img)) < 0.8f) {
        db_->log_audit({req_id, identifier, "invalid_document", 0.0f, ""});
        response["status"] = "error";
        response["message"] = "Invalid document.";
        return response;
    }

    auto [feat_d, q_d, _] = extract_feature_with_rotation(d_img);
    auto [feat_s, q_s, __] = extract_feature_with_rotation(s_img);

    if (feat_d.empty() || feat_s.empty()) {
        db_->log_audit({req_id, identifier, "face_not_found", 0.0f, ""});
        response["status"] = "error";
        response["message"] = "Face not found.";
        return response;
    }

    float sim = 0.0f;
    for (int i = 0; i < 512; ++i) sim += feat_d[i] * feat_s[i];

    if (sim < similarity_threshold_) {
        db_->log_audit({req_id, identifier, "mismatch", sim, ""});
        response["status"] = "mismatch";
        return response;
    }

    auto [ex_id, top_sim] = index_->search_face(feat_s);
    if (ex_id != -1 && top_sim > 0.85f) {
        db_->log_audit({req_id, identifier, "already_enrolled", top_sim, ""});
        response["status"] = "already_enrolled";
        return response;
    }

    std::string s_path = save_audit_image(selfie_bytes, "selfies");
    save_audit_image(document_bytes, "documents");
    db_->log_audit({req_id, identifier, "success", sim, s_path});
    
    int label = static_cast<int>(std::chrono::system_clock::now().time_since_epoch().count() % 1000000);
    index_->add_face(label, feat_s);
    index_->save();

    response["status"] = "success";
    response["similarity"] = sim;
    response["request_id"] = req_id;

    return response;
}
