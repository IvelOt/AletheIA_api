# Aletheia Edge API: High-Performance Biometric Verification

Aletheia Edge is a standalone, ultra-fast biometric verification engine written in **C++**. It provides a "plug-and-play" Identity Verification (IDV) service optimized for Brazilian identity documents (CNH and RG), designed to run on edge devices or as a high-performance microservice.

---

## Key Features

- **Zero-Configuration Setup**: Run the entire stack with a single command via **Docker**.
- **Document Screening**: Uses a MobileNetV3 ONNX model to verify Brazilian IDs.
- **Biometric Matching (1:1)**: High-precision face matching using the **InspireFace SDK**.
- **Biometric Deduplication (1:N)**: Built-in **HNSWLib** index for ultra-fast local search.
- **Local Audit Log**: Integrated **SQLite3** database for compliance and tracking.
- **Cross-Platform**: Works on Windows, Linux, and macOS via containerization.

---

## Quick Start (The "Easy" Way)

The recommended way to run Aletheia Edge is using **Docker**, as it handles all C++ dependencies (OpenCV, ONNX, InspireFace) for you.

### 1. Prerequisites
- [Docker](https://www.docker.com/get-started) and [Docker Compose](https://docs.docker.com/compose/install/) installed.

### 2. Run the API
Clone the repository and run:
```bash
docker compose up --build -d
```

### 3. Verify it's running
The API will be available at `http://localhost:8080`. You can check the logs with:
```bash
docker compose logs -f
```

---

## Manual Installation (Native Build)

If you prefer to build the binary directly on your host machine (Linux only recommended):

1. Run the setup script: `bash setup.sh` and choose option **2 (Native)**.
2. Ensure you have the required system libraries: `opencv`, `sqlite3`, `onnxruntime`.
3. The binary will be located in `app/build/aletheia_edge`.

---

## API Documentation

### **1. Face Verification (1:1)**
Compare a selfie against a document.

**Endpoint**: `POST /v1/verify`  
**Payload**: `multipart/form-data`

| Field | Type | Description |
| :--- | :--- | :--- |
| `selfie` | File | User's live selfie (JPEG/PNG) |
| `document` | File | Brazilian ID image (CNH/RG) |

**Example**:
```bash
curl -X POST http://localhost:8080/v1/verify \
  -F "selfie=@selfie.jpg" \
  -F "document=@document.jpg"
```

### **2. User Enrollment (1:N + Audit)**
Validate, match, and register a user into the local biometric index.

**Endpoint**: `POST /v1/enroll`  
**Payload**: `multipart/form-data`

**Example**:
```bash
curl -X POST http://localhost:8080/v1/enroll \
  -F "identifier=123.456.789-00" \
  -F "selfie=@selfie.jpg" \
  -F "document=@document.jpg"
```

---

## Configuration

Settings are managed via the `.env` file. Key variables include:
- `SIMILARITY_THRESHOLD`: Required score for a match (default: 0.55).
- `QUALITY_THRESHOLD`: Minimum face quality (default: 0.45).

---

## Directory Structure

- `app/src/`: Core C++ source code.
- `models/`: ONNX and InspireFace model files.
- `storage/`: Audit images (mapped to container).
- `aletheia.db`: SQLite database file.

---

## License
Academic Project - Brazilian Identity Verification research.
