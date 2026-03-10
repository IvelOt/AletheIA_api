#!/bin/bash

# Aletheia Edge Benchmark Script
ENDPOINT="http://localhost:8080/v1/verify"
SELFIE="/home/levirenato/Faculdade/aletheia-api/Examples/foto_1.jpeg"
DOC="/home/levirenato/Faculdade/aletheia-api/Examples/identidade_1.jpg"
REQUESTS=50 # Total de requisições para o teste

echo "------------------------------------------------"
echo "   Iniciando Benchmark: Aletheia Edge (1 CPU)   "
echo "   Total de Requisições: $REQUESTS              "
echo "------------------------------------------------"

TOTAL_TIME=0
SUCCESS_COUNT=0

for ((i=1; i<=REQUESTS; i++)); do
    # Mede o tempo da requisição individual em segundos
    START_TIME=$(date +%s%N)
    
    RESPONSE=$(curl -s -X POST $ENDPOINT 
      -F "selfie=@$SELFIE" 
      -F "document=@$DOC")
    
    END_TIME=$(date +%s%N)
    
    # Verifica se a resposta contém sucesso ou mismatch (ambos são válidos)
    if [[ $RESPONSE == *"status"* ]]; then
        ((SUCCESS_COUNT++))
        
        # Calcula duração em milissegundos
        DURATION=$(( (END_TIME - START_TIME) / 1000000 ))
        TOTAL_TIME=$(( TOTAL_TIME + DURATION ))
        
        echo "Req $i: ${DURATION}ms"
    else
        echo "Req $i: FALHOU"
    fi
done

if [ $SUCCESS_COUNT -gt 0 ]; then
    AVG_TIME=$(( TOTAL_TIME / SUCCESS_COUNT ))
    echo "------------------------------------------------"
    echo "   RESULTADO FINAL                              "
    echo "   Sucessos: $SUCCESS_COUNT/$REQUESTS           "
    echo "   Tempo Médio: ${AVG_TIME}ms                   "
    echo "------------------------------------------------"
else
    echo "O teste falhou completamente."
fi
