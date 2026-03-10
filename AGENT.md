# AGENT.md - Aletheia Edge API (Dockerized C++ Architecture)

## 1. Project Vision
**Aletheia Edge** is a professional-grade biometric engine. By moving to a **Dockerized Monolithic C++** architecture, we ensure that the high-performance C++ code is easily deployable across different environments (Dev, Cloud, and high-end Edge) with zero dependency friction for the end user.

## 2. Updated Architecture
The system is now distributed as a containerized microservice.

### Components:
- **Container Host**: Any OS with Docker support.
- **Service Stack**:
    - **Base Image**: Ubuntu 22.04 (LTS).
    - **Core Engine**: Compiled C++ binary linking OpenCV, ONNX Runtime, and InspireFace SDK.
    - **Persistent Layer**: Docker Volumes for `aletheia.db` (SQLite), `faces.index` (HNSW), and `storage/` (Images).

## 3. Build & Deployment Logic
We use **Multi-stage Docker Builds**:
1.  **Stage 1 (Builder)**: Installs `build-essential`, `cmake`, and downloads/configures the binary SDKs. Compiles the binary.
2.  **Stage 2 (Runtime)**: Copies only the final binary and the required `.so` (shared objects), resulting in a lean, production-ready image.

## 4. Portability Strategy
- **x86_64**: Current default for Desktop and Server.
- **ARM64 (Raspberry Pi/Jetson)**: The Dockerfile can be adapted to use the ARM64 versions of ONNX and InspireFace, enabling identical deployment logic on powerful Edge IoT devices.

## 5. Development Workflow
To add a new feature:
1.  Modify code in `app/src/`.
2.  Run `docker compose up --build`.
3.  Test via `curl` or automated tools against `localhost:8080`.

## 6. Security
- **Isolation**: The C++ engine runs in a restricted container namespace.
- **Data Persistence**: Sensitive biometric indices and audit databases are stored in host-mapped volumes, allowing for easy backups and secure handling.
