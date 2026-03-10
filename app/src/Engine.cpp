#include "Engine.hpp"
#include <iostream>
#include <chrono>
#include <cmath>
#include <cstring>

AletheiaEngine::AletheiaEngine(const std::string& classifier_model_path, 
                               const std::string& inspireface_model_path) {
    
    std::cout << "[Aletheia Engine] Inicializando Motor Paralelo..." << std::endl;
    classifier_ = std::make_unique<OnnxClassifier>(classifier_model_path);
    init_inspireface(inspireface_model_path);
}

AletheiaEngine::~AletheiaEngine() {
    if (session_selfie_) HFReleaseInspireFaceSession(session_selfie_);
    if (session_doc_) HFReleaseInspireFaceSession(session_doc_);
}

HFSession AletheiaEngine::create_session() {
    HFSession session = nullptr;
    HFSessionCustomParameter param;
    std::memset(&param, 0, sizeof(param));
    param.enable_recognition = 1;
    param.enable_face_quality = 1;
    
    if (HFCreateInspireFaceSession(param, HF_DETECT_MODE_ALWAYS_DETECT, 1, 160, -1, &session) != HSUCCEED) {
        throw std::runtime_error("InspireFace: Session creation failed.");
    }
    return session;
}

void AletheiaEngine::init_inspireface(const std::string& model_path) {
    if (HFLaunchInspireFace(model_path.c_str()) != HSUCCEED) {
        throw std::runtime_error("InspireFace: Launch failed.");
    }
    
    // Cria duas sessões para processamento paralelo
    session_selfie_ = create_session();
    session_doc_ = create_session();
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

FaceExtractionResult AletheiaEngine::extract_feature_parallel(const cv::Mat& img, HFSession session) {
    FaceExtractionResult result;
    if (img.empty()) return result;

    for (int angle : {0, 90, 180, 270}) {
        cv::Mat rotated = ImageProcessor::rotate(img, angle);
        HFImageStream stream = create_image_stream(rotated);
        HFMultipleFaceData faces;
        
        if (HFExecuteFaceTrack(session, stream, &faces) == HSUCCEED && faces.detectedNum > 0) {
            HFFaceBasicToken token = faces.tokens[0];
            float quality = 0.0f;
            
            if (HFFaceQualityDetect(session, token, &quality) == HSUCCEED && quality >= quality_threshold_) {
                HFFaceFeature feature_data;
                if (HFFaceFeatureExtract(session, stream, token, &feature_data) == HSUCCEED) {
                    result.feature.assign(feature_data.data, feature_data.data + 512);
                    
                    // Normalização L2
                    float norm = 0.0f;
                    for (float v : result.feature) norm += v * v;
                    norm = std::sqrt(norm);
                    if (norm > 0) for (float& v : result.feature) v /= norm;
                    
                    result.quality = quality;
                    result.success = true;
                    HFReleaseImageStream(stream);
                    return result;
                }
            }
        }
        HFReleaseImageStream(stream);
    }
    return result;
}

json AletheiaEngine::verify_images(const std::string& selfie_bytes, const std::string& document_bytes) {
    json response;
    auto start_total = std::chrono::steady_clock::now();
    
    std::string transaction_id = "tx_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    response["transaction_id"] = transaction_id;

    // --- PARALELISMO START ---
    
    // 1. Decodificação em paralelo
    auto future_selfie_img = std::async(std::launch::async, ImageProcessor::decode, std::cref(selfie_bytes));
    auto future_doc_img = std::async(std::launch::async, ImageProcessor::decode, std::cref(document_bytes));

    cv::Mat selfie_img = future_selfie_img.get();
    cv::Mat doc_img = future_doc_img.get();

    if (selfie_img.empty() || doc_img.empty()) {
        response["status"] = "error";
        response["message"] = "Invalid images.";
        return response;
    }

    // 2. Extração em paralelo (A tarefa mais pesada)
    // Enquanto o doc é extraído, também rodamos o ONNX no mesmo thread do documento
    auto task_doc = std::async(std::launch::async, [&]() {
        FaceExtractionResult res;
        // Valida documento via ONNX (Thread-Safe)
        if (classifier_->predict(ImageProcessor::prepare_for_classifier(doc_img)) < 0.8f) {
            return res; // success = false
        }
        return extract_feature_parallel(doc_img, session_doc_);
    });

    auto task_selfie = std::async(std::launch::async, [&]() {
        return extract_feature_parallel(selfie_img, session_selfie_);
    });

    // 3. Sincronização (Join)
    FaceExtractionResult res_doc = task_doc.get();
    FaceExtractionResult res_selfie = task_selfie.get();

    if (!res_doc.success || !res_selfie.success) {
        response["status"] = "error";
        response["message"] = "Document invalid or face not detected.";
        return response;
    }

    // 4. Comparação final
    float sim = 0.0f;
    for (int i = 0; i < 512; ++i) sim += res_doc.feature[i] * res_selfie.feature[i];
    
    auto end_total = std::chrono::steady_clock::now();
    long long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_total - start_total).count();

    response["status"] = (sim >= similarity_threshold_) ? "success" : "mismatch";
    response["similarity"] = sim;
    response["processing_time_ms"] = elapsed;
    
    return response;
}
