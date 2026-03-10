# Aletheia Edge API: High-Performance Biometric Verification (Stateless)

Aletheia Edge é um motor de verificação biométrica independente e ultra-rápido escrito em **C++**. Ele fornece um serviço de verificação de identidade (IDV) "plug-and-play" otimizado para documentos brasileiros (CNH e RG), projetado para rodar em dispositivos de borda ou como um microsserviço de alta performance.

---

## 🚀 Diferenciais

- **Stateless & Private**: Não armazena imagens, nomes ou bancos de dados. Processa tudo em RAM e esquece após a resposta.
- **Zero-Configuration**: Setup completo com um único comando via **Docker**.
- **Brazilian ID Screening**: Validação automática de integridade de documentos (CNH/RG) via ONNX.
- **High Precision**: Match facial 1:1 utilizando o **InspireFace SDK** (Modelo Pikachu).
- **Sub-500ms**: Resposta extremamente rápida para fluxos de onboarding síncronos.

---

## 🛠️ Início Rápido

### 1. Rodar a API
Clone o repositório e inicie o container:
```bash
docker compose up --build -d
```

### 2. Verificar Status
```bash
docker compose logs -f
```
A API estará disponível em `http://localhost:8080`.

---

## 📖 Documentação da API

### **Verificação de Identidade (1:1)**
Compara uma selfie contra um documento e retorna o resultado instantaneamente.

**Endpoint**: `POST /v1/verify`  
**Payload**: `multipart/form-data`

| Campo | Tipo | Descrição |
| :--- | :--- | :--- |
| `selfie` | Arquivo | Foto atual do rosto do usuário (JPEG/PNG) |
| `document` | Arquivo | Imagem do documento brasileiro (CNH ou RG) |

**Exemplo de Chamada**:
```bash
curl -X POST http://localhost:8080/v1/verify \
  -F "selfie=@sua_selfie.jpg" \
  -F "document=@seu_documento.jpg"
```

**Resposta de Sucesso**:
```json
{
  "status": "success",
  "similarity": 0.89,
  "transaction_id": "tx_1710000000"
}
```

---

## 🏗️ Estrutura de Pastas

- `app/src/`: Código fonte C++ (Engine, Image Processor, API).
- `models/`: Ativos de rede neural (ONNX e InspireFace).
- `setup.sh`: Script de auxílio para ambiente local.

---

## 📄 Detalhes Técnicos
Para informações sobre o pipeline de visão computacional, consulte o [ARCHITECTURE.md](./ARCHITECTURE.md).
