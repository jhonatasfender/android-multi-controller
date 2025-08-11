# Migração para Streaming Nativo - Arquitetura Similar ao Scrcpy

## Visão Geral

Esta proposta detalha a migração do sistema atual (captura de screenshots via ADB) para uma arquitetura de streaming nativo similar ao scrcpy, implementando um servidor C++ nativo no Android que faz streaming H.264 direto para a aplicação desktop.

## Arquitetura Atual vs Nova

### Atual (ADB Screenshots)
```
Desktop App (Qt) → ADB Command → Android Device → Screenshot PNG → Desktop App
```
- **Latência**: 500-1000ms
- **FPS**: 1-10 FPS
- **Qualidade**: PNG estático
- **Dependências**: ADB ativo

### Nova (Streaming Nativo)
```
Desktop App (Qt) ← TCP Socket ← Android Native Server (C++) ← MediaCodec H.264 ← Screen Capture
```
- **Latência**: <100ms
- **FPS**: 30-60 FPS
- **Qualidade**: H.264 streaming
- **Dependências**: Servidor nativo deployado uma vez

## Componentes Principais

### 1. Servidor Android Nativo (C++/NDK)

**Localização**: `android_server/`

**Componentes**:
- **Screen Capture**: MediaProjection API ou SurfaceFlinger (root)
- **Video Encoder**: MediaCodec H.264 hardware acceleration
- **Audio Capture**: AudioRecord para streaming de áudio
- **TCP Server**: Socket server para comunicação
- **Input Injection**: uinput ou accessibility para controle
- **Protocol Handler**: Implementação do protocolo de comunicação

**Estrutura**:
```
android_server/
├── src/
│   ├── main.cpp                 # Entry point do servidor
│   ├── screen_capture/
│   │   ├── media_projection_capture.cpp
│   │   └── surface_flinger_capture.cpp
│   ├── video_encoder/
│   │   ├── mediacodec_h264_encoder.cpp
│   │   └── encoder_config.cpp
│   ├── audio_capture/
│   │   └── audio_record_capture.cpp
│   ├── network/
│   │   ├── tcp_server.cpp
│   │   └── socket_handler.cpp
│   ├── input/
│   │   ├── input_injector.cpp
│   │   └── touch_handler.cpp
│   └── protocol/
│       ├── protocol_handler.cpp
│       ├── packet_builder.cpp
│       └── metadata_builder.cpp
├── jni/
│   └── android_jni_bridge.cpp
├── CMakeLists.txt
├── Application.mk
└── Android.mk
```

### 2. Protocolo de Comunicação

**Inspirado no protocolo scrcpy**:

**Packet Types**:
```cpp
enum class PacketType : uint8_t {
    METADATA = 0x01,        // Device info, resolution, codec
    VIDEO_CONFIG = 0x02,    // H.264 SPS/PPS configuration
    VIDEO_DATA = 0x03,      // H.264 frame data
    AUDIO_CONFIG = 0x04,    // AAC configuration
    AUDIO_DATA = 0x05,      // AAC frame data
    CONTROL_EVENT = 0x06,   // Input events (touch/keyboard)
    HEARTBEAT = 0x07        // Keep-alive
};
```

**Estrutura de Pacotes**:
```cpp
struct PacketHeader {
    uint32_t magic;         // 0x53435250 ("SCRP")
    PacketType type;
    uint32_t length;
    uint64_t timestamp;
};

struct VideoDataPacket {
    PacketHeader header;
    bool keyframe;
    uint64_t pts;
    uint32_t data_size;
    uint8_t data[];
};

struct MetadataPacket {
    PacketHeader header;
    char device_name[256];
    uint32_t screen_width;
    uint32_t screen_height;
    uint32_t video_codec;   // H.264 = 1
    uint32_t audio_codec;   // AAC = 1
    uint32_t bitrate;
    uint32_t fps;
};
```

### 3. Cliente Desktop (Qt/FFmpeg)

**Localização**: `src/infrastructure/streaming/`

**Componentes**:
- **TCP Client**: Conexão com servidor Android
- **H.264 Decoder**: FFmpeg para decodificação
- **Video Renderer**: Qt/QML rendering
- **Audio Player**: Qt Multimedia para playback
- **Input Handler**: Envio de eventos touch/keyboard

**Estrutura**:
```cpp
class NativeStreamingService : public QObject {
    Q_OBJECT
    
public:
    bool connectToDevice(const QString& deviceId, const QString& serverAddress);
    void disconnectFromDevice(const QString& deviceId);
    bool sendTouchEvent(const QString& deviceId, int x, int y, int action);
    
signals:
    void frameReceived(const QString& deviceId, const QImage& frame);
    void audioReceived(const QString& deviceId, const QByteArray& audioData);
    void connectionEstablished(const QString& deviceId);
    void connectionLost(const QString& deviceId);
    
private slots:
    void onDataReceived();
    void onConnectionError();
    
private:
    QMap<QString, std::unique_ptr<TcpClient>> m_clients;
    QMap<QString, std::unique_ptr<H264Decoder>> m_decoders;
    QMap<QString, std::unique_ptr<ProtocolHandler>> m_protocolHandlers;
};
```

### 4. Sistema de Deployment

**Localização**: `src/infrastructure/deployment/`

**Funcionalidades**:
- **ADB Push**: Deploy automático do servidor nativo
- **Permission Setup**: Configuração de permissões necessárias
- **Auto-start**: Inicialização automática do servidor
- **Port Management**: Gerenciamento de portas TCP
- **Compatibility Check**: Verificação de suporte do dispositivo

```cpp
class ServerDeploymentService : public QObject {
    Q_OBJECT
    
public:
    bool deployServerToDevice(const QString& deviceId);
    bool startServerOnDevice(const QString& deviceId, int port = 8080);
    bool stopServerOnDevice(const QString& deviceId);
    bool isServerRunning(const QString& deviceId);
    
private:
    bool pushServerBinary(const QString& deviceId);
    bool setupPermissions(const QString& deviceId);
    bool checkCompatibility(const QString& deviceId);
};
```

## Plano de Implementação

### Fase 1: Servidor Android Básico
- [ ] Setup do projeto NDK
- [ ] Implementar captura de tela via MediaProjection
- [ ] Configurar MediaCodec H.264 encoder
- [ ] Criar servidor TCP básico
- [ ] Testar streaming H.264 simples

### Fase 2: Protocolo de Comunicação  
- [ ] Implementar estrutura de pacotes
- [ ] Protocolo de handshake
- [ ] Metadados de dispositivo
- [ ] Configuração de codec
- [ ] Controle de qualidade adaptativo

### Fase 3: Cliente Desktop
- [ ] Integração FFmpeg para H.264
- [ ] Cliente TCP para recepção
- [ ] Renderização em tempo real
- [ ] Sincronização de áudio/vídeo
- [ ] Interface Qt/QML adaptada

### Fase 4: Controle e Input
- [ ] Injeção de eventos touch
- [ ] Suporte a teclado
- [ ] Gestos multi-touch
- [ ] Controle de zoom/rotação
- [ ] Latência otimizada

### Fase 5: Recursos Avançados
- [ ] Streaming de áudio AAC
- [ ] Gravação de sessão
- [ ] Controle de múltiplos dispositivos
- [ ] Qualidade adaptativa
- [ ] Compressão otimizada

## Benefícios da Nova Arquitetura

### Performance
- **Latência**: Redução de 500-1000ms para <100ms
- **FPS**: Aumento de 1-10 FPS para 30-60 FPS
- **Qualidade**: H.264 vs PNG estático
- **Eficiência**: Hardware acceleration via MediaCodec

### Recursos
- **Streaming contínuo**: Vídeo fluido em tempo real
- **Controle responsivo**: Input com baixa latência
- **Áudio**: Suporte a streaming de áudio
- **Escalabilidade**: Múltiplos dispositivos simultaneamente

### Manutenibilidade
- **Menos dependência ADB**: Servidor independente
- **Protocolo padrão**: Similar ao scrcpy
- **Modular**: Componentes bem separados
- **Testável**: Cada componente isolado

## Considerações Técnicas

### Compatibilidade
- **Android 5.0+**: MediaProjection API
- **Android 9.0+**: Recomendado para melhor performance
- **Root**: Opcional para SurfaceFlinger (melhor performance)
- **Hardware**: Encoder H.264 necessário

### Segurança
- **Permissões**: RECORD_AUDIO, CAPTURE_VIDEO_OUTPUT
- **Comunicação**: TCP local (sem criptografia inicial)
- **Deployment**: Via ADB (desenvolvimento)
- **Produção**: Possível implementar TLS

### Performance
- **Target latency**: <100ms end-to-end
- **Bitrate**: 1-8 Mbps adaptativo
- **CPU usage**: <30% no dispositivo
- **Memory**: <200MB no dispositivo

## Próximos Passos

1. **Configurar ambiente NDK** para desenvolvimento Android
2. **Implementar proof-of-concept** de captura + encoder
3. **Testar streaming TCP básico** entre Android e desktop
4. **Integrar FFmpeg** no cliente Qt para decodificação
5. **Implementar protocolo** de comunicação completo
6. **Otimizar performance** e latência
7. **Testar multi-device** streaming
8. **Implementar deployment** automático

## Estrutura de Arquivos Atualizada

```
src/
├── infrastructure/
│   ├── streaming/              # Nova camada de streaming
│   │   ├── native_streaming_service.cpp
│   │   ├── h264_decoder.cpp
│   │   ├── protocol_handler.cpp
│   │   ├── tcp_client.cpp
│   │   └── stream_metadata.cpp
│   ├── deployment/             # Sistema de deployment
│   │   ├── server_deployment_service.cpp
│   │   └── device_compatibility.cpp
│   └── adb/                    # ADB mantido para deployment
│       └── ...
├── use_case/
│   ├── native_streaming_use_case.cpp    # Novo caso de uso
│   └── ...
└── presentation/
    ├── view_models/
    │   ├── native_streaming_view_model.cpp
    │   └── ...
    └── qml/
        ├── NativeStreamingView.qml
        └── ...

android_server/                 # Novo servidor Android
├── src/
│   ├── main.cpp
│   ├── screen_capture/
│   ├── video_encoder/
│   ├── network/
│   └── protocol/
├── jni/
└── build/
```

Esta arquitetura fornecerá uma base sólida para streaming de alta performance similar ao scrcpy, mantendo a estrutura SOLID existente e permitindo expansão futura. 