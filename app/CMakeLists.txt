cmake_minimum_required(VERSION 3.10)
project(AletheiaEdge VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Reativar OpenCV (Instalado via pacman) - Selecionando apenas componentes necessários
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs)

include_directories(
    ${CMAKE_SOURCE_DIR}/third_party
    ${CMAKE_SOURCE_DIR}/third_party/hnswlib
    ${CMAKE_SOURCE_DIR}/third_party/inspireface/include
    ${CMAKE_SOURCE_DIR}/src
    ${OpenCV_INCLUDE_DIRS}
    /usr/include/onnxruntime
)

# Adicionar o diretório da lib do InspireFace
link_directories(${CMAKE_SOURCE_DIR}/third_party/inspireface/lib)

# Biblioteca central (Motor, DB, Biometria, Imagem e ONNX)
add_library(aletheia_core 
    src/Database.cpp 
    src/BiometricIndex.cpp 
    src/ImageProcessor.cpp
    src/OnnxClassifier.cpp
    src/Engine.cpp
)
target_link_libraries(aletheia_core sqlite3 ${OpenCV_LIBS} onnxruntime InspireFace)
# RPATH para a lib do InspireFace
set_target_properties(aletheia_core PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE INSTALL_RPATH "${CMAKE_SOURCE_DIR}/third_party/inspireface/lib")

# Testes
add_executable(test_database tests/test_main.cpp tests/test_database.cpp)
target_link_libraries(test_database aletheia_core pthread)

add_executable(test_biometrics tests/test_main.cpp tests/test_biometrics.cpp)
target_link_libraries(test_biometrics aletheia_core pthread)

add_executable(test_opencv tests/test_main.cpp tests/test_opencv.cpp)
target_link_libraries(test_opencv aletheia_core pthread)

add_executable(test_onnx tests/test_main.cpp tests/test_onnx.cpp)
target_link_libraries(test_onnx aletheia_core pthread)

add_executable(test_inspireface tests/test_main.cpp tests/test_inspireface.cpp)
target_link_libraries(test_inspireface aletheia_core pthread)

# EXECUTÁVEL FINAL (API)
add_executable(aletheia_edge src/main.cpp)
target_link_libraries(aletheia_edge aletheia_core pthread)
set_target_properties(aletheia_edge PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE INSTALL_RPATH "${CMAKE_SOURCE_DIR}/third_party/inspireface/lib")
set_target_properties(aletheia_edge PROPERTIES LINK_FLAGS "-Wl,-rpath,${CMAKE_SOURCE_DIR}/third_party/inspireface/lib")
