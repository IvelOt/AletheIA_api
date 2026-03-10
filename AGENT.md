# AGENT.md - Aletheia Edge API (C++ Implementation)

## 1. Project Vision
**Aletheia Edge** is a high-performance, standalone biometric verification engine written in C++. It is designed to be a "plug-and-play" Identity Verification (IDV) solution for third-party systems, optimized for edge computing and embedded environments. It eliminates heavy infrastructure (RabbitMQ, Postgres, Redis) in favor of a single, ultra-fast binary.

## 2. System Architecture
The system follows a **monolithic edge architecture**, where the web server, inference engine, and database reside within a single process.

### Components:
- **Web Server (`cpp-httplib`)**: Embedded HTTP server handling synchronous multipart/form-data requests.
- **Inference Engine**:
    - **Document Classifier**: ONNX Runtime (MobileNetV3) for Brazilian ID screening.
    - **Face Engine**: InspireFace SDK (C++) for detection, quality assessment, and feature extraction (512-D).
- **Vector Search (`HNSWLib`)**: Local, in-memory index for ultra-fast 1:N biometric deduplication.
- **Metadata/Audit (`SQLite3`)**: Local relational database for tracking requests, CPF/RG mapping, and audit trails.
- **Storage**: Local filesystem storage for encrypted/obfuscated audit images.

## 3. Technical Requirements

### Build Tools:
- **Compiler**: GCC 9+ or Clang (C++17 support required).
- **Build System**: CMake 3.10+.

### Core Dependencies:
1. **OpenCV 4.x**: Image decoding and preprocessing.
2. **ONNX Runtime (C++ API)**: For document classification models.
3. **InspireFace SDK**: Core biometric matching and face detection.
4. **SQLite3**: Audit logging and user metadata.
5. **HNSWLib**: High-performance local vector indexing.
6. **nlohmann/json**: Modern JSON for C++ communication.
7. **cpp-httplib**: Header-only HTTP/HTTPS server.

## 4. API Specification

### `POST /v1/verify` (1:1 Verification)
Compares a selfie against a document image without persistent enrollment.
- **Input**: `multipart/form-data` (`selfie`, `document`).
- **Output**: Similarity score and match status.

### `POST /v1/enroll` (1:N Enrollment + Audit)
Registers a user in the local biometric index and logs metadata.
- **Input**: `multipart/form-data` (`identifier` (CPF/RG), `selfie`, `document`).
- **Logic**:
    1. Validate Document (ONNX).
    2. Face Match 1:1 (Selfie vs Doc).
    3. Biometric Search 1:N (Check if face already exists).
    4. Save images to `storage/` and metadata to `aletheia.db`.
    5. Update HNSW index.
- **Output**: `status`, `request_id`, `similarity_score`.

## 5. File Structure
```text
app/
├── CMakeLists.txt          # Build configuration
├── aletheia.db             # SQLite Audit DB (Auto-generated)
├── faces.index             # HNSW Vector Index (Auto-generated)
├── models/                 # .onnx models and InspireFace data
├── storage/                # Audit image files
├── src/
│   ├── main.cpp            # HTTP Server & Endpoints
│   ├── Engine.cpp          # Core Logic (Biometrics & DB)
│   └── Engine.hpp          # Class definitions
└── third_party/            # Header-only libs (JSON, HTTP, HNSWLib)
```

## 6. Security & Privacy Standards
- **Local-First**: All biometric data stays on the device/server; no external cloud calls.
- **Audit Trail**: Every verification is logged with a timestamp and unique `request_id`.
- **Performance**: Targeted sub-500ms response time for full verification on standard CPUs.

## 7. Build Instructions
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
./aletheia_edge
```
