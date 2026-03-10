#include <iostream>
#include <memory>
#include "httplib.h"
#include "json.hpp"
#include "Engine.hpp"
#include "Config.hpp"

using json = nlohmann::json;

std::unique_ptr<AletheiaEngine> engine;

int main(int argc, char** argv) {
    // 1. Carregar Configuração
    Config& config = Config::getInstance();
    if (!config.load("../../.env")) {
        std::cerr << "[Aletheia Edge] Aviso: Arquivo .env não encontrado. Usando padrões." << std::endl;
    }

    // 2. Configurar caminhos e parâmetros do Engine
    std::string classifier_path = config.get("CLASSIFIER_PATH", "../../models/document_classifier.onnx");
    std::string isf_model_path = config.get("INSPIREFACE_MODEL_PATH", "../../models/.inspireface/ms/tunmxy/InspireFace/Pikachu");
    std::string db_path = config.get("DB_PATH", "aletheia.db");
    int port = config.getInt("PORT", 8080);
    std::string bind_addr = config.get("BIND_ADDR", "0.0.0.0");
    
    try {
        engine = std::make_unique<AletheiaEngine>(classifier_path, isf_model_path, db_path);
    } catch (const std::exception& e) {
        std::cerr << "[Aletheia Edge] ERRO FATAL ao iniciar motor: " << e.what() << std::endl;
        return 1;
    }

    httplib::Server svr;

    // Rota 1: Verificação 1:1
    svr.Post("/v1/verify", [](const httplib::Request& req, httplib::Response& res) {
        if (!req.is_multipart_form_data()) {
            res.status = 400;
            res.set_content("{\"status\":\"error\",\"message\":\"Multipart expected\"}", "application/json");
            return;
        }

        std::string s_bytes, d_bytes;
        if (req.form.has_file("selfie")) s_bytes = req.form.get_file("selfie").content;
        if (req.form.has_file("document")) d_bytes = req.form.get_file("document").content;

        if (s_bytes.empty() || d_bytes.empty()) {
            res.status = 400;
            res.set_content("{\"status\":\"error\",\"message\":\"Missing selfie or document\"}", "application/json");
            return;
        }

        json result = engine->verify_images(s_bytes, d_bytes);
        res.set_content(result.dump(), "application/json");
    });

    // Rota 2: Cadastro 1:N
    svr.Post("/v1/enroll", [](const httplib::Request& req, httplib::Response& res) {
        if (!req.is_multipart_form_data()) {
            res.status = 400;
            return;
        }

        std::string id, s_bytes, d_bytes;
        if (req.form.has_field("identifier")) id = req.form.get_field("identifier");
        else if (req.form.has_file("identifier")) id = req.form.get_file("identifier").content;
        
        if (req.form.has_file("selfie")) s_bytes = req.form.get_file("selfie").content;
        if (req.form.has_file("document")) d_bytes = req.form.get_file("document").content;

        if (id.empty() || s_bytes.empty() || d_bytes.empty()) {
            res.status = 400;
            res.set_content("{\"status\":\"error\",\"message\":\"Missing identifier, selfie or document\"}", "application/json");
            return;
        }

        json result = engine->enroll_user(id, s_bytes, d_bytes);
        res.set_content(result.dump(), "application/json");
    });

    std::cout << "[Aletheia Edge] API pronta em http://" << bind_addr << ":" << port << std::endl;
    svr.listen(bind_addr.c_str(), port);

    return 0;
}
