#ifndef BIOMETRIC_INDEX_HPP
#define BIOMETRIC_INDEX_HPP

#include <string>
#include <vector>
#include <tuple>
#include "hnswlib/hnswlib.h"

class BiometricIndex {
public:
    // dim = 512 para o ArcFace/InspireFace
    BiometricIndex(const std::string& index_path, int dim = 512, int max_elements = 10000);
    ~BiometricIndex();

    bool init();
    
    // Adiciona um novo rosto ao índice
    void add_face(int label_id, const std::vector<float>& feature);
    
    // Busca o rosto mais parecido. Retorna (Label_ID, Distância_Cosseno)
    std::tuple<int, float> search_face(const std::vector<float>& feature);

    // Salva no disco
    void save();

private:
    std::string index_path_;
    int dim_;
    int max_elements_;
    
    hnswlib::SpaceInterface<float>* space_ = nullptr;
    hnswlib::HierarchicalNSW<float>* alg_hnsw_ = nullptr;
};

#endif // BIOMETRIC_INDEX_HPP
