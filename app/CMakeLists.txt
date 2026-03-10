cmake_minimum_required(VERSION 3.10)
project(AletheiaEdge VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs)

if(NOT INSPIREFACE_DIR)
    set(INSPIREFACE_DIR ${CMAKE_SOURCE_DIR}/third_party/inspireface)
endif()

if(NOT ONNX_DIR)
    set(ONNX_INCLUDE_DIR /usr/include/onnxruntime)
    set(ONNX_LIB onnxruntime)
else()
    set(ONNX_INCLUDE_DIR ${ONNX_DIR}/include)
    set(ONNX_LIB ${ONNX_DIR}/lib/libonnxruntime.so)
endif()

include_directories(
    ${CMAKE_SOURCE_DIR}/third_party
    ${INSPIREFACE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${OpenCV_INCLUDE_DIRS}
    ${ONNX_INCLUDE_DIR}
)

link_directories(${INSPIREFACE_DIR}/lib)

# Biblioteca central Stateless
add_library(aletheia_core 
    src/ImageProcessor.cpp
    src/OnnxClassifier.cpp
    src/Engine.cpp
)
target_link_libraries(aletheia_core ${OpenCV_LIBS} ${ONNX_LIB} InspireFace)

# Executável Final
add_executable(aletheia_edge src/main.cpp)
target_link_libraries(aletheia_edge aletheia_core pthread)

# --- Suíte de Testes TDD ---
macro(add_aletheia_test name src)
    add_executable(${name} tests/test_main.cpp ${src})
    target_link_libraries(${name} aletheia_core pthread)
    set_target_properties(${name} PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE INSTALL_RPATH "/usr/lib:/usr/local/lib:${INSPIREFACE_DIR}/lib")
    if(UNIX AND NOT APPLE)
        set_target_properties(${name} PROPERTIES LINK_FLAGS "-Wl,-rpath,/usr/lib:/usr/local/lib:${INSPIREFACE_DIR}/lib")
    endif()
endmacro()

add_aletheia_test(test_opencv tests/test_opencv.cpp)
add_aletheia_test(test_onnx tests/test_onnx.cpp)
add_aletheia_test(test_inspireface tests/test_inspireface.cpp)
add_aletheia_test(test_engine tests/test_engine.cpp)

set_target_properties(aletheia_edge PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE INSTALL_RPATH "/usr/lib:/usr/local/lib:${INSPIREFACE_DIR}/lib")
if(UNIX AND NOT APPLE)
    set_target_properties(aletheia_edge PROPERTIES LINK_FLAGS "-Wl,-rpath,/usr/lib:/usr/local/lib:${INSPIREFACE_DIR}/lib")
endif()
