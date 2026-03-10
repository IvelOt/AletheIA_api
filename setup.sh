#!/bin/bash
# Aletheia Edge - Ultra Fast Setup

set -e

echo "------------------------------------------"
echo "   Aletheia Edge Biometric API Setup      "
echo "------------------------------------------"

# 1. Verificar Modelos
if [ ! -f "models/document_classifier.onnx" ] || [ ! -f "models/.inspireface/ms/tunmxy/InspireFace/Pikachu" ]; then
    echo "[!] AVISO: Modelos neurais não detectados na pasta models/."
    echo "Certifique-se de que os arquivos 'document_classifier.onnx' e 'Pikachu' estão nos locais corretos."
    exit 1
fi

# 2. Criar arquivos de persistência se não existirem (evita erro de permissão do Docker)
touch aletheia.db faces.index
chmod 666 aletheia.db faces.index

# 3. Disparar Docker
echo "[+] Iniciando containers..."
if command -v docker-compose &> /dev/null; then
    docker-compose up --build
else
    docker compose up --build
fi
