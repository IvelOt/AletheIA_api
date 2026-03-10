cmake_minimum_required(VERSION 3.10)
project(AletheiaEdge VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 1. Encontrar OpenCV
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs)

# 2. Configurar caminhos do InspireFace (Suporta override via Docker ou CLI)
if(NOT INSPIREFACE_DIR)
    set(INSPIREFACE_DIR ${CMAKE_SOURCE_DIR}/third_party/inspireface)
endif()

# 3. Configurar caminhos do ONNX Runtime
if(NOT ONNX_DIR)
    set(ONNX_INCLUDE_DIR /usr/include/onnxruntime)
    set(ONNX_LIB onnxruntime)
else()
    set(ONNX_INCLUDE_DIR ${ONNX_DIR}/include)
    set(ONNX_LIB ${ONNX_DIR}/lib/libonnxruntime.so)
endif()

include_directories(
    ${CMAKE_SOURCE_DIR}/third_party
    ${CMAKE_SOURCE_DIR}/third_party/hnswlib
    ${INSPIREFACE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${OpenCV_INCLUDE_DIRS}
    ${ONNX_INCLUDE_DIR}
)

# Linkar bibliotecas
link_directories(${INSPIREFACE_DIR}/lib)

add_library(aletheia_core 
    src/Database.cpp 
    src/BiometricIndex.cpp 
    src/ImageProcessor.cpp
    src/OnnxClassifier.cpp
    src/Engine.cpp
)
target_link_libraries(aletheia_core sqlite3 ${OpenCV_LIBS} ${ONNX_LIB} InspireFace)

# Executável Final
add_executable(aletheia_edge src/main.cpp)
target_link_libraries(aletheia_edge aletheia_core pthread)

# Configurar RPATH para encontrar a .so no runtime (Nativo e Docker)
set_target_properties(aletheia_edge PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE INSTALL_RPATH "/usr/lib:/usr/local/lib:${INSPIREFACE_DIR}/lib")
if(UNIX AND NOT APPLE)
    set_target_properties(aletheia_edge PROPERTIES LINK_FLAGS "-Wl,-rpath,/usr/lib:/usr/local/lib:${INSPIREFACE_DIR}/lib")
endif()
