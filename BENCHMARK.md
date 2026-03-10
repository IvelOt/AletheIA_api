# Performance Benchmark - Aletheia Edge API

Este documento registra a evolução da performance do motor síncrono C++ após a implementação de paralelismo e otimizações de recursos.

## Ambiente de Teste
- **Processador**: Intel Iris Xe Graphics (Host) / Docker Resource Limits (Container).
- **Memória RAM**: 1GB.
- **Sistema Operacional**: Ubuntu 22.04 (Container).
- **Imagens**: Selfie (JPEG) e RG Brasileiro (JPEG).

## Comparativo: Sequencial vs. Paralelo

A tabela abaixo compara o motor inicial (processamento um após o outro) com a **Fase A** (processamento simultâneo de Selfie e Documento).

| Métrica | Motor Sequencial (1 CPU) | Motor Paralelo (2 CPUs) | Ganho / Mudança |
| :--- | :---: | :---: | :---: |
| **Latência Média da API** | 11ms | 10ms | -9% (Overhead de Rede) |
| **Tempo de Processamento Interno** | ~320ms* | **167ms** | **~48% mais rápido** |
| **Throughput Máximo** | 1.93 req/sec | 2.10 req/sec | +8.8% |
| **Uso de CPU** | 1 Core | 2 Cores | Dobro de núcleos ativos |
| **Thread Safety** | Single Session | Multi-Session | Arquitetura Paralela |

*\*Valor estimado com base no tempo de inferência síncrona total dos modelos.*

## Análise Técnica dos Ganhos

### 1. Latência de Ponta a Ponta (Fase A)
A implementação de `std::async` permitiu que a decodificação das imagens e a extração de embeddings ocorressem simultaneamente. O tempo real que o usuário espera (Internal Processing) caiu praticamente pela metade, aproximando-se do tempo da inferência mais lenta entre os dois processos.

### 2. Eficiência da IA
- O thread que processa o **Documento** agora executa o classificador ONNX e a extração InspireFace em paralelo ao thread da **Selfie**.
- A CPU Iris Xe conseguiu lidar com a carga de 2 threads de inferência sem estrangulamento térmico imediato.

## Próximos Passos (Roadmap de Performance)
- **Fase B (Vulkan)**: Migrar a inferência pesada (ONNX/MNN) da CPU para a GPU integrada Iris Xe via Vulkan Execution Provider. Expectativa de aumentar o throughput em 3x e liberar a CPU para outras tarefas.
