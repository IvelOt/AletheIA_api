#!/bin/bash

# Aletheia Edge Setup Script
# Use this script to download dependencies and models after a fresh clone.

set -e

PROJECT_ROOT=$(pwd)
CPP_ROOT="$PROJECT_ROOT/app"
THIRD_PARTY="$CPP_ROOT/third_party"
MODELS_DIR="$PROJECT_ROOT/models"

echo "[Aletheia] Initializing setup..."

# 1. Install System Dependencies (Arch Linux)
if [ -f /etc/arch-release ]; then
    echo "[Aletheia] Detected Arch Linux. Installing OpenCV and SQLite..."
    # Note: Requires sudo. If running in a CI/Server, ensure sudo is available or dependencies are pre-installed.
    sudo pacman -Sy --needed --noconfirm opencv sqlite onnxruntime
fi

# 2. Download InspireFace C++ SDK (lib + headers)
if [ ! -d "$THIRD_PARTY/inspireface" ]; then
    echo "[Aletheia] Downloading InspireFace C++ SDK v1.2.3..."
    mkdir -p "$THIRD_PARTY/inspireface"
    wget -O /tmp/inspireface.zip https://github.com/HyperInspire/InspireFace/releases/download/v1.2.3/inspireface-linux-x86-manylinux2014-1.2.3.zip
    unzip -o /tmp/inspireface.zip -d /tmp/inspireface_sdk
    find /tmp/inspireface_sdk -name "include" -type d -exec cp -r {} "$THIRD_PARTY/inspireface/" \;
    find /tmp/inspireface_sdk -name "lib" -type d -exec cp -r {} "$THIRD_PARTY/inspireface/" \;
    rm -rf /tmp/inspireface.zip /tmp/inspireface_sdk
    echo "[Aletheia] SDK installed in $THIRD_PARTY/inspireface"
fi

# 3. Handle Models
mkdir -p "$MODELS_DIR"

if [ ! -f "$MODELS_DIR/document_classifier.onnx" ]; then
    echo "[Aletheia] Warning: 'document_classifier.onnx' not found."
    echo "[Aletheia] Please place the document_classifier.onnx model in the 'models/' directory."
fi

# 4. Handle InspireFace (Pikachu) Model
INSPIRE_PATH="$MODELS_DIR/.inspireface/ms/tunmxy/InspireFace"
if [ ! -f "$INSPIRE_PATH/Pikachu" ]; then
    echo "[Aletheia] Setting up InspireFace Pikachu model directory..."
    mkdir -p "$INSPIRE_PATH"
    # Note: The model is usually very specific or private. 
    # If there is a public URL for the Pikachu model, add it here with wget.
    echo "[Aletheia] Warning: 'Pikachu' model not found in $INSPIRE_PATH."
    echo "[Aletheia] Please place the 'Pikachu' model file in the above directory."
fi

# 5. Build dependencies (JSON, HTTP, HNSWLib headers are already in third_party)
echo "[Aletheia] Building the project..."
cd "$CPP_ROOT"
mkdir -p build && cd build
cmake ..
make -j$(nproc)

echo "[Aletheia] Setup complete! You can now run './aletheia_edge' from the build directory."
