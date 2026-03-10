# Aletheia Edge API: High-Performance Biometric Verification

Aletheia Edge is a standalone, ultra-fast biometric verification engine written in **C++**. It provides a "plug-and-play" Identity Verification (IDV) service optimized for Brazilian identity documents (CNH and RG), designed to run on edge devices or as a high-performance microservice.

---

## Key Features

- **Document Screening**: Uses a MobileNetV3 ONNX model to verify if the uploaded image is a valid Brazilian ID.
- **Biometric Matching (1:1)**: High-precision face matching between a document and a selfie using the **InspireFace SDK**.
- **Biometric Deduplication (1:N)**: Built-in **HNSWLib** index for ultra-fast local search (finding if a face is already enrolled in milliseconds).
- **Local Audit Log**: Integrated **SQLite3** database for tracking request metadata, CPF/RG mapping, and compliance.
- **Zero-Infrastructure**: No need for external databases or message queues. Everything is contained within a single binary.
- **In-Memory Performance**: Image decoding and neural inference happen entirely in RAM for sub-500ms response times.

---

## Project Architecture

```text
[Client] -> [HTTP API (C++)] -> [OpenCV Image Processing]
                                      |
              +-----------------------+-----------------------+
              |                       |                       |
      [ONNX Classifier]      [InspireFace SDK]        [HNSWLib Index]
      (ID Screening)         (Face Match/Quality)     (1:N Deduplication)
                                      |
                              [SQLite3 Audit DB]
```

---

## Quick Start

### 1. Prerequisites (Arch Linux)
The system is optimized for Arch Linux. Ensure you have a C++17 compiler and CMake installed.

### 2. Installation & Setup
Clone the repository and run the automated setup script. This script installs system dependencies (OpenCV, SQLite, ONNX Runtime), downloads the InspireFace C++ SDK, and builds the project.

```bash
git clone <your-repo-url>
cd aletheia-api
bash setup.sh
```

### 3. Running the API
After the setup is complete, you can start the server from the build directory:

```bash
cd app/build
./aletheia_edge
```
The API will be available at `http://0.0.0.0:8080`.

---

## API Documentation

### **1. Face Verification (1:1)**
Compare a selfie against a document to verify identity.

**Endpoint**: `POST /v1/verify`  
**Payload**: `multipart/form-data`

| Field | Type | Description |
| :--- | :--- | :--- |
| `selfie` | File | User's live selfie (JPEG/PNG) |
| `document` | File | Brazilian ID image (CNH/RG) |

**Example**:
```bash
curl -X POST http://localhost:8080/v1/verify 
  -F "selfie=@selfie.jpg" 
  -F "document=@document.jpg"
```

### **2. User Enrollment (1:N + Audit)**
Validate, match, and register a user into the local biometric index.

**Endpoint**: `POST /v1/enroll`  
**Payload**: `multipart/form-data`

| Field | Type | Description |
| :--- | :--- | :--- |
| `identifier` | Text | User's CPF or RG |
| `selfie` | File | User's live selfie |
| `document` | File | Brazilian ID image |

**Example**:
```bash
curl -X POST http://localhost:8080/v1/enroll 
  -F "identifier=123.456.789-00" 
  -F "selfie=@selfie.jpg" 
  -F "document=@document.jpg"
```

---

## Configuration

You can tune the system behavior using the `.env` file:

- `PORT`: API port (default: 8080).
- `SIMILARITY_THRESHOLD`: Cosine similarity required for a match (default: 0.55).
- `QUALITY_THRESHOLD`: Minimum face quality required (default: 0.45).
- `DB_PATH`: Path to the SQLite audit database.

---

## Directory Structure

- `app/src/`: Core C++ source code (Engine, Database, Biometrics).
- `app/third_party/`: Header-only and shared libraries (httplib, json, hnswlib, inspireface).
- `models/`: ONNX and InspireFace neural network models.
- `storage/`: Local directory where audit images are saved.
- `Examples/`: Test images for verification.

---

## License
Academic Project - Developed for Brazilian Identity Verification research.
