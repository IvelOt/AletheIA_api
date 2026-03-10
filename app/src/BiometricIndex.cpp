#include "BiometricIndex.hpp"
#include <iostream>
#include <fstream>

BiometricIndex::BiometricIndex(const std::string& index_path, int dim, int max_elements) 
    : index_path_(index_path), dim_(dim), max_elements_(max_elements) {
    // Usaremos similaridade de Cosseno (Padrão para Biometria Facial)
    space_ = new hnswlib::InnerProductSpace(dim_);
}

BiometricIndex::~BiometricIndex() {
    delete alg_hnsw_;
    delete space_;
}

bool BiometricIndex::init() {
    std::ifstream f(index_path_.c_str(), std::ios::binary | std::ios::ate);
    if (f.good() && f.tellg() > 0) {
        // Arquivo existe e não está vazio, carrega do disco
        f.close();
        alg_hnsw_ = new hnswlib::HierarchicalNSW<float>(space_, index_path_, false, max_elements_);
        return true;
    } else {
        // Arquivo não existe ou está vazio, cria um novo índice
        alg_hnsw_ = new hnswlib::HierarchicalNSW<float>(space_, max_elements_, 16, 200);
        return true;
    }
}

void BiometricIndex::add_face(int label_id, const std::vector<float>& feature) {
    if (feature.size() != dim_) return;
    
    // Como usamos InnerProduct, para emular Cosine Similarity, os vetores devem ser normalizados
    // (Assumimos que o Engine.cpp já passa os vetores normalizados)
    alg_hnsw_->addPoint(feature.data(), label_id);
}

std::tuple<int, float> BiometricIndex::search_face(const std::vector<float>& feature) {
    if (!alg_hnsw_ || alg_hnsw_->cur_element_count == 0) {
        return {-1, 0.0f}; // Vazio
    }

    // Busca o vizinho mais próximo (k=1)
    auto result = alg_hnsw_->searchKnn(feature.data(), 1);
    
    if (result.empty()) {
        return {-1, 0.0f};
    }

    // result.top() retorna um pair<Distance, Label>
    float distance = result.top().first;
    int label = result.top().second;

    // Distância no HNSWLib para InnerProduct é: 1.0 - (A dot B)
    // Então, Similarity = 1.0 - distance
    float similarity = 1.0f - distance;

    return {label, similarity};
}

void BiometricIndex::save() {
    if (alg_hnsw_) {
        alg_hnsw_->saveIndex(index_path_);
    }
}
