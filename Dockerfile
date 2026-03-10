# --- Estágio 1: Build ---
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    wget \
    unzip \
    pkg-config \
    libopencv-dev \
    libsqlite3-dev \
    uuid-dev \
    && rm -rf /var/lib/apt/lists/*

# 2. Baixar e instalar ONNX Runtime C++ (v1.20.1)
RUN wget https://github.com/microsoft/onnxruntime/releases/download/v1.20.1/onnxruntime-linux-x64-1.20.1.tgz -O /tmp/onnx.tgz \
    && tar -zxvf /tmp/onnx.tgz -C /opt \
    && mv /opt/onnxruntime-linux-x64-1.20.1 /opt/onnxruntime \
    && rm /tmp/onnx.tgz

# 3. Baixar e instalar InspireFace SDK v1.2.3
RUN wget https://github.com/HyperInspire/InspireFace/releases/download/v1.2.3/inspireface-linux-x86-manylinux2014-1.2.3.zip -O /tmp/inspireface.zip \
    && unzip /tmp/inspireface.zip -d /tmp/inspireface_sdk \
    && mkdir -p /opt/inspireface \
    && cp -r /tmp/inspireface_sdk/inspireface-linux-x86-manylinux2014-1.2.3/InspireFace/* /opt/inspireface/ \
    && rm -rf /tmp/inspireface.zip /tmp/inspireface_sdk

WORKDIR /build
COPY app/ /build/app/

RUN cd /build/app && \
    mkdir -p build && cd build && \
    cmake -DINSPIREFACE_DIR=/opt/inspireface -DONNX_DIR=/opt/onnxruntime .. && \
    make -j$(nproc) aletheia_edge

# --- Estágio 2: Runtime ---
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libopencv-core4.5d \
    libopencv-imgproc4.5d \
    libopencv-imgcodecs4.5d \
    libsqlite3-0 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copiar binário
COPY --from=builder /build/app/build/aletheia_edge .

# Copiar bibliotecas e configurar links simbólicos para ONNX
COPY --from=builder /opt/onnxruntime/lib/libonnxruntime.so.1.20.1 /usr/lib/
RUN ln -s /usr/lib/libonnxruntime.so.1.20.1 /usr/lib/libonnxruntime.so.1 && \
    ln -s /usr/lib/libonnxruntime.so.1 /usr/lib/libonnxruntime.so

# Copiar InspireFace
COPY --from=builder /opt/inspireface/lib/libInspireFace.so /usr/lib/libInspireFace.so

# Configura o sistema para reconhecer as novas libs no /usr/lib
RUN ldconfig

RUN mkdir -p storage/selfies storage/documents models

EXPOSE 8080

CMD ["./aletheia_edge"]
