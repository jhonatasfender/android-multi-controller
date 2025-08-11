# Sistema de Alocação Dinâmica de Portas

## Visão Geral

O sistema de alocação dinâmica de portas resolve o problema de ter portas hard-coded para dispositivos específicos, permitindo que qualquer número de dispositivos Android se conecte automaticamente com portas alocadas dinamicamente.

## Implementação

### Estrutura de Dados

```cpp
// Mapa que associa deviceId -> porta alocada
QMap<QString, quint16> m_devicePortMap;

// Pool de portas disponíveis
QSet<quint16> m_availablePorts;

// Mutex para thread safety
QMutex m_portMutex;

// Faixa de portas disponíveis
static const quint16 PORT_RANGE_START = 8080;
static const quint16 PORT_RANGE_END = 8100;
```

### Métodos Principais

#### `initializePortPool()`
- Inicializa o pool de portas disponíveis (8080-8100)
- Limpa mapeamentos anteriores
- Chamado no construtor da classe

#### `allocatePortForDevice(const QString& deviceId)`
- Aloca uma porta para um dispositivo específico
- Verifica se o dispositivo já tem uma porta alocada
- Remove a porta do pool de disponíveis
- Retorna 0 se não há portas disponíveis

#### `releasePortForDevice(const QString& deviceId)`
- Libera a porta quando um dispositivo é desconectado
- Retorna a porta para o pool de disponíveis
- Chamado automaticamente quando streaming é parado

#### `getDevicePort(const QString& deviceId)`
- Método principal que retorna a porta para um dispositivo
- Verifica se já existe uma porta alocada
- Aloca uma nova porta se necessário
- Usa fallback para porta 8080 em caso de erro

### Características

#### **Escalabilidade**
- Suporta até 21 dispositivos simultâneos (8080-8100)
- Facilmente extensível alterando `PORT_RANGE_END`

#### **Thread Safety**
- Todas as operações são protegidas por `QMutex`
- Uso de `QMutexLocker` para RAII

#### **Gerenciamento Automático**
- Portas são liberadas automaticamente quando streaming para
- Reutilização de portas quando dispositivos se desconectam

#### **Logging Detalhado**
- Rastreamento completo de alocação/liberação de portas
- Informações de debug para monitoramento

## Exemplo de Uso

```cpp
// Dispositivo se conecta
QString deviceId = "0057878038";
quint16 port = getDevicePort(deviceId);  // Retorna 8080

// Segundo dispositivo se conecta
QString deviceId2 = "06066330CP000019";
quint16 port2 = getDevicePort(deviceId2); // Retorna 8081

// Primeiro dispositivo se desconecta
releasePortForDevice(deviceId);  // Porta 8080 fica disponível novamente
```

## Logs de Exemplo

```
Initialized port pool with 21 ports
Port range: 8080 to 8100
Allocated port 8080 for device 0057878038
Remaining available ports: 20
Allocated port 8081 for device 06066330CP000019
Remaining available ports: 19
Released port 8080 from device 0057878038
Available ports: 20
```

## Benefícios

1. **Flexibilidade**: Funciona com qualquer número de dispositivos
2. **Eficiência**: Reutiliza portas automaticamente
3. **Robustez**: Tratamento de erros e fallbacks
4. **Manutenibilidade**: Código limpo e bem documentado
5. **Escalabilidade**: Fácil ajuste da faixa de portas

## Configuração

Para alterar a faixa de portas, modifique as constantes:

```cpp
static const quint16 PORT_RANGE_START = 8080;
static const quint16 PORT_RANGE_END = 8150;  // Exemplo: 71 portas
```

## Integração com Port Forwarding

O sistema funciona perfeitamente com o port forwarding do ADB:

```bash
# O sistema aloca automaticamente:
# Dispositivo 1: porta 8080
# Dispositivo 2: porta 8081
# Dispositivo 3: porta 8082
# etc.

adb -s 0057878038 forward tcp:8080 tcp:8080
adb -s 06066330CP000019 forward tcp:8081 tcp:8080
adb -s RX8X2002L2X forward tcp:8082 tcp:8080
```

## Considerações Técnicas

- **Capacity**: 21 dispositivos simultâneos por padrão
- **Performance**: Operações O(1) para alocação/liberação
- **Memory**: Uso mínimo de memória com QSet e QMap
- **Thread Safety**: Totalmente thread-safe
- **Error Handling**: Graceful degradation com fallback

---

**Status**: ✅ Implementado e funcional
**Versão**: 1.0
**Data**: $(date) 